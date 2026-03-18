#include <iostream>
#include <algorithm>
#include "LoadInfo.h"
#include "ScoringEngine.h"

int main()
{
    loadPlayers("../Draft_Sim/JSONS/Players.json");
    loadTeams("../Draft_Sim/JSONS/TeamConfig.json");
    loadDraftOrder("../Draft_Sim/JSONS/DraftOrder.json");

    // build a team lookup map
    std::map<std::string, Team*> teamMap;
    for (auto& t : teamList)
        teamMap[std::string(t.getId())] = &t;

    // available players — we'll remove from this as picks are made
    std::vector<Player> availablePool = draftPool;

    // run through every pick in order
    for (const auto& pick : draftOrder)
    {
        Team* team = teamMap[pick.teamId];
        if (team == nullptr) continue;

        // score every available player for this team
        std::vector<std::pair<double, int>> scored; // score, index
        for (int i = 0; i < availablePool.size(); i++)
            scored.push_back({scorePlayer(availablePool[i], *team, availablePool), i});

        // find the highest scored player
        auto best = std::max_element(scored.begin(), scored.end());
        const Player& drafted = availablePool[best->second];

        std::cout << "Pick " << pick.overall
          << " | Round " << pick.round
          << " | " << team->getName()
          << " select " << drafted.getName()
          << " | " << drafted.getPosition()
          << " | Consensus Rank: " << drafted.getConsensusRank()
          << " | Score: " << best->first << "\n";

        // remove drafted player from pool
        availablePool.erase(availablePool.begin() + best->second);
    }

    return 0;
}