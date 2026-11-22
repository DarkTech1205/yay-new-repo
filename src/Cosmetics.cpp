#include "Screens.hpp"
#include "Ads.hpp"
#include <Geode/binding/GameManager.hpp>
#include <Geode/modify/GameManager.hpp>
#include <unordered_set>
#include <functional>

namespace gcu {
    struct Cat { char const* name; char const* key; int type; int baseId; bool real; };
    // type: IconType value, or -1 for mod-only categories
    static const Cat CATS[] = {
        {"Cubes", "cube", (int)IconType::Cube, 30, true}, {"Ships", "ship", (int)IconType::Ship, 30, true}, {"Balls", "ball", (int)IconType::Ball, 30, true},
        {"UFOs", "ufo", (int)IconType::Ufo, 30, true}, {"Waves", "wave", (int)IconType::Wave, 30, true}, {"Robots", "robot", (int)IconType::Robot, 10, true},
        {"Spiders", "spider", (int)IconType::Spider, 10, true}, {"Swings", "swing", (int)IconType::Swing, 10, true},
        {"Trails", "trail", (int)IconType::Special, 1, true}, {"Death Effects", "death", (int)IconType::DeathEffect, 1, true},
        {"Ship Fires", "fire", (int)IconType::ShipFire, 1, true}, {"Colors", "color", -1, 100, false},
        {"Custom Particles", "particle", -1, 200, false}, {"Profile Decorations", "deco", -1, 300, false},
    };
    static constexpr int NCAT = sizeof(CATS) / sizeof(CATS[0]);
    static const char* RAR[] = {"Common", "Uncommon", "Rare", "Epic", "Legendary", "Mythic"};
    static const ccColor3B RCOL[] = {{170, 180, 190}, {80, 230, 120}, {60, 140, 255}, {185, 110, 255}, {255, 176, 0}, {255, 61, 129}};
    static const int RPTS[] = {1, 3, 8, 20, 50, 120};
    static const int PATTERN[8] = {0, 0, 1, 1, 2, 3, 4, 5};
    static const char* ADJ[] = {"Neon", "Rusty", "Cosmic", "Glitch", "Velvet", "Turbo", "Frozen", "Solar", "Plasma", "Hollow", "Prism", "Toxic"};
    static const char* NOUN[] = {"Pulse", "Drift", "Bloom", "Shard", "Echo", "Ember", "Flux", "Haze"};

    static std::string itemId(int c, int i) { return std::string("cos.") + CATS[c].key + "." + std::to_string(i); }
    static std::string itemName(int c, int i) {
        auto h = Store::fnv(itemId(c, i));
        return std::string(ADJ[h % 12]) + " " + NOUN[(h >> 5) % 8];
    }
    static Price rarityPrice(int r) {
        Price p;
        switch (r) {
            case 0: p.orbs = 400; break; case 1: p.orbs = 1200; break; case 2: p.diamonds = 45; break;
            case 3: p.diamonds = 130; p.cents = 499; break; case 4: p.gems = 1800; break; default: p.shards = 150; p.cents = 2999; break;
        }
        return p;
    }
    // Rare and above leave the shop for long stretches; the slot length is a setting.
    static bool inRotation(int c, int i, int rarity, int64_t* slotsLeft) {
        if (rarity < 2) return true;
        static const int CYCLE[] = {0, 0, 6, 9, 16, 30}, WIN[] = {0, 0, 3, 3, 2, 1};
        int64_t slotLen = std::max<int64_t>(1, Mod::get()->getSettingValue<int64_t>("rotation-hours")) * 3600;
        int64_t slot = Store::now() / slotLen;
        int64_t ph = (slot + (int64_t)(Store::fnv(itemId(c, i)) % 1000)) % CYCLE[rarity];
        if (slotsLeft) *slotsLeft = ph < WIN[rarity] ? WIN[rarity] - ph : CYCLE[rarity] - ph;
        return ph < WIN[rarity];
    }

    int collectionScore() {
        int s = 0;
        for (int c = 0; c < NCAT; c++) for (int i = 1; i <= 8; i++) if (Eco::bought(itemId(c, i))) s += RPTS[PATTERN[i - 1]];
        for (int k = 0; k < 8; k++) if (Eco::bought("cos.set." + std::to_string(k))) s += 60;
        for (int k = 0; k < 4; k++) if (Eco::bought("cos.rob." + std::to_string(k))) s += 200;
        return s;
    }

