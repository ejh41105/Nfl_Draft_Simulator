#include <Player.h>
#include <LoadInfo.h>
#include <iostream>

int main()
{
    loadTeams("../Draft_Sim/JSONS/TeamConfig.json");

    for (const auto& t : teamList) {
        std::cout << "The first pick that " << t.getName() << " has is "
           << t.getPicks()[0].overall
           << " and their need for QB is a "
           << t.getPositionalNeed().at("QB") << "\n";
    }
}