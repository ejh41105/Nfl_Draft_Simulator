#pragma once

#include "SimDraft.h"
#include <string>
#include <vector>
#include <map>
#include <optional>

struct DraftConfig
{
    std::vector<std::string> selectedTeams; // use team IDs like "DAL"
    int rounds{7};
};

struct SuggestedPlayer
{
    std::string id;
    std::string name;
    std::string college;
    std::string position;
    int consensusRank{};
};

struct DraftPickResult
{
    int overall{};
    int round{};
    std::string teamId;
    std::string teamName;
    SuggestedPlayer player;
    double score{};
    bool userPick{false};
};

struct DraftState
{
    bool started{false};
    bool complete{false};

    int currentPickNumber{0};   // 1-based display
    int overall{0};
    int round{0};

    std::string onClockTeamId;
    std::string onClockTeamName;
    bool isUserPick{false};

    int availableCount{0};

    bool hasSuggestedPlayer{false};
    SuggestedPlayer suggestedPlayer{};
    std::vector<int> recommendedConsensusRanks{};
};

class DraftSession
{
public:
    bool start(const DraftConfig& config, const std::string& dataRoot);
    void reset();

    DraftState getState() const;

    bool advanceOnePick();              // CPU only
    bool makeUserPick(int consensusRank);

    std::vector<Player> getAvailablePlayers() const;
    std::vector<Pick> getPicks() const;
    std::vector<DraftPickResult> getResults() const;

private:
    std::optional<int> getBestPlayerIndex() const;
    std::vector<int> getTopRecommendedConsensusRanks(std::size_t count) const;
    std::optional<int> findPlayerByConsensusRank(int rank) const;
    Team* getCurrentTeam();
    const Team* getCurrentTeam() const;

private:
    bool started_{false};
    size_t currentPickIndex_{0};

    std::vector<Player> availablePool_;
    std::vector<Team> teams_;
    std::vector<Pick> picks_;
    std::vector<DraftPickResult> results_;

    std::map<std::string, Team*> teamMap_;
};
