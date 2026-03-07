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
    int noiseRange_m{8};

    // Priority order replaces individual weights
    // index 0 = highest priority (30%)
    // index 6 = lowest priority (5%)
    // values are category names:
    // "consensusRank", "positionalNeed", "positionalValue",
    // "RAS", "floorCeiling", "miscConcern", "positionalScarcity"
    std::vector<std::string> priorities_m{};

public:
    Team(std::string_view id, std::string_view name, std::string_view city,
         const std::vector<Pick>& picks,
         const std::map<std::string, int>& positionalNeed,
         int noiseRange,
         const std::vector<std::string>& priorities)
        : id_m{id}, name_m{name}, city_m{city},
          picks_m{picks}, positionalNeed_m{positionalNeed},
          noiseRange_m{noiseRange},
          priorities_m{priorities}
    {}

    // Getters
    std::string_view getId()                              const { return id_m; }
    std::string_view getName()                            const { return name_m; }
    std::string_view getCity()                            const { return city_m; }
    const std::vector<Pick>& getPicks()                   const { return picks_m; }
    const std::map<std::string, int>& getPositionalNeed() const { return positionalNeed_m; }
    int getNoiseRange()                                   const { return noiseRange_m; }
    const std::vector<std::string>& getPriorities()       const { return priorities_m; }

    int getNeedForPosition(std::string_view position)     const;

    // Declarations for Team.cpp
    void updateNeedAfterPick(std::string_view position);
    void addPick(const Pick& pick);
    void removePick(int overallPickNumber);
    Pick getNextPick() const;
};

#endif //MOCKDRAFTPROJECT_TEAM_H