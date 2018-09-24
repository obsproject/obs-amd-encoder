# 2.5.0 - New AMF SDK and code cleanup
The plugin has been updated to the new 1.4.9.0 AMF SDK, which should make newer drivers work better with the plugin. Additionally some code cleanup has been done in order to reduce the work necessary for a Linux supporting build in the future - however there is no clear date on this yet. Various other errors were also fixed that could have caused crashes during startup, encoding and shutdown.

For developers: The project now has clang-format support and cppcheck built in, which should reduce the amount of coding errors happening. Jenkins is now partially supported as a CI, once the libvirt plugin support Pipeline projects it will be fully supported. CMake can now tell apart commit versions, but that is only used for the plugin version string in version.h.in.

## Changelog
### 2.5.0
* Updated AMF to 1.4.9.0.
* Updated english locale text for Pre-Pass to include encoding cost.
* Fixed various erroneous usages of short codes for printf in log text.
* Added support for Git commit detection to CMake.
* Added support for clang-format.
* Added support for CppCheck to CMake. (Thanks to Streamlabs for this one)
