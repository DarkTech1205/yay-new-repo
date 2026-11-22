#include "Screens.hpp"

// Ubisoft intro: Cocos2d-x has no video player, so the supplied MP4 was converted at build time into 88 frames
// (resources/gcu-intro-001..088.png, 15 fps) plus resources/gcu-intro.ogg. Playback swaps one sprite per frame.
// Every failure path (missing frames, missing audio, bad setting, null engine) falls through to the normal menu.
namespace gcu {
    namespace {
        constexpr int FRAMES = 88;
        constexpr float FPS = 15.f;
        bool g_started = false;   // once per game launch: going back to the main menu never replays it

        std::string frameName(int i) { return fmt::format("gcu-intro-{:03}.png", i); }

        class UbiIntro : public CCLayerColor {
            std::function<void()> m_done;
            CCSprite* m_frame = nullptr;
            float m_t = 0; int m_shown = 0; bool m_finished = false, m_audio = false;

            bool init(std::function<void()> done) {
                if (!CCLayerColor::initWithColor({0, 0, 0, 255})) return false;
                m_done = std::move(done);
                this->setTouchPriority(-1000);          // above every menu button: nothing behind can be pressed
                this->setTouchMode(kCCTouchesOneByOne);
                this->setTouchEnabled(true);
                this->setKeypadEnabled(true);
                auto win = CCDirector::get()->getWinSize();
                auto hint = CCLabelBMFont::create("Tap to skip", "chatFont.fnt");
                hint->setScale(.6f); hint->setOpacity(0); hint->setAnchorPoint({1, 0});
                hint->setPosition({win.width - 8, 6});
                hint->runAction(CCSequence::create(CCDelayTime::create(1.f), CCFadeTo::create(.4f, 110), nullptr));
                this->addChild(hint, 5);
                this->showFrame(1);
                if (!m_frame) return false;             // first frame missing/unloadable: caller falls back to the normal menu
                this->startAudio();
                this->scheduleUpdate();
                return true;
            }
            void startAudio() {
                // Audio is optional: if the file or the engine is unavailable the intro just plays silently.
                try {
                    auto fu = CCFileUtils::get();
                    auto fm = FMODAudioEngine::get();
                    if (!fu || !fm) return;
                    // The packager may or may not keep the "resources/" prefix for raw files: try both, use whichever exists.
                    auto dir = Mod::get()->getResourcesDir();
                    std::string path;
                    for (auto& cand : { fmt::format("{}", dir / "gcu-intro.ogg"), fmt::format("{}", dir / "resources" / "gcu-intro.ogg") }) {
                        if (fu->isFileExist(cand.c_str())) { path = cand; break; }
                    }
                    if (path.empty()) return;           // no audio: silent intro
                    fm->pauseMusic(0);                  // hush the menu loop; resumed in finish()
                    fm->playEffect(path);
                    m_audio = true;
                } catch (...) {}
            }
            void showFrame(int i) {
                auto path = spr(frameName(i).c_str());
                auto next = CCSprite::create(path.c_str());
                if (!next) return;                     // keep holding the previous frame
                auto win = CCDirector::get()->getWinSize();
                auto cs = next->getContentSize();
                if (cs.width <= 0 || cs.height <= 0) return;
                next->setScale(std::min(win.width / cs.width, win.height / cs.height));
                next->setPosition({win.width / 2, win.height / 2});
                this->addChild(next, 1);
                if (m_frame) {                         // free the previous frame's texture; ~30 MB would pile up otherwise
                    auto tex = m_frame->getTexture();
                    m_frame->removeFromParent();
                    if (tex) CCTextureCache::get()->removeTexture(tex);
                }
                m_frame = next;
            }
            void finish() {
                if (m_finished) return;
                m_finished = true;
                this->unscheduleUpdate();
                try {
                    if (m_audio) { if (auto fm = FMODAudioEngine::get()) { fm->stopAllEffects(); fm->resumeMusic(0); } }
                } catch (...) {}
                if (m_frame) {
                    auto tex = m_frame->getTexture();
                    m_frame->removeFromParent(); m_frame = nullptr;
                    if (tex) CCTextureCache::get()->removeTexture(tex);
                }
                auto cb = std::move(m_done);
                this->runAction(CCSequence::create(CCFadeOut::create(.25f), CCCallFunc::create(this, callfunc_selector(UbiIntro::remove)), nullptr));
                this->setTouchEnabled(false);
                if (cb) cb();
            }
            void remove() { this->removeFromParent(); }
        public:
            void update(float dt) override {
                if (m_finished) return;
                m_t += std::min(dt, .1f);               // a hitch must not skip a big chunk of the intro
                int want = (int)(m_t * FPS) + 1;
                if (want > FRAMES) { this->finish(); return; }
                if (want != m_shown) { m_shown = want; this->showFrame(want); }
            }
            bool ccTouchBegan(CCTouch*, CCEvent*) override { if (m_t > .3f) this->finish(); return true; }
            void keyBackClicked() override { this->finish(); }
            static UbiIntro* create(std::function<void()> done) {
                auto r = new UbiIntro();
                if (r->init(std::move(done))) { r->autorelease(); return r; }
                delete r; return nullptr;
            }
        };
    }

    // Called from MenuLayer::init. Runs the intro the first time only, then the acquisition notice the first time only.
    void maybeStartUbisoftLaunchSequence(CCNode* menu) {
        if (!menu) return;
        if (g_started) return;                         // later visits to the main menu: nothing to do
        g_started = true;

        bool intro = Mod::get()->getSettingValue<bool>("enable-intro");
        if (intro) {
            if (auto layer = UbiIntro::create([]() { showUbisoftNotice(); })) {
                menu->addChild(layer, 100000);
                return;
            }
        }
        // intro disabled / files missing / creation failed: go straight to the notice (which has its own setting)
        Ref<CCNode> keep = menu;
        queueInMainThread([keep]() { showUbisoftNotice(); });
    }
}
