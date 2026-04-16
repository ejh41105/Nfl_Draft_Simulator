#include "DraftSession.h"
#include <algorithm>
#include <filesystem>

extern std::vector<Player> draftPool;
extern std::vector<Team> teamList;
extern std::vector<Pick> draftOrder;

bool DraftSession::start(const DraftConfig& config, const std::string& dataRoot)
{
    reset();

    const std::filesystem::path root(dataRoot);

    loadPlayers((root / "Players.json").string());
    loadTeams((root / "TeamConfig.json").string());
    loadDraftOrder((root / "DraftOrder.json").string());

    if (draftPool.empty() || teamList.empty() || draftOrder.empty())
    {
        reset();
        return false;
    }

    availablePool_ = draftPool;
    teams_ = teamList;
    picks_ = draftOrder;

    // trim rounds if needed
    picks_.erase(
        std::remove_if(picks_.begin(), picks_.end(),
            [&](const Pick& p) { return p.round > config.rounds; }),
        picks_.end()
    );

    // mark selected teams by ID, not by full name
    for (auto& t : teams_)
    {
        bool selected = false;
        for (const auto& id : config.selectedTeams)
        {
            if (id == std::string(t.getId()))
            {
                selected = true;
                break;
            }
        }
        t.setSelected(selected);
        teamMap_[std::string(t.getId())] = &t;
    }

    started_ = true;
    currentPickIndex_ = 0;
    return true;
}

void DraftSession::reset()
{
    started_ = false;
    currentPickIndex_ = 0;
    availablePool_.clear();
    teams_.clear();
    picks_.clear();
    results_.clear();
    teamMap_.clear();
}

DraftState DraftSession::getState() const
{
    DraftState state{};
    state.started = started_;

    if (!started_)
    {
        state.complete = true;
        state.availableCount = static_cast<int>(availablePool_.size());
        return state;
    }

    if (currentPickIndex_ >= picks_.size())
    {
        state.complete = true;
        state.currentPickNumber = static_cast<int>(picks_.size());
        state.availableCount = static_cast<int>(availablePool_.size());

        if (!picks_.empty())
        {
            state.overall = picks_.back().overall + 1;
            state.round = picks_.back().round;
        }

        return state;
    }

    const Pick& pick = picks_[currentPickIndex_];
    const Team* team = getCurrentTeam();

    state.complete = false;
    state.currentPickNumber = static_cast<int>(currentPickIndex_) + 1;
    state.overall = pick.overall;
    state.round = pick.round;
    state.availableCount = static_cast<int>(availablePool_.size());

    if (team)
    {
        state.onClockTeamId = pick.teamId;
        state.onClockTeamName = team->getName();
        state.isUserPick = team->getSelected();

        auto bestIndex = getBestPlayerIndex();
        if (bestIndex.has_value())
        {
            const Player& p = availablePool_[*bestIndex];
            state.hasSuggestedPlayer = true;
            state.suggestedPlayer.id = std::to_string(p.getplayerId());
            state.suggestedPlayer.name = p.getName();
            state.suggestedPlayer.college = p.getCollege();
            state.suggestedPlayer.position = p.getPosition();
            state.suggestedPlayer.consensusRank = p.getConsensusRank();
        }

        if (state.isUserPick)
        {
            state.recommendedConsensusRanks = getTopRecommendedConsensusRanks(3);
        }
    }

    return state;
}

bool DraftSession::advanceOnePick()
{
    if (!started_ || currentPickIndex_ >= picks_.size())
        return false;

    Team* team = getCurrentTeam();
    if (!team)
    {
        ++currentPickIndex_;
        return true;
    }

    if (team->getSelected())
        return false;

    auto bestIndex = getBestPlayerIndex();
    if (!bestIndex.has_value())
        return false;

    const Pick& pick = picks_[currentPickIndex_];
    Player drafted = availablePool_[*bestIndex];
    double score = scorePlayer(drafted, *team, availablePool_, pick);

    team->updateNeedAfterPick(drafted.getPosition(), drafted);

    DraftPickResult result{};
    result.overall = pick.overall;
    result.round = pick.round;
    result.teamId = pick.teamId;
    result.teamName = team->getName();
    result.player.id = std::to_string(drafted.getplayerId());
    result.player.name = drafted.getName();
    result.player.college = drafted.getCollege();
    result.player.position = drafted.getPosition();
    result.player.consensusRank = drafted.getConsensusRank();
    result.score = score;
    result.userPick = false;

    results_.push_back(result);
    availablePool_.erase(availablePool_.begin() + *bestIndex);
    ++currentPickIndex_;
    return true;
}

