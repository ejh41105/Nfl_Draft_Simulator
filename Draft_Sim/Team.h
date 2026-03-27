#ifndef MOCKDRAFTPROJECT_TEAM_H
#define MOCKDRAFTPROJECT_TEAM_H

#include <string>
#include <string_view>
#include <map>
#include <vector>
#include "Player.h"

struct Pick
{
    int overall{0};
    int round{0};
    int selection{0};
    int tradeValue{0};
    int noiseRange{8};
    std::string teamId{"???"};

    Pick(int overall, int round, int selection, int tradeValue, int noiseRange, const std::string& teamID)
        : overall{overall}, round{round}, selection{selection}, tradeValue{tradeValue}, noiseRange{noiseRange}, teamId{teamID}
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

    // Priority order replaces individual weights
    // index 0 = highest priority (30%)
    // index 6 = lowest priority (5%)
    // values are category names:
    // "consensusRank", "positionalNeed", "positionalValue",
    // "RAS", "floorCeiling", "miscConcern", "positionalScarcity"
    std::vector<std::string> priorities_m{};
    bool selected_m{false};

public:
    Team(std::string_view id, std::string_view name, std::string_view city,
         const std::vector<Pick>& picks,
         const std::map<std::string, int>& positionalNeed,
         const std::vector<std::string>& priorities, bool selected)
        : id_m{id}, name_m{name}, city_m{city},
          picks_m{picks}, positionalNeed_m{positionalNeed},
          priorities_m{priorities}, selected_m{selected}
    {}

    // Getters
    std::string_view getId()                              const { return id_m; }
    std::string_view getName()                            const { return name_m; }
    std::string_view getCity()                            const { return city_m; }
    const std::vector<Pick>& getPicks()                   const { return picks_m; }
    const std::map<std::string, int>& getPositionalNeed() const { return positionalNeed_m; }
    const std::vector<std::string>& getPriorities()       const { return priorities_m; }
    bool getSelected()                                    const { return selected_m; }

    int getNeedForPosition(std::string_view position)     const;

    // Declarations for Team.cpp
    void updateNeedAfterPick(std::string_view position, const Player&);
    void addPick(const Pick& pick);
    void removePick(int overallPickNumber);
    Pick getNextPick() const;

    //This is for when user selects a team (changes false to true)//
    void setSelected(bool choice)
    {
        selected_m = choice;
    }
};

#endif //MOCKDRAFTPROJECT_TEAM_H