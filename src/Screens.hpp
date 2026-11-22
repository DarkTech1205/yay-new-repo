#pragma once
#include "UI.hpp"
// Entry points for every custom screen. Each opens a MenuPopup built from live Store data.
namespace gcu {
    void openMainShop();        // hub: cosmetics, store, exchange, pass, quests, treasure, creator, subs, wallet
    void openCosmeticShop();
    void openStore();           // bundles
    void openExchange();
    void openPass();
    void openQuests();
    void openTreasure();
    void openCreator();
    void openSubs();            // moderator membership, editor premium, pass premium
    void openWallet();
    void openOnlineHub();       // feature list for Online Levels
    void openModPanel();
    void openRobTop(bool deeper);
    void onlineEntrance(std::function<void()> proceed);
    int  collectionScore();
    void grantRobTopCosmetic(int idx);

    // Ubisoft Dash update
    void maybeStartUbisoftLaunchSequence(CCNode* menu);  // intro (once per launch) -> acquisition notice (once per launch)
    void showUbisoftNotice();
    void openPackScreen();                               // Geometry Dash 2.21 Pack: two Demon levels, sold separately
    bool isPackLevel(GJGameLevel* l);                            // the acquisition announcement popup
}
