# Research notes and feature checklist

## What I could verify (done in the build sandbox)
* Geode SDK tag **v6.0.0-beta.1** (commit 0bf4857, 2026-11-20) cloned; `VERSION` reads `6.0.0-beta.1`. beta.1 changelog is two small fixes on top of alpha.3; API used by this mod is unchanged.
* Bindings for **GD 2.2091** inspected at commit 78d5bb4 (2026-11-20, one minute before the beta.1 tag). Windows, macOS and Android availability was checked per function (Android links by symbol for every hooked class). Hooked functions:
  `MenuLayer::onCreator`, `CreatorLayer::{onOnlineLevels,onFeaturedLevels,onDailyLevel,onWeeklyLevel,onEventLevel,onGauntlets,onTopLists,onSavedLevels,onMapPacks,onLeaderboards,onPaths,onTreasureRoom}`,
  `LevelBrowserLayer::init(GJSearchObject*)`, `PlayLayer::{init(GJGameLevel*,bool),resetLevel,levelComplete}`, `EditorUI::onPlaytest`.
* Real menus in 2.2091 that exist beyond the prompt's 12-button list: Magic, Latest Stars, Friends, Hall of Fame, Event Level, Map Packs, Leaderboards. All are monetized.
* Quick-search buttons (Most Downloaded, Most Liked, Recent, Trending, Magic, Latest Stars, Followed) are bound on Windows (and by symbol on Android) in this bindings commit, but NOT on macOS.
  macOS gates them when `LevelBrowserLayer` opens with the matching `SearchType`.
* Popup API in beta.1 is the non-template `geode::Popup` with `init(float, float, ...)` (read from the SDK header).

## What I could NOT do
* **The video.** `youtube.com` was not fetchable from the sandbox, so I have no transcript, captions or timestamps. I did not watch it.
  The "directly demonstrated" column below is therefore EMPTY on purpose. Nothing is claimed as appearing in the video.
  Action for you: paste the transcript or timestamps and I will fill this in.
* **Geode v6 migration guide** (docs.geode-sdk.org) was blocked by the fetch allowlist; I used the SDK headers and changelog instead.
* **Image generation** was unavailable. All art is procedural (docs/make_art.py), original, and not AI-generated.
* **Compile / run**: no Windows toolchain, no Geometry Dash.

## Feature checklist
| Feature | Source | Status |
|---|---|---|
| Features shown in the video | video | UNKNOWN (no transcript) |
| Reward table by difficulty ($0.10 to $25) | prompt | implemented, unit tested |
| $4.99 Online Levels entrance | prompt | implemented, untested in game |
| $2.99 per Online Levels button (12 listed + 7 more found) | prompt | implemented, untested in game |
| $100/month Moderator Membership + panel with all 9 listed buttons | prompt | implemented, untested in game |
| ROBTop's Secret Menu + second hidden menu | prompt | implemented, untested in game |
| Gems, energy (25 max), reworked currencies, exchange | prompt | implemented |
| Rewarded / forced / banner ads | prompt | implemented |
| 15-style cosmetics, 6 rarities, rotation, collection score | prompt | implemented (colors/particles/decorations are mod-only flags) |
| Treasure Room, GeoPass, objectives, creator program, $20 top comment | prompt | implemented |
| Editor Premium $5/month | prompt | playtest gate only (see TESTING.md) |
| Save + reset | prompt | implemented, unit tested |

## Sources
* https://github.com/geode-sdk/geode (tag v6.0.0-beta.1)
* https://github.com/geode-sdk/build-geode-mod (action.yml and examples/multi-platform.yml, basis for build.yml.txt)
* https://github.com/geode-sdk/bindings (commit 78d5bb4eb0732dd636ae16d4dcf6e5ce84ec1221, `bindings/2.2091/GeometryDash.bro`)


## v1.2.0 API notes
Checked in Geode 6.0.0-beta.1 headers: `ButtonSettingV3` (mod.json type `button`, `buttons` object) + `ButtonSettingPressedEventV3(mod,key).listen(..).leak()`; `createQuickPopup`; `Mod::saveData`; `Mod::getResourcesDir`; `resources.files` is used by Geode's own mod.json. Bindings 78d5bb4: `FMODAudioEngine::playEffect(std::string)`, `pauseMusic/resumeMusic(int)`, `stopAllEffects`.


# v2.0.0 research (Ubisoft Dash corporate overhaul)

