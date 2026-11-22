#include "Screens.hpp"
#include "Brand.hpp"
#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/binding/LevelInfoLayer.hpp>
#include <Geode/binding/MusicDownloadManager.hpp>
#include <Geode/utils/file.hpp>
#include <filesystem>
#include <fstream>
#include <optional>

// Geometry Dash 2.21 Pack: two Demon levels that are SOLD SEPARATELY (independent ownership flags, independent purchases).
// Level data lives in resources/gcu-pack-<key>.json (the supplied .gdlvl files, decoded, with the 3 user coins of each level
// removed and nothing else touched). Playing hands the level to Geometry Dash's own level page (LevelInfoLayer), so creator,
// song, difficulty and the Play button all come from the game's native UI.
namespace gcu {
    namespace {
        struct PackLevel {
            char const* key;        // ownership flag: bought("pack221.<key>")
            char const* title;      // as stored in the supplied file
            char const* creator;    // creator credit
            char const* file;       // resources/<file>
            char const* icon;       // sprite
            int songID;
            int64_t priceCents;     // each level has its own price
            int fakeID;             // negative id: can never collide with a real online level
        };
        constexpr PackLevel LEVELS[2] = {
            {"explorers", "Explorers 2", "Mathiscreator", "gcu-pack-explorers.json", "gcu-lvl-explorers.png", 1183462, 999, -2210001},
            {"firebird",  "Firebird",    "seannnn",       "gcu-pack-firebird.json",  "gcu-lvl-firebird.png",  10007217, 999, -2210002},
        };
        std::string ownKey(int i) { return std::string("pack221.") + LEVELS[i].key; }
        Ref<GJGameLevel> g_levels[2];   // cached so repeated plays do not rebuild (or leak) 4+ MB of level text

        // The packager may or may not keep the "resources/" prefix for raw files: try both.
        std::optional<std::string> readResourceText(char const* name) {
            auto dir = Mod::get()->getResourcesDir();
            for (auto const& p : { dir / name, dir / "resources" / name }) {
                auto r = utils::file::readString(p);
                if (r.isOk()) return r.unwrap();
            }
            return std::nullopt;
        }
        std::optional<std::vector<uint8_t>> readResourceBytes(std::string const& name) {
            auto dir = Mod::get()->getResourcesDir();
            for (auto const& p : { dir / name, dir / "resources" / name }) {
                auto r = utils::file::readBinary(p);
                if (r.isOk()) return r.unwrap();
            }
            return std::nullopt;
        }

        // Copies the supplied song into GD's song folder the first time (never overwrites a song the player already has).
        void installSong(int id) {
            try {
                auto dest = MusicDownloadManager::sharedState()->pathForSong(id);
                if (dest.empty() || CCFileUtils::get()->isFileExist(dest)) return;
                auto bytes = readResourceBytes(fmt::format("gcu-song-{}.mp3", id));
                if (!bytes || bytes->empty()) return;
                std::error_code ec;
                std::filesystem::create_directories(std::filesystem::path(dest).parent_path(), ec);
                std::ofstream out(dest, std::ios::binary);
                if (out) out.write(reinterpret_cast<char const*>(bytes->data()), (std::streamsize)bytes->size());
            } catch (...) {}   // no song is better than a crash: the level then plays with GD's default music
        }

        GJGameLevel* getLevel(int i) {
            if (g_levels[i]) return g_levels[i];
            auto const& P = LEVELS[i];
            auto text = readResourceText(P.file);
            if (!text) return nullptr;
            auto parsed = matjson::parse(*text);
            if (parsed.isErr()) return nullptr;
            auto j = parsed.unwrap();
            auto str = j["levelString"].asString().unwrapOr("");
            if (str.size() < 100) return nullptr;
            auto lv = GJGameLevel::create();
            if (!lv) return nullptr;
            lv->m_levelString = str;                                   // level data, byte for byte as shipped
            lv->m_levelName = j["name"].asString().unwrapOr(P.title);
            lv->m_creatorName = P.creator;                             // creator credit
            lv->m_levelLength = (int)j["length"].asInt().unwrapOr(3);
            lv->m_songID = P.songID; lv->m_audioTrack = 0;
            lv->m_songIDs = j["songIDs"].asString().unwrapOr("");
            lv->m_sfxIDs = j["sfxIDs"].asString().unwrapOr("");
            lv->m_stars = 10; lv->m_demon = 1; lv->m_demonDifficulty = 0;   // Demon
            lv->m_difficulty = GJDifficulty::Demon;
            lv->m_isVerified = true;
            lv->m_coins = 0;                                           // user coins were removed from the level
            lv->m_levelType = GJLevelType::Saved;
            lv->m_levelID = P.fakeID;
            g_levels[i] = lv;
            return lv;
        }

        void playLevel(int i) {
            auto lv = getLevel(i);
            if (!lv) { toast("This level's data could not be loaded. Reinstall the mod (resources/gcu-pack-*.json).", false); return; }
            installSong(LEVELS[i].songID);
            CCDirector::get()->pushScene(CCTransitionFade::create(.5f, LevelInfoLayer::scene(lv, false)));
        }

