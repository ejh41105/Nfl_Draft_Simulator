#ifndef RANDOM_H
#define RANDOM_H

#include <chrono>
#include <random>

namespace Random
{
    // Seeded once at program start using the current time
    inline std::mt19937 generate()
    {
        std::seed_seq ss{ static_cast<std::seed_seq::result_type>(
            std::chrono::steady_clock::now().time_since_epoch().count()) };
        return std::mt19937{ ss };
    }

    inline std::mt19937 mt{ generate() }; // one global engine for the whole program

    // Returns a random int between [min, max] inclusive
    inline int getInt(int min, int max)
    {
        std::uniform_int_distribution dist{ min, max };
        return dist(mt);
    }

    // Returns a random double between [min, max] inclusive
    inline double getDouble(double min, double max)
    {
        std::uniform_real_distribution dist{ min, max };
        return dist(mt);
    }

    // Returns true roughly `percent`% of the time (0-100)
    inline bool getChance(int percent)
    {
        return getInt(1, 100) <= percent;
    }

}

#endif