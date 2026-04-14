#include "SimDraft.h"

void RunDraft()
{
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

    std::map<std::string, Team*> teamMap;
    for (auto& t : teamList)
        teamMap[std::string(t.getId())] = &t;

    std::vector<Player> availablePool = draftPool;

    for (const auto& pick : draftOrder)
    {
        Team* team = teamMap[pick.teamId];
        if (!team) continue;

        std::vector<std::pair<double, int>> scoredPlayers;
        for (int i = 0; i < availablePool.size(); ++i)
        {
            double score = scorePlayer(availablePool[i], *team, availablePool, pick);
            scoredPlayers.emplace_back(score, i);
        }

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

            team->updateNeedAfterPick(drafted.getPosition(), drafted);
            availablePool.erase(availablePool.begin() + bestIndex);
        }
        else
        {
            // Rename outer variable to avoid shadowing
            Player& suggestedPlayer = availablePool[bestIndex];
            int rank{};
            bool validPick = false;

            while (!validPick)
            {
                std::cout << team->getName() << " are on the clock. Suggested: "
                    << suggestedPlayer.getName()
                    << " (Consensus Rank: " << suggestedPlayer.getConsensusRank()
                    << "). Enter consensus rank of player you wish to draft: ";
                std::cin >> rank;

                auto it = std::find_if(availablePool.begin(), availablePool.end(),
                    [rank](const Player& p) { return p.getConsensusRank() == rank; });

                if (it == availablePool.end())
                {
                    std::cout << "Player not found or already drafted, try again.\n";
                    continue;
                }

                const Player& drafted = *it;

                std::cout << "Pick " << pick.overall
                    << " | Round " << pick.round
                    << " | " << team->getName()
                    << " selects " << drafted.getName()
                    << " | " << drafted.getCollege() << " University"
                    << " | " << drafted.getPosition()
                    << " | Consensus Rank: " << drafted.getConsensusRank() << "\n";

                team->updateNeedAfterPick(drafted.getPosition(), drafted);
                availablePool.erase(it);
                validPick = true;
            }
        }
    }
}

void getBigBoard()
{
    loadPlayers("JSONS/Players.json");
    loadTeams("JSONS/TeamConfig.json");

    std::cout << "Please select a team to get their big board: \n";
    std::string teampick{};
    std::cin >> teampick;

    Pick pick1{ 1, 1, 1, 1, 1, "Dal" };

    auto it = std::find_if(teamList.begin(), teamList.end(),
        [&teampick](const Team& t) { return t.getName() == teampick; });

    if (it == teamList.end())
    {
        std::cout << "Team not found: " << teampick << "\n";
        return;
    }
    Team selectedTeam = *it;

    std::vector<std::pair<double, int>> scoredPlayers;
    for (int i = 0; i < draftPool.size(); ++i)
    {
        double score = scorePlayer(draftPool[i], selectedTeam, draftPool, pick1);
        scoredPlayers.emplace_back(score, i);
    }

    std::sort(scoredPlayers.begin(), scoredPlayers.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });

    for (int i = 0; i < scoredPlayers.size(); i++)
    {
        const Player& p = draftPool[scoredPlayers[i].second];
        std::cout << "Player #" << i + 1
            << " | " << p.getName()
            << " | " << p.getPosition()
            << " | Consensus Rank: " << p.getConsensusRank() << "\n";
    }
}