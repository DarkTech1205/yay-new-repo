#include "Screens.hpp"
#include "Ads.hpp"
#include "Rewards.hpp"
#include "Brand.hpp"

namespace gcu {
    static std::string mmss(int64_t s) { return fmt::format("{}:{:02}", s / 60, s % 60); }
    static std::string header() {
        return fmt::format("{} {}  |  {} gems  |  energy {}/{}{}", Brand::wallet(), Eco::money(Eco::cents()), Eco::cur("gems"), Eco::energy(), Eco::energyMax(),
            Eco::energy() < Eco::energyMax() ? " (next " + mmss(Eco::secondsToNextEnergy()) + ")" : "");
    }
    static MenuPopup* mk(std::string const& title, std::function<std::vector<Row>()> b, ccColor3B tint = {255, 255, 255}) {
        auto p = MenuPopup::create(title, std::move(b), header, tint); openPopup(p); return p;
    }

    // ---------------- store bundles ----------------
    struct Bundle { char const* name; Price cost; Price give; char const* sub; };
    static std::vector<Bundle> bundles() {
        auto P = [](int64_t c) { Price p; p.cents = c; return p; };
        auto G = [](int64_t g, int64_t o = 0, int64_t k = 0, int64_t s = 0, int64_t d = 0) { Price p; p.gems = g; p.orbs = o; p.keys = k; p.shards = s; p.diamonds = d; return p; };
        return {
            {"Starter Bundle", P(99), G(100, 500, 1), "100 gems, 500 orbs, 1 key. One per wallet."},
            {"Mega Starter Bundle", P(499), G(600, 3000, 5, 5), "Starter, but 6x bigger."},
            {"Pile of Gems", P(199), G(200), "200 gems"}, {"Bag of Gems", P(499), G(550), "550 gems"},
            {"Crate of Gems", P(999), G(1200), "1200 gems"}, {"Vault of Gems", P(4999), G(7000), "7000 gems"},
            {"Demon Keys x5", P(299), G(0, 0, 5), "5 demon keys"}, {"Demon Keys x25", P(999), G(0, 0, 25), "25 demon keys"},
            {"Shard Pouch", P(399), G(0, 0, 0, 20), "20 shards"}, {"Shard Chest", P(1999), G(0, 0, 0, 120), "120 shards"},
        };
    }
    static void giveAll(Price const& g) {
        Eco::addCur("gems", g.gems); Eco::addCur("orbs", g.orbs); Eco::addCur("keys", g.keys); Eco::addCur("shards", g.shards); Eco::addCur("diamonds", g.diamonds);
    }
    void openStore() {
        mk("Store", []() {
            std::vector<Row> r;
            for (auto& b : bundles()) {
                bool once = std::string(b.name) == "Starter Bundle" && Eco::bought("bundle.starter");
                r.push_back({b.name, once ? "Already claimed" : b.sub, once ? "Owned" : Eco::money(b.cost.cents), [b]() {
                    if (std::string(b.name) == "Starter Bundle" && Eco::bought("bundle.starter")) { toast("One per wallet. Rules are rules.", false); return; }
                    confirmPurchase(b.name, b.cost, [b]() { if (std::string(b.name) == "Starter Bundle") Eco::markBought("bundle.starter"); giveAll(b.give); Store::add("quest.buy", 1); toast(std::string(b.name) + " delivered"); });
                }, {255, 230, 120}});
            }
            // premium level packs, song packs, quest slots
            struct Pk { char const* id; char const* n; int64_t c; char const* s; };
            for (auto& k : std::vector<Pk>{{"pack.neon", "Premium Level Pack: Neon", 399, "3 premium levels (cosmetic unlock flag)"}, {"pack.demon", "Premium Level Pack: Demons", 999, "4 premium demons"},
                                         {"song.1", "Song Pack: Subscription Funk", 249, "6 songs. Downloads cost an ad each."}, {"song.2", "Song Pack: Gem Grinder", 249, "6 more songs"},
                                         {"slot.1", "Extra Quest Slot", 299, "A 4th daily objective"}, {"slot.2", "Extra Quest Slot II", 499, "A 5th daily objective"}}) {
                bool own = Eco::bought(k.id);
                r.push_back({k.n, own ? "Owned" : k.s, own ? "Owned" : Eco::money(k.c), [k]() {
                    if (Eco::bought(k.id)) { toast("You already own this", false); return; }
                    Price p; p.cents = k.c;
                    confirmPurchase(k.n, p, [k]() { Eco::markBought(k.id); Store::add("quest.buy", 1); toast(std::string(k.n) + " unlocked"); });
                }, {120, 220, 255}});
            }
            return r;
        }, {255, 230, 120});
    }

