# 1.4.3.5 - Performance Tracking, Translation Update & Minor Fixes
A new feature is added in this version to the Debug checkbox: Performance Tracking! Performance Tracking allows you to figure out which frames took too long to create, convert or encode - without needing to touch an IDE at all. Simply check the Debug checkbox and the information will be printed into the log file!
Performance Tracking outputs all times in nanosecond precision, so even the slightest delay will be visible. But be careful when trying to figure out why something took so long - it can't work faster than what the internal OBS frame timer ticks at.

This update also fixes a rare crash when quitting OBS, makes the Usage modes 'Low Latency' and 'Ultra Low Latency' work again and allows the encoder to start properly with other color formats than NV12 again. Additionally the locale files have been updated to the latest ones again.

## Changelog
* Fixed: Encoder can now work with other color formats than NV12.
* Fixed: Usage 'Low Latency' and 'Ultra Low Latency' should now work properly.
* Fixed: Occasional rare crash on OBS exit due to memory deallocation happening too early.
* Changed: Log messages for library loading.
* Changed: Locale files have been updated to match CrowdIn again.
* Added: Performance Tracking ability with Debug checked.