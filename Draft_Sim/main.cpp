#include <iostream>
#include "ScoringEngine.h"
#include "LoadInfo.h"

int main()
{
    loadPlayers("../Draft_Sim/JSONS/Players.json");
    loadTeams("../Draft_Sim/JSONS/TeamConfig.json");

    // find the Raiders in teamList
    Team* cowboys = nullptr;
    for (auto& t : teamList)
    {
        if (t.getId() == "DAL")
        {
            cowboys = &t;
            break;
        }
    }

    if (cowboys == nullptr)
    {
        std::cerr << "cowboys not found!\n";
        return 1;
    }

    for (const auto& p : draftPool)
    {
        std::cout << "Scoring: " << p.getName() << "\n";
        std::cout << p.getName() << " gets a positional score for Cowboys of "
                  << scorePositionalNeed(p, *cowboys) << '\n';
    }

    return 0;
}