    // ---------------- exchange (deliberately lossy) ----------------
    void openExchange() {
        struct Ex { char const* label; char const* from; int64_t n; char const* to; int64_t m; };
        static const Ex EX[] = {
            {"500 orbs -> 1 key", "orbs", 500, "keys", 1}, {"3 coins -> 10 diamonds", "coins", 3, "diamonds", 10}, {"20 stars -> 400 orbs", "stars", 20, "orbs", 400},
            {"10 moons -> 40 diamonds", "moons", 10, "diamonds", 40}, {"100 diamonds -> 20 shards", "diamonds", 100, "shards", 20},
            {"50 shards -> 1 gem (yes, 1)", "shards", 50, "gems", 1}, {"100 gems -> $0.50 wallet", "gems", 100, "cents", 50}, {"$1.00 wallet -> 40 gems", "cents", 100, "gems", 40},
        };
        mk("Currency Exchange", []() {
            std::vector<Row> r;
            for (auto& e : EX) {
                r.push_back({e.label, "Fees applied. Rates change when we feel like it.", "Swap", [e]() {
                    int64_t have = std::string(e.from) == "cents" ? Eco::cents() : Eco::cur(e.from);
                    if (have < e.n) { toast(fmt::format("Need {} {} (have {})", e.n, e.from, have), false); return; }
                    if (std::string(e.from) == "cents") Eco::addCents(-e.n); else Eco::addCur(e.from, -e.n);
                    if (std::string(e.to) == "cents") Eco::addCents(e.m); else Eco::addCur(e.to, e.m);
                    toast(std::string("Exchanged: ") + e.label);
                }, {255, 255, 255}});
            }
            return r;
        });
    }

    // ---------------- wallet / energy ----------------
    void openWallet() {
        mk("Wallet", []() {
            std::vector<Row> r;
            r.push_back({"Wallet: " + Eco::money(Eco::cents()), "Fictional credits. Not real money. Not withdrawable.", "", nullptr});
            r.push_back({fmt::format("{} {}/{}", Brand::energy(), Eco::energy(), Eco::energyMax()), Eco::energyBlurb(), "Ad +5", []() { showAd(AdKind::Rewarded, "Restore energy", nullptr); }});
            r.push_back({"Refill energy: $0.99", "Instantly fills all 25 energy", "$0.99", []() { Price p; p.cents = 99; confirmPurchase("Energy Refill", p, []() { Eco::addEnergy(25); toast("Energy full"); }); }});
            r.push_back({"Currencies", fmt::format("{} stars, {} moons, {} coins, {} orbs", Eco::cur("stars"), Eco::cur("moons"), Eco::cur("coins"), Eco::cur("orbs")), "", nullptr});
            r.push_back({"More currencies", fmt::format("{} diamonds, {} keys, {} shards, {} gems", Eco::cur("diamonds"), Eco::cur("keys"), Eco::cur("shards"), Eco::cur("gems")), "", nullptr});
            r.push_back({"Currency Exchange", "Swap one thing for a worse thing", "Open", []() { openExchange(); }});
            r.push_back({"Add funds (simulated)", "Adds fictional credits. No real payment is ever taken.", "+$5", []() { Eco::addCents(500); toast("Added $5.00 of pretend money"); }});
            r.push_back({"Reset ALL save data", "Wipes wallet, purchases, pass, everything", "Reset", []() {
                Price p; confirmPurchase("RESET SAVE (free, irreversible)", p, []() { Store::reset(); toast("Save reset. Welcome back, new customer."); });
            }, {255, 120, 120}});
            return r;
        });
    }

