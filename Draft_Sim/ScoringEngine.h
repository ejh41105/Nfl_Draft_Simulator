#ifndef MOCKDRAFTPROJECT_SCORINGENGINE_H
#define MOCKDRAFTPROJECT_SCORINGENGINE_H

#include "Player.h"
#include "Team.h"
#include <vector>

double scoreConsensus(const Player& player, int totalPlayers);

double scorePositionalValue(const Player& player);

double scoreRAS(const Player& player);

double scoreMiscConcern(const Player& player);

double scoreFloorCeiling(const Player& player);

double scorePositionalNeed(const Player& player, const Team& team);

double scorePositionalScarcity(const Player& player, const Team& team);

double scorePlayer(const Player& player, const Team& team, const std::vector<Player>& pool);

int getBestPlayerIndex(const Team& team, const std::vector<Player>& pool);

int scoreScarcity(std::string_view position, const std::vector<Player>& pool);

#endif