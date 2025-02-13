# Horizon Zero Dawn Remastered Gameplay Tweaks & Cheat Menu

Source code for the Horizon Zero Dawn Remastered mod.

## Installation

- For developers, edit `CMakeUserEnvVars.json` and set `GAME_ROOT_DIRECTORY` to Horizon's root directory. The build script will automatically copy library files to the game folder.

- For manual Steam installs, copy `winhttp.dll` and `mod_config.ini` folder to the game's root folder. An example path is: `C:\Program Files (x86)\Steam\steamapps\common\Horizon Zero Dawn Remastered\`.

## Building

### Requirements

- This repository and all of its submodules cloned.
- **Visual Studio 2022** 17.9.6 or newer.
- **CMake** 3.26 or newer.
- **Vcpkg**.

### hzdr-gameplay-tweaks (Option 1, Visual Studio UI)

1. Open `CMakeLists.txt` directly or open the root folder containing `CMakeLists.txt`.
2. Select one of the preset configurations from the dropdown, e.g. `Universal Release x64`.
3. Build and wait for compilation.
4. Build files are written to the bin folder. Done.

### hzdr-gameplay-tweaks (Option 2, Powershell Script)

1. Open a Powershell command window.
2. Run `.\Make-Release.ps1` and wait for compilation.
3. Build files from each configuration are written to the bin folder and archived. Done.

## Changelog

<details>
  <summary>Click to expand.</summary><br/>

**Version 0.71**
  - Fixed UnlockEntitlementExtras not correctly applying ingame, once again.
  
**Version 0.7**
  - Added option ForceLeftAlignedCamera to lock Aloy's character on the left side of the screen, Forbidden West style.
  - Added asset override to disable atmospheric fog and haze.
  
**Version 0.6**
  - Added missing/remaining items to the inventory editor.
  - Added map tile LOD settings to the existing LOD bias asset override. Now has parity with the Forbidden West version.
  - Fixed god mode toggles not working properly.
  - Moved the in-game graphical LOD slider to Miscellaneous.
  
**Version 0.5**
  - Changed DisableCameraMagnetism logic so it behaves more like the HFW version.
  - Fixed auto-loot not automatically gathering rocks.
  
**Version 0.4**
  - Added cheat option for auto-loot (automatic gathering of plants and machine parts).
  - Added asset override to unlock all merchant shop items, including adept weapon and armor variants.
  
**Version 0.3**
  - Added slider for graphical LOD bias under Gameplay.
  - Added asset override to increase the level of detail (LOD) bias.
  - Fixed potentially buggy NewGame+ behavior when UnlockEntitlementExtras was in use.
  
**Version 0.2**
  - Added simple inventory editor under Cheats.
  - Added simple weather editor under Cheats.
  - Added asset override to remove the automatic slowdown while riding horses.
  - Added asset override to remove ambient looping sound effects on the Shield Weaver armor.
  - Added asset override to remove the shield overlay on the Shield Weaver armor.
  - Fixed EnableFreeCrafting cheat not applying to merchants.
  
**Version 0.1**
  - Initial release.
</details>

## License

- No license provided. TBD.
- Dependencies are under their respective licenses.
