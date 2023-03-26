cmake_minimum_required(VERSION 3.24...3.25)

legacy_check()

add_library(enc-amf MODULE)
add_library(OBS::enc-amf ALIAS enc-amf)

target_sources(
  enc-amf
  PRIVATE include/amf.hpp
          include/amf-capabilities.hpp
          include/amf-encoder.hpp
          include/amf-encoder-h264.hpp
          include/enc-h264.hpp
          include/amf-encoder-h265.hpp
          include/enc-h265.hpp
          include/api-base.hpp
          include/api-host.hpp
          include/api-opengl.hpp
          include/utility.hpp
          include/plugin.hpp
          include/strings.hpp
          source/amf.cpp
          source/amf-capabilities.cpp
          source/amf-encoder.cpp
          source/amf-encoder-h264.cpp
          source/enc-h264.cpp
          source/amf-encoder-h265.cpp
          source/enc-h265.cpp
          source/api-base.cpp
          source/api-host.cpp
          source/api-opengl.cpp
          source/utility.cpp
          source/plugin.cpp
          include/api-d3d9.hpp
          include/api-d3d11.hpp
          source/api-d3d9.cpp
          source/api-d3d11.cpp)

target_include_directories(enc-amf PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"
                                           "${CMAKE_CURRENT_SOURCE_DIR}/AMF/amf/public/include")

set(_VERSION_MAJOR 2)
set(_VERSION_MINOR 8)
set(_VERSION_PATCH 0)

configure_file(include/version.hpp.in version.hpp)
target_sources(enc-amf PRIVATE version.hpp)

configure_file(cmake/windows/obs-module.rc.in enc-amf.rc)
target_sources(enc-amf PRIVATE enc-amf.rc)

target_compile_options(enc-amf PRIVATE /wd4828)

target_link_libraries(enc-amf PRIVATE OBS::libobs version winmm)

set_target_properties_obs(enc-amf PROPERTIES PREFIX "" FOLDER plugins)
