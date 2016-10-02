So you want to get started with helping? Well then read this carefully.

# Coding Standard

The entire Project is written in C++ and as such follows common C++ writing practices.

## Namespaces

Namespaces should be named in *Pascal Case*. You may create a nested namespace that is 3 levels deep, not more.

## Enumerations

Enumerations should be named in *Pascal Case* and be inside a namespace or class.  
Enumeration Constants should always be prefixed by the enumeration name so no ambiguity exists.

## Functions

Functions should be named in *Pascal Case* and should always be inside a namespace or class.
Ideally your function should be either inlinable or have parameters passed by reference for optimization.

## Classes

Classes should be named in *Pascal Case* and should always be inside a namespace - a class not in a namespace must be really important to everything.

### Methods (Class-specific Functions)

Methods should be named in *Pascal Case* and always reside in the class it belongs to.

### Members (Class-specific Variables)

Members should be prefixed by 'm_' and then use a *Pascal Case* name. There is no special prefix per-type, per-usage or per-source.

## Variables

Variables should be named in *camel Case*


# Getting Started (Building)

## Building Pre-Requisites
* Visual Studio® 2013 or Visual Studio® 2015 (Community or better)
* Windows 10 SDK (Version 10586 or better)
* [OBS Studio](https://github.com/jp9000/obs-studio)
* [AMF SDK](https://github.com/GPUOpen-LibrariesAndSDKs/AMF)
* Time


# Getting Started (Testing)

## Testing Pre-Requisites
* [AMD Radeon GPU or APU with VCE core](https://github.com/Xaymar/OBS-AMD-Advanced-Media-Framework/wiki/Hardware,-GCN-and-VCE-Limits)
* Latest AMD Driver
* Windows® 7 (SP1 with the Platform Update), Windows® 8.1, or Windows® 10

## Getting Started
1. Clone the Project to your Disk
2. Set up any #ThirdParty directory junctions, see the README.md inside the folder for more info.
3. Open the Solution
4. Build the Solution (or Batch Build)
5. Binaries should be located in /#Build/$(Configuration)/

## Committing Changes

Read [this wiki page for information](https://github.com/Xaymar/OBS-AMD-Advanced-Media-Framework/wiki/Contributing) and please follow it. Rebasing a bunch of pushed commits is not good.