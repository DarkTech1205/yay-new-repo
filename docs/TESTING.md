# Test results (honest)

## Passed (run here)
* Logic tests, g++ against a stub Geode header (`Economy.cpp`, `Store.cpp` only):
  all 11 reward values exact incl. Extreme Demon = $25.00; wallet charge and shortfall text; purchases persist;
  energy regen and 25 cap; $100 subscription renews then lapses when the wallet is short; reset clears everything;
  corrupted save strings are skipped key by key without losing valid keys; `Store::version()` bumps on writes; `Store::fnv` is deterministic.
* Symbol check against bindings `78d5bb4` (2.2091): every hooked function, every enumerator (`SearchType`, `GJDifficulty`, `GJLevelType`,
  `GJTimedLevelType`, `IconType`) and every member used exists. Per-platform address availability was read from the bindings and drives `Platform.hpp`.
* Geode 6.0.0-beta.1 headers checked for: `Popup::init(float,float,...)`, `setTitle`, `m_closeBtn`, `m_size`, `Notification::create`,
  `CCMenuItemExt::createSpriteExtra`, `addChildAtPosition` overloads, `Mod::expandSpriteName`, `queueInMainThread`, `Ref`, `getChildByIDRecursive`.
* `build.yml.txt` parses as YAML and uses only inputs that exist in the current `build-geode-mod` `action.yml`.

## NOT tested
No compile against Geode and no game run. So these are unverified: it loads, menus render, ads, hooks fire, hidden button, cosmetics unlocking,
macOS/Android behaviour, `.geode` packaging, and co-existence with your other mods.

## Crash hardening in this version (written, not yet proven in game)
* Popups redraw only when saved data changed (version counter), never mid-click on a timer.
* Every GD sprite frame goes through `safeFrame` (blank box if missing); mod PNGs are optional and skipped if absent.
* Purchase-then-continue flows hold the layer and sender in `Ref` and re-enter the hook instead of keeping raw `this`.
* The banner is found by node ID after an ad instead of captured (no dangling pointer, no retain cycle).
* Forced ads hide the popup X and ignore back/Escape; rewarded ads can be skipped.
* Online Levels: direct button hooks where the platform has them. Where it does not, the screen is left with `backOutNextFrame`, which only acts if
  the layer is still alive, still running and (for scenes) is the current scene. A stale layer can never pop the scene stack.
* Hooks that have no address on a platform are not compiled for it (`GCU_*` macros), instead of hooking something unbound.
* `GameManager::unlockIcon` (Windows-only inline) is no longer called; purchases are honoured through the `isIconUnlocked` hook.

## Known risks / first things to check
1. Calling the original from a lambda: the code re-enters the hook after a purchase rather than calling the base directly, which is the safest pattern I know, but unverified.
2. "Gated when the page opens" on macOS shows a one-frame flash of the target screen.
3. `"resources": {"sprites": [...]}` packaging and `expandSpriteName` paths: if textures fail to load the UI still works without them.
4. Level difficulty detection is best effort (`m_ratings/m_ratingsSum`, then `m_difficulty`); unrated levels pay as Auto.
5. Editor Premium gates playtesting only.
6. iOS is not supported.

## v1.2.0 test status (honest)

Run here (stub Geode header, g++): reward table, wallet, energy, subscriptions, save corruption tolerance (earlier) plus new:
money formatting with separators, `giveAllMoney` == 214748364700 cents and persisted in the serialized state, saturation on
`INT64_MAX` additions, no negative wallet, hostile saved values clamped, `resetModData` returns to defaults. All pass.

NOT run (no Geode SDK build, GD or Geode CLI in the sandbox) - please verify in game on Windows/macOS/Android:
1. Intro plays once, covers the menu, skip works, no replay after visiting the Creator menu and coming back, intro toggle works.
2. Audio plays (`playEffect` on an absolute path; `pauseMusic(0)`/`resumeMusic(0)` assume the menu loop is on channel 0).
3. `resources.files` packaging: audio is looked up at `<resources>/gcu-intro.ogg` and `<resources>/resources/gcu-intro.ogg`.
4. Both settings buttons appear (Geode built-in `button` setting type), fire their popups, and the balance survives a restart.
5. Notice popup layout at your screen size.


## v2.0.0 test status (honest)
Run here (stub Geode header + g++, `Economy.cpp`/`Store.cpp`, 4 suites, all pass):
* **Rating -> tier**: unrated pays nothing; stars 1..10 map to Auto/Easy/Normal/Hard/Harder/Insane/Demon; all five demon sub-types map to a demon tier; 10 stars without the flag is a demon.
* **Economy**: every demon pays exactly $25.00 (2500 cents); Auto $0.10; replay = 10%; duplicate-reward key persists in the save string.
* **Energy**: defaults (2/level) pay exactly the table; payout is monotonic in burned energy; multiplier changes the slope; capped at 300%; energy off pays 25% (never zero); invalid settings (negative, huge, NaN-like) are clamped; capacity lowering clamps stored energy; regeneration follows the setting and never exceeds capacity; energy never negative.
* **Pack ownership**: Explorers and Firebird are separate flags; buying one leaves the other locked; both persist in the serialized save and vanish only on RESET MOD DATA.
* **Level files**: the shipped JSON level strings were re-decoded and compared with the originals: header identical, all 69,392 / 43,210 remaining objects identical and in order, exactly 3 coins (ID 1329) removed from each, none left.
* **Art**: all PNGs generated and looked at on a contact sheet (icons, logo, pack art, backgrounds).

NOT run (no Geode build, no GD in the sandbox). Please verify in game on each platform:
1. The mod compiles (new files: Packs.cpp, Brand.hpp; changed: Economy, Rewards, Ads, Notice, Shop, main, UI).
2. Pack screen layout; Purchase / Play buttons; insufficient-funds message; ownership after restart.
3. **Playing the pack levels**: Play opens `LevelInfoLayer::scene` with a synthetic Saved-type level (negative id). If GD's level page misbehaves for such a level (comments/likes buttons, back navigation), report it. The 4-7 MB level strings load from the mod resource JSON.
4. **Songs**: copied to the path from `MusicDownloadManager::pathForSong(id)`. Explorers (1183462) should work; Firebird (10007217) is in the Music Library id range, where GD may expect a different file name/extension; if it plays silent, tell me what file GD created.
5. The levels are 2.21-era (object IDs up to 4xxx): fine on 2.2091 but I could not play them.
6. Settings page: the new groups and sliders, energy values taking effect, branding toggle.
7. Reward payout with a real online level of each rating, and with an unrated level (expect "Unrated level" toast, no money).
8. Memory: frame textures of the intro are freed one by one; pack levels are cached (2 x ~5 MB strings) and reused.
