#include "Ads.hpp"
#include "Rewards.hpp"
#include "Brand.hpp"

namespace gcu {
    namespace {
        struct AdText { char const* brand; char const* line; char const* fine; ccColor3B a, b; };
        const AdText ADS[] = {
            {"GEM HOARD", "99% of gems are fake!", "Download in 4 easy payments", {0, 114, 255}, {0, 198, 255}},
            {"SOCK CLUB PREMIUM", "One sock. Monthly.", "Cancel anytime (you cannot)", {255, 61, 129}, {247, 151, 30}},
            {"WALLET WIZARD", "Consolidate your debt into more debt", "Rates may vary wildly", {17, 153, 142}, {56, 239, 125}},
            {"MEGA SUPER ADS", "Remove ads with this ad", "Now with 40% more ads", {142, 45, 226}, {249, 83, 198}},
            {"CRYPTO CAT COIN", "Your cat. On a blockchain.", "Not financial advice. Barely advice.", {245, 175, 25}, {18, 194, 233}},
        };
    }
    bool adsEnabled() { return Mod::get()->getSettingValue<bool>("enable-ads"); }

    class AdPopup : public Popup {
        AdKind m_kind; std::function<void(bool)> m_done; int m_left = 5; bool m_granted = false;
        CCLabelBMFont* m_timer = nullptr; CCMenuItemSpriteExtra* m_close = nullptr; CCMenuItemSpriteExtra* m_claim = nullptr;

