# User Experience Update
This update changes the behaviour of presets so that they now disable interaction with properties that end up being overriden and limit the range of properties that are allowed to be changed. The reason for this is to reduce 'bug' reports in which the user just forgot to deselect the preset again.

The defaults and the presets have also been slightly modified. VBV Buffer Strictness now defaults to 0% and all presets no longer override VBV Buffer properties.

A few bugs have also been fixed. Stable APU drivers should now work fine with this version again, the submission queue shouldn't fill up at the beginning of encoding while VCE isn't yet ready and color profile and range are now correctly applied from OBS Advanced Settings. Full Range is only supported on some drivers and hardware so far.

## Notes

Due to the nature of changes since 1.3 users might experience that their settings have vanished or are incorrect. Please revalidate your settings or even create a clean profile to work off of.

## Changelog

* Added: Experimental Full Range encoding support for certain hardware and drivers.
* Fixed: Color Profile and Range will now be taken from OBS Studios Advanced settings.
* Fixed: Stable APU drivers should now work fine again.
* Fixed: Submission queue should no longer fill up when first starting encoding by waiting up to 5 seconds for the first frame to actually be submitted.
* Changed: Presets will disable options that they override now and limit changeable within the allowed range.
* Changed: Default of VBV Buffer Strictness is now 0%.
* Changed: All presets no longer override VBV Buffer properties.