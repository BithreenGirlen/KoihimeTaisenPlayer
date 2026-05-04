# KoihimeTaisenPlayer

某寢室用

## How to play

First, prepare the files, commented below, with proper directory.

<pre>
remoteAssets
├ adv
│  ├ scenario // Script folder
│  │  ├ ...
│  │  ├ char_100100102.json // Script file to be selected
│  │  └ ...
│  ├ spine // cut-in folder
│  │  ├ r_1001001
│  │  │  ├ cutin1.atlas	// Spine atlas
│  │  │  ├ cutin1.bin	// Spine skeleton
│  │  │  ├ cutin1.png	// Spine texture
│  │  │  ├ cutin2.atlas
│  │  │  ├ cutin2.bin
│  │  │  └ cutin2.png
│  │  └ ...
│  └ stillAnim // Still folder
│     ├ r_1001001
│     │  ├ image_00000.webp
│     │  ├ ...
│     │  └ image_00083.webp
│     └ ...
└ sound
   ├ se
   └ voice // voice folder
      └ adv
         ├ ...
         ├ 100100102
         │  ├ vo_100100102_1001001_001.mp3
         │  ├ ...
         │  └ vo_100100102_1001001_058.mp3
         └ ...
</pre>

Then select a script file namd as `char_*_2.json` under `remoteAssets/adv/scenario` directory from application menu `File->Open`.
  -  It is desirable to filter files by `char_*2.json` as WinAPI supports. But SDL provides filtering only on extension, and returns error if filename filter like this were passed. So it is up to user to select a script file of which name matches expected pattern.

## Preference

The following preference can be configured through `setting.txt` in the same directory of the executable file.
- Font file with which the text will be drawn.

## External libraries

- [SDL-3.4.4](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.0)
- [SDL_image-3.4.2](https://github.com/libsdl-org/SDL_image/releases/tag/release-3.2.6)
- [libwebp](https://github.com/libsdl-org/libwebp/tree/42611dee80e9dd744e6fca13c11c682d36463b9d)
- [SDL_mixer-3.2.0](https://github.com/libsdl-org/SDL_mixer/releases/tag/release-3.2.0)
- [SDL_ttf-3.2.2](https://github.com/libsdl-org/SDL_ttf/releases/tag/release-3.2.2)
- [spine-cpp-3.8](https://github.com/EsotericSoftware/spine-runtimes/tree/3.8)
- [Dear ImGui v1.92.7](https://github.com/ocornut/imgui)

## Build

Visual Studio is required.

1. Open `KoihimeTaisenPlayer` directory with Visual Studio.
2. Wait for downloading external libraries to be done.
2. Install `KoihimeTaisen`.
3. Build all. 
