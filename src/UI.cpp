#include "UI.hpp"
#include "Brand.hpp"
#include <Geode/ui/Notification.hpp>

namespace gcu {
    CCNode* artOrFallback(char const* frame, float size, ccColor3B col) {
        if (auto s = CCSprite::create(frame)) {
            s->setScale(size / std::max(1.f, s->getContentSize().width));
            return s;
        }
        auto d = CCDrawNode::create();
        CCPoint sq[4] = {ccp(0, 0), ccp(size, 0), ccp(size, size), ccp(0, size)};
        d->drawPolygon(sq, 4, ccc4f(col.r / 255.f, col.g / 255.f, col.b / 255.f, 1.f), 2.f, ccc4f(1, 1, 1, 1));
        d->setContentSize({size, size});
        d->setAnchorPoint({.5f, .5f});
        d->ignoreAnchorPointForPosition(false);
        return d;
    }

    CCSprite* safeFrame(char const* name, float sz) {
        if (auto s = CCSprite::createWithSpriteFrameName(name)) return s;
        auto b = CCSprite::create();
        b->setContentSize({sz, sz});
        return b;
    }

    void toast(std::string const& msg, bool good) {
        Notification::create(msg, good ? NotificationIcon::Success : NotificationIcon::Error, 2.2f)->show();
    }

    void shake(CCNode* n) {
        if (!n) return;
        n->runAction(CCSequence::create(CCMoveBy::create(.04f, {8, 0}), CCMoveBy::create(.04f, {-16, 0}),
            CCMoveBy::create(.04f, {16, 0}), CCMoveBy::create(.04f, {-16, 0}), CCMoveBy::create(.04f, {8, 0}), nullptr));
    }

    void cashRain(CCNode* parent, std::string const& text) {
        if (!parent) return;
        auto win = CCDirector::get()->getWinSize();
        auto layer = CCNode::create();
        parent->addChild(layer, 9999);
        for (int i = 0; i < 28; i++) {
            auto lbl = CCLabelBMFont::create("$", "goldFont.fnt");
            lbl->setScale(.5f + (rand() % 60) / 100.f);
            lbl->setPosition({(float)(rand() % (int)win.width), win.height + 20.f});
            lbl->setRotation((float)(rand() % 60 - 30));
            lbl->runAction(CCSequence::create(CCDelayTime::create((rand() % 60) / 100.f),
                CCSpawn::create(CCMoveBy::create(1.1f, {(float)(rand() % 80 - 40), -win.height - 60.f}), CCFadeOut::create(1.1f), nullptr),
                CCCallFunc::create(lbl, callfunc_selector(CCNode::removeFromParent)), nullptr));
            layer->addChild(lbl);
        }
        auto big = CCLabelBMFont::create(text.c_str(), "goldFont.fnt");
        big->setPosition({win.width / 2, win.height / 2 + 40});
        big->setScale(.2f);
        big->runAction(CCSequence::create(CCEaseBackOut::create(CCScaleTo::create(.35f, 1.1f)), CCDelayTime::create(1.2f), CCFadeOut::create(.4f), nullptr));
        layer->addChild(big);
        layer->runAction(CCSequence::create(CCDelayTime::create(2.6f), CCCallFunc::create(layer, callfunc_selector(CCNode::removeFromParent)), nullptr));
    }

