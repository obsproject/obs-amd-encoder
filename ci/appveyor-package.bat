cd /D build\distrib
ECHO -- Building 7z Archive --
7z a -t7z -mx=9 -m0=lzma2 -aoa -mfb=64 -md=32m -ms=on "../%PACKAGE_NAME%.7z" * > nul
ECHO -- Building Zip Archive --
7z a -tzip -mx=9 "../%PACKAGE_NAME%.zip" * > nul
ECHO -- Building Installer --
"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" /Qp "..\64\ci\installer.iss" > nul