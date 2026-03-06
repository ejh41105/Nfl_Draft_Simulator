#ifndef MOCKDRAFTPROJECT_PLAYER_H
#define MOCKDRAFTPROJECT_PLAYER_H

#include <string>
#include <string_view>

class Player
{
private:
    std::string name_m{"???"};
    std::string position_m{"???"};
    std::string college_m{"???"};
    std::string majorStats_m{"???"};
    int number_m{0};
    int age_m{0};
    int height_m{0};
    int weight_m{0};
    int consensusRanking_m{999};
    int positionalRanking_m{999};
    double RAS_m{0.0};
    int miscellaneousConcerns_m{0};

public:
    Player(std::string_view name, std::string_view position, std::string_view college, std::string_view majorStats, int number, int age, int height, int weight, int consensusRanking, int positionalRanking, double RAS, int miscellaneousConcerns)
        : name_m{name}, position_m{position}, college_m{college}, majorStats_m{majorStats}, number_m{number}, age_m{age}, height_m{height},
          weight_m{weight}, consensusRanking_m{consensusRanking}, positionalRanking_m{positionalRanking}, RAS_m{RAS}, miscellaneousConcerns_m{miscellaneousConcerns}
    {}

    std::string_view getName()           const { return name_m; }
    std::string_view getPosition()       const { return position_m; }
    std::string_view getCollege()        const { return college_m; }
    std::string_view getMajorStats()     const { return majorStats_m; }
    int              getNumber()         const { return number_m; }
    int              getAge()            const { return age_m; }
    int              getHeight()         const { return height_m; }
    int              getWeight()         const { return weight_m; }
    int              getConsensusRank()  const { return consensusRanking_m; }
    int              getPositionalRank() const { return positionalRanking_m; }
    double           getRAS()            const { return RAS_m; }
    int              getMiscConcern()    const { return miscellaneousConcerns_m; }
};

#endif //MOCKDRAFTPROJECT_PLAYER_H