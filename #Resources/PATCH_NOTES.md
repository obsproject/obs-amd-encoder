# The Multi-GPU Update
Once again back to change things around, the multi-GPU code has finally been rewritten and properly implemented - which means that DirectX 11 and DirectX 9 are now fully supported. So rejoice Windows 7 users, you can now choose which GPU the encoding should take place on (with the limitation that a Monitor must be connected)! Needless to say, this change removes the old multi-GPU selection properties with the new revamped ones: 'Video API' and 'Video Adapter'.

Some changes to the available properties and internal logic have also been done. Properties are now reordered to be closer to the order they are applied in, some properties have been renamed and the 'Master' view mode has grown quite a bit. 'Frame Skipping' no longer defaults to being enabled, 'B-Frame QP' is now properly hidden, 'Profile' now has two new options ('Constrained Baseline' and 'Constrained High'), 'Video Adapter' and 'OpenCL' are now properly hidden and changing 'Preset' should now properly reset property states. Also 'Unlock Properties' was removed due to it causing more issues that it was worth.

Some changes were also done internally to make the plugin run better. Startup log output has been reduced to one line at the request of people handling support, a crash with very high Keyframe Intervals has been fixed and the frame queue has been reduced from 3 seconds to 1 second.

One major change was done to how frames are submitting and packets are retrieved and that is that the plugin will now attempt to submit a frame or retrieve a packet for a specific amount of time (1000/framerate milliseconds) before giving up. Due to this change OBS will now show the 'Encoding overloaded' message when the encoder is overloaded.

## Notes
This change includes renaming some properties so some settings might be lost and have to be set again (B-Frames, Experimental Properties). Please revalidate your settings or even create a clean profile to work off of.

## Changelog
* Added: New Multi-GPU code with full support for DirectX 11 and DirectX 9 and partial support for OpenGL and Host mode.
* Added: Additional experimental Properties to View Mode 'Master': 'Coding Type', 'Maximum LTR Frames', 'Header Insertion Spacing', 'Slices Per Frame', 'Slice Mode', 'Maximum Slice Size', 'Slice Control Mode', 'Slice Control Size', 'Intra-Refresh Number of Stripes', 'Intra-Refresh Macroblocks Per Slot', 'Wait For Task', 'Pre-Analysis Pass', 'VBAQ', 'GOP Size', 'GOP Alignment', 'Maximum Reference Frames'.
* Added: H264 Profiles 'Constrained Baseline' and 'Constrained High'.
* Changed: Added timestamps to logs created with the 'Debug' property enabled.
* Changed: Reduced startup log output to only a single line.
* Changed: Reduced frame queue size from 3 seconds to 1 second.
* Changed: Renamed properties that used 'Picture' to use 'Frame' instead.
* Changed: Default for 'Frame Skipping' is 'Disabled' now.
* Changed: Order of properties is now similar to how they are applied.
* Fixed: H264 Encoder will only be listed if it detected a GPU with AVC encoding support.
* Fixed: Hide property 'B-Frame QP' when B-Frames aren't used or not supported.
* Fixed: Hide properties 'Video Adapter' and 'OpenCL' when the selected API does not support them.
* Fixed: Changing presets should now reset property states correctly.
* Fixed: OBS now shows 'Encoding overloaded' when encoding is actually taking too long.
* Fixed: Crash with very high Keyframe Intervals.
* Removed: Property 'Unlock Properties' due to difficulties.