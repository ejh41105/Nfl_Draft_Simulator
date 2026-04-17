#include "crow_all.h"
#include "DraftSession.h"
#include "LoadInfo.h"
#include "json.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>
#include <random>
#include <stdexcept>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdlib>

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace
{
    constexpr const char* kDraftSessionHeader = "X-Draft-Session-Id";
    constexpr auto kSessionTtl = std::chrono::hours(12);

    struct SessionEntry
    {
        DraftSession session;
        std::chrono::steady_clock::time_point lastAccess{std::chrono::steady_clock::now()};
    };

    std::unordered_map<std::string, SessionEntry> gSessions;
    std::mutex gSessionMutex;

    fs::path resolvePath(const fs::path& relativePath)
    {
        const std::vector<fs::path> candidates = {
            fs::current_path() / relativePath,
            fs::current_path() / "Draft_Sim" / relativePath,
            fs::current_path() / "Draft_Sim" / "BackEnd" / "Data" / relativePath,
            fs::current_path().parent_path() / relativePath,
            fs::current_path().parent_path() / "Draft_Sim" / relativePath,
            fs::current_path().parent_path() / "Draft_Sim" / "BackEnd" / "Data" / relativePath
        };

        for (const auto& candidate : candidates)
        {
            if (fs::exists(candidate))
            {
                return candidate;
            }
        }

        return candidates.front();
    }

    fs::path resolveDataPath(const fs::path& relativePath)
    {
        const std::vector<fs::path> candidates = {
            fs::current_path() / "Draft_Sim" / "BackEnd" / "Data" / relativePath,
            fs::current_path().parent_path() / "Draft_Sim" / "BackEnd" / "Data" / relativePath,
            fs::current_path() / relativePath,
            fs::current_path() / "Draft_Sim" / relativePath,
            fs::current_path().parent_path() / relativePath,
            fs::current_path().parent_path() / "Draft_Sim" / relativePath
        };

        for (const auto& candidate : candidates)
        {
            if (fs::exists(candidate))
            {
                return candidate;
            }
        }

        return candidates.front();
    }

    std::string readFileOrEmpty(const fs::path& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            return {};
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    crow::response serveFile(const fs::path& path, const std::string& contentType)
    {
        const std::string body = readFileOrEmpty(path);
        if (body.empty() && !fs::exists(path))
        {
            return crow::response(404, "File not found.");
        }

        crow::response res;
        res.code = 200;
        res.set_header("Content-Type", contentType);
        res.body = body;
        return res;
    }

    void addJsonHeaders(crow::response& res)
    {
        res.set_header("Content-Type", "application/json");
        res.set_header("Cache-Control", "no-store");
    }

    crow::response jsonResponse(const json& payload, int code = 200)
    {
        crow::response res(code);
        addJsonHeaders(res);
        res.body = payload.dump();
        return res;
    }

    crow::response errorResponse(int code, const std::string& message)
    {
        return jsonResponse(json{{"ok", false}, {"error", message}}, code);
    }

    json toJson(const Player& player)
    {
        return json{
            {"playerId", player.getplayerId()},
            {"name", std::string(player.getName())},
            {"position", std::string(player.getPosition())},
            {"college", std::string(player.getCollege())},
            {"majorStats", std::string(player.getMajorStats())},
            {"number", player.getNumber()},
            {"age", player.getAge()},
            {"height", player.getHeight()},
            {"weight", player.getWeight()},
            {"consensusRanking", player.getConsensusRank()},
            {"positionalRanking", player.getPositionalRank()},
            {"RAS", player.getRAS()},
            {"miscConcern", player.getMiscConcern()},
            {"floorCeiling", player.getFloorCeiling()}
        };
    }

    json toJson(const Pick& pick)
    {
        return json{
            {"overall", pick.overall},
            {"round", pick.round},
            {"selection", pick.selection},
            {"tradeValue", pick.tradeValue},
            {"noiseRange", pick.noiseRange},
            {"teamId", pick.teamId}
        };
    }

    json toJson(const SuggestedPlayer& player)
    {
        return json{
            {"id", player.id},
            {"name", player.name},
            {"college", player.college},
            {"position", player.position},
            {"consensusRank", player.consensusRank}
        };
    }

    json toJson(const DraftPickResult& result)
    {
        return json{
            {"overall", result.overall},
            {"round", result.round},
            {"teamId", result.teamId},
            {"teamName", result.teamName},
            {"player", {
                {"id", result.player.id},
                {"name", result.player.name},
                {"college", result.player.college},
                {"position", result.player.position},
                {"consensusRank", result.player.consensusRank}
            }},
            {"score", result.score},
            {"userPick", result.userPick}
        };
    }

    json toJson(const DraftState& state, const std::vector<DraftPickResult>& results)
    {
        json payload{
            {"started", state.started},
            {"complete", state.complete},
            {"currentPickNumber", state.currentPickNumber},
            {"overall", state.overall},
            {"round", state.round},
            {"onClockTeamId", state.onClockTeamId},
            {"onClockTeamName", state.onClockTeamName},
            {"isUserPick", state.isUserPick},
            {"availableCount", state.availableCount},
            {"hasSuggestedPlayer", state.hasSuggestedPlayer},
            {"results", json::array()}
        };

        if (state.hasSuggestedPlayer)
        {
            payload["suggestedPlayer"] = toJson(state.suggestedPlayer);
        }

        payload["recommendedConsensusRanks"] = json::array();
        for (const int rank : state.recommendedConsensusRanks)
        {
            payload["recommendedConsensusRanks"].push_back(rank);
        }

        for (const auto& result : results)
        {
            payload["results"].push_back(toJson(result));
        }

        return payload;
    }

    std::optional<json> parseBody(const crow::request& req)
    {
        if (req.body.empty())
        {
            return std::nullopt;
        }

        try
        {
            return json::parse(req.body);
        }
        catch (const json::parse_error&)
        {
            return std::nullopt;
        }
    }

    std::string trim(std::string value)
    {
        const auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
        value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
        value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
        return value;
    }

    std::optional<std::string> getSessionKey(const crow::request& req)
    {
        const std::string sessionId = trim(req.get_header_value(kDraftSessionHeader));
        if (sessionId.empty())
        {
            return std::nullopt;
        }

        return sessionId;
    }

    std::string generateSessionId()
    {
        static constexpr char alphabet[] = "0123456789abcdef";
        std::random_device rng;

        std::string id;
        id.reserve(32);

        for (int i = 0; i < 32; ++i)
        {
            id.push_back(alphabet[rng() & 0x0F]);
        }

        return id;
    }

    void pruneExpiredSessions()
    {
        const auto now = std::chrono::steady_clock::now();

        for (auto it = gSessions.begin(); it != gSessions.end();)
        {
            if (now - it->second.lastAccess > kSessionTtl)
            {
                it = gSessions.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    SessionEntry* findSession(const crow::request& req)
    {
        const auto sessionId = getSessionKey(req);
        if (!sessionId.has_value())
        {
            return nullptr;
        }

        auto it = gSessions.find(*sessionId);
        if (it == gSessions.end())
        {
            return nullptr;
        }

        it->second.lastAccess = std::chrono::steady_clock::now();
        return &it->second;
    }

    SessionEntry& createSession()
    {
        pruneExpiredSessions();

        std::string sessionId;
        do
        {
            sessionId = generateSessionId();
        } while (gSessions.contains(sessionId));

        auto [it, inserted] = gSessions.emplace(sessionId, SessionEntry{});
        it->second.lastAccess = std::chrono::steady_clock::now();
        return it->second;
    }
}

int main()
{
    crow::SimpleApp app;

    const fs::path frontEndRoot = resolvePath(fs::path("Front End"));
    const fs::path dataRoot = resolveDataPath(fs::path("JSONS"));

    CROW_ROUTE(app, "/")([frontEndRoot]() {
        return serveFile(frontEndRoot / "HTML" / "HomePage.html", "text/html; charset=utf-8");
    });

    CROW_ROUTE(app, "/draft")([frontEndRoot]() {
        return serveFile(frontEndRoot / "HTML" / "DraftPage.html", "text/html; charset=utf-8");
    });

    CROW_ROUTE(app, "/HomePage.html")([frontEndRoot]() {
        return serveFile(frontEndRoot / "HTML" / "HomePage.html", "text/html; charset=utf-8");
    });

    CROW_ROUTE(app, "/DraftPage.html")([frontEndRoot]() {
        return serveFile(frontEndRoot / "HTML" / "DraftPage.html", "text/html; charset=utf-8");
    });

    CROW_ROUTE(app, "/CSS/<string>")([frontEndRoot](const std::string& filename) {
        return serveFile(frontEndRoot / "CSS" / filename, "text/css; charset=utf-8");
    });

    CROW_ROUTE(app, "/JavaScript/<string>")([frontEndRoot](const std::string& filename) {
        return serveFile(frontEndRoot / "JavaScript" / filename, "application/javascript; charset=utf-8");
    });

    CROW_ROUTE(app, "/api/players")([dataRoot](const crow::request& req) {
        const std::string position = req.url_params.get("position") ? req.url_params.get("position") : "";
        std::string search = req.url_params.get("search") ? req.url_params.get("search") : "";
        std::transform(search.begin(), search.end(), search.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });

        std::lock_guard<std::mutex> lock(gSessionMutex);

        std::vector<Player> players;
        SessionEntry* sessionEntry = findSession(req);
        DraftState state = sessionEntry ? sessionEntry->session.getState() : DraftState{};
        if (state.started && sessionEntry)
        {
            players = sessionEntry->session.getAvailablePlayers();
        }
        else
        {
            loadPlayers(dataRoot.string() + "/Players.json");
            players = draftPool;
        }

        json payload;
        payload["players"] = json::array();

        for (const auto& player : players)
        {
            const std::string playerPosition = std::string(player.getPosition());
            const std::string playerName = std::string(player.getName());
            const std::string playerCollege = std::string(player.getCollege());

            if (!position.empty() && position != playerPosition)
            {
                continue;
            }

            if (!search.empty())
            {
                std::string searchableName = playerName;
                std::string searchableCollege = playerCollege;
                std::transform(searchableName.begin(), searchableName.end(), searchableName.begin(), [](unsigned char ch) {
                    return static_cast<char>(std::tolower(ch));
                });
                std::transform(searchableCollege.begin(), searchableCollege.end(), searchableCollege.begin(), [](unsigned char ch) {
                    return static_cast<char>(std::tolower(ch));
                });

                if (searchableName.find(search) == std::string::npos &&
                    searchableCollege.find(search) == std::string::npos)
                {
                    continue;
                }
            }

            payload["players"].push_back(toJson(player));
        }

        payload["count"] = payload["players"].size();
        return jsonResponse(payload);
    });

    CROW_ROUTE(app, "/api/picks")([dataRoot](const crow::request& req) {
        json payload = json::array();
        std::lock_guard<std::mutex> lock(gSessionMutex);
        SessionEntry* sessionEntry = findSession(req);
        DraftState state = sessionEntry ? sessionEntry->session.getState() : DraftState{};

        const std::vector<Pick> picks = state.started && sessionEntry ? sessionEntry->session.getPicks() : [&dataRoot]() {
            loadDraftOrder(dataRoot.string() + "/DraftOrder.json");
            return draftOrder;
        }();

        for (const auto& pick : picks)
        {
            payload.push_back(toJson(pick));
        }

        return jsonResponse(payload);
    });

    CROW_ROUTE(app, "/api/draft/start").methods(crow::HTTPMethod::Post)([dataRoot](const crow::request& req) {
        const auto body = parseBody(req);
        if (!body.has_value())
        {
            return errorResponse(400, "Invalid JSON body.");
        }

        DraftConfig config;
        config.rounds = std::max(1, std::min(7, body->value("rounds", 1)));

        if (body->contains("teams") && (*body)["teams"].is_array())
        {
            config.selectedTeams = (*body)["teams"].get<std::vector<std::string>>();
        }

        crow::response res;
        addJsonHeaders(res);

        std::lock_guard<std::mutex> lock(gSessionMutex);
        SessionEntry& sessionEntry = createSession();
        const bool started = sessionEntry.session.start(config, dataRoot.string());
        if (!started)
        {
            return errorResponse(500, "Could not load draft data.");
        }

        DraftState state = sessionEntry.session.getState();
        res.code = 200;
        res.body = json{
            {"ok", true},
            {"state", toJson(state, sessionEntry.session.getResults())}
        }.dump();
        return res;
    });

    CROW_ROUTE(app, "/api/draft/state")([](const crow::request& req) {
        std::lock_guard<std::mutex> lock(gSessionMutex);
        SessionEntry* sessionEntry = findSession(req);
        if (!sessionEntry)
        {
            return jsonResponse(toJson(DraftState{}, {}));
        }

        DraftState state = sessionEntry->session.getState();
        return jsonResponse(toJson(state, sessionEntry->session.getResults()));
    });

    CROW_ROUTE(app, "/api/draft/advance").methods(crow::HTTPMethod::Post)([](const crow::request& req) {
        std::lock_guard<std::mutex> lock(gSessionMutex);
        SessionEntry* sessionEntry = findSession(req);
        if (!sessionEntry)
        {
            return errorResponse(400, "Draft session has not started.");
        }

        DraftState before = sessionEntry->session.getState();

        if (!before.started)
        {
            return errorResponse(400, "Draft session has not started.");
        }

        if (before.complete)
        {
            return jsonResponse(json{
                {"ok", true},
                {"advanced", false},
                {"state", toJson(before, sessionEntry->session.getResults())}
            });
        }

        if (before.isUserPick)
        {
            return errorResponse(409, "Current pick belongs to a user-controlled team.");
        }

        const bool advanced = sessionEntry->session.advanceOnePick();
        DraftState after = sessionEntry->session.getState();

        return jsonResponse(json{
            {"ok", advanced},
            {"advanced", advanced},
            {"state", toJson(after, sessionEntry->session.getResults())}
        });
    });

    CROW_ROUTE(app, "/api/draft/pick").methods(crow::HTTPMethod::Post)([](const crow::request& req) {
        const auto body = parseBody(req);
        if (!body.has_value())
        {
            return errorResponse(400, "Invalid JSON body.");
        }

        const int consensusRank = body->value("consensusRank", -1);
        if (consensusRank < 0)
        {
            return errorResponse(400, "Missing consensusRank.");
        }

        std::lock_guard<std::mutex> lock(gSessionMutex);
        SessionEntry* sessionEntry = findSession(req);
        if (!sessionEntry)
        {
            return errorResponse(400, "Draft session has not started.");
        }

        DraftState before = sessionEntry->session.getState();

        if (!before.started)
        {
            return errorResponse(400, "Draft session has not started.");
        }

        if (!before.isUserPick)
        {
            return errorResponse(409, "It is not currently a user pick.");
        }

        const bool ok = sessionEntry->session.makeUserPick(consensusRank);
        if (!ok)
        {
            return errorResponse(400, "Could not draft that player.");
        }

        DraftState after = sessionEntry->session.getState();
        return jsonResponse(json{
            {"ok", true},
            {"state", toJson(after, sessionEntry->session.getResults())}
        });
    });

    const char* port_env = std::getenv("PORT");
    int port = port_env ? std::atoi(port_env) : 8080;
    app.bindaddr("127.0.0.1").port(port).multithreaded().run();
    return 0;
}