    // ---------------- subscriptions ----------------
    void openSubs() {
        struct Sb { char const* id; char const* name; int64_t cents; char const* perk; };
        static const Sb SUBS[] = {
            {"mod", "MODERATOR MEMBERSHIP", 10000, "Badge, moderator panel, stats, special privileges"},
            {"editor", "Editor Premium", 500, "Playtesting in the level editor"},
            {"pass", "GeoPass Premium", 1499, "Premium reward track this season"},
        };
        mk("Subscriptions", []() {
            std::vector<Row> r;
            for (auto& s : SUBS) {
                bool on = Eco::subActive(s.id);
                std::string sub = on ? fmt::format("ACTIVE. Renews in {}{}", mmss(Eco::subSecondsLeft(s.id)), Store::get(std::string("subcancel.") + s.id, 0) ? " (ending)" : "") : std::string(s.perk);
                r.push_back({std::string(s.name) + "  " + Eco::money(s.cents) + "/mo", sub, on ? "Cancel" : "Subscribe", [s, on]() {
                    if (on) { Eco::subCancel(s.id); toast("Cancelled. It will not renew (we are sad)."); return; }
                    Price p; p.cents = s.cents;
                    confirmPurchase(std::string(s.name) + " (monthly)", p, [s]() { Eco::subStart(s.id, s.cents); toast(std::string(s.name) + " active"); });
                }, s.cents >= 10000 ? ccColor3B{255, 90, 90} : ccColor3B{255, 255, 255}});
            }
            r.push_back({"A 'month' lasts " + std::to_string(Mod::get()->getSettingValue<int64_t>("sub-period-minutes")) + " real minutes", "Auto-renews from your wallet. Lapses if you cannot pay.", "", nullptr});
            return r;
        }, {255, 120, 120});
    }

    // ---------------- daily objectives ----------------
    struct Obj { char const* key; char const* text; int64_t need; int xp; };
    static const Obj OBJ[] = {{"complete", "Complete a level", 1, 80}, {"deaths", "Die 20 times", 20, 40}, {"ads", "Watch an ad", 1, 40}, {"buy", "Buy something", 1, 60}, {"qcomplete3", "Complete 3 levels", 3, 150}};
    void openQuests() {
        mk(Brand::daily(), []() {
            std::vector<Row> r;
            int64_t day = Rewards::dailyIndex();
            if (Store::get("quest.day", -1) != day) {
                Store::set("quest.day", day);
                for (auto& o : OBJ) { Store::set(std::string("qclaim.") + o.key, 0); }
                Store::set("quest.complete", 0); Store::set("quest.deaths", 0); Store::set("quest.ads", 0); Store::set("quest.buy", 0);
            }
            int slots = 3 + (Eco::bought("slot.1") ? 1 : 0) + (Eco::bought("slot.2") ? 1 : 0);
            for (int i = 0; i < slots && i < 5; i++) {
                auto& o = OBJ[i];
                std::string k = std::string(o.key) == "qcomplete3" ? "complete" : o.key;
                int64_t have = Store::get("quest." + k, 0); bool done = have >= o.need; bool claimed = Store::get(std::string("qclaim.") + o.key, 0) != 0;
                r.push_back({o.text, fmt::format("{}/{}  reward: {} pass xp, 1 moon", std::min<int64_t>(have, o.need), o.need, o.xp), claimed ? "Claimed" : done ? "Claim" : "Locked", [o, done, claimed]() {
                    if (claimed) { toast("Already claimed", false); return; }
                    if (!done) { toast("Not done yet", false); return; }
                    // claiming rewards shows an ad first, as is tradition
                    showAd(AdKind::Forced, "Claiming a quest reward", [o](bool) { Store::set(std::string("qclaim.") + o.key, 1); Rewards::grantPassXP(o.xp); Eco::addCur("moons", 1); toast("Objective claimed"); });
                }});
            }
            r.push_back({"Buy more quest slots", "Slots cost money. Of course they do.", "Store", []() { openStore(); }});
            return r;
        });
    }

    // ---------------- GeoPass ----------------
    void openPass() {
        mk("GeoPass: " + seasonName(), []() {
            std::vector<Row> r;
            int tier = passTier(); bool prem = Eco::subActive("pass");
            r.push_back({fmt::format("Tier {}/30  -  {}/250 xp to next", tier, passXP() % 250), seasonKey() + " season ends at month end. Everything resets.", "", nullptr});
            r.push_back({prem ? "Premium: ACTIVE" : "Premium track locked", prem ? "Gold rewards are yours" : "$14.99/month unlocks the glitter", prem ? "" : "Buy", []() { openSubs(); }, {255, 215, 0}});
            r.push_back({"Skip a tier", "$1.50 for 250 xp. Time is money.", "$1.50", []() { Price p; p.cents = 150; confirmPurchase("Pass tier skip", p, []() { Rewards::grantPassXP(250); }); }});
            for (int t = 1; t <= 30; t++) {
                bool reached = t <= tier;
                bool fc = Store::get("pass.f." + std::to_string(t), 0), pc = Store::get("pass.p." + std::to_string(t), 0);
                int64_t fOrbs = 100 * t, pGems = 20 + 6 * t;
                r.push_back({fmt::format("Tier {}  free: {} orbs", t, fOrbs), fmt::format("PREMIUM: {} gems + {} shards{}", pGems, t % 5 == 0 ? 5 : 0, t % 10 == 0 ? " + exclusive cosmetic" : ""),
                    !reached ? "Locked" : (fc && (pc || !prem)) ? "Done" : "Claim", [t, reached, prem, fOrbs, pGems]() {
                        if (!reached) { toast("Reach tier " + std::to_string(t) + " first", false); return; }
                        bool any = false;
                        if (!Store::get("pass.f." + std::to_string(t), 0)) { Eco::addCur("orbs", fOrbs); Store::set("pass.f." + std::to_string(t), 1); any = true; }
                        if (prem && !Store::get("pass.p." + std::to_string(t), 0)) { Eco::addCur("gems", pGems); if (t % 5 == 0) Eco::addCur("shards", 5); if (t % 10 == 0) Eco::markBought("cos.set." + std::to_string(t / 10 - 1)); Store::set("pass.p." + std::to_string(t), 1); any = true; }
                        toast(any ? "Tier rewards claimed" : "Nothing left to claim here", any);
                    }, reached && prem ? ccColor3B{255, 215, 0} : ccColor3B{200, 200, 200}});
            }
            return r;
        }, {255, 215, 0});
    }

    // ---------------- treasure room ----------------
    struct Chest { char const* name; Price cost; Price give; char const* setItem; };
    static std::vector<Chest> chests() {
        auto K = [](int64_t k) { Price p; p.keys = k; return p; }; auto GM = [](int64_t g) { Price p; p.gems = g; return p; };
        auto C = [](int64_t c) { Price p; p.cents = c; return p; };
        auto R = [](int64_t o, int64_t d, int64_t s, int64_t g = 0) { Price p; p.orbs = o; p.diamonds = d; p.shards = s; p.gems = g; return p; };
        return {
            {"Bronze Chest", K(1), R(200, 5, 0), nullptr}, {"Silver Chest", K(5), R(600, 20, 2), "Neon Nostalgia: Cube"},
            {"Gold Chest", K(10), R(1500, 40, 5), "Neon Nostalgia: Trail"}, {"Gem Chest (gems)", GM(250), R(0, 40, 6), "Ledger of Doom: Ship"},
            {"Premium Safe", C(1499), R(0, 0, 10, 1000), "Premium Vault: Death Effect"}, {"Wallet Drainer", C(9999), R(0, 0, 150, 9000), "Premium Vault: Full Set"},
        };
    }
    void openTreasure() {
        if (!Eco::bought("treasure.entry")) {
            Price p; p.cents = 999;
            confirmPurchase("Treasure Room admission", p, []() { Eco::markBought("treasure.entry"); openTreasure(); });
            return;
        }
        mk("Treasure Room", []() {
            std::vector<Row> r;
            r.push_back({"Contents are shown BEFORE you pay", "No random drops. What you see is exactly what you get.", "", nullptr});
            int i = 0;
            for (auto& c : chests()) {
                std::string id = "chest." + std::to_string(i++);
                bool opened = Eco::bought(id);
                std::string contents = c.give.str() + (c.setItem ? std::string(" + ") + c.setItem : std::string());
                r.push_back({std::string(c.name) + "  [" + c.cost.str() + "]", contents + (opened ? " (set item owned; dupes refund 10 shards)" : ""), "Open", [c, id, opened]() {
                    confirmPurchase(c.name, c.cost, [c, id, opened]() {
                        giveAll(c.give);
                        if (c.setItem) { if (opened) { Eco::addCur("shards", 10); toast("Duplicate set item: refunded 10 shards"); } else Eco::markBought(id); }
                        toast(std::string(c.name) + " opened");
                    });
                }, {255, 200, 80}});
            }
            r.push_back({fmt::format("Showcase: collection score {}", collectionScore()), "Set pieces owned: " + std::to_string([]() { int n = 0; for (int k = 0; k < 6; k++) n += Eco::bought("chest." + std::to_string(k)) ? 1 : 0; return n; }()) + "/6", "", nullptr});
            return r;
        }, {255, 200, 80});
    }

