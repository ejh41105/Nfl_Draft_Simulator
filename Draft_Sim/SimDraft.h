#ifndef MOCKDRAFTPROJECT_SIMDRAFT_H
#define MOCKDRAFTPROJECT_SIMDRAFT_H

#include <iostream>
#include <algorithm>
#include "LoadInfo.h"
#include "ScoringEngine.h"


void Draft()
{
    loadPlayers("../Draft_Sim/JSONS/Players.json");
    loadTeams("../Draft_Sim/JSONS/TeamConfig.json");
    loadDraftOrder("../Draft_Sim/JSONS/DraftOrder.json");

    std::map<std::string, Team*> teamGetter;
    for (const auto& t : teamList)
    {

    }
}

#endif //MOCKDRAFTPROJECT_SIMDRAFT_H