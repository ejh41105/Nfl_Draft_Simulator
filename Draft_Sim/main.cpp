#include <LoadInfo.h>
#include <iostream>


int main()
{
    loadDraftOrder("../Draft_Sim/JSONS/DraftOrder.json");

    for (const auto& t : draftOrder)
    {
        std::cout << "Pick " << t.overall << " belongs to " << t.teamId << ".\n";
    }

}

//test