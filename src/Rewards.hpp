#pragma once
#include "UI.hpp"

namespace gcu {
    struct Rewards {
        static void onAdReward();                       // rewarded ad payout: +5 energy, +$0.05
        static void grantPassXP(int xp);
        static int dailyIndex();                        // objective rotation
    };
    // GeoPass helpers shared with Shop
    int passTier();
    int passXP();
    std::string seasonName();
    std::string seasonKey();
}
