#pragma once
#include "UI.hpp"

namespace gcu {
    enum class AdKind { Rewarded, Forced };
    // done(true) if a rewarded ad was watched to the end and the reward was granted.
    void showAd(AdKind kind, std::string const& why, std::function<void(bool)> done = nullptr);
    // Small dismissible banner (only when banner ads are enabled).
    CCNode* makeBanner();
    bool adsEnabled();
}
