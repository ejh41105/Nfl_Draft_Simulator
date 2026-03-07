#include "Team.h"
#include <algorithm>
#include <stdexcept>

int Team::getNeedForPosition(std::string_view position) const
{
    auto I{positionalNeed_m.find(std::string(position))};
    if (I != positionalNeed_m.end())
        return I->second;
    return 3;
}

    void Team::updateNeedAfterPick(std::string_view position)
    {
        auto it = positionalNeed_m.find(std::string(position));
        if (it != positionalNeed_m.end())
        {
            // reduce need by 1, clamp at minimum of 1
            it->second = std::max(1, it->second - 1);
        }
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