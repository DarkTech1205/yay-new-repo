#include "Screens.hpp"
#include "Ads.hpp"
#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#if GCU_MAC_FALLBACKS
#include <Geode/modify/DailyLevelPage.hpp>
#include <Geode/modify/GauntletSelectLayer.hpp>
#endif
#include <Geode/binding/GJSearchObject.hpp>

// Pay-per-button paywall. Only the client UI is gated; no requests, accounts or servers are touched.
namespace gcu {
    struct Feat { char const* key; char const* label; };
    static const Feat FEATS[] = {
        {"search", "Search"}, {"featured", "Featured"}, {"recent", "Recent"}, {"downloaded", "Most Downloaded"}, {"liked", "Most Liked"},
        {"trending", "Trending"}, {"daily", "Daily Level"}, {"weekly", "Weekly Demon"}, {"gauntlets", "Gauntlets"}, {"lists", "Lists"},
        {"saved", "Saved Levels"}, {"followed", "Followed Creators"},
        // extra buttons that exist in the real 2.2091 menus, monetized too
        {"magic", "Magic"}, {"awarded", "Latest Stars"}, {"friends", "Friends"}, {"fame", "Hall of Fame"}, {"event", "Event Level"},
        {"mappacks", "Map Packs"}, {"leaderboards", "Leaderboards"},
    };
    static constexpr int64_t FEATURE_CENTS = 299, ENTRY_CENTS = 499;

    static bool hasEntry() { return Eco::bought("online.entry"); }
    static bool hasFeat(std::string const& k) { return Eco::bought("online.f." + k); }
    static char const* labelOf(std::string const& k) { for (auto& f : FEATS) if (k == f.key) return f.label; return "Feature"; }

    static void buyFeature(std::string const& key) {
        Price p; p.cents = FEATURE_CENTS;
        confirmPurchase(std::string("Online Levels: ") + labelOf(key), p, [key]() {
            Eco::markBought("online.f." + key);
            toast(std::string(labelOf(key)) + " unlocked permanently. Press it again.");
        });
    }
    // true => allowed to proceed
    static bool gate(std::string const& key) {
        if (!hasEntry()) { toast("Buy Online Levels access first ($4.99)", false); return false; }
        if (hasFeat(key)) return true;
        buyFeature(key);
        return false;
    }
    static void buyEntry(std::function<void()> then) {
        Price p; p.cents = ENTRY_CENTS;
        confirmPurchase("Online Levels access", p, [then]() { Eco::markBought("online.entry"); toast("Online Levels unlocked. Features sold separately."); if (then) then(); });
    }

    void openOnlineHub() {
        auto p = MenuPopup::create("Online Levels Features", []() {
            std::vector<Row> r;
            r.push_back({"Online Levels access", hasEntry() ? "Unlocked" : "Required to enter the menu", hasEntry() ? "Owned" : "$4.99", []() { if (!hasEntry()) buyEntry(nullptr); }, {255, 230, 120}});
            for (auto& f : FEATS) {
                bool own = hasFeat(f.key);
                r.push_back({f.label, own ? "Unlocked" : "Each button is sold separately", own ? "Owned" : "$2.99", [k = std::string(f.key)]() {
                    if (!hasEntry()) { toast("Buy access first ($4.99)", false); return; }
                    if (!hasFeat(k)) buyFeature(k); else toast("Already unlocked. Use it from the Creator menu.");
                }});
            }
            return r;
        }, []() { return "Wallet " + Eco::money(Eco::cents()); }, {255, 230, 120});
        openPopup(p);
    }

    static std::string featureForSearch(SearchType t) {
        switch (t) {
            case SearchType::Search: case SearchType::Users: return "search";
            case SearchType::Downloaded: return "downloaded"; case SearchType::MostLiked: return "liked";
            case SearchType::Trending: return "trending"; case SearchType::Recent: return "recent";
            case SearchType::Featured: return "featured"; case SearchType::Magic: return "magic";
            case SearchType::Awarded: return "awarded"; case SearchType::Followed: return "followed";
            case SearchType::Friends: return "friends"; case SearchType::HallOfFame: return "fame";
            case SearchType::DailySafe: return "daily"; case SearchType::WeeklySafe: return "weekly";
            case SearchType::EventSafe: return "event"; case SearchType::SavedLevels: return "saved";
            case SearchType::LevelListsOnClick: return "lists";
            default: return "";
        }
    }

    // Creator button = "Online Levels" entrance; called from the MenuLayer hook in main.cpp
    void onlineEntrance(std::function<void()> proceed) {
        if (hasEntry()) { proceed(); return; }
        buyEntry(proceed);
    }

    // After a failed gate we only show the purchase dialog; we never touch the scene stack here.
    // Every "leave the screen" path goes through backOutNextFrame, which refuses to act on stale layers.
    static void promptAfterBackOut(std::string key) {
        if (hasEntry()) buyFeature(key); else toast("Buy Online Levels access first ($4.99)", false);
    }

