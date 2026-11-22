#pragma once
#include <Geode/Geode.hpp>
#include <functional>

// Which hooks have bindings on which platform (checked against bindings/2.2091 @ 78d5bb4).
// Android links by symbol (all hooked classes are [[link(android)]]), so it has everything.
#if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_ANDROID)
    #define GCU_FULL_BINDINGS 1          // Daily/Gauntlets/Saved buttons and LevelSearchLayer quick-search buttons
#else
    #define GCU_FULL_BINDINGS 0
#endif
#if GCU_FULL_BINDINGS || defined(GEODE_IS_INTEL_MAC)
    #define GCU_WEEKLY_EVENT_BINDINGS 1  // CreatorLayer::onWeeklyLevel / onEventLevel
#else
    #define GCU_WEEKLY_EVENT_BINDINGS 0
#endif
#if defined(GEODE_IS_MACOS)
    #define GCU_MAC_FALLBACKS 1          // gate pages after they open, because their buttons have no mac address
#else
    #define GCU_MAC_FALLBACKS 0
#endif

namespace gcu {
    // Sprite path inside this mod's resources (adds the mod id prefix).
    inline std::string spr(char const* name) { return geode::Mod::get()->expandSpriteName(name); }

    // Leave a screen on the next frame, but only if it is still the live one.
    // A stale layer must never pop the scene stack or the game can exit to the desktop.
    inline void backOutNextFrame(geode::Ref<cocos2d::CCLayer> layer, bool isScene, std::function<void()> after) {
        geode::queueInMainThread([layer, isScene, after]() {
            auto dir = cocos2d::CCDirector::get();
            bool live = layer->getParent() != nullptr && layer->isRunning();
            if (live && isScene) live = dir->getRunningScene() == layer->getParent();
            if (live) layer->keyBackClicked();
            if (after) after();
        });
    }
}
