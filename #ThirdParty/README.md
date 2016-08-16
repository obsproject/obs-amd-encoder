# Third Party Content for the OBS AMF Plugin

## OBS Studio Source & Build Files

1. Clone the OBS Studio Source Code
2. Build it (both x86 and x64) with the same Visual Studio version you built the plugin with.
3. Create a soft link (directory junction) to the OBS Studio Source Code root named OBS-Studio here.<br>
Ex (Windows): mklink /J OBS-Studio C:/Src/OBS/

## AMD AMF SDK

1. Clone the [AMD AMF SDK](https://github.com/GPUOpen-LibrariesAndSDKs/AMF)
2. Create a Soft Link (Directory Junction) to <SDK root>/amf/public/ called AMD-AMF-SDK.<br>
Ex (Windows): mklink /J AMD-AMF-SDK C:/Src/AMF/