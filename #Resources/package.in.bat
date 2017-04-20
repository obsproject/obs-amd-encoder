@ECHO OFF
REM Shut up, I'm just as lazy as anyone else.
SET "CURDIR=%~dp0"
SET "FINALNAME=AMD-Encoder-for-OBS-Studio.@enc-amf_VERSION_MAJOR@.@enc-amf_VERSION_MINOR@.@enc-amf_VERSION_PATCH@"

SET "SevenZip=C:\Program Files\7-Zip\7z.exe"
SET "InnoSetup=C:\Program Files (x86)\Inno Setup 5\Compil32.exe"

PUSHD %CURDIR%

PUSHD ..\#Build\
"%SevenZip%" a -t7z -mx=9 -m0=lzma2 -aoa -mfb=64 -md=32m -ms=on "..\#Resources\Output\%FINALNAME%.7z" "*"
"%SevenZip%" a -tzip -mx=9 "..\#Resources\Output\%FINALNAME%.zip" "*"
POPD

"%InnoSetup%" /cc "Installer.iss"

POPD
