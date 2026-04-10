#include "Include/SimDraft.h"

void RunDraft()
{
    // Load players, teams, and draft order
    loadPlayers("JSONS/Players.json");
    loadTeams("JSONS/TeamConfig.json");
    loadDraftOrder("JSONS/DraftOrder.json");

    std::vector<std::string_view> selectedTeams{};
    selectedTeams.emplace_back("Cowboys");

    for (auto& select : teamList)
    {
        for (auto i : selectedTeams)
        {
            if (i == select.getName())
            {
                select.Team::setSelected(true);
            }
        }
    }

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

        if (!team->getSelected())
        {
            const Player& drafted = availablePool[bestIndex];

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

            team->updateNeedAfterPick(drafted.getPosition(), drafted);

            int needAfter = team->getNeedForPosition(drafted.getPosition());

            // Remove drafted player from the pool
            availablePool.erase(availablePool.begin() + bestIndex);
        }
        else
        {
            Player& drafted = availablePool[bestIndex];
            int rank{};
            bool validPick = false;

            while (!validPick)
            {
                std::cout << team->getName() << " are selecting. Suggested Player is " << drafted.getName() << " Enter consensus rank: ";
                std::cin >> rank;

                auto it = std::find_if(availablePool.begin(), availablePool.end(),
                    [rank](const Player& p) { return p.getConsensusRank() == rank; });

                if (it == availablePool.end())
                {
                    std::cout << "Player not found or already drafted, try again\n";
                    continue;
                }

                const Player& drafted = *it;

                std::cout << "Pick " << pick.overall
                          << " | Round " << pick.round
                          << " | " << team->getName()
                          << " selects " << drafted.getName()
                          << " | " << drafted.getCollege() << " University"
                          << " | " << drafted.getPosition()
                          << " | Consensus Rank: " << drafted.getConsensusRank()
                          << " | Score: " << bestIt->first << "\n";

                int needBefore = team->getNeedForPosition(drafted.getPosition());

                team->updateNeedAfterPick(drafted.getPosition(), drafted);

                int needAfter = team->getNeedForPosition(drafted.getPosition());

                availablePool.erase(it);
                validPick = true;
            }
        }
    }
}