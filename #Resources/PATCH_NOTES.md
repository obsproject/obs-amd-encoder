# 2.3.0 - A fix for many Crashes
And what a fix it is. With this version, the plugin will refuse to load if the AMF Runtime exposed by the AMD driver is too old, thus preventing startup crashes, runtime crashes, broken output and other weird stuff that AMD has released.

That means from this version on, all Drivers that expose an AMF Runtime older than 1.4.6.0 will no longer load, and if you no longer have the AMD Encoder listed it means it's time for you to update your drivers. If you absolutely have to use an older driver, you always have the option to install an older one from the plugin releases.

Sorry everyone, this is just how it has to be until AMD Drivers are better in code quality and crash less.

## Changelog
### 2.3.0
* Added a blacklist for drivers exposing an AMF Runtime older than 1.4.6.0.
* Removed all hidden properties no longer exposed by the newest AMF Runtime.