# 2.3.3 - Changes to the Blacklist, Out-of-Process AMF Capability testing and more (Hotfix 1)
The Blacklist introduced with 2.3.0 unfortunately had the side effect that all Windows 8 users were no longer able to use the plugin and had to downgrade. With this version, the blacklist is instead now a warning written into the log file. This is all thanks to @jp9000's work on moving the initial AMF Capability test into a different process, stopping broken drivers from crashing OBS Studio.

Not only that, but a bunch of previously unidentified issues were fixed. H264 VBR should no longer limit the Peak Bitrate to the Target Bitrate, H265 encoding shouldn't get stuck anymore, some older Drivers may now work again with the plugin and Direct 3D 11.1 is now used if it is available.

Hotfix: Configuration Version should now match the plugin version again.

## Changelog
### 2.3.3 (Hotfix)
* Fixed H264 Plugin not properly adjusting config version number.
* Reduced subprocess 'amf-test' size, which is used to check for AMF compatibility.

### 2.3.2
* Fixed a bunch of startup crashed by moving AMF Capability tests to another process, thanks @jp9000 for doing a lot of the work to make this happen.
* Fixed the H265 encoder not stopping which was caused by 'Keyframe Interval' and 'GOP Size' being stuck at 0 internally.
* Replaced the blacklist with a warning message to allow Windows 8 users to use current versions.
* Fixed a crash with older Drivers which do not have Pre-Pass or VBAQ.
* Fixed 'Target Bitrate' being set as 'Peak Bitrate' in VBR rate control mode.
* Fixed Direct3D 11.1 not being used even if available, resulting in minimally lower performance.

### 2.3.1
* Skipped 2.3.0 as it was not included with obs-studio.
* Improved support for the "New Networking Code" and "Replay Buffer" features by renaming 'Bitrate.Target' to 'bitrate'.

### 2.3.0
* Added a blacklist for drivers exposing an AMF Runtime older than 1.4.6.0.
* Removed all hidden properties no longer exposed by the newest AMF Runtime.
