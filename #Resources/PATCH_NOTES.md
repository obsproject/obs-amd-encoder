# 2.3.1 - The Blacklist and imroved support for Replay Buffer
It unfortunately had to come to this. The plugin now finally blacklists older drivers, as it otherwise would turn into an unmaintainable mess trying to support everything back to 16.11.5, the first driver that had AMD AMF support. That means that from this day on, all future versions will only work with a minimum runtime version of 1.4.6.0. So if you're still using an older driver: What are you doing? Update already!

In addition to that, the plugin should now work better with the new networking code as well as replay buffer. The 'Target Bitrate' property is now renamed internally so that external code can properly read it.

## Changelog
### 2.3.1
* Skipped 2.3.0 as it was not included with obs-studio.
* Improved support for the "New Networking Code" and "Replay Buffer" features by renaming 'Bitrate.Target' to 'bitrate'.

### 2.3.0
* Added a blacklist for drivers exposing an AMF Runtime older than 1.4.6.0.
* Removed all hidden properties no longer exposed by the newest AMF Runtime.