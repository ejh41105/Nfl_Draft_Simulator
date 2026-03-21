#include <iostream>
#include <algorithm>
#include <map>
#include <vector>
#include "LoadInfo.h"
#include "ScoringEngine.h"
#include "Team.h"


int main()
{
    // Load players, teams, and draft order
    loadPlayers("JSONS/Players.json");
    loadTeams("JSONS/TeamConfig.json");
    loadDraftOrder("JSONS/DraftOrder.json");

    // Build a map for quick team lookup by ID
    std::map<std::string, Team*> teamMap;
    for (auto& t : teamList)
        teamMap[std::string(t.getId())] = &t;

    // Make a copy of draft pool
    std::vector<Player> availablePool = draftPool;

    // Loop through each pick in the draft
    for (const auto& pick : draftOrder)
    {
        Team* team = teamMap[pick.teamId];
        if (!team) continue; // Skip if team not found

        // Score all available players for this team
        std::vector<std::pair<double, int>> scoredPlayers;
        for (int i = 0; i < availablePool.size(); ++i)
        {
            double score = scorePlayer(availablePool[i], *team, availablePool, pick);
            scoredPlayers.emplace_back(score, i);
        }

        // Find the player with the highest score
        auto bestIt = std::max_element(scoredPlayers.begin(), scoredPlayers.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });
        int bestIndex = bestIt->second;
        const Player& drafted = availablePool[bestIndex];

        // Show draft pick information
        std::cout << "Pick " << pick.overall
                  << " | Round " << pick.round
                  << " | " << team->getName()
                  << " selects " << drafted.getName()
                  << " | " << drafted.getCollege() << " University"
                  << " | " << drafted.getPosition()
                  << " | Consensus Rank: " << drafted.getConsensusRank()
                  << " | Score: " << bestIt->first << "\n";

        // Debug: show positional need before updating
        int needBefore = team->getNeedForPosition(drafted.getPosition());

        // Update team's positional need based on the drafted player
        team->updateNeedAfterPick(drafted.getPosition(), drafted);

        // Debug: show positional need after updating
        int needAfter = team->getNeedForPosition(drafted.getPosition());

        // Remove drafted player from the pool
        availablePool.erase(availablePool.begin() + bestIndex);
    }

    return 0;
}
