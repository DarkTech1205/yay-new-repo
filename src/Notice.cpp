#include "Screens.hpp"

// "UBISOFT HAS ACQUIRED GEOMETRY DASH!" - a deliberately cheap mobile-game announcement popup.
// Satire: this is a parody mod; no real acquisition, no real offers, no real money.
namespace gcu {
    class UbiNotice : public Popup {
        bool init() {
            if (!Popup::init(400.f, 270.f)) return false;
            this->setTitle("UBISOFT ENTERTAINMENT  |  CORPORATE COMMUNICATIONS");
            if (auto t = this->getChildByIDRecursive("title-label")) t->setScale(t->getScale() * .62f);   // long corporate title

            auto head = CCLabelBMFont::create("Ubisoft Announces Strategic Acquisition\nof Geometry Dash and RobTop Games", "goldFont.fnt");
            head->setAlignment(kCCTextAlignmentCenter);
            head->limitLabelWidth(360.f, .62f, .2f);
            m_mainLayer->addChildAtPosition(head, Anchor::Top, ccp(0, -52));

            // Press-release body. Parody: no acquisition has occurred; the "spokesperson" does not exist.
            auto body = CCLabelBMFont::create(
                "PARIS - Ubisoft today announced the strategic acquisition of RobTop Games, expanding its entertainment portfolio with one of the industry's most engaging rhythm platformers. The transaction creates long-term development opportunities, enhanced player engagement and significant monetization opportunities across the franchise.\n\n"
                "Geometry Dash will be integrated into Ubisoft's ecosystem, with future content initiatives, premium experiences and a refreshed Credits-based economy to be announced in due course.\n\n"
                "\"We are excited to unlock synergies that maximize player value,\" said a Ubisoft spokesperson.",
                "chatFont.fnt", 590.f, kCCTextAlignmentLeft);
            body->setScale(.6f);
            m_mainLayer->addChildAtPosition(body, Anchor::Center, ccp(0, -2));

            auto fine = CCLabelBMFont::create("Forward-looking statements. This is a parody: no acquisition has occurred and no real money is involved.", "chatFont.fnt");
            fine->limitLabelWidth(370.f, .45f, .2f); fine->setOpacity(150);
            m_mainLayer->addChildAtPosition(fine, Anchor::Bottom, ccp(0, 52));

            auto ok = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Acknowledge", 130, 0, .7f, false, "goldFont.fnt", "GJ_button_01.png", 30.f),
                [this](CCMenuItemSpriteExtra*) { this->onClose(nullptr); });
            m_buttonMenu->addChildAtPosition(ok, Anchor::Bottom, ccp(0, 26));
            return true;
        }
    public:
        static UbiNotice* create() {
            auto r = new UbiNotice();
            if (r->init()) { r->autorelease(); return r; }
            delete r; return nullptr;
        }
    };

    void showUbisoftNotice() {
        static bool shownThisLaunch = false;
        if (shownThisLaunch) return;
        if (!Mod::get()->getSettingValue<bool>("enable-ubisoft-notice")) return;
        shownThisLaunch = true;
        if (auto p = UbiNotice::create()) p->show();
    }
}
