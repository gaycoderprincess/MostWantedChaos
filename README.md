# Most Wanted Chaos

A mod for Need for Speed: Most Wanted that occasionally does random things

Featuring Vergil from the Devil May Cry™ series

Similar to [NFS Chat Chaos Mod by berkayylmao](https://github.com/berkayylmao/NFS-Chat-Chaos-Mod) and [NFS Most Wanted Chaos Mod (ZChaos) by Zolika1351](https://zolika.dev/mods/nfsmwchaos), but please don't take this as an equivalent or replacement for them!  
This is just a fun side project for me to mess around with while I learn how the game works :3

## Disclaimer

Due to the current programming landscape, I feel that it's necessary to explicitly state that this project had zero assistance or any other kind of involvement from any sort of "AI agent" and it never will.  
This mod was entirely built by hand, by a human being, and I believe that any project that cannot also claim this about itself is not worth people's time. The only acceptable amount of AI use is zero AI use.

## Installation

- Make sure you have v1.3 of the game, as this is the only version this plugin is compatible with. (exe size of 6029312 bytes, other regions such as the Asian version are not and will never be supported)
- Plop the files into your game folder, press F5 in game to open the menu.
- If you're feeling adventurous, place a ROM of the US version of Super Mario 64 next to the files, renamed to `baserom.us.z64`.
- Enjoy, nya~ :3

## Recommended mods

- [Open Limit Adjuster](https://github.com/gaycoderprincess/MostWantedOpenLimitAdjuster/releases/latest)
- [Xbox 360 Stuff Pack](https://nfsmods.xyz/mod/1200)
- [XtendedInput](https://github.com/xan1242/NFS-XtendedInput/releases/latest) (only if you're using a controller!)

## Incompatible mods

- Extended Customization (crashes the game during Acid Trip)
- Sun Set (fully incompatible)

## Building

Building is done on an Arch Linux system with CLion being used for the build process. 

Before you begin, clone [nya-common](https://github.com/gaycoderprincess/nya-common), [nya-common-nfsmw](https://github.com/gaycoderprincess/nya-common-nfsmw), [CwoeeMenuLib](https://github.com/gaycoderprincess/CwoeeMenuLib) and [CwoeeModelImporter](https://github.com/gaycoderprincess/CwoeeModelImporter) to folders next to this one, so they can be found.

Required packages: `mingw-w64-gcc vcpkg`

To install all dependencies, use:
```console
vcpkg install tomlplusplus:x86-mingw-static box3d:x86-mingw-static
```

To install the BASS audio library:

Download the Win32 version from [here](https://www.un4seen.com/bass.html) and extract it somewhere

Once that's done, copy `bass.lib` from the `c` folder into `nya-common/lib32`

You should be able to build the project now in CLion.
