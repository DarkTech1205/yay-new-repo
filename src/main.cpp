#include "Screens.hpp"
#include "Ads.hpp"
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

namespace gcu {
    // Main menu: Shop button, status bar, banner ad, hidden RobTop button.
    class $modify(GCUMenu, MenuLayer) {
        struct Fields { CCLabelBMFont* status = nullptr; int taps = 0; int64_t firstTap = 0; };

        bool init() {
            if (!MenuLayer::init()) return false;
            for (auto& m : Eco::tickSubs()) toast(m, false);
            auto win = CCDirector::get()->getWinSize();

            auto menu = CCMenu::create(); menu->setPosition({0, 0}); this->addChild(menu, 50);

            auto shop = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("SHOP", 70, 0, .6f, false, "goldFont.fnt", "GJ_button_01.png", 28.f),
                [](CCMenuItemSpriteExtra*) { openMainShop(); });
            shop->setPosition({46, win.height - 24}); menu->addChild(shop);

            auto subs = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("MOD $100", 80, 0, .55f, false, "goldFont.fnt", "GJ_button_06.png", 28.f),
                [](CCMenuItemSpriteExtra*) { if (Eco::subActive("mod")) openModPanel(); else openSubs(); });
            subs->setPosition({46, win.height - 56}); menu->addChild(subs);

            auto online = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("ONLINE PRICES", 110, 0, .5f, false, "goldFont.fnt", "GJ_button_06.png", 26.f),
                [](CCMenuItemSpriteExtra*) { openOnlineHub(); });
            online->setPosition({62, win.height - 86}); menu->addChild(online);

            if (Mod::get()->getSettingValue<bool>("pack-menu-button")) {
                auto pack = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("2.21 PACK", 90, 0, .5f, false, "goldFont.fnt", "GJ_button_02.png", 26.f),
                    [](CCMenuItemSpriteExtra*) { openPackScreen(); });
                pack->setPosition({52, win.height - 114}); menu->addChild(pack);
            }

            m_fields->status = CCLabelBMFont::create("", "bigFont.fnt");
            m_fields->status->setScale(.4f); m_fields->status->setAnchorPoint({1, .5f});
            m_fields->status->setPosition({win.width - 8, win.height - 12});
            this->addChild(m_fields->status, 50);

            // optional art: each block is skipped if its texture is missing
            if (auto logo = CCSprite::create(spr("gcu-logo.png").c_str())) { logo->setScale(.28f); logo->setPosition({win.width - 70, win.height - 60}); this->addChild(logo, 50); }
            if (auto promo = CCSprite::create(spr("gcu-promo.png").c_str())) { promo->setScale(.22f); promo->setPosition({win.width - 62, 90}); this->addChild(promo, 50); }
            auto gem = artOrFallback(spr("gcu-gem.png").c_str(), 16.f, {255, 79, 163}); gem->setPosition({win.width - 232, win.height - 12}); this->addChild(gem, 50);
            auto en = artOrFallback(spr("gcu-energy.png").c_str(), 16.f, {255, 225, 77}); en->setPosition({win.width - 212, win.height - 12}); this->addChild(en, 50);

            this->schedule(schedule_selector(GCUMenu::updateStatus), 1.f);
            this->updateStatus(0);

            // hidden button: 7 quick taps on the dim square in the bottom-left corner
            auto hidden = CCMenuItemExt::createSpriteExtra(safeFrame("square02_small.png"),
                [this](CCMenuItemSpriteExtra*) {
                    auto now = Store::now();
                    if (now - m_fields->firstTap > 4) { m_fields->taps = 0; m_fields->firstTap = now; }
                    if (++m_fields->taps >= 7) { m_fields->taps = 0; toast("You found something..."); openRobTop(false); }
                });
            hidden->setOpacity(18); hidden->setScale(.6f); hidden->setPosition({14, 14}); menu->addChild(hidden);

            if (adsEnabled() && Store::get("banner.hide", 0) < Store::now()) {
                auto banner = makeBanner(); banner->setPosition({win.width / 2 - 150, 2}); this->addChild(banner, 60);
            }
            // Ubisoft Dash: intro (once per launch), then the acquisition notice (once per launch). Added last, so it covers everything.
            maybeStartUbisoftLaunchSequence(this);
            return true;
        }

        void updateStatus(float) {
            if (!m_fields->status) return;
            m_fields->status->setString(fmt::format("{}  {} gems  energy {}/{}{}", Eco::money(Eco::cents()), Eco::cur("gems"), Eco::energy(), Eco::energyMax(), Eco::subActive("mod") ? "  [MOD]" : "").c_str());
            m_fields->status->limitLabelWidth(215.f, .4f, .15f);   // a billionaire's wallet must not run over the icons
        }

        // Entering the Creator menu ("Online Levels") costs $4.99. The purchase dialog is modal and may
        // finish after this call returns, so hold the layer and sender alive until then.
        void onCreator(CCObject* sender) {
            Ref<MenuLayer> self = this;
            Ref<CCObject> keep = sender;
            // After a successful purchase this simply re-enters the hook, which now lets the call through.
            onlineEntrance([self, keep]() { self->onCreator(keep); });
        }
    };

    // Editor: building and saving are free. Playtesting needs Editor Premium ($5/month).
    class $modify(GCUEditorUI, EditorUI) {
        void onPlaytest(CCObject* sender) {
            if (Eco::subActive("editor")) { EditorUI::onPlaytest(sender); return; }
            Ref<EditorUI> self = this; Ref<CCObject> keep = sender;
            Price p; p.cents = 500;
            confirmPurchase("Editor Premium (monthly): playtesting", p, [self, keep]() {
                Eco::subStart("editor", 500); toast("Editor Premium active");
                self->onPlaytest(keep);   // re-enters the hook, which now sees the active subscription
            });
        }
    };
}
