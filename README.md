# AMD Encoder for OBS Studio
'obs-amd-encoder' is a plugin for [Open Broadcaster Software Studio](https://obsproject.com/) which adds support for native AMD Hardware encoding through the use of [AMDs Advanced Media Framework](https://github.com/GPUOpen-LibrariesAndSDKs/AMF). It offers a user-friendly UI integration for the complex settings that AMD AMF offers.

This plugin is integrated in OBS Studio since Version 0.16.2. It replaced the older MFT (Media Foundation Transforms) based approach and performs much better compared to it. Many users have since then switched from software to hardware encoding on AMD machines as it is actually working at a decent performance now.

## Build Status
| Jenkins | AppVeyor |
|---------|----------|
| [![Build Status](https://ci.xaymar.com/job/Xaymar/job/obs-amd-encoder/job/master/badge/icon)](https://ci.xaymar.com/job/Xaymar/job/obs-amd-encoder/job/master/) | [![AppVeyor Status](https://ci.appveyor.com/api/projects/status/github/Xaymar/obs-amd-encoder?branch=master&svg=true)](https://ci.appveyor.com/project/Xaymar/obs-amd-encoder) |

## Installation
The plugin ships with an Installer which will try to find the installation directory of OBS Studio and replaces the integrated version. You will also need the following:

* Windows 7, Windows 8, Windows 10 or newer
* A [supported AMD GPU or APU](https://github.com/obsproject/obs-amd-encoder/wiki/Hardware-Support)
* [Microsoft Visual C++ 2017 Redistributables](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads) (32-bit and 64-bit)
* Latest [AMD Graphics Driver](https://support.amd.com/en-us/download)
* Latest [Open Broadcaster Software Studio](https://obsproject.com/)

## More Information and Troubleshooting
Up to date information can always be found [on the Wiki](https://github.com/obsproject/obs-amd-encoder/wiki), including a [Troubleshooting Guide](https://github.com/obsproject/obs-amd-encoder/wiki/Guide%3A-Troubleshooting) that you should always follow if you have any issues!

# Special Thanks
Special Thanks go to all the people that have enabled me to actually write this plugin:

* /u/PeteRaw for [the suggestion to create this](https://www.reddit.com/r/Amd/comments/4s38ju/amd_should_officially_and_financially_support_the/d5669tb/).
* [/r/AMD](https://www.reddit.com/r/Amd/) and the [OBS Project Community](https://obsproject.com/forum/) for the continued help and support.
* All the Patreon Subscribers: Anaz Haidhar, AJ, Benhamin Hoffmeister, Bo, Bryan Furia, DaOrgest, Dominik roth, Jeremy "razorlikes" Nieth, Kristian Kirkesæther, Kuo Sith, Kytos, Nicholas Kreimeyer, Nico Thate, NoxiousPluK, nwgat.ninj, Oldgam3r, Omega Drik Mage, prefixs, Rene "vDex" Dirks, shiny, Simon Vacker, SneakyJoe, Spikeypop, Vinicius Guilherme.
* All the Translators (before CrowdIn): max20091, Marcos Vidal Martinez, Viacheslav, nwgat, wazer, Horváth Dániel, M4RK22.
* All the Translators on CrowdIn.
* All [contributors to the project](https://github.com/obsproject/obs-amd-encoder/graphs/contributors).
* Richard Stanway for various fixes and improvements.
* Advanced Micro Devices (TM) for releasing Advanced Media Framework SDK and keeping hardware encoding supported on their platform.
* Mikhail Mironov (AMD Employee) for continued development support and test hardware.
* jackun for the VCE Fork of Open Broadcaster Software Classic.
* jp9000/Jim for Open Broadcaster Software Studio and continued development help.
