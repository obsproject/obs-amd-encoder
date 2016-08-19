# AMD Advanced Media Framework Plugin for OBS Studio
This is a Plugin for OBS Studio to enable native AMF integration.

# Troubleshooting

## Frequently Asked Questions
These are common Questions that are repeatedly asked. If you see someone asking one of these questions, link them here.

### Q: I can't find the Encoder!
You need to select "Advanced" as the Output Mode for it to show up.

### Q: There are two encoders called the same, what do I do?!
Select the one that is called "H264 Encoder (AMD Advanced Media Framework)".

## Reporting Issues

Before reporting an Issue, please [read this first](https://github.com/Xaymar/OBS-AMD-Media-Framework/wiki/Reporting-Issues).
You will not get help unless you include all necessary information listed on that page.

# Contributing

## Pre-requisites
* AMD Radeon Software Crimson Edition 16.7.3 (16.30.2311) or newer
* Windows® 7 (SP1 with the Platform Update), Windows® 8.1, or Windows® 10
* Visual Studio® 2013 or Visual Studio® 2015 (Community or better)
* Windows 10 SDK (Version 10586 or better)
* [OBS Studio](https://github.com/jp9000/obs-studio)
* [AMF SDK](https://github.com/GPUOpen-LibrariesAndSDKs/AMF)

## Getting Started
1. Clone the Project to your Disk
2. Set up any #ThirdParty directory junctions, see the README.md inside the folder for more info.
3. Open the Solution
4. Build the Solution (or Batch Build)
5. Binaries should be located in /#Build/$(Configuration)/

# License

## MIT License

Copyright (c) 2016 Michael Fabian Dirks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.