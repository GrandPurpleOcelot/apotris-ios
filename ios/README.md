# Apotris for iPhone and iPad

A native iOS build of Apotris. It reuses the Android touch layer
(`source/liba_android.cpp`, built with `-DMOBILE -DIOS`) on top of SDL2 and
OpenGL ES, and links into a small Xcode app.

## Build and run on your iPhone

On a Mac with Xcode 15 or newer:

```bash
brew install meson ninja cmake pkg-config xcodegen
git clone --recursive https://github.com/GrandPurpleOcelot/apotris-ios.git
cd apotris-ios
xcodegen --spec ios/project.yml   # creates ios/Apotris.xcodeproj
open ios/Apotris.xcodeproj
```

In Xcode:

1. Select the **Apotris** target, then **Signing & Capabilities**, and pick
   your **Team** (a free Apple ID works). If Xcode says the bundle ID is
   taken, change it to something unique such as `com.yourname.apotris`.
2. Plug in your iPhone (or pick it over Wi-Fi), select it as the run
   destination, and press **Run**.
3. First time only: on the iPhone, open **Settings > General > VPN & Device
   Management** and trust your developer certificate. With a free Apple ID
   you'll also need **Settings > Privacy & Security > Developer Mode** on.

The first build takes a while (around 10 minutes) because `ios/build.sh`
compiles SDL2, SDL2_mixer and all of Apotris's libraries for iOS. Later
builds only recompile what changed.

With a free Apple ID the app stops launching after 7 days; press Run again
to reinstall it. A paid developer account extends that to a year.

## Files

Saves, skins and the game's sound and music files live in the app's
Documents folder. Open **Files > On My iPhone > Apotris** to back up
`Apotris.sav` or to add custom music to `assets/` (same as on Android).

## Not ported yet

- Online multiplayer (needs libdatachannel/curl built for iOS). The menu
  entry is hidden, as on Switch.
- Safe-area handling: on phones with a notch or Dynamic Island, move the
  touch buttons with the in-game touch layout editor if one sits under it.

## How the build fits together

- `ios/build.sh` builds SDL2 and SDL2_mixer with CMake, then the game with
  meson (`darwin` host with an `ios` subsystem), and merges everything into
  `ios/build/out/<platform>/libApotrisAll.a`. It also stages `assets/`.
- `ios/project.yml` is the XcodeGen spec. Its pre-build phase runs
  `build.sh`, so pressing Run in Xcode rebuilds the game too.
- `ios/ios_platform.mm` provides the iOS versions of the hooks Android
  implements with JNI: the Documents path and haptic feedback.
- `.github/workflows/ios.yml` builds for the simulator, launches the game,
  takes a screenshot, and checks that a device build compiles.
