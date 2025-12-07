# Most Wanted Chaos

A mod for Need for Speed: Most Wanted that occasionally does random things

Similar to [NFS Chat Chaos Mod by berkayylmao](https://github.com/berkayylmao/NFS-Chat-Chaos-Mod) and [NFS Most Wanted Chaos Mod (ZChaos) by Zolika1351](https://zolika.dev/mods/nfsmwchaos), but please don't take this as an equivalent or replacement for them!  
This is just a fun side project for me to mess around with while I learn how the game works :3

## Installation

- Make sure you have v1.3 of the game, as this is the only version this plugin is compatible with. (exe size of 6029312 bytes)
- Plop the files into your game folder, press F5 in game to open the menu.
- Enjoy, nya~ :3

## Building

Building is done on an Arch Linux system with CLion being used for the build process. 

Before you begin, clone [nya-common](https://github.com/gaycoderprincess/nya-common), [nya-common-nfsmw](https://github.com/gaycoderprincess/nya-common-nfsmw) and [FlatOutUCMenuLib](https://github.com/gaycoderprincess/FlatOutUCMenuLib) to folders next to this one, so they can be found.

Required packages: `mingw-w64-gcc vcpkg`

To install all dependencies, use:
```console
vcpkg install tomlplusplus:x86-mingw-static assimp:x86-mingw-static
```

To install the BASS audio library:

Download the Win32 version from [here](https://www.un4seen.com/bass.html) and extract it somewhere

Once that's done, copy `bass.lib` from the `c` folder into `nya-common/lib32`

You should be able to build the project now in CLion.