    // GameManager::unlockIcon has a Windows-only inline binding, so it cannot be called on macOS or Android.
    // Instead we answer isIconUnlocked() for icons the player bought here. That function is bound everywhere.
    static std::unordered_set<int> g_modIcons;
    static uint64_t g_modIconsVersion = 0;
    static void rebuildIcons() {
        g_modIcons.clear();
        for (int c = 0; c < NCAT; c++) {
            if (!CATS[c].real) continue;
            for (int i = 1; i <= 8; i++) if (Eco::bought(itemId(c, i))) g_modIcons.insert(CATS[c].type * 100000 + CATS[c].baseId + i);
        }
        g_modIconsVersion = Store::version();
    }
    static void grantReal(int, int) { g_modIconsVersion = 0; }   // forces a rebuild on the next query

    class $modify(GCUGameManager, GameManager) {
        bool isIconUnlocked(int id, IconType type) {
            if (GameManager::isIconUnlocked(id, type)) return true;
            if (g_modIconsVersion != Store::version()) rebuildIcons();
            return g_modIcons.contains((int)type * 100000 + id);
        }
    };

    static void equip(int c, int i) {
        auto gm = GameManager::get(); int id = CATS[c].baseId + i;
        switch ((IconType)CATS[c].type) {
            case IconType::Cube: gm->setPlayerFrame(id); break; case IconType::Ship: gm->setPlayerShip(id); break;
            case IconType::Ball: gm->setPlayerBall(id); break; case IconType::Ufo: gm->setPlayerBird(id); break;
            case IconType::Wave: gm->setPlayerDart(id); break; case IconType::Robot: gm->setPlayerRobot(id); break;
            case IconType::Spider: gm->setPlayerSpider(id); break; case IconType::Swing: gm->setPlayerSwing(id); break;
            default: break;
        }
        Store::set(std::string("equip.") + CATS[c].key, i);
        toast("Equipped " + itemName(c, i));
    }

    static void openCategory(int c) {
        auto p = MenuPopup::create(CATS[c].name, [c]() {
            std::vector<Row> rows;
            for (int i = 1; i <= 8; i++) {
                int r = PATTERN[i - 1]; int64_t left = 0; bool avail = inRotation(c, i, r, &left);
                Row row; row.text = itemName(c, i); row.color = RCOL[r];
                bool own = Eco::bought(itemId(c, i));
                if (own) {
                    row.sub = std::string(RAR[r]) + " - owned" + (CATS[c].real ? "" : "");
                    row.btn = Store::get(std::string("equip.") + CATS[c].key, 0) == i ? "Equipped" : "Equip";
                    row.onClick = [c, i]() { equip(c, i); };
                } else if (!avail) {
                    row.sub = std::string(RAR[r]) + " - vaulted, returns in ~" + std::to_string(left) + " rotations";
                    row.btn = "Vaulted"; row.onClick = [left]() { toast("Gone for now. Check back in " + std::to_string(left) + " rotations.", false); };
                } else {
                    auto price = rarityPrice(r);
                    row.sub = std::string(RAR[r]) + (r >= 2 ? (" - leaves in " + std::to_string(left) + " rotations") : "");
                    row.btn = price.cents && !price.gems && !price.orbs && !price.diamonds && !price.shards ? Eco::money(price.cents) : "Buy";
                    row.onClick = [c, i, price]() {
                        confirmPurchase(itemName(c, i) + " (" + RAR[PATTERN[i - 1]] + " " + CATS[c].name + ")", price, [c, i]() {
                            Eco::markBought(itemId(c, i)); grantReal(c, i); Store::add("quest.buy", 1);
                            toast("Unlocked " + itemName(c, i));
                        });
                    };
                }
                rows.push_back(row);
            }
            return rows;
        }, [c]() { return fmt::format("Wallet {}  |  Collection score {}", Eco::money(Eco::cents()), collectionScore()); });
        openPopup(p);
    }

    void openCosmeticShop() {
        auto p = MenuPopup::create("Cosmetic Shop", []() {
            std::vector<Row> rows;
            for (int c = 0; c < NCAT; c++) {
                int own = 0; for (int i = 1; i <= 8; i++) own += Eco::bought(itemId(c, i)) ? 1 : 0;
                rows.push_back({CATS[c].name, std::to_string(own) + "/8 owned", "Browse", [c]() { openCategory(c); }, {255, 255, 255}});
            }
            return rows;
        }, []() { return fmt::format("Collection score: {} (public leaderboard: you are #1 of 1)", collectionScore()); }, {255, 220, 90});
        openPopup(p);
    }
}
