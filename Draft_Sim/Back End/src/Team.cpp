#include "Include/Team.h"
#include <algorithm>
#include <stdexcept>


int Team::getNeedForPosition(std::string_view position) const
{
    auto I{positionalNeed_m.find(std::string(position))};
    if (I != positionalNeed_m.end())
        return I->second;
    return 3;
}

void Team::updateNeedAfterPick(std::string_view position, const Player& player)
{
    auto it = positionalNeed_m.find(std::string(position));
    if (it == positionalNeed_m.end()) return; // guard

    int rank = player.getConsensusRank();
    int increment = 0;

    if (rank <= 24)            // top 24 players
        increment = 3;
    else if (rank <= 59)       // next 35 players
        increment = 2;
    else if (rank <= 149)      // next 90 players
        increment = 1;
    else
        increment = 0;         // very low-ranked, minimal effect

    it->second = std::min(it->second + increment, 5);
}

void Team::addPick(const Pick& pick)
{
    picks_m.push_back(pick);

    // keep picks sorted by overall pick number
    // so getNextPick() always returns the earliest pick
    std::sort(picks_m.begin(), picks_m.end(),
        [](const Pick& a, const Pick& b)
        {
            return a.overall < b.overall;
        });
}

void Team::removePick(int overallPickNumber)
{
    auto it = std::find_if(picks_m.begin(), picks_m.end(),
        [overallPickNumber](const Pick& p)
        {
            return p.overall == overallPickNumber;
        });

    if (it != picks_m.end())
        picks_m.erase(it);
    else
        throw std::runtime_error("Pick not found: " + std::to_string(overallPickNumber));
}

Pick Team::getNextPick() const
{
    if (picks_m.empty())
        throw std::runtime_error(std::string(name_m) + " has no picks remaining");
    return picks_m.front(); // already sorted in addPick
}