#include "Screens.hpp"
#include "Ads.hpp"

// Fictional developer console. Local demo data only. No real admin access of any kind.
namespace gcu {
    static const char* NEWS[] = {
        "v2.9.9: Added a loading screen. Skipping it costs $0.99.", "Reminder: wallet credits are fictional. The feelings are real.",
        "Extreme Demons now pay $25. Players are expected to be happy.", "Upcoming: a subscription that removes the subscription screen.",
        "Maintenance: nothing is wrong, we just wanted you to see an ad.", "Moderator Membership is now $100/month because we could.",
    };
    static const char* ROB[] = {"RobTop Top Hat Cube", "RobTop Rainbow Ledger Trail", "RobTop Admin Profile", "RobTop Server Meltdown"};
    void grantRobTopCosmetic(int i) { Eco::markBought("cos.rob." + std::to_string(i)); }

    static void ultra() {
        openPopup(MenuPopup::create("??? ULTRA SECRET ???", []() {
            return std::vector<Row>{
                {"Monetize the monetization", "Charges you $0.01 for each purchase tax", Store::get("secret.tax", 0) ? "ON" : "OFF", []() { Store::set("secret.tax", !Store::get("secret.tax", 0)); toast("Tax toggled"); }, {255, 90, 90}},
                {"Pay to unpay", "Refunds $1 for $1.50", "Do it", []() { Price p; p.cents = 150; if (!Eco::canAfford(p)) { toast("Need $1.50 to get $1.00 back", false); return; } Eco::charge(p); Eco::addCents(100); toast("Unpaid. Net -$0.50."); }},
                {"Invert wallet (cosmetic)", "Shows your balance as a debt joke", "Look", []() { toast("You owe the universe " + Eco::money(Eco::cents()) + ". Just kidding. Mostly."); }},
                {"Confetti of cash", "Pure vibes", "Rain", []() { cashRain(CCDirector::get()->getRunningScene(), "ULTRA SECRET"); }, {255, 215, 0}},
                {"Delete all ads forever", "Forever = this session", Store::get("secret.noads", 0) ? "Gone" : "Delete", []() { Store::set("secret.noads", 1); toast("Forced ads disabled"); }},
                {"Final boss: unlock all RobTop cosmetics", "4 items", "Unlock", []() { for (int i = 0; i < 4; i++) grantRobTopCosmetic(i); toast("All RobTop cosmetics unlocked"); }, {255, 176, 0}},
            };
        }, []() { return "The most ridiculous interface in the mod."; }, {255, 80, 80}));
    }

    void openRobTop(bool deeper) {
        if (deeper) { ultra(); return; }
        auto p = MenuPopup::create("ROBTop's Secret Menu", []() {
            std::vector<Row> r;
            int64_t spent = Store::get("stat.spent", 0), earned = Store::get("stat.earned", 0);
            r.push_back({"Developer statistics", fmt::format("{} attempts, {} completions, {} ads seen", Store::get("stat.attempts", 0), Store::get("stat.completions", 0), Store::get("stat.ads", 0)), "", nullptr});
            r.push_back({"Monetization dashboard", fmt::format("Spent {}  Earned {}  Net {}  (ARPU: infinite)", Eco::money(spent), Eco::money(earned), Eco::money(earned - spent)), "", nullptr});
            r.push_back({"Economy: +$100 wallet", "Local demo data only", "+$100", []() { Eco::addCents(10000); toast("+$100.00 (pretend)"); }, {120, 255, 160}});
            r.push_back({"Economy: +1000 gems, +50 keys, +100 shards", "All currencies", "Give", []() { Eco::addCur("gems", 1000); Eco::addCur("keys", 50); Eco::addCur("shards", 100); Eco::addCur("orbs", 5000); Eco::addCur("diamonds", 500); toast("Currencies granted"); }, {120, 255, 160}});
            r.push_back({"Economy: refill energy", fmt::format("{}/{}", Eco::energy(), Eco::energyMax()), "Refill", []() { Eco::addEnergy(Eco::energyMax()); toast("Energy full"); }});
            r.push_back({"Cosmetic demo: " + std::string(ROB[0]), "RobTop-themed", Eco::bought("cos.rob.0") ? "Owned" : "Unlock", []() { grantRobTopCosmetic(0); toast("Unlocked"); }, {255, 176, 0}});
            r.push_back({"Cosmetic demo: " + std::string(ROB[1]), "RobTop-themed", Eco::bought("cos.rob.1") ? "Owned" : "Unlock", []() { grantRobTopCosmetic(1); toast("Unlocked"); }, {255, 176, 0}});
            r.push_back({"Cosmetic demo: " + std::string(ROB[2]), "RobTop-themed", Eco::bought("cos.rob.2") ? "Owned" : "Unlock", []() { grantRobTopCosmetic(2); toast("Unlocked"); }, {255, 176, 0}});
            r.push_back({"Cosmetic demo: " + std::string(ROB[3]), "RobTop-themed", Eco::bought("cos.rob.3") ? "Owned" : "Unlock", []() { grantRobTopCosmetic(3); toast("Unlocked"); }, {255, 176, 0}});
            r.push_back({"Level rating controls", "Open the demo moderator panel", "Open", []() {
                Eco::subStart("mod", 0); Eco::subCancel("mod"); toast("Demo moderator access granted (local)"); openModPanel();
            }});
            r.push_back({"Secret setting: disable forced ads", Store::get("secret.noads", 0) ? "ON" : "OFF", "Toggle", []() { Store::set("secret.noads", !Store::get("secret.noads", 0)); toast("Toggled"); }});
            r.push_back({"Announcement", NEWS[Store::get("rob.news", 0) % 6], "Next", []() { Store::add("rob.news", 1); toast(NEWS[Store::get("rob.news", 0) % 6]); }});
            r.push_back({"Reset ALL save data", "Wipes everything", "Reset", []() { Price p; confirmPurchase("RESET SAVE", p, []() { Store::reset(); toast("Save reset"); }); }, {255, 120, 120}});
            r.push_back({"??? Do not press ???", "A button that reveals more buttons", "Press", []() { openRobTop(true); }, {255, 80, 80}});
            return r;
        }, []() { return "ROBTOP INTERNAL BUILD 2.2091-satire  |  Wallet " + Eco::money(Eco::cents()); }, {255, 176, 0});
        openPopup(p);
    }
}