        CCScale9Sprite* card(float w, float h, GLubyte op = 90) {
            auto s = CCScale9Sprite::create("square02b_001.png", {0, 0, 80, 80});
            s->setContentSize({w, h}); s->setColor({0, 0, 0}); s->setOpacity(op);
            return s;
        }
    }

    bool isPackLevel(GJGameLevel* l) { return l && l->m_levelID.value() <= -2210001 && l->m_levelID.value() >= -2210002; }

    class PackPopup : public Popup {
        CCNode* m_content = nullptr;
        bool init() {
            if (!Popup::init(440.f, 290.f)) return false;
            this->setTitle("Geometry Dash 2.21 Pack");
            this->rebuild();
            return true;
        }
        void rebuild() {
            if (m_content) m_content->removeFromParent();
            m_content = CCNode::create();
            m_content->setContentSize(m_mainLayer->getContentSize());
            m_mainLayer->addChild(m_content);
            auto menu = CCMenu::create(); menu->setPosition({0, 0}); m_content->addChild(menu);
            auto at = [&](CCNode* n, float x, float y) { n->setPosition({m_size.width / 2 + x, m_size.height / 2 + y}); };

            // artwork + pack info
            auto art = artOrFallback(spr("gcu-pack-221.png").c_str(), 104.f, {120, 180, 255});
            at(art, -155, 38); m_content->addChild(art);
            auto name = CCLabelBMFont::create("2.21 PACK", "goldFont.fnt"); name->limitLabelWidth(110, .65f, .2f); at(name, -155, -32); m_content->addChild(name);
            auto sub = CCLabelBMFont::create("2 Demon levels\nSOLD SEPARATELY", "chatFont.fnt"); sub->setAlignment(kCCTextAlignmentCenter); sub->setScale(.55f);
            at(sub, -155, -56); m_content->addChild(sub);

            for (int i = 0; i < 2; i++) {
                float y = 62.f - i * 82.f;
                auto const& P = LEVELS[i];
                bool owned = Eco::bought(ownKey(i));
                auto bg = card(262.f, 72.f); at(bg, 55, y); m_content->addChild(bg);
                auto icon = artOrFallback(spr(P.icon).c_str(), 52.f, {255, 90, 90}); at(icon, -62, y); m_content->addChild(icon);
                auto nm = CCLabelBMFont::create(P.title, "bigFont.fnt"); nm->setAnchorPoint({0, .5f}); nm->limitLabelWidth(120, .55f, .2f); at(nm, -32, y + 22); m_content->addChild(nm);
                auto by = CCLabelBMFont::create(fmt::format("by {}", P.creator).c_str(), "goldFont.fnt"); by->setAnchorPoint({0, .5f}); by->limitLabelWidth(120, .4f, .15f); at(by, -32, y + 4); m_content->addChild(by);
                auto dm = artOrFallback(spr("gcu-demon.png").c_str(), 18.f, {200, 40, 40}); at(dm, -24, y - 16); m_content->addChild(dm);
                auto dl = CCLabelBMFont::create(fmt::format("Demon  10*  pays {}", Eco::money(rewardCents(Tier::HardDemon))).c_str(), "chatFont.fnt");
                dl->setAnchorPoint({0, .5f}); dl->setScale(.5f); at(dl, -12, y - 16); m_content->addChild(dl);

                if (owned) {
                    auto own = CCLabelBMFont::create("OWNED", "goldFont.fnt"); own->setScale(.45f); own->setColor({120, 255, 140}); at(own, 150, y + 18); m_content->addChild(own);
                    auto play = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Play", 70, 0, .6f, false, "goldFont.fnt", "GJ_button_01.png", 28.f),
                        [i](CCMenuItemSpriteExtra*) { playLevel(i); });
                    at(play, 150, y - 8); menu->addChild(play);
                } else {
                    auto pr = CCLabelBMFont::create(Eco::money(P.priceCents).c_str(), "goldFont.fnt"); pr->setScale(.55f); at(pr, 150, y + 18); m_content->addChild(pr);
                    Ref<PackPopup> self = this;
                    auto buy = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Purchase", 78, 0, .55f, false, "goldFont.fnt", "GJ_button_02.png", 28.f),
                        [self, i](CCMenuItemSpriteExtra*) {
                            if (Eco::bought(ownKey(i))) { toast("You already own this level."); self->rebuild(); return; }   // never charge twice
                            Price price; price.cents = LEVELS[i].priceCents;
                            confirmPurchase(LEVELS[i].title, price, [self, i]() {
                                if (Eco::bought(ownKey(i))) return;
                                Eco::markBought(ownKey(i));                 // only THIS level; the other one stays locked
                                toast(fmt::format("{} acquired!", LEVELS[i].title));
                                self->rebuild();
                            });
                        });
                    at(buy, 150, y - 8); menu->addChild(buy);
                }
            }
            auto foot = CCLabelBMFont::create("Each level is sold separately. Buying one does not unlock the other.", "chatFont.fnt");
            foot->limitLabelWidth(380, .5f, .2f); foot->setOpacity(170); at(foot, 0, -118); m_content->addChild(foot);
        }
    public:
        static PackPopup* create() {
            auto r = new PackPopup();
            if (r->init()) { r->autorelease(); return r; }
            delete r; return nullptr;
        }
    };

    void openPackScreen() { if (auto p = PackPopup::create()) p->show(); }
}
