#ifndef MOCKDRAFTPROJECT_SCORINGENGINE_H
#define MOCKDRAFTPROJECT_SCORINGENGINE_H

#include "Player.h"
#include "Team.h"
#include <vector>

double scorePlayer(const Player& player, const Team& team, const std::vector<Player>& pool);

int getBestPlayerIndex(const Team& team, const std::vector<Player>& pool);

double calculateScarcity(std::string_view position, const std::vector<Player>& pool);

#endif