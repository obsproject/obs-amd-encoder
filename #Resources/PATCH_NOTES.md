# 2.2.2 - Pre-Pass, VBAQ and more fixes! (Hotfix 2)
With the newly released Driver 17.7.2, AMD fixed many reported issues and added some much wanted features:

- Pre-Pass and VBAQ are finally working,
- H265/HEVC content can distinguish between IDR- and I- Frames
- and also supports Color Range and Color Space now,
- and the 32-bit only text log crash was fixed.

And now it's time for the plugin to also be updated! This release primarily aims at adding support for AMF 1.4.4 and improving support for older drivers, with some of the recently discovered bugs and crashes fixed.

Hotfix 1: Fixed VBAQ and Pre-Pass being used with Constant QP, causing significant corruption issues and changed some default values for H265.
Hotfix 2: Fixed an at-exit crash caused by an AMF trace function.

## Changelog
### 2.2.2
* Fixed an at-exit crash caused by calling amf::AMFTrace::TraceEnableAsync(true).

### 2.2.1
* Fixed a frame corruption bug with Constant QP caused by toggling VBAQ and Pre-Pass after it had already been applied.
* Changed the default value of 'Enforce HRD' to 'Enabled' for H265/HEVC encoding.
* Changed the default value of 'Deblocking Filter' to 'Disabled' for H265/HEVC encoding to fix corruption in strict decoders. (Premiere Pro, After Effects, Vegas, VLC (Software))

### 2.2.0
* Added support for AMF 1.4.4 and improved support for older drivers like 16.11.5 and 16.12.1.
* Fixed a memory corruption crash due to misuse of managed DirectX 11 objects.
* Fixed a frame corruption bug caused by applying 'VBAQ' and 'Pre-Pass' when using 'Constant QP' Rate Control Method.
* Fixed a string comparison bug causing Simple Output Mode to always use the Profile 'Constrained Baseline'
* Fixed a crash when enabling 'Debug' on 32-bit OBS caused by different type sizes.
* Fixed a crash with H265/HEVC caused by use of the undocumented Intra-Refresh properties.
* Changed the default value for 'Profile' to 'High'.
* Changed the default value for 'VBAQ' to 'Disabled' for improved performance.
* Changed the default value for 'Queue Size' to 8 for improved stability in spontaneous load situations.
* Changed the log crash work around to only apply to older AMF versions than 1.4.4.
* Removed unnecessary configuration changes.
* Removed unsupported Pre-Pass options 'Half Size' and 'Quarter Size'.