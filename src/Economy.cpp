#include "Economy.hpp"
#include <cstdio>
#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace gcu {
    std::string Price::str() const {
        std::string s;
        auto add = [&](std::string const& x) { if (!s.empty()) s += " + "; s += x; };
        if (cents) add(Eco::money(cents));
        if (gems) add(std::to_string(gems) + " gems");
        if (orbs) add(std::to_string(orbs) + " orbs");
        if (diamonds) add(std::to_string(diamonds) + " diamonds");
        if (keys) add(std::to_string(keys) + " keys");
        if (shards) add(std::to_string(shards) + " shards");
        return s.empty() ? "Free" : s;
    }

    int64_t Eco::cents() { return std::clamp<int64_t>(Store::get("cents", 500), 0, MAX_CENTS); }  // $5.00 starter wallet
    std::string Eco::money(int64_t c) {
        bool neg = c < 0;
        uint64_t u = neg ? 0 - static_cast<uint64_t>(c) : static_cast<uint64_t>(c);  // safe for INT64_MIN
        std::string whole = std::to_string(u / 100);
        for (int i = (int)whole.size() - 3; i > 0; i -= 3) whole.insert(i, ",");
        char frac[8]; std::snprintf(frac, sizeof frac, ".%02u", (unsigned)(u % 100));
        return (neg ? "-$" : "$") + whole + frac;
    }
    void Eco::addCents(int64_t c) {
        int64_t cur = cents(), out;
        if (c >= 0) out = (c > MAX_CENTS - cur) ? MAX_CENTS : cur + c;   // saturate instead of overflowing
        else out = (c < -cur) ? 0 : cur + c;
        Store::set("cents", out);
    }
    void Eco::giveAllMoney() { Store::set("cents", MAX_CENTS); }
    void Eco::resetModData() { Store::reset(); }
    int64_t Eco::cur(char const* n) { return Store::get(std::string("cur.") + n, 0); }
    void Eco::addCur(char const* n, int64_t d) { Store::set(std::string("cur.") + n, std::max<int64_t>(0, cur(n) + d)); }

    EnergyCfg Eco::energyCfg() {
        EnergyCfg c; auto m = Mod::get();
        c.enabled = m->getSettingValue<bool>("enable-energy");
        c.perLevel = std::clamp<int>((int)m->getSettingValue<int64_t>("energy-per-level"), 0, 10);
        c.perAttempt = std::clamp<int>((int)m->getSettingValue<int64_t>("energy-per-attempt"), 0, 5);
        c.regenSec = std::clamp<int>((int)m->getSettingValue<int64_t>("energy-regen-seconds"), 10, 3600);
        c.maxEnergy = std::clamp<int>((int)m->getSettingValue<int64_t>("energy-max"), 5, 100);
        double mu = m->getSettingValue<double>("energy-reward-multiplier");
        c.mult = (mu == mu) ? std::clamp(mu, 0.25, 3.0) : 1.0;       // NaN-safe
        c.disabledPct = std::clamp<int>((int)m->getSettingValue<int64_t>("energy-disabled-percent"), 1, 50);
        return c;
    }
    int Eco::energyMax() { return energyCfg().maxEnergy; }
    int Eco::energyRegenSec() { return energyCfg().regenSec; }

    int64_t Eco::payoutCents(Tier t, bool first, EnergyCfg const& c, int burned) {
        int64_t base = rewardCents(t);
        if (!first) base = std::max<int64_t>(1, base / 10);          // replays pay 10%: restarting is not a money printer
        double f = c.enabled ? std::clamp(1.0 + (std::clamp(burned, 0, 1000) - 2) * 0.25 * c.mult, 0.10, 3.0)
                             : c.disabledPct / 100.0;
        return std::max<int64_t>(1, (int64_t)std::llround((double)base * f));
    }
    std::string Eco::energyBlurb() {
        auto c = energyCfg();
        if (!c.enabled) return fmt::format("Energy is OFF: no limits, but payouts are only {}%.", c.disabledPct);
        auto f = [&](int b) { return (int)std::llround(std::clamp(1.0 + (b - 2) * 0.25 * c.mult, 0.10, 3.0) * 100); };
        return fmt::format("{} energy per level pays {}%{}. More energy burned = more money.", c.perLevel, f(c.perLevel),
                           c.perAttempt ? fmt::format(" (+{} per attempt)", c.perAttempt) : "");
    }

    int Eco::energy() {
        int mx = energyMax(), rg = energyRegenSec();
        int e = std::clamp<int>((int)Store::get("energy", mx), 0, mx);   // capacity may have been lowered in settings
        auto last = Store::get("energy.t", Store::now());
        auto now = Store::now();
        if (e < mx) {
            auto gain = (now - last) / rg;
            if (gain > 0) {
                e = (int)std::min<int64_t>(mx, e + gain);
                Store::set("energy", e);
                Store::set("energy.t", e >= mx ? now : last + gain * rg);
            }
        } else if (last != now) Store::set("energy.t", now);
        return e;
    }
    void Eco::addEnergy(int d) {
        int mx = energyMax(), e = energy();
        if (e >= mx && d < 0) Store::set("energy.t", Store::now());   // regen clock starts when we drop below max
        Store::set("energy", std::clamp<int64_t>((int64_t)e + d, 0, mx));
    }
    int Eco::secondsToNextEnergy() {
        if (energy() >= energyMax()) return 0;
        return (int)std::max<int64_t>(0, energyRegenSec() - (Store::now() - Store::get("energy.t", Store::now())));
    }

    bool Eco::canAfford(Price const& p) {
        return cents() >= p.cents && cur("gems") >= p.gems && cur("orbs") >= p.orbs && cur("diamonds") >= p.diamonds
            && cur("keys") >= p.keys && cur("shards") >= p.shards;
    }
    std::string Eco::missing(Price const& p) {
        std::string s;
        auto chk = [&](char const* label, int64_t have, int64_t need, bool money) {
            if (have >= need) return;
            if (!s.empty()) s += ", ";
            s += money ? Eco::money(need - have) : std::to_string(need - have) + " " + label;
        };
        chk("", cents(), p.cents, true);
        chk("gems", cur("gems"), p.gems, false); chk("orbs", cur("orbs"), p.orbs, false);
        chk("diamonds", cur("diamonds"), p.diamonds, false); chk("keys", cur("keys"), p.keys, false);
        chk("shards", cur("shards"), p.shards, false);
        return s;
    }
    void Eco::charge(Price const& p) {
        addCents(-p.cents); addCur("gems", -p.gems); addCur("orbs", -p.orbs);
        addCur("diamonds", -p.diamonds); addCur("keys", -p.keys); addCur("shards", -p.shards);
        trackSpend(p.cents);
    }
    bool Eco::bought(std::string const& id) { return Store::get("own." + id, 0) != 0; }
    void Eco::markBought(std::string const& id) { Store::set("own." + id, 1); }
    void Eco::trackSpend(int64_t c) { if (c > 0) Store::add("stat.spent", c); }

    static int64_t periodSec() { return (int64_t)Mod::get()->getSettingValue<int64_t>("sub-period-minutes") * 60; }
    bool Eco::subActive(std::string const& id) { return Store::get("sub." + id, 0) > Store::now(); }
    int64_t Eco::subSecondsLeft(std::string const& id) { return std::max<int64_t>(0, Store::get("sub." + id, 0) - Store::now()); }
    void Eco::subStart(std::string const& id, int64_t price) {
        Store::set("sub." + id, Store::now() + periodSec());
        Store::set("subprice." + id, price);
        Store::set("subcancel." + id, 0);
    }
    void Eco::subCancel(std::string const& id) { Store::set("subcancel." + id, 1); }
    std::vector<std::string> Eco::tickSubs() {
        std::vector<std::string> out;
        for (auto id : {"mod", "editor", "pass"}) {
            std::string sid = id;
            auto end = Store::get("sub." + sid, 0);
            if (end == 0 || end > Store::now()) continue;
            auto price = Store::get("subprice." + sid, 0);
            if (Store::get("subcancel." + sid, 0)) { Store::set("sub." + sid, 0); out.push_back(sid + " subscription ended."); continue; }
            Price p; p.cents = price;
            if (canAfford(p)) { charge(p); Store::set("sub." + sid, Store::now() + periodSec()); out.push_back(sid + " renewed for " + money(price) + "."); }
            else { Store::set("sub." + sid, 0); out.push_back(sid + " lapsed: wallet could not cover " + money(price) + "."); }
        }
        return out;
    }

    int64_t rewardCents(Tier t) {
        static const int64_t r[] = {10, 25, 50, 100, 200, 500, 2500, 2500, 2500, 2500, 2500};   // every Demon: exactly $25.00
        return r[(int)t];
    }
    char const* tierName(Tier t) {
        static const char* n[] = {"Auto", "Easy", "Normal", "Hard", "Harder", "Insane", "Easy Demon", "Medium Demon", "Hard Demon", "Insane Demon", "Extreme Demon"};
        return n[(int)t];
    }
    int tierStars(Tier t) { static const int s[] = {1, 2, 3, 4, 6, 8, 10, 10, 10, 10, 10}; return s[(int)t]; }

    std::optional<Tier> tierFromRating(int stars, bool demon, int demonDifficulty, bool autoFlag) {
        if (demon || stars >= 10) {
            switch (demonDifficulty) {                // 3 easy, 4 medium, 0/other hard, 5 insane, 6 extreme; every demon pays $25
                case 3: return Tier::EasyDemon;
                case 4: return Tier::MediumDemon;
                case 5: return Tier::InsaneDemon;
                case 6: return Tier::ExtremeDemon;
                default: return Tier::HardDemon;
            }
        }
        if (stars <= 0) return std::nullopt;          // unrated
        if (autoFlag || stars == 1) return Tier::Auto;
        if (stars == 2) return Tier::Easy;
        if (stars == 3) return Tier::Normal;
        if (stars <= 5) return Tier::Hard;
        if (stars <= 7) return Tier::Harder;
        return Tier::Insane;
    }
}
