# 2.5.2 - Support for Driver 19.x on AMD Vega (Hotfix 1)
The 19.x Driver started enforcing additional restrictions, which seem to only apply to AMD Vega GPUs and APUs. Due to this, the plugin would fail to properly initialize the encoder and users would have to manually set some options.

With this patch, this is no longer necessary. The encoder options are now applied correctly and should no longer cause any issues. Additionally a bug was fixed for Automatic Profile Level which caused it to occasionally select an unsupported Profile Level.

Hotfix 1: Improved hover text (long description) for properties and updated translations from CrowdIn.

## Changelog
### 2.5.2 Hotfix
* Improved descriptions (hover text) for various options of the H264 and H265 encoder.
* Updated translations from CrowdIn.

### 2.5.1
* Fixed error during initialization for H264 and H265 on Driver 19.x and above.
* Fixed a bug with automatic Profile Level which sometimes caused an unsupported Profile Level to be selected.

### 2.5.0
* Updated AMF to 1.4.9.0.
* Updated english locale text for Pre-Pass to include encoding cost.
* Fixed various erroneous usages of short codes for printf in log text.
* Added support for Git commit detection to CMake.
* Added support for clang-format.
* Added support for CppCheck to CMake. (Thanks to Streamlabs for this one)
