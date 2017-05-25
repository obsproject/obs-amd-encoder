# 2.1.6 - Now with full-range colors! (Hotfix 2)
Gone are the days of boring bland partial range recordings, here are the days of full range videos! With 2.1 the plugin implements several much wanted features, upgrading a few features from experimental to normal use and fixing a bunch of potential bugs.

Hotfix 1: 'Encoding overloaded!' should no longer show up for systems which were perfectly capable recording at the same fps in versions before 2.x. Also fixed debug log lines showing up without the Debug option being checked.
Hotfix 2: Added a workaround for a Driver-induced crash when running 32-bit OBS Studio.
Hotfix 3: Fixed a bug with the code handling the 'Enforce Streaming Service Settings' option which caused some settings to no longer work as intended.

## Changelog
### 2.1.6
* Fixed a bug that causes 'Enforce Streaming Service Settings' to always be applied to Target Bitrate, Peak Bitrate, Rate Control Method, Profile, Quality Preset and Keyframe Interval. (#228)

### 2.1.5
* Added a workaround for a 32-bit crash caused by AMD Driver 17.5.2 and earlier.

### 2.1.4
* Updated translations from CrowdIn and fixed a typo in en-US.

### 2.1.3
* Fixed debug log lines showing up even though 'Debug' wasn't checked.
* Fixed 'Encoding overloaded!' by making Asynchronous Queue the default behaviour and removing that option.
* Added 'Multi-Threading' and 'Queue Size' option which can be used to fine-tune the Asynchronous Queue behaviour.

### 2.1.2
* Fixed full range color causing crushed blacks and whites.
* Further improved the internal main encoding loop code.

### 2.1.1
* Updated Translations from CrowdIn.
* Fixed plugin starting encoding before encoder is actually ready.
* Fixed some properties not being applied.

### 2.1.0
* Fixed B-Frames not working properly. (#234)
* Added experimental I/P/B/Skip Period and Interval. (#220)
* Fixed a possible crash with OBS calling functions on a destroyed instance.

### 2.0.0
* Redesigned the internal structure to be much faster for much less CPU usage.
* Fixed several object lifetime issues that were only visible while debugging.
* Massively improved capability testing which now allows us to see the exact limits of the encoder.
* Fixed a crash-to-desktop on closing OBS.
* Added H264/AVC and H265/HEVC encoders.
* Slightly redesigned the UI to further improve quality of life.
* Removed 'OpenGL' and 'Host' entries from the Video API field as they aren't actual APIs. (#216)
* Removed the useless 'Usage' field, which was causing a lot of PEBKAC issues. (#210)
* Added the ability to use Keyframe Interval in 'Master' View Mode.
* Updated preset 'High Quality' to use a QP value of 18/18/18.
* Updated preset 'Indistinguishable' to use a QP value of 15/15/15.
* Fixed a crash with 'Automatic' VBV Buffer while using 'Constant QP'.
* Fixed 'Filler Data' and 'Enforce HRD' not working properly at all times. (#215)
* Changed the behaviour of 'Automatic' VBV Buffer to be linear with 'Constant QP'.
* Massively improved output timestamps.
* Fired Steve.
* Fixed a rare crash that could happen with certain translations.
* Changed the default for 'VBAQ' to 'Enabled' for H264 and H265.
* Added an Asynchronous Queue mode which enables safe multi-threading for slightly higher CPU usage and encoding latency. (#211)
* Split the 'OpenCL' field into 'OpenCL Transfer' (Use OpenCL to send the frame to the GPU?) and 'OpenCL Conversion' (Use OpenCL instead of DirectCompute for frame conversion?). (#212, #214)
* Fixed certain presets permanently locking the Keyframe Interval and IDR Period to low numbers. (#213)
* Improved Performance Tracking which is visible when starting OBS with --verbose --log_unfiltered.
