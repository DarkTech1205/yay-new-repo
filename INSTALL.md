# Build and installation

## One mod, two upload parts
The project is split only because GitHub's **web** upload accepts at most 100 files at a time. Both parts unzip into the **same folder** and together are ONE mod. Part 2 is NOT optional (intro frames, 2.21 pack art, level data and songs).

1. Unzip `UbisoftDash-PART1-main-project.zip` and `UbisoftDash-PART2-additional-resources.zip` into the same empty folder (merge if asked). The result must contain `mod.json`, `src/` and `resources/` side by side; `MANIFEST.txt` lists every file and which part it came from.
2. Check: `resources/` must hold 88 `gcu-intro-###.png`, `gcu-pack-221.png`, `gcu-pack-explorers.json`, `gcu-pack-firebird.json`, two `gcu-song-*.mp3` and `gcu-intro.ogg`.

## Build with GitHub Actions (recommended)
1. Create an empty repo. Upload Part 1 via the web UI, commit, then upload Part 2 into the same repo (keep the `resources/` folder name), commit. (Or use git: `git add . && git commit && git push` uploads everything at once with no 100-file limit.)
2. Copy `build.yml.txt` to `.github/workflows/build.yml` (the web UI cannot create dot-folders from an upload, so use "Create new file" and type the path).
3. Push; download the combined `.geode` (Windows + macOS + Android) from the workflow artifacts. Put it in your Geode `mods` folder.

## Local build
See README.md ("Build"): Geode SDK tag v6.0.0-beta.1, bindings commit 78d5bb4, `geode build`.

## Requirements
Geometry Dash 2.2091, Geode 6.0.0-beta.1. No dependencies on other mods.
