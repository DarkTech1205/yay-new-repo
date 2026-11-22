#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <functional>
#include "Economy.hpp"
#include "Platform.hpp"

namespace gcu {
    using namespace geode::prelude;

    // Sprite from the mod's resources, or a procedural fallback if the texture is missing.
    CCNode* artOrFallback(char const* file, float size, ccColor3B col);
    // Sprite frame from GD's own sheets; never returns null (a blank box is used if the frame is missing).
    CCSprite* safeFrame(char const* frameName, float fallbackSize = 30.f);
    void toast(std::string const& msg, bool good = true);
    void shake(CCNode* n);
    void cashRain(CCNode* parent, std::string const& text);

    // 1) Price shown  2) confirmation  3) wallet checked  4) charged  5) unlock  6) caller saves.
    void confirmPurchase(std::string const& name, Price const& price, std::function<void()> onSuccess);

    struct Row {
        std::string text, sub, btn;
        std::function<void()> onClick;
        ccColor3B color = {255, 255, 255};
    };

    // Generic paged list popup used by every custom screen.
    class MenuPopup : public Popup {
    protected:
        std::function<std::vector<Row>()> m_build;
        std::function<std::string()> m_header;
        std::function<void(MenuPopup*)> m_footerBuilder;
        CCNode* m_content = nullptr;
        CCLabelBMFont* m_headerLabel = nullptr;
        int m_page = 0;
        uint64_t m_seenVersion = 0;
        void poll(float);
        bool init(std::string const& title, std::function<std::vector<Row>()> build, std::function<std::string()> header, ccColor3B tint);
        void onPrev(CCObject*);
        void onNext(CCObject*);
    public:
        static MenuPopup* create(std::string const& title, std::function<std::vector<Row>()> build,
                                 std::function<std::string()> header = nullptr, ccColor3B tint = {255, 255, 255});
        void refresh();
    };

    void openPopup(MenuPopup* p);
}