        bool init(AdKind kind, std::string const& why, std::function<void(bool)> done) {
            if (!Popup::init(330.f, 220.f)) return false;
            m_kind = kind; m_done = std::move(done);
            auto const& ad = ADS[rand() % (sizeof(ADS) / sizeof(ADS[0]))];
            this->setTitle(kind == AdKind::Rewarded ? Brand::rewardedAd() : Brand::ad());
            if (kind == AdKind::Forced && m_closeBtn) { m_closeBtn->setVisible(false); m_closeBtn->setEnabled(false); }
            // Geometry-Dash-styled promo panel (outlined, beveled, gold trim), tinted per advertiser; plain gradient if the art is missing.
            CCNode* grad = nullptr;
            if (auto panel = CCSprite::create(spr("gcu-ad-panel.png").c_str())) {
                grad = CCNode::create(); grad->setContentSize({290, 110});
                panel->setColor(ad.a); panel->setScale(290.f / std::max(1.f, panel->getContentWidth()));
                panel->setPosition({145, 55}); grad->addChild(panel, -1);
            } else {
                auto g = CCLayerGradient::create(ccc4(ad.a.r, ad.a.g, ad.a.b, 255), ccc4(ad.b.r, ad.b.g, ad.b.b, 255));
                g->setContentSize({290, 110}); grad = g;
            }
            m_mainLayer->addChildAtPosition(grad, Anchor::Center, ccp(-145, -35), ccp(0, 0), false);
            // procedural "product shots": spinning square and triangle
            auto d = CCDrawNode::create();
            CCPoint sq[4] = {ccp(-18, -18), ccp(18, -18), ccp(18, 18), ccp(-18, 18)};
            d->drawPolygon(sq, 4, ccc4f(1, 1, 1, .9f), 1, ccc4f(0, 0, 0, 1));
            d->runAction(CCRepeatForever::create(CCRotateBy::create(2.f, 360.f)));
            d->setPosition({50, 55}); grad->addChild(d);
            auto brand = CCLabelBMFont::create(ad.brand, "goldFont.fnt"); brand->limitLabelWidth(170, .7f, .2f);
            brand->setPosition({195, 82}); grad->addChild(brand);
            auto line = CCLabelBMFont::create(ad.line, "bigFont.fnt"); line->limitLabelWidth(170, .45f, .15f);
            line->setPosition({195, 55}); grad->addChild(line);
            auto fine = CCLabelBMFont::create(ad.fine, "chatFont.fnt"); fine->limitLabelWidth(170, .6f, .2f);
            fine->setPosition({195, 30}); grad->addChild(fine);
            auto ctx = CCLabelBMFont::create(why.c_str(), "chatFont.fnt"); ctx->setScale(.6f); ctx->setOpacity(170);
            m_mainLayer->addChildAtPosition(ctx, Anchor::Top, ccp(0, -38));
            m_timer = CCLabelBMFont::create("", "goldFont.fnt"); m_timer->setScale(.55f);
            m_mainLayer->addChildAtPosition(m_timer, Anchor::Bottom, ccp(0, 62));

            m_close = CCMenuItemExt::createSpriteExtra(ButtonSprite::create(kind == AdKind::Rewarded ? "Skip (no reward)" : "Close", 120, 0, .6f, false, "goldFont.fnt", "GJ_button_06.png", 25.f),
                [this](CCMenuItemSpriteExtra*) { this->finish(false); });
            m_buttonMenu->addChildAtPosition(m_close, Anchor::Bottom, ccp(kind == AdKind::Rewarded ? -62 : 0, 28));
            if (kind == AdKind::Rewarded) {
                m_claim = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Claim", 100, 0, .6f, false, "goldFont.fnt", "GJ_button_01.png", 25.f),
                    [this](CCMenuItemSpriteExtra*) { this->grant(); });
                m_claim->setEnabled(false); m_claim->setOpacity(80);
                m_buttonMenu->addChildAtPosition(m_claim, Anchor::Bottom, ccp(80, 28));
            } else { m_close->setEnabled(false); m_close->setOpacity(80); }   // forced ads: wait it out
            this->schedule(schedule_selector(AdPopup::tick), 1.f);
            this->tick(0);
            return true;
        }
        void tick(float) {
            if (m_left > 0) m_timer->setString(fmt::format("{} {}s", m_kind == AdKind::Rewarded ? "Reward in" : "Skippable in", m_left).c_str());
            else {
                m_timer->setString(m_kind == AdKind::Rewarded ? "Reward ready!" : "You may now close this ad.");
                if (m_kind == AdKind::Rewarded) { m_claim->setEnabled(true); m_claim->setOpacity(255); }
                else { m_close->setEnabled(true); m_close->setOpacity(255); }
                this->unschedule(schedule_selector(AdPopup::tick));
            }
            m_left--;
        }
        void grant() {
            if (m_granted) return;
            m_granted = true;
            Rewards::onAdReward();
            this->finish(true);
        }
        void finish(bool granted) {
            this->unschedule(schedule_selector(AdPopup::tick));
            Store::add("stat.ads", 1);
            Store::add("quest.ads", 1);
            auto cb = m_done;
            this->onClose(nullptr);
            if (cb) cb(granted);
        }
        void keyBackClicked() override { /* ads cannot be escaped with the back key */ }
    public:
        static AdPopup* create(AdKind k, std::string const& why, std::function<void(bool)> d) {
            auto r = new AdPopup();
            if (r->init(k, why, std::move(d))) { r->autorelease(); return r; }
            delete r; return nullptr;
        }
    };

    void showAd(AdKind kind, std::string const& why, std::function<void(bool)> done) {
        if (kind == AdKind::Forced && (!adsEnabled() || Store::get("secret.noads", 0))) { if (done) done(false); return; }
        if (auto p = AdPopup::create(kind, why, std::move(done))) p->show();
    }

    CCNode* makeBanner() {
        auto root = CCNode::create();
        root->setID(fmt::format("{}/banner", Mod::get()->getID()));
        root->setContentSize({300, 30});
        auto bg = CCLayerGradient::create(ccc4(255, 61, 129, 255), ccc4(91, 44, 255, 255), ccp(1, 0));
        bg->setContentSize({300, 30}); root->addChild(bg);
        auto t = CCLabelBMFont::create("AD: Gems 90% off (of an inflated price)!", "bigFont.fnt");
        t->limitLabelWidth(250, .4f, .15f); t->setPosition({140, 15}); root->addChild(t);
        auto menu = CCMenu::create(); menu->setPosition({0, 0}); root->addChild(menu);
        auto x = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("x", 20, 0, .5f, false, "bigFont.fnt", "GJ_button_06.png", 20.f),
            [](CCMenuItemSpriteExtra*) {
                // closing a banner is itself monetized: it opens a rewarded ad first.
                // The banner is looked up by ID afterwards, so no closure keeps a pointer to it (no dangling node, no retain cycle).
                showAd(AdKind::Rewarded, "Close the banner ad by watching an ad", [](bool ok) {
                    if (!ok) return;
                    Store::set("banner.hide", Store::now() + 600);
                    if (auto scene = CCDirector::get()->getRunningScene())
                        if (auto n = scene->getChildByIDRecursive(fmt::format("{}/banner", Mod::get()->getID()))) n->removeFromParent();
                });
            });
        x->setPosition({284, 15}); menu->addChild(x);
        return root;
    }
}
