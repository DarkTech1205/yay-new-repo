# Changelog

## v2.0.0 - Ubisoft Dash: corporate overhaul
**Economy**
* Online rating detection fixed: rewards use the level's real star value, demon flag and demon sub-type (was: everything paid as Auto). Unrated levels pay nothing.
* Every Demon (Easy/Medium/Hard/Insane/Extreme) now pays exactly **$25.00**. Auto $0.10, Easy $0.25, Normal $0.50, Hard $1.00, Harder $2.00, Insane $5.00 (unchanged).
* Single reward formula (`Eco::payoutCents`): table x (first completion 100%, replays 10%) x energy factor.
**Energy rework** (all in mod settings, all clamped to safe ranges)
* Enable/disable energy; energy per level; energy per attempt; maximum energy; seconds per energy; energy reward multiplier; payout percent when energy is off.
* Energy off: no limits but payouts are reduced (default 25%). Energy on: 2 energy per level pays exactly the table; +/-25% per energy point (times the multiplier), clamped 10%..300%.
**Ubisoft identity**
* Acquisition announcement rewritten as a corporate press release.
* Optional corporate terminology (Ubisoft Credits, Player Engagement Energy, Engagement Incentive, Promotional Opportunity, Content Acquisition, Daily Engagement Rewards) via `ubisoft-branding`.
**Visuals**
* All mod sprites redrawn in a Geometry-Dash-inspired style (outlines, gradients, highlights, crisp 4x-supersampled edges); ads use a new outlined promo panel; new logo, demon badge, pack and level art.
**2.21 Pack**
* New pack screen: artwork, name, two Demon levels (Explorers 2 by Mathiscreator, Firebird by seannnn) with icons, difficulty, reward, price, ownership, **separate purchases** ($9.99 each), independent persistent ownership, Play opens GD's own level page. Reachable from the main menu button and the Shop.
* Level data from the supplied .gdlvl files with the 3 user coins of each level removed; supplied songs bundled.
**Settings**: grouped (Branding, Promotional Opportunities, Energy, Subscriptions, Level Packs, Mod Data).
**Packaging**: project delivered as two upload parts (<=100 files each).

## v1.2.0
Ubisoft intro, acquisition notice, RESET MOD DATA, GIVE ME ALL THE MONEY.

## v1.1.0
Geode 6.0.0-beta.1, macOS + Android support, crash hardening.
