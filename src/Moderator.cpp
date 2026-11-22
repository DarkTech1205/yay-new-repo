#include "Screens.hpp"
#include "Ads.hpp"

// Parody moderator panel. Operates ONLY on local demo data stored in this mod's save.
// It cannot rate real levels, ban real players, or call any server.
namespace gcu {
    struct DemoLvl { char const* name; char const* by; };
    static const DemoLvl LV[] = {{"Neon Descent", "PixelPriya"}, {"Glitch Dojo", "Voltaic"}, {"Hyper Furnace", "NeonNate"}, {"Prism Gauntlet", "Cubewright"}, {"Atomic Skyline", "MoonMiko"}, {"Turbo Circuit", "SpikeSmith"}};
    static const char* RATE[] = {"Unrated", "Rated", "Featured", "Epic", "Legendary", "Mythic"};
    static int rateOf(int i) { return (int)Store::get("mod.rate." + std::to_string(i), 0); }

    static bool memberOnly() {
        if (Eco::subActive("mod")) return true;
        toast("Moderator Membership required ($100/month)", false);
        return false;
    }
    static void setRate(int i, int r, char const* msg) { Store::set("mod.rate." + std::to_string(i), r); Store::add("mod.actions", 1); toast(std::string(LV[i].name) + ": " + msg); }

    static void openLevelActions(int i) {
        openPopup(MenuPopup::create(std::string("Level: ") + LV[i].name, [i]() {
            return std::vector<Row>{
                {"Rate Level", "Current: " + std::string(RATE[rateOf(i)]), "Rate", [i]() { setRate(i, std::max(1, rateOf(i)), "rated"); }},
                {"Feature Level", "Adds to the demo Featured tab", "Feature", [i]() { setRate(i, std::max(2, rateOf(i)), "featured"); }},
                {"Epic Rating", "Purple glow", "Epic", [i]() { setRate(i, std::max(3, rateOf(i)), "Epic"); }, {185, 110, 255}},
                {"Legendary Rating", "Gold glow", "Legendary", [i]() { setRate(i, std::max(4, rateOf(i)), "Legendary"); }, {255, 176, 0}},
                {"Mythic Rating", "The rating we invented just now", "Mythic", [i]() { setRate(i, 5, "Mythic"); }, {255, 61, 129}},
                {"Send to Hall of Fame", Store::get("mod.fame." + std::to_string(i), 0) ? "In the Hall of Fame" : "Demo list only", "Send", [i]() { Store::set("mod.fame." + std::to_string(i), 1); Store::add("mod.actions", 1); toast("Sent to demo Hall of Fame"); }},
                {"Unrate", "Reset to unrated", "Reset", [i]() { Store::set("mod.rate." + std::to_string(i), 0); Store::set("mod.fame." + std::to_string(i), 0); toast("Unrated"); }, {200, 200, 200}},
            };
        }, []() { return "Local demo data only. Real servers are never contacted."; }, {120, 200, 255}));
    }

    static const char* REPORTS[] = {"'Name is a pun about wallets'", "'Contains 4 spikes in a row, unacceptable'", "'Impossible without paying for energy'", "'Song is too good, possible plagiarism'"};
    static const char* COMMENTS[] = {"first!", "the ship part ate my wallet", "this level should cost $1 more", "rate pls", "refund my 3 attempts"};
    static const char* PLAYERS[] = {"Voltaic: 48,210 stars, 312 demons", "MoonMiko: 30,120 stars, 95 demons", "SpikeSmith: 22,115 stars, 40 demons", "PixelPriya: 9,100 stars, 4 demons"};

    void openModPanel() {
        if (!memberOnly()) { openSubs(); return; }
        openPopup(MenuPopup::create("MODERATOR PANEL", []() {
            std::vector<Row> r;
            for (int i = 0; i < 6; i++) r.push_back({LV[i].name, std::string("by ") + LV[i].by + " - " + RATE[rateOf(i)] + (Store::get("mod.fame." + std::to_string(i), 0) ? " + Hall of Fame" : ""), "Actions", [i]() { openLevelActions(i); }, {140, 210, 255}});
            r.push_back({"Review Reports", std::to_string(4 - (int)Store::get("mod.reports", 0)) + " open", "Review", []() {
                int idx = (int)Store::get("mod.reports", 0);
                if (idx >= 4) { toast("Inbox zero. Suspicious.", false); return; }
                Store::set("mod.reports", idx + 1); Store::add("mod.actions", 1);
                toast(std::string("Resolved: ") + REPORTS[idx]);
            }});
            r.push_back({"Manage Comments", std::to_string(5 - (int)Store::get("mod.comments", 0)) + " pending", "Delete", []() {
                int idx = (int)Store::get("mod.comments", 0);
                if (idx >= 5) { toast("No comments left to delete", false); return; }
                Store::set("mod.comments", idx + 1); Store::add("mod.actions", 1);
                toast(std::string("Deleted: '") + COMMENTS[idx] + "'");
            }});
            r.push_back({"View Player Profiles", "Demo players only", "View", []() {
                int i = (int)(Store::get("mod.view", 0) % 4); Store::add("mod.view", 1); toast(PLAYERS[i]);
            }});
            r.push_back({"Moderator Statistics", fmt::format("{} actions, {} rated, {} in Hall of Fame", Store::get("mod.actions", 0),
                [] { int n = 0; for (int i = 0; i < 6; i++) n += rateOf(i) > 0; return n; }(), [] { int n = 0; for (int i = 0; i < 6; i++) n += (int)Store::get("mod.fame." + std::to_string(i), 0); return n; }()), "", nullptr});
            r.push_back({"Special Privileges", "Skip ads (1 min), free energy refill", "Use", []() { Eco::addEnergy(25); Store::set("secret.noads", 1); toast("Privileges used: energy full, ads off"); }, {255, 215, 0}});
            return r;
        }, []() { return fmt::format("[MOD] badge active - renews in {}:{:02}", Eco::subSecondsLeft("mod") / 60, Eco::subSecondsLeft("mod") % 60); }, {255, 215, 0}));
    }
}
