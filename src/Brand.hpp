#pragma once
#include <Geode/Geode.hpp>

// Corporate terminology (setting "ubisoft-branding"). Only the labels change, never the behavior or the saved data.
// Familiar terms stay where they help usability (buttons still say "Purchase", prices still use "$").
namespace gcu::Brand {
    inline bool on() { return Mod::get()->getSettingValue<bool>("ubisoft-branding"); }
    inline char const* wallet()  { return on() ? "Ubisoft Credits" : "Wallet"; }
    inline char const* energy()  { return on() ? "Player Engagement Energy" : "Energy"; }
    inline char const* reward()  { return on() ? "Engagement Incentive" : "Reward"; }
    inline char const* ad()      { return on() ? "Promotional Opportunity" : "Advertisement"; }
    inline char const* rewardedAd() { return on() ? "Rewarded Promotional Opportunity" : "Rewarded Advertisement"; }
    inline char const* acquire() { return on() ? "Confirm Content Acquisition" : "Confirm Purchase"; }
    inline char const* daily()   { return on() ? "Daily Engagement Rewards" : "Daily Objectives"; }
}
