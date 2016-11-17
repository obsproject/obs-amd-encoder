# Multi-GPU Support And Filler Data Fix (Hotfix 3)
With this update the encoder plugin now supports multi-GPU setups, such as RX 480 + R9 390, R9 285 + R9 290, and others. You can select which GPU to use with the Advanced View Mode. The UI will also now update according to the supported features of the selected GPU and by default uses the primary GPU. Unsupported features will be hidden, just like unused features.

The 'Filler Data' property has now been fixed, Delta QP for B-Pictures is now visible when not using Constant QP, 'Memory Type' and 'Surface Format' have been removed and 'CABAC' has been replaced with 'Coding Type'. 'Memory Type' is now automatically using the best available and 'Surface Format' is taken from OBS settings.

Additionally a crash with AMD Hybrid/Switchable GPU setups was fixed and these systems should now be able to use the encoder. It will now default to using the best available AMD GPU in the system if it can detect it.

Hotfix 3: Fixed a crash due to use of incomplete DirectX9 backend.
Hotfix 2: Fixed a potential crash when opening settings.
Hotfix 1: Fixed Presets not being applied properly

## Notes

Due to the nature of changes since 1.3 users might experience that their settings have vanished or are incorrect. Please revalidate your settings or even create a clean profile to work off of.

## Changelog

* (Hotfix 3) Fixed: Crash due to use of incomplete DirectX9 backend.
* (Hotfix 2) Fixed: Potential crash when opening the settings.
* (Hotfix 1) Fixed: Presets were not being applied properly.
* Added: Full multi-GPU encoding support.
* Added: 'Coding Type' property which replaces 'CABAC'.
* Fixed: Filler Data was always being forced on for CBR.
* Fixed: Users should now be able to modify Delta QP for B-Pictures even when not using Constant QP.
* Fixed: Crash on OBS start on AMD Hybrid/Switchable GPU systems.
* Changed: Default device is always the primary/best available AMD GPU in the system.
* Changed: B-Picture properties will be properly hidden now if not in use.
* Changed: B-Pictures defaults to 0 and B-Picture Reference defaults to Disabled.
* Removed: 'Memory Type' property as the plugin will now always use the best available API.
* Removed: 'Surface Format' property as the plugin will now use the OBS settings for this.
* Removed: Deprecated encoder entry has been fully removed now.