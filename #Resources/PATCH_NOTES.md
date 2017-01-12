# 1.4.3.8 - Settings Transfer, Automatic VBV Buffer adjustment and Fixes (Hotfix 2)
Another day, another new feature: this time it is transferring settings between versions, so that you will no longer use settings when a change to a setting is made. Since it only just now started tracking the config version, it will only work with settings created between 1.4.3.0 and 1.4.3.5, any other version might end up with broken settings.

Another change has been done to the Automatic VBV Buffer Size, which will now behave much more predictable. A value of 0% is completely unrestricted, 50% matches the calculated bitrate and 100% matches the calculated strict bitrate.

Presets will also now use the proper minimum and maximum QP values and the minimum QP default value has been increased to 11.

Hotfix 1: Fix enumeration based properties not working correctly due to a programming error.
Hotfix 2: Actually fix the enumeration based properties for real this time.

## Changelog
* Added: Version-specific setting transfer code which should reduce the lost settings between updates.
* Changed: VBV Buffer Strictness is now linear with three steps: 100000 (0%), Target Bitrate (50%) and Strict Target Bitrate (100%).
* Changed: Default for Minimum QP is now 11.
* Fixed: Presets not using the proper QP Minimum and Maximum.
* Fixed: Startup log messages not showing proper error codes.
* Hotfix: Fix enumeration based properties not using the correct values.
* Hotfix: Fix the default value for B-Frame Pattern being '-1' due to an oversight in code.