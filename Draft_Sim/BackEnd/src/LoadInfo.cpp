#include "LoadInfo.h"
#include "json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

std::vector<Player> draftPool;
std::vector<Team>   teamList;
std::vector<Pick>   draftOrder;

void loadPlayers(const std::string& path)
{
    draftPool.clear();
    std::ifstream file(path);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open " << path << "\n";
        return;
    }

    json data = json::parse(file);

    for (const auto& p : data) {
        draftPool.emplace_back(
            p["name"].get<std::string>(),
            p["playerId"].get<int>(),
            p["position"].get<std::string>(),
            p["college"].get<std::string>(),
            p["majorStats"].is_null() ? "No stats available" : p["majorStats"].get<std::string>(),
            p["number"].get<int>(),
            p["age"].get<int>(),
            p["height"].get<int>(),
            p["weight"].get<int>(),
            p["consensusRanking"].get<int>(),
            p["positionalRanking"].get<int>(),
            p["RAS"].get<double>(),
            p["miscConcern"].get<int>(),
            p["floorCeiling"].get<int>()
        );
    }
}

void loadTeams(const std::string& path)
{
    teamList.clear();
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open " << path << "\n";
        return;
    }

    json data = json::parse(file);

    for (const auto& t : data)
    {
        std::vector<Pick> picks;
        for (const auto& pk : t["picks"])
        {
            picks.push_back({
                pk["overall"].get<int>(),
                pk["round"].get<int>(),
                pk["selection"].get<int>(),
                pk["tradeValue"].get<int>(),
                pk["noiseRange"].get<int>(),
                t["id"].get<std::string>()
            });
        }
        std::vector<std::string> priorities = t["priorities"].get<std::vector<std::string>>();

        teamList.emplace_back
        (
            t["id"].get<std::string>(),
            t["name"].get<std::string>(),
            t["city"].get<std::string>(),
            picks,
            t["positionalNeed"].get<std::map<std::string, int>>(),
            priorities,
            t["selected"].get<bool>()
        );
    }
}

void loadDraftOrder(const std::string& path)
{
    draftOrder.clear();
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open " << path << "\n";
        return;
    }

    json data = json::parse(file);

    for (const auto& d : data)
    {
        draftOrder.emplace_back
        (
            d["overall"].get<int>(),
            d["round"].get<int>(),
            d["selection"].get<int>(),
            d["tradeValue"].get<int>(),
            d["noiseRange"].get<int>(),
            d["teamId"].get<std::string>()
        );
    }
}


