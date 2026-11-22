#include "Screens.hpp"
#include <Geode/loader/SettingV3.hpp>

// Mod settings buttons ("button" setting type, mod.json -> settings). Both act on THIS mod's saved state only.
// Geometry Dash's own player data, accounts, level progress, icons, achievements and settings are never touched.
namespace gcu {
    static void resetModData() {
        createQuickPopup(
            "RESET MOD DATA?",
            "This erases ALL data of this mod: wallet, gems, purchases, subscriptions, pass, stats and every other saved value.\n\n"
            "Your real Geometry Dash account, progress, icons, achievements and settings are <cg>NOT</c> touched.\n\n"
            "This cannot be undone. Ubisoft will miss your money.",
            "Cancel", "RESET",
            [](FLAlertLayer*, bool confirmed) {
                if (!confirmed) return;
                Eco::resetModData();
                (void)Mod::get()->saveData();
                toast("Mod data reset. Welcome back, new customer.");
                FLAlertLayer::create(nullptr, "MOD DATA RESET",
                    "All mod data has been reset to factory defaults.\nGeometry Dash itself was not touched.", "OK", nullptr)->show();
            });
    }

    static void giveAllMoney() {
        Eco::giveAllMoney();                          // wallet := 2147483647 dollars via the normal Store path (persisted below)
        (void)Mod::get()->saveData();                  // write immediately so it survives a crash or a killed app
        if (auto scene = CCDirector::get()->getRunningScene()) cashRain(scene, "$");
        FLAlertLayer::create(nullptr, "!!! JACKPOT !!!",
            fmt::format("CONGRATULATIONS! YOU ARE NOW A BILLIONAIRE!!!\nUBISOFT IS PROUD OF YOU!\n\nWallet: {}", Eco::money(Eco::cents())),
            "AMAZING!!!", nullptr)->show();
    }

    $on_mod(Loaded) {
        ButtonSettingPressedEventV3(Mod::get(), "reset-mod-data").listen([](std::string_view) { resetModData(); }).leak();
        ButtonSettingPressedEventV3(Mod::get(), "give-all-money").listen([](std::string_view) { giveAllMoney(); }).leak();
    }
}