bool DraftSession::makeUserPick(int consensusRank)
{
    if (!started_ || currentPickIndex_ >= picks_.size())
        return false;

    Team* team = getCurrentTeam();
    if (!team || !team->getSelected())
        return false;

    auto chosenIndex = findPlayerByConsensusRank(consensusRank);
    if (!chosenIndex.has_value())
        return false;

    const Pick& pick = picks_[currentPickIndex_];
    Player drafted = availablePool_[*chosenIndex];
    double score = scorePlayer(drafted, *team, availablePool_, pick);

    team->updateNeedAfterPick(drafted.getPosition(), drafted);

    DraftPickResult result{};
    result.overall = pick.overall;
    result.round = pick.round;
    result.teamId = pick.teamId;
    result.teamName = team->getName();
    result.player.id = std::to_string(drafted.getplayerId());
    result.player.name = drafted.getName();
    result.player.college = drafted.getCollege();
    result.player.position = drafted.getPosition();
    result.player.consensusRank = drafted.getConsensusRank();
    result.score = score;
    result.userPick = true;

    results_.push_back(result);
    availablePool_.erase(availablePool_.begin() + *chosenIndex);
    ++currentPickIndex_;
    return true;
}

std::vector<Player> DraftSession::getAvailablePlayers() const
{
    return availablePool_;
}

std::vector<Pick> DraftSession::getPicks() const
{
    return picks_;
}

std::vector<DraftPickResult> DraftSession::getResults() const
{
    return results_;
}

std::optional<int> DraftSession::getBestPlayerIndex() const
{
    if (!started_ || currentPickIndex_ >= picks_.size())
        return std::nullopt;

    const Team* team = getCurrentTeam();
    if (!team)
        return std::nullopt;

    const Pick& pick = picks_[currentPickIndex_];

    std::vector<std::pair<double, int>> scoredPlayers;
    for (int i = 0; i < static_cast<int>(availablePool_.size()); ++i)
    {
        double score = scorePlayer(availablePool_[i], *team, availablePool_, pick);
        scoredPlayers.emplace_back(score, i);
    }

    if (scoredPlayers.empty())
        return std::nullopt;

    auto bestIt = std::max_element(scoredPlayers.begin(), scoredPlayers.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });

    return bestIt->second;
}

std::vector<int> DraftSession::getTopRecommendedConsensusRanks(std::size_t count) const
{
    if (!started_ || currentPickIndex_ >= picks_.size() || count == 0)
        return {};

    const Team* team = getCurrentTeam();
    if (!team)
        return {};

    const Pick& pick = picks_[currentPickIndex_];

    std::vector<std::pair<double, int>> scoredPlayers;
    scoredPlayers.reserve(availablePool_.size());

    for (int i = 0; i < static_cast<int>(availablePool_.size()); ++i)
    {
        double score = scorePlayer(availablePool_[i], *team, availablePool_, pick);
        scoredPlayers.emplace_back(score, i);
    }

    std::sort(scoredPlayers.begin(), scoredPlayers.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });

    std::vector<int> recommendedRanks;
    recommendedRanks.reserve(std::min(count, scoredPlayers.size()));

    for (std::size_t i = 0; i < count && i < scoredPlayers.size(); ++i)
    {
        recommendedRanks.push_back(availablePool_[scoredPlayers[i].second].getConsensusRank());
    }

    return recommendedRanks;
}

std::optional<int> DraftSession::findPlayerByConsensusRank(int rank) const
{
    for (int i = 0; i < static_cast<int>(availablePool_.size()); ++i)
    {
        if (availablePool_[i].getConsensusRank() == rank)
            return i;
    }
    return std::nullopt;
}

Team* DraftSession::getCurrentTeam()
{
    if (!started_ || currentPickIndex_ >= picks_.size())
        return nullptr;

    auto it = teamMap_.find(picks_[currentPickIndex_].teamId);
    if (it == teamMap_.end()) return nullptr;
    return it->second;
}

const Team* DraftSession::getCurrentTeam() const
{
    if (!started_ || currentPickIndex_ >= picks_.size())
        return nullptr;

    auto it = teamMap_.find(picks_[currentPickIndex_].teamId);
    if (it == teamMap_.end()) return nullptr;
    return it->second;
}
