# Contributing
It's great to hear that you want to contribute to the project! So let's get you started.

## Reporting Issues, Requesting Features, etc.
Before you report an issue, please read the [Troubleshooting Guide](https://github.com/Xaymar/obs-studio_amf-encoder-plugin/wiki/Troubleshooting-Guide). It contains a step-by-step guide on how to solve your issue and even includes a FAQ for you to read through. If you request a certain feature, make sure that it is actually possible to implement this feature.

## Changing the Project
The first step to starting off is to get the project building so that you can actually test your changes yourself.

### Prerequisites
* [CMake Build Tool](https://cmake.org/)
* [Visual Studio® 2013](http://visualstudio.com/), [Visual Studio® 2015](http://visualstudio.com/) or newer
* [Open Broadcaster Software Studio](https://github.com/jp9000/obs-studio)
* [AMDs Advanced Media Framework SDK](https://github.com/GPUOpen-LibrariesAndSDKs/AMF)
* Windows 10 SDK (Version 10586 or better) *[Windows Only]*
* Optional: [InnoSetup](http://www.jrsoftware.org/isinfo.php) for building a Standalone Installer

### Step 1: Build Open Broadcaster Studio
This is necessary so the plugin itself has something to link against. Simply build OBS Studio with the same configuration (or do a Batch Build for all configurations) and you are good to go.

### Step 2: Configure the Project
1. Point CMake at the Source Code and tell it to put binaries in /build32 (32-Bit) or /build64 (64-bit).
2. Configure the Project
3. Set PATH_AMDAMFSDK to where you cloned the AMD Advanced Media Framework SDK to.
4. Set PATH_OBSStudio to where you cloned Open Broadcaster Software Studio to.
5. Generate the Project files
6. You should now have a Solution file in the /build32 or /build64 folder.

### Step 3: Building the Project
To verify that everything worked right you should now attempt to build the project with the generated solution file. If something did not work right, make sure that you have all prerequisites and start again from step 1.
Once the building is complete you will notice a new folder called /#Build. This folder will always contain the latest files from a build, no matter the configuration. 

### Step 4: Test the built Project
Copy the files from /#Build to a OBS Studio installation (Portable or Static) and try to use your changes.

## Git and the Project
Version Control plays a large role in managing things and keeping in touch with changes.

### Modules
The plugin itself is made up of several sub-modules:

* AMF: amf.cpp, amf.h, amf-capabilities.cpp, amf-capabilities.h
* Encoder: amf-encoder.cpp, amf-encoder.h
* H264: amf-encoder-h264.cpp, amf-encoder-h264.h, enc-h264.cpp, enc-h264.h
* H265: amf-encoder-h265.cpp, amf-encoder-h265.h, enc-h265.cpp, enc-h265.h
* API: api-base.cpp, api-base.h
* API-OpenGL: api-opengl.cpp, api-opengl.h
* API-Direct3D9: api-d3d9.cpp, api-d3d9.h
* API-Direct3D11: api-d3d11.cpp, api-d3d11.h
* API-Host: api-host.cpp, api-host.h
* Plugin: plugin.cpp, plugin.h, CMakeLists.txt
* Utilities: utility.cpp, utility.h
* Locale: strings.h, Any locale files
* Resources: Any resource files

### Commits
Commits should ideally only contain one group of changes. For example if you change how the H264 Color Format is called, that change would be prefixed by H264 and contain all the files this changes.

#### Messages
The first line of a message should not be longer than 80 characters and be prefixed by the module it changes. If it changes more than one separate them by comma. If it changes all of them use the prefix 'project'.

### Pull Requests

Pull Request titles should follow the guidelines for committing changes and also have a short description of what the change does.

## Coding Standard
The entire Project is written in C++ and as such follows common C++ writing practices.

### Namespaces
Namespaces should be named in *Pascal Case*. You may create a nested namespace that is 3 levels deep, not more.

### Structures and Classes
Structures and Classes should be named using *Pascal Case* and should be inside a namespace. A structure or class not in a namespace must have relevance to the entire code.

#### Methods
Methods should be named in *Pascal Case* and always reside in the class it belongs to.

#### Members
Members should be prefixed by 'm_' and then use a *Pascal Case* name. There is no special prefix per-type, per-usage or per-source.

### Enumerations
Enumerations should be named in *Pascal Case* and be inside a namespace or class.  
Enumeration Constants should always be prefixed by the enumeration name so no ambiguity exists.

### Functions
Functions should be named in *Pascal Case* and should always be inside a namespace or class.
Ideally your function should be either inlinable or have parameters passed by reference for optimization.

### Variables
Variables should be named in *camel Case*.

