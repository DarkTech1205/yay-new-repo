#pragma once
#include "Store.hpp"
#include <string>
#include <vector>
#include <optional>

namespace gcu {
    // Fictional wallet is stored in cents. No real money is ever involved.
    struct Price {
        int64_t cents = 0, gems = 0, orbs = 0, diamonds = 0, keys = 0, shards = 0;
        std::string str() const;
        bool free() const { return !(cents || gems || orbs || diamonds || keys || shards); }
    };

    // Fixed reward table (Auto $0.10 ... every Demon exactly $25.00). GJDifficulty tiers map onto these exactly.
    enum class Tier { Auto, Easy, Normal, Hard, Harder, Insane, EasyDemon, MediumDemon, HardDemon, InsaneDemon, ExtremeDemon };
    int64_t rewardCents(Tier t);
    char const* tierName(Tier t);
    int tierStars(Tier t);
    // Pure rating -> tier mapping (unit-tested). nullopt = unrated, which never pays.
    std::optional<Tier> tierFromRating(int stars, bool demonFlag, int demonDifficulty, bool autoFlag);

    // Energy settings, always clamped to safe ranges (see mod.json). Read fresh on every call.
    struct EnergyCfg {
        bool enabled = true;
        int perLevel = 2;        // energy burned when a level starts       (0..10)
        int perAttempt = 0;      // energy burned on every attempt          (0..5)
        int regenSec = 120;      // seconds per regenerated point           (10..3600)
        int maxEnergy = 25;      // capacity                                (5..100)
        double mult = 1.0;       // slope of the energy bonus               (0.25..3.0)
        int disabledPct = 25;    // payout percent when energy is off       (1..50)
    };

    struct Eco {
        static EnergyCfg energyCfg();
        static int energyMax();
        static int energyRegenSec();
        // The ONE reward formula. payout = table * (first ? 1 : 10%) * energy factor.
        //   energy off : factor = disabledPct%
        //   energy on  : factor = 1 + (burned - 2) * 0.25 * mult, clamped to [0.10, 3.00]   (burned 2 == exactly the table value)
        static int64_t payoutCents(Tier t, bool firstCompletion, EnergyCfg const& cfg, int burnedEnergy);
        static std::string energyBlurb();              // one-line explanation shown in the UI

        // "GIVE ME ALL THE MONEY": exactly 2147483647 dollars (INT32_MAX), kept in cents. This is also the wallet ceiling,
        // so nothing can overflow: additions saturate here.
        static constexpr int64_t MAX_DOLLARS = 2147483647LL;
        static constexpr int64_t MAX_CENTS = MAX_DOLLARS * 100;

        static int64_t cents();                        // always within [0, MAX_CENTS]
        static std::string money(int64_t cents);       // "$1,234.50" with thousands separators
        static void addCents(int64_t c);               // saturating at 0 and MAX_CENTS
        static void giveAllMoney();                    // wallet := MAX_CENTS, saved through the normal Store path
        static void resetModData();                    // wipes ONLY this mod's saved state; never touches GD data

        static int64_t cur(char const* name);         // "gems","orbs","diamonds","keys","shards","stars","moons","coins"
        static void addCur(char const* name, int64_t d);

        static int energy();                           // applies regeneration first
        static void addEnergy(int d);
        static int secondsToNextEnergy();

        static bool canAfford(Price const& p);
        static std::string missing(Price const& p);
        static void charge(Price const& p);            // caller must check canAfford

        static bool bought(std::string const& id);
        static void markBought(std::string const& id);

        // Subscriptions: "mod" $100, "editor" $5, "pass" $14.99. A month is a configurable number of real minutes.
        static bool subActive(std::string const& id);
        static int64_t subSecondsLeft(std::string const& id);
        static void subStart(std::string const& id, int64_t priceCents);
        static void subCancel(std::string const& id);
        static std::vector<std::string> tickSubs();    // renews or lapses; returns messages

        static void trackSpend(int64_t cents);         // for the fake monetization dashboard
    };

}