## Phase 1.1 - audit of the existing project (done by reading every source file)
* Architecture is sound (one `Store` key/value save, `Eco` economy layer, `MenuPopup` screens, hooks that always call the original), so **no rewrite**: systems were extended in place.
* Hooks (each calls the original): `MenuLayer::init`, `MenuLayer::onCreator`, `CreatorLayer::*`, `LevelBrowserLayer::init`, `LevelSearchLayer` quick-search buttons, `GameManager::isIconUnlocked`, `PlayLayer::{init,resetLevel,levelComplete}`, `EditorUI::onPlaytest`. v2 adds **no new hooks** (the pack uses GD's own `LevelInfoLayer`, the settings buttons use Geode events).
* Root cause of "online levels only pay Auto": `tierFor` read the averaged player vote and the difficulty face. For a level coming from the online browser those are empty or N/A, so the code fell to its default branch (Auto). The authoritative data is the **star value** (`m_stars`, 0 = unrated) with the demon flag and sub-type.
* Energy was a fixed 25 / 120 s constant pair spent as 1 per level; now fully settings-driven.

## Phase 1.2 - moderator functionality (what is public and what is not)
Findings from the pinned bindings (GD 2.2091) and public sources:
* Client-side moderator UI exists in the game: `LevelInfoLayer::onRateStars / onRateStarsMod / onRateDemon / onRate`, `RateStarsLayer`, `RateStarsModLayer`, `RateDemonLayer`, `LevelFeatureLayer` (feature/epic), `GJRateLevelLayer`, `AdminActionConfirmPopup`, `AccountBanPopup`, `InfoLayer` mod actions, and `GJAccountManager`/`GameLevelManager` calls such as `rateStars`, `setLevelFeatured(id, rank, epic)`. Account badge field: `m_modBadge`.
* These screens only do anything when the **server** says the account is a moderator; the actions are server-side requests. They are privileged operations. This mod does **not** call them, does not spoof `m_modBadge`, does not impersonate a moderator and never grants real moderation rights.
* Public rating facts (RobTop's rating guideline PDF): Auto 1 star, Easy 2, Normal 3, Hard 4-5, Harder 6-7, Insane 8-9, Demon 10. Rating tiers: Rated, Featured (2 creator points), Epic (3), then Legendary and Mythic. Demons are assigned by RobTop; the sub-type (Easy ... Extreme) only changes the face, and the **reward is the same for every subtype**. That is the basis for "every Demon = $25" and for reading stars instead of faces.
* Third-party guides describe moderators as reviewing reported levels, suggesting/rating and awarding Featured/Epic; I treat that as low-confidence (it is an unofficial guide with no mod/elder-mod detail). I could not fetch RobTop's moderator FAQ (redirect loop) and could not reach Geode community discussions from the sandbox, so nothing is derived from them.
* Result: the in-mod "Moderator Membership" panel stays a **local parody on local demo data**, visually imitating a moderator menu; no server permission is bypassed.

## Phase 5 - visual language
Studied from the reference screenshots you supplied (the 2.21 "The Map" popup, Firebird, Explorers): thick near-black outlines, glossy vertical gradients, bevelled highlights, hard drop shadows, chunky pixel type, saturated palette. `docs/make_art.py` draws every icon at 4x and downsamples (crisp, transparent where appropriate). No GD art is copied; the pack icon uses the screenshots only as colour/shape references (ice-blue map base, red brick tower + flames + orange spikes for Firebird, purple chains + skull + orbs + green spikes for Explorers). I could not open GD itself, so the match to the game's exact look is by eye from your screenshots, not measured.

## Level files (.gdlvl)
* A `.gdlvl` is base64url(gzip(plist XML)). Inside, `k4` is the level string (itself base64url(gzip)). Both files decoded cleanly; each contains exactly **3 user coins** (object ID 1329), removed as you asked. Everything else (header, 69,392 / 43,210 objects, order) was verified byte-identical against the originals (see docs/TESTING.md).
* The files contain **no creator, difficulty or rating fields**: creators (Mathiscreator, seannnn) and the Demon rating are set by the mod. The Explorers file's own name is **"Explorers 2"**; it is kept as stored.
* The files reference custom songs 1183462 (Explorers) and 10007217 (Firebird); your two MP3s ship as `gcu-song-<id>.mp3` and are copied into GD's song folder on first play (never overwriting an existing song).
