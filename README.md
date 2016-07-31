# OBS-AMD-Media-Framework
AMF support for OBS via plugins

# Contributing

## Pre-requisites

* Visual Studio 2015 Community (or similar)
* Windows 10 SDK (Bundled with Visual Studio 2015)
* [AMD Media SDK](http://developer.amd.com/tools-and-sdks/media-sdk/), Version 1.1
* Newest [AMF Binaries](http://www.amd.com/en-us/innovations/software-technologies/enhanced-media)

## Getting Started

1. Clone to your Disk
2. Create a soft link (directory junction) to the AMD Media SDK root as /#ThirdParty/AMD-Media-SDK.  
Example: AMD-Media-SDK -> C:\SDKs\AMD\AMD Media SDK
3. Create a soft link (directory junction) to the OBS Studio Source Code as /#ThirdParty/OBS-Studio.
3.1 Build OBS Studio with the same Visual Studio version if you didn't yet.
4. Open the Solution and build the Solution.
5. Binaries should be located in /#Build/$(Configuration)/

## Coding Standard

The coding formatting is currently loosely based on [Googles C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

# Notes

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