#ifndef MOCKDRAFTPROJECT_TEAM_H
#define MOCKDRAFTPROJECT_TEAM_H

#include <string>
#include <string_view>
#include <map>
#include <vector>

struct Pick
{
    int overall{0};
    int round{0};
    int selection{0};

    Pick(int overall, int round, int selection)
        : overall{overall}, round{round}, selection{selection}
    {}
};

class Team
{
private:
    std::string id_m{"???"};
    std::string name_m{"???"};
    std::string city_m{"???"};
    std::vector<Pick> picks_m{};
    std::map<std::string, int> positionalNeed_m{};
    int    noiseRange_m{8};
    double weightConsensus_m{0.30};
    double weightNeed_m{0.25};
    double weightPosValue_m{0.20};
    double weightRAS_m{0.15};
    double weightAge_m{0.05};
    double weightMisc_m{0.05};

public:
    Team(std::string_view id, std::string_view name, std::string_view city,
         const std::vector<Pick>& picks,
         const std::map<std::string, int>& positionalNeed,
         int noiseRange,
         double wConsensus, double wNeed, double wPosValue,
         double wRAS, double wAge, double wMisc)
        : id_m{id}, name_m{name}, city_m{city},
          picks_m{picks}, positionalNeed_m{positionalNeed},
          noiseRange_m{noiseRange},
          weightConsensus_m{wConsensus}, weightNeed_m{wNeed},
          weightPosValue_m{wPosValue}, weightRAS_m{wRAS},
          weightAge_m{wAge}, weightMisc_m{wMisc}
    {}

    // Getters
    std::string_view getId()                            const { return id_m; }
    std::string_view getName()                          const { return name_m; }
    std::string_view getCity()                          const { return city_m; }
    const std::vector<Pick>& getPicks()                 const { return picks_m; }
    const std::map<std::string, int>& getPositionalNeed() const { return positionalNeed_m; }
    int    getNoiseRange()                              const { return noiseRange_m; }
    double getWeightConsensus()                         const { return weightConsensus_m; }
    double getWeightNeed()                              const { return weightNeed_m; }
    double getWeightPosValue()                          const { return weightPosValue_m; }
    double getWeightRAS()                               const { return weightRAS_m; }
    double getWeightAge()                               const { return weightAge_m; }
    double getWeightMisc()                              const { return weightMisc_m; }

    int getNeedForPosition(std::string_view position)   const;

    // Declarations for Team.cpp
    void updateNeedAfterPick(std::string_view position);
    void addPick(const Pick& pick);
    void removePick(int overallPickNumber);
};

#endif //MOCKDRAFTPROJECT_TEAM_H
