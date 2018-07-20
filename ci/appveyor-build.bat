cmake -H. -B"build/32" -G"Visual Studio 15 2017" -DOBS_STUDIO_DIR="%CD%/%OBS_STUDIO_32%" -DCMAKE_INSTALL_PREFIX="%CD%/build/distrib"
cmake -H. -B"build/64" -G"Visual Studio 15 2017 Win64" -T"host=x64" -DOBS_STUDIO_DIR="%CD%/%OBS_STUDIO_64%" -DCMAKE_INSTALL_PREFIX="%CD%/build/distrib"
cmake --build build/32 --target INSTALL --config RelWithDebInfo
cmake --build build/64 --target INSTALL --config RelWithDebInfo