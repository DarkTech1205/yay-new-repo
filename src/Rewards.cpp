#include "Rewards.hpp"
#include "Ads.hpp"
#include "Brand.hpp"
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/binding/GJGameLevel.hpp>
#include <ctime>
#include <optional>

namespace gcu {
    void Rewards::onAdReward() {
        Eco::addEnergy(5);
        Eco::addCents(5);
        toast(fmt::format("{}: +5 {} and $0.05", Brand::reward(), Brand::energy()));
    }

    static const char* SEASONS[12] = {"Frozen Frames", "Candy Circuits", "Spring Spikes", "Bloom Boosters", "Beach Blocks", "Solar Surge",
                                      "Hot Hazards", "Thunder Thrusters", "Spooky Spikes", "Neon Harvest", "Holiday Heists", "Final Fees"};
    static std::tm nowTm() { std::time_t t = std::time(nullptr); std::tm out{}; 
#ifdef _WIN32
        localtime_s(&out, &t);
#else
        localtime_r(&t, &out);
#endif
        return out; }
    std::string seasonName() { return SEASONS[nowTm().tm_mon]; }
    std::string seasonKey() { auto t = nowTm(); return std::to_string(t.tm_year + 1900) + "-" + std::to_string(t.tm_mon + 1); }
    int Rewards::dailyIndex() { return (int)(Store::now() / 86400); }

    static void ensureSeason() {
        // monthly reset: progress belongs to one season key
        auto key = Store::fnv(seasonKey()) % 1000000;
        if (Store::get("pass.season", -1) != (int64_t)key) {
            Store::set("pass.season", (int64_t)key); Store::set("pass.xp", 0);
            for (int i = 1; i <= 30; i++) { Store::set("pass.f." + std::to_string(i), 0); Store::set("pass.p." + std::to_string(i), 0); }
        }
    }
    int passXP() { ensureSeason(); return (int)Store::get("pass.xp", 0); }
    int passTier() { return std::min(30, passXP() / 250); }
    void Rewards::grantPassXP(int xp) { ensureSeason(); Store::add("pass.xp", xp); }

    // Online rating detection. The authoritative rating data of a GJGameLevel is its STAR VALUE (m_stars, 0 = unrated), the
    // demon flag (m_demon) and the demon sub-type (m_demonDifficulty); that is what the server sends for rated levels.
    // The old code looked at the averaged player vote / difficulty face, which is empty or N/A for levels coming from the
    // online browser, so every online level fell through to "Auto".
    // Star values: 1 Auto, 2 Easy, 3 Normal, 4-5 Hard, 6-7 Harder, 8-9 Insane, 10 Demon.
    // Returns nothing for UNRATED levels: they pay no reward.
    std::optional<Tier> tierFor(GJGameLevel* l) {
        if (!l) return std::nullopt;
        return tierFromRating(l->m_stars.value(), l->m_demon.value() > 0, l->m_demonDifficulty, l->m_autoLevel);
    }

    class $modify(GCUPlayLayer, PlayLayer) {
        struct Fields { bool rewarded = false; int burned = 0; int attemptsThisSession = 0; };

        bool init(GJGameLevel* level, bool dontCreateObjects) {
            if (!PlayLayer::init(level, dontCreateObjects)) return false;
            // Player Engagement Energy: you are never blocked from playing, but what you burn decides what you earn.
            this->burnEnergy(Eco::energyCfg().perLevel);
            return true;
        }

        void burnEnergy(int want) {
            auto cfg = Eco::energyCfg();
            if (!cfg.enabled || want <= 0) return;
            int spend = std::min(want, Eco::energy());          // never negative: at most what is left
            if (spend > 0) { Eco::addEnergy(-spend); m_fields->burned += spend; }
        }

        void resetLevel() {
            PlayLayer::resetLevel();
            this->burnEnergy(Eco::energyCfg().perAttempt);
            Store::add("stat.attempts", 1);
            Store::add("quest.deaths", 1);
            m_fields->attemptsThisSession++;
            int n = (int)Mod::get()->getSettingValue<int64_t>("ad-every-n-attempts");
            if (n > 0 && m_fields->attemptsThisSession % n == 0 && adsEnabled())
                showAd(AdKind::Forced, fmt::format("Attempt {}: a short break from your progress", m_fields->attemptsThisSession));
        }

        void levelComplete() {
            PlayLayer::levelComplete();
            // Guards: one payout per play session, none in practice/testmode, none for the player's own editor levels.
            if (m_fields->rewarded || m_isPracticeMode || m_isTestMode || !m_level) return;
            if (m_level->m_levelType == GJLevelType::Editor) return;
            m_fields->rewarded = true;

            auto tierOpt = tierFor(m_level);
            if (!tierOpt) { toast("Unrated level: no Engagement Incentive paid."); return; }   // unrated levels never pay
            auto tier = *tierOpt;
            std::string lid = fmt::format("lvl.{}.{}", (int)m_level->m_levelID.value(), (int)m_level->m_levelType);
            bool first = Store::get(lid, 0) == 0;              // one full payout per level; replays pay 10%
            auto cfg = Eco::energyCfg();
            int64_t cents = Eco::payoutCents(tier, first, cfg, m_fields->burned);
            std::string note = !cfg.enabled ? fmt::format(" (energy off: {}%)", cfg.disabledPct) : "";
            Store::set(lid, 1);
            Eco::addCents(cents);
            Store::add("stat.completions", 1);
            Store::add("stat.earned", cents);
            Store::add("quest.complete", 1);

            // Energy bonus lands in other currencies so the cash table stays exact
            int orbs = tierStars(tier) * 25 * (m_fields->burned > 0 ? 2 : 1);
            Eco::addCur("orbs", orbs); Eco::addCur("stars", tierStars(tier));
            if (tierStars(tier) >= 10) { Eco::addCur("keys", 1); Eco::addCur("shards", 2); }
            Rewards::grantPassXP(tierStars(tier) * 10);

            cashRain(this, "+" + Eco::money(cents) + note);
            toast(fmt::format("{} complete! {} Engagement Incentive: {}{}", m_level->m_levelName, tierName(tier), Eco::money(cents), note));
            if (adsEnabled() && Store::get("secret.noads", 0) == 0)
                showAd(AdKind::Forced, "Level complete. Here is an ad to celebrate");
        }
    };
}