    // ---------- purchase confirmation ----------
    class ConfirmPopup : public Popup {
        std::string m_name; Price m_price; std::function<void()> m_ok;
        bool init(std::string const& name, Price const& price, std::function<void()> ok) {
            if (!Popup::init(300.f, 190.f)) return false;
            m_name = name; m_price = price; m_ok = std::move(ok);
            this->setTitle(Brand::acquire());
            auto item = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
            item->limitLabelWidth(260.f, .6f, .2f);
            m_mainLayer->addChildAtPosition(item, Anchor::Center, ccp(0, 45));
            auto pr = CCLabelBMFont::create(("Price: " + price.str()).c_str(), "goldFont.fnt");
            pr->limitLabelWidth(260.f, .7f, .2f);
            m_mainLayer->addChildAtPosition(pr, Anchor::Center, ccp(0, 15));
            auto w = CCLabelBMFont::create(("Wallet: " + Eco::money(Eco::cents()) + "  |  " + std::to_string(Eco::cur("gems")) + " gems").c_str(), "chatFont.fnt");
            w->setScale(.8f);
            m_mainLayer->addChildAtPosition(w, Anchor::Center, ccp(0, -10));
            auto cancel = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Cancel", "goldFont.fnt", "GJ_button_06.png", .8f),
                [this](CCMenuItemSpriteExtra*) { this->onClose(nullptr); });
            auto buy = CCMenuItemExt::createSpriteExtra(ButtonSprite::create("Purchase", "goldFont.fnt", "GJ_button_01.png", .8f),
                [this](CCMenuItemSpriteExtra*) {
                    // Wallet is checked at the moment of confirmation, never earlier.
                    if (!Eco::canAfford(m_price)) {
                        toast("Not enough funds. Missing: " + Eco::missing(m_price), false);
                        shake(m_mainLayer); return;
                    }
                    Eco::charge(m_price);
                    auto cb = m_ok;
                    this->onClose(nullptr);
                    if (cb) cb();
                });
            m_buttonMenu->addChildAtPosition(cancel, Anchor::Bottom, ccp(-62, 28));
            m_buttonMenu->addChildAtPosition(buy, Anchor::Bottom, ccp(62, 28));
            return true;
        }
    public:
        static ConfirmPopup* create(std::string const& n, Price const& p, std::function<void()> ok) {
            auto r = new ConfirmPopup();
            if (r->init(n, p, std::move(ok))) { r->autorelease(); return r; }
            delete r; return nullptr;
        }
    };

    void confirmPurchase(std::string const& name, Price const& price, std::function<void()> onSuccess) {
        if (auto p = ConfirmPopup::create(name, price, std::move(onSuccess))) p->show();
    }

    // ---------- generic paged list ----------
    bool MenuPopup::init(std::string const& title, std::function<std::vector<Row>()> build, std::function<std::string()> header, ccColor3B tint) {
        if (!Popup::init(380.f, 270.f)) return false;
        m_build = std::move(build); m_header = std::move(header);
        this->setTitle(title);
        {   // themed procedural art (see resources/), faint behind the rows; skipped if the texture is missing
            auto has = [&](char const* k) { return title.find(k) != std::string::npos; };
            std::string art = "gcu-shop-bg.png";
            if (has("SECRET") || has("ROB")) art = "gcu-robtop-bg.png";
            else if (has("MODERATOR")) art = "gcu-mod-bg.png";
            else if (has("GeoPass")) art = "gcu-pass-bg.png";
            else if (has("Treasure")) art = "gcu-treasure-bg.png";
            if (auto bg = CCSprite::create(spr(art.c_str()).c_str())) {
                if (bg->getContentWidth() > 1.f && bg->getContentHeight() > 1.f) {
                    bg->setScale(std::max((m_size.width - 16) / bg->getContentWidth(), (m_size.height - 16) / bg->getContentHeight()));
                    bg->setOpacity(70);
                    m_mainLayer->addChildAtPosition(bg, Anchor::Center, ccp(0, 0), false);
                    bg->setZOrder(-1);
                }
            }
        }
        if (m_header) {
            m_headerLabel = CCLabelBMFont::create("", "chatFont.fnt");
            m_headerLabel->setColor(tint);
            m_mainLayer->addChildAtPosition(m_headerLabel, Anchor::Top, ccp(0, -40));
        }
        auto prev = CCMenuItemSpriteExtra::create(safeFrame("GJ_arrow_01_001.png"), this, menu_selector(MenuPopup::onPrev));
        auto next = safeFrame("GJ_arrow_01_001.png"); next->setFlipX(true);
        auto nextB = CCMenuItemSpriteExtra::create(next, this, menu_selector(MenuPopup::onNext));
        prev->setScale(.7f); nextB->setScale(.7f);
        m_buttonMenu->addChildAtPosition(prev, Anchor::Bottom, ccp(-60, 20));
        m_buttonMenu->addChildAtPosition(nextB, Anchor::Bottom, ccp(60, 20));
        this->refresh();
        this->schedule(schedule_selector(MenuPopup::poll), 0.5f);
        return true;
    }
    void MenuPopup::onPrev(CCObject*) { if (m_page > 0) { m_page--; refresh(); } }
    void MenuPopup::onNext(CCObject*) { m_page++; refresh(); }

    void MenuPopup::poll(float) {
        if (m_headerLabel && m_header) m_headerLabel->setString(m_header().c_str());
        if (m_seenVersion != Store::version()) refresh();   // only redraw when something actually changed
    }

    void MenuPopup::refresh() {
        m_seenVersion = Store::version();
        if (m_content) { m_content->removeFromParent(); m_content = nullptr; }
        if (m_headerLabel && m_header) m_headerLabel->setString(m_header().c_str());
        auto rows = m_build();
        constexpr int PER = 5;
        int pages = std::max(1, ((int)rows.size() + PER - 1) / PER);
        m_page = std::clamp(m_page, 0, pages - 1);
        m_content = CCNode::create();
        m_content->setContentSize(m_size);
        m_content->setAnchorPoint({.5f, .5f});
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        m_content->addChild(menu);
        for (int i = 0; i < PER; i++) {
            int idx = m_page * PER + i;
            if (idx >= (int)rows.size()) break;
            auto& r = rows[idx];
            float y = m_size.height - 78.f - i * 34.f;
            auto lbl = CCLabelBMFont::create(r.text.c_str(), "bigFont.fnt");
            lbl->setColor(r.color); lbl->setAnchorPoint({0, .5f});
            lbl->limitLabelWidth(205.f, .45f, .15f);
            lbl->setPosition({18, y + (r.sub.empty() ? 0.f : 7.f)});
            m_content->addChild(lbl);
            if (!r.sub.empty()) {
                auto s = CCLabelBMFont::create(r.sub.c_str(), "chatFont.fnt");
                s->setAnchorPoint({0, .5f}); s->setScale(.7f); s->setOpacity(190);
                s->setPosition({18, y - 8.f}); m_content->addChild(s);
            }
            if (!r.btn.empty()) {
                auto fn = r.onClick;
                auto b = CCMenuItemExt::createSpriteExtra(ButtonSprite::create(r.btn.c_str(), 90, 0, .55f, false, "goldFont.fnt", "GJ_button_01.png", 22.f),
                    [fn](CCMenuItemSpriteExtra*) { if (fn) fn(); });
                b->setPosition({m_size.width - 62.f, y});
                menu->addChild(b);
            }
        }
        auto pg = CCLabelBMFont::create(fmt::format("{}/{}", m_page + 1, pages).c_str(), "goldFont.fnt");
        pg->setScale(.45f); pg->setPosition({m_size.width / 2, 20});
        m_content->addChild(pg);
        m_mainLayer->addChildAtPosition(m_content, Anchor::Center);
    }

    MenuPopup* MenuPopup::create(std::string const& title, std::function<std::vector<Row>()> build, std::function<std::string()> header, ccColor3B tint) {
        auto r = new MenuPopup();
        if (r->init(title, std::move(build), std::move(header), tint)) { r->autorelease(); return r; }
        delete r; return nullptr;
    }
    void openPopup(MenuPopup* p) { if (p) p->show(); }
}
