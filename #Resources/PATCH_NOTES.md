# 2.4.2 - Stability Improvements
A new version, but it's merely a few stability improvements. A crash on missing D3D11 or D3D9 has been fixed, capability testing always writes supported GPUs to the log file and the H264 and H265 encoder now follow the specification of the codec and set Profile, Level and Tier before Resolution and Frame Rate - which might actually improve support for higher resolution and frame rate capture.

Hotfix 1: Additional stability improvements by fixing a crash with a very old driver.
Hotfix 2: Fix a bug introduced in 2.4.0.

## Changelog
### 2.4.2 (Hotfix 2)
* Fix Profile Level being stuck at 1.0 in both H264 and H265.

### 2.4.1 (Hotfix 1)
* Refactored CMake build configuration for CI support, allowing for people to test [bleeding-edge builds](https://ci.appveyor.com/project/Xaymar/obs-studio-amf-encoder-plugin).
* Fixed a crash caused by very old drivers that predate H265 support.

### 2.4.0
* Updated AMF SDK to 1.4.7.0.
* Fixed a crash in api::base if Direct3D 9 or Direct3D 11 was not found.
* Changed how the capability testing reports supported devices to the log file.
* Fixed the order that codec properties are applied. (Thanks to Qiang Wen, see commit c292f6de41f22a0521300e8ce0b74bbe5e4d0edf)