    class $modify(GCUCreatorLayer, CreatorLayer) {
        void onOnlineLevels(CCObject* s) { if (gate("search")) CreatorLayer::onOnlineLevels(s); }
        void onFeaturedLevels(CCObject* s) { if (gate("featured")) CreatorLayer::onFeaturedLevels(s); }
        void onTopLists(CCObject* s) { if (gate("lists")) CreatorLayer::onTopLists(s); }
        void onMapPacks(CCObject* s) { if (gate("mappacks")) CreatorLayer::onMapPacks(s); }
        void onLeaderboards(CCObject* s) { if (gate("leaderboards")) CreatorLayer::onLeaderboards(s); }
        // Paths are replaced by the monthly battle pass; Treasure Room is reworked
        void onPaths(CCObject*) { openPass(); }
        void onTreasureRoom(CCObject*) { openTreasure(); }
#if GCU_FULL_BINDINGS
        void onDailyLevel(CCObject* s) { if (gate("daily")) CreatorLayer::onDailyLevel(s); }
        void onGauntlets(CCObject* s) { if (gate("gauntlets")) CreatorLayer::onGauntlets(s); }
        void onSavedLevels(CCObject* s) { if (gate("saved")) CreatorLayer::onSavedLevels(s); }
#endif
#if GCU_WEEKLY_EVENT_BINDINGS
        void onWeeklyLevel(CCObject* s) { if (gate("weekly")) CreatorLayer::onWeeklyLevel(s); }
        void onEventLevel(CCObject* s) { if (gate("event")) CreatorLayer::onEventLevel(s); }
#endif
    };

    // Search screen buttons. Present on every platform: Search, Search User, Friends.
    // Quick-search buttons (Most Liked, Trending, ...) have addresses on Windows and (by symbol) Android only.
    class $modify(GCUSearchLayer, LevelSearchLayer) {
        void onSearch(CCObject* s) { if (gate("search")) LevelSearchLayer::onSearch(s); }
        void onSearchUser(CCObject* s) { if (gate("search")) LevelSearchLayer::onSearchUser(s); }
        void onFriends(CCObject* s) { if (gate("friends")) LevelSearchLayer::onFriends(s); }
#if GCU_FULL_BINDINGS
        void onMostDownloaded(CCObject* s) { if (gate("downloaded")) LevelSearchLayer::onMostDownloaded(s); }
        void onMostLikes(CCObject* s) { if (gate("liked")) LevelSearchLayer::onMostLikes(s); }
        void onMostRecent(CCObject* s) { if (gate("recent")) LevelSearchLayer::onMostRecent(s); }
        void onTrending(CCObject* s) { if (gate("trending")) LevelSearchLayer::onTrending(s); }
        void onMagic(CCObject* s) { if (gate("magic")) LevelSearchLayer::onMagic(s); }
        void onLatestStars(CCObject* s) { if (gate("awarded")) LevelSearchLayer::onLatestStars(s); }
        void onFollowed(CCObject* s) { if (gate("followed")) LevelSearchLayer::onFollowed(s); }
#endif
    };

    // Last line of defence for entry points with no hookable button (Hall of Fame, and every quick-search
    // button on macOS). The browser opens, and if the feature is locked we leave on the next frame.
    class $modify(GCUBrowser, LevelBrowserLayer) {
        bool init(GJSearchObject* obj) {
            if (!LevelBrowserLayer::init(obj)) return false;
            if (!obj) return true;
            auto key = featureForSearch(obj->m_searchType);
            if (key.empty() || hasFeat(key)) return true;
            backOutNextFrame(Ref<CCLayer>(this), true, [key]() { promptAfterBackOut(key); });
            return true;
        }
    };

#if GCU_MAC_FALLBACKS
    // macOS has no address for the Daily / Gauntlet / Saved buttons, so gate the pages they open.
    class $modify(GCUDailyPage, DailyLevelPage) {
        bool init(GJTimedLevelType type) {
            if (!DailyLevelPage::init(type)) return false;
            std::string key = type == GJTimedLevelType::Daily ? "daily" : type == GJTimedLevelType::Weekly ? "weekly" : "event";
            if (hasFeat(key)) return true;
            backOutNextFrame(Ref<CCLayer>(this), false, [key]() { promptAfterBackOut(key); });
            return true;
        }
    };
    class $modify(GCUGauntlets, GauntletSelectLayer) {
        bool init(int unused) {
            if (!GauntletSelectLayer::init(unused)) return false;
            if (hasFeat("gauntlets")) return true;
            backOutNextFrame(Ref<CCLayer>(this), true, []() { promptAfterBackOut("gauntlets"); });
            return true;
        }
    };
#endif
}
