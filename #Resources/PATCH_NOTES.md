# 2.0.0.0 - The 'Ashes of the Phoenix' Update
The plugin rises from the no-update-phase back to bring you an even better experience with High Efficiency Video Coding (HEVC) encoding, Variance Based Adaptive Quantitization (VBAQ), Pre-Pass support and Asynchronous Queue mode.

High Efficiency Video Coding (H265/HEVC) is available on the Polaris architecture and offers massively better quality for the same bitrate, essentially resulting in lower bitrates looking much better. This means that at 1000 kbit H265/HEVC you can get close or surpass the quality of 2500 kbit H264/AVC in many types of scenes and motion. Unfortunately it never took off as it only got slightly better quality than VP9 and AV1 is already beating it in terms of speed and quality.

Variance Based Adaptive Quantitization (VBAQ) and Pre-Pass are both methods to better distribute the Bitrate in a given frame. VBAQ works on the principle of visual perception, while Pre-Pass looks at which areas need more Bitrate to not end up being blocky. Enabling both will result in a much better output with no change in Bitrate.

Asynchronous Queue is a new feature that used to be the standard behaviour in earlier versions. Since no two CPUs are the same, Asynchronous Queue offers a way to use multiple cores of the CPU for the encoding task, instead of just handling everything on a single core. This feature is in very early stages, so it will probably cause issues unless absolutely needed.

## Changelog
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