    // ---------------- creator partner program ----------------
    struct DemoLevel { char const* name; int rating; bool featured; };
    static const DemoLevel DL[] = {{"Neon Ledger", 5, true}, {"Cube Tax", 3, false}, {"Spikes of Debt", 4, false}, {"Paywall Run", 2, false}};
    void openCreator() {
        mk("Creator Partner Program", []() {
            std::vector<Row> r;
            if (!Eco::bought("creator.joined")) {
                r.push_back({"Join the Creator Partner Program", "Free to join. Fees apply to everything after.", "Join", []() { Eco::markBought("creator.joined"); toast("Welcome, creator!"); }});
                return r;
            }
            int64_t pending = Store::get("creator.pending", 0);
            r.push_back({"Pending earnings: " + Eco::money(pending), "Fictional credits earned by the demo levels below", pending ? "Collect" : "-", [pending]() {
                if (!pending) { toast("Nothing to collect yet. Simulate a day.", false); return; }
                showAd(AdKind::Forced, "Collecting creator earnings", [pending](bool) { Eco::addCents(pending); Store::set("creator.pending", 0); toast("Collected " + Eco::money(pending)); });
            }});
            r.push_back({"Simulate a day of plays", "Plays, likes and engagement update", "Simulate", []() {
                int64_t total = 0;
                for (int i = 0; i < 4; i++) {
                    auto& d = DL[i]; int plays = 200 + (rand() % 800) * d.rating; Store::add(std::string("creator.plays.") + std::to_string(i), plays);
                    // plays * rating * popularity, with a featured multiplier
                    total += (int64_t)(plays * 0.004 * d.rating * (d.featured ? 2.5 : 1.0) * 100) / 100;
                }
                Store::add("creator.pending", total);
                toast("A day passed. Earned " + Eco::money(total));
            }});
            for (int i = 0; i < 4; i++) {
                auto& d = DL[i];
                r.push_back({d.name, fmt::format("{} plays, {} stars{}", Store::get("creator.plays." + std::to_string(i), 0), d.rating, d.featured ? ", FEATURED x2.5" : ""), "", nullptr});
            }
            // Daily-level top comment: $20 fictional credit once per day
            int64_t day = Rewards::dailyIndex(); int64_t likes = Store::get("creator.likes", 0);
            if (Store::get("creator.likeday", -1) != day) { Store::set("creator.likeday", day); Store::set("creator.likes", 0); Store::set("creator.topclaim", 0); likes = 0; }
            r.push_back({fmt::format("Daily level comment: {} likes (top = 100)", likes), "Reach 100 likes to be the top comment: $20.00 credit", "Boost", []() {
                Store::add("creator.likes", 25 + rand() % 40); toast("Comment likes rising");
            }});
            r.push_back({"Claim top-comment reward: $20.00", Store::get("creator.topclaim", 0) ? "Already claimed today" : "Needs 100 likes", "Claim", []() {
                if (Store::get("creator.topclaim", 0)) { toast("Already claimed today", false); return; }
                if (Store::get("creator.likes", 0) < 100) { toast("Not top comment yet", false); return; }
                Store::set("creator.topclaim", 1); Eco::addCents(2000); toast("Top comment! +$20.00");
            }});
            return r;
        }, {120, 255, 180});
    }

    // ---------------- hub ----------------
    void openMainShop() {
        showAd(AdKind::Forced, "Opening the shop. Please enjoy this first", [](bool) {
            mk("GeoCash Shop", []() {
                return std::vector<Row>{
                    {"Cosmetic Shop", "Icons, trails, effects. Rotating stock.", "Open", []() { openCosmeticShop(); }, {255, 220, 90}},
                    {"Store", "Gems, keys, shards, packs, quest slots", "Open", []() { openStore(); }, {255, 230, 120}},
                    {"GeoPass", "Season: " + seasonName(), "Open", []() { openPass(); }, {255, 215, 0}},
                    {"Daily Objectives", "Rewards for doing things", "Open", []() { openQuests(); }},
                    {"Geometry Dash 2.21 Pack", "Two Demon levels. Sold separately.", "Open", []() { openPackScreen(); }, {255, 140, 90}},
                    {"Treasure Room", "Chests with visible contents", "Open", []() { openTreasure(); }, {255, 200, 80}},
                    {"Creator Program", "Earn fictional credits", "Open", []() { openCreator(); }, {120, 255, 180}},
                    {"Subscriptions", "Moderator $100/mo. Editor $5/mo. Pass $14.99/mo.", "Open", []() { openSubs(); }, {255, 120, 120}},
                    {"Wallet and Energy", "Balances, refills, exchange, reset", "Open", []() { openWallet(); }},
                };
            });
        });
    }
}
