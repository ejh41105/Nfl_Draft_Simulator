#include "ScoringEngine.h"
#include "loadInfo.h"
#include <random>

std::map<std::string, double> getWeights(const Team& team) {
    const std::array<double, 7> weights = {0.33, 0.19, 0.16, 0.12, 0.09, 0.07, 0.04};
    std::map<std::string, double> weightMap;

    const auto& priorities = team.getPriorities();
    for (int i = 0; i < priorities.size(); i++) {
        weightMap[priorities[i]] = weights[i];
    }

    return weightMap;
}

double scoreConsensus(const Player& player, int totalPlayers)
{
    int rank = 1;
    for (const auto& p : draftPool)
    {
        if (p.getConsensusRank() < player.getConsensusRank())
        {
            rank++;
        }
    }

    return 100.0 * std::exp(-0.025 * (rank - 1));
}

double scorePositionalValue(const Player& player)
{
    static const std::map<std::string, double> positionValue = {
        {"QB",   100.0},
        {"OT",   80.0},
        {"EDGE", 80.0},
        {"CB",   80.0},
        {"WR",   80.0},
        {"DT",   60.0},
        {"OG",   60.0},
        {"RB",   60.0},
        {"ILB",  60.0},
        {"OC",   60.0},
        {"TE",   40.0},
        {"S",    55.0},
        {"K",    20.0},
        {"P",    20.0},
        {"FB",   20.0},
    };

    auto it = positionValue.find(std::string(player.getPosition()));
    if (it != positionValue.end())
        return it->second;
    return 50.0; // default
}

double scoreRAS(const Player& player)
{
    double ras = player.getRAS();

    if      (ras < 5.0)  return 0.0;
    else if (ras < 6.0)  return 20.0;
    else if (ras < 7.0)  return 40.0;
    else if (ras < 8.0)  return 50.0;
    else if (ras < 9.0)  return 70.0;
    else if (ras < 10.0) return 90.0;
    else                 return 100.0;
}

double scoreMiscConcern(const Player& player)
{
    return (player.getMiscConcern() - 1) / 9.0 * 100.0;
}

double scoreFloorCeiling(const Player& player)
{
    return (player.getFloorCeiling() - 1) / 9.0 * 100.0;
}

double scorePositionalNeed(const Player& player, const Team& team)
{
    std::string_view pos = player.getPosition();

    for (const auto& t : team.getPositionalNeed())
    {
        if (pos == t.first) { return (t.second * -20) + 120; }
    }
    return 50.0;
}

int scoreScarcity(std::string_view position, const std::vector<Player>& pool)
{
    size_t limit = std::min(pool.size(), size_t(32));
    int count = 0;
    for (size_t i = 0; i < limit; ++i)
    {
        if (pool[i].getPosition() == position)
                count++;
    }

    switch (count)
    {
        case 1: return 100;
        case 2: return 80;
        case 3: return 60;
        case 4: return 40;
        case 5: return 20;
        default: return 0;
    }
}

double scorePlayer(const Player& player, const Team& team, const std::vector<Player>& pool)
{
    std::map<std::string, double> weights = getWeights(team);

    double rawTotal =
        scoreConsensus(player, pool.size())             * weights["consensusRank"]      +
        scorePositionalNeed(player, team)               * weights["positionalNeed"]     +
        scorePositionalValue(player)                    * weights["positionalValue"]    +
        scoreRAS(player)                                * weights["RAS"]                +
        scoreFloorCeiling(player)                       * weights["floorCeiling"]       +
        scoreMiscConcern(player)                        * weights["miscConcern"]        +
        scoreScarcity(player.getPosition(), pool)       * weights["positionalScarcity"];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> noise(-team.getNoiseRange() / 15.0, team.getNoiseRange() / 10);

    return rawTotal + noise(gen);
}

int getBestPlayerIndex(const Team& team, const std::vector<Player>& pool)
{

}

