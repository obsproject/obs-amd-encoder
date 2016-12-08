# 1.4.3.3 - Crimson ReLive Compatibility Update
Crimson ReLive changed how the Full Range flag is applied, which caused the plugin to break since it expected another property there. This has been fixed and some log messages have been changed to also result in better readability for users and support.

## Notes
There is currently a warning from AMF that can be safely ignored in the log files. Hopefully this will be fixed by a driver update from AMD.

## Changelog
* Fixed: Experimental Full Range Color mode no longer causes encoding to fail. (Fixes #175)
* Changed: Queue status log messages have been reduced to once per second instead of once per frame.
* Changed: First submission log message now show the time in seconds instead of nanoseconds.
