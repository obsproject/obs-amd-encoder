# 1.4.3.4 - Crimson ReLive Compatibility Update (Hotfix 1)
Crimson ReLive changed how the Full Range flag is applied, which caused the plugin to break since it expected another property there. This has been fixed and some log messages have been changed to also result in better readability for users and support.

Hotfix 1: The Driver update also broke VBAQ, but it caused less issues than the Full Range flag. This has been fixed.

## Changelog
* Hotfix 1: Experimental Property VBAQ would not be properly set.
* Hotfix 1: Runtime and Compiled Against version numbers were switched around.
* Fixed: Experimental Full Range Color mode no longer causes encoding to fail. (Fixes #175)
* Changed: Queue status log messages have been reduced to once per second instead of once per frame.
* Changed: First submission log message now show the time in seconds instead of nanoseconds.
