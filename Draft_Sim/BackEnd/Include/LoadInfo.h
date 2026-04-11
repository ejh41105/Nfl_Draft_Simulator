#ifndef LOADINFO_H
#define LOADINFO_H

#include <vector>
#include "Team.h"
#include "Player.h"

extern std::vector<Player> draftPool;
extern std::vector<Team>   teamList;
extern std::vector<Pick>   draftOrder;

void loadPlayers(const std::string& path);
void loadTeams(const std::string& path);
void loadDraftOrder(const std::string& path);

#endif

