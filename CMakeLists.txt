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

# Since the locale files are in "resources" instead of "data", handle locale files install/copy here.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources")
  file(GLOB_RECURSE data_files "${CMAKE_CURRENT_SOURCE_DIR}/resources/*")
  foreach(data_file IN LISTS data_files)
    cmake_path(RELATIVE_PATH data_file BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/" OUTPUT_VARIABLE
               relative_path)
    cmake_path(GET relative_path PARENT_PATH relative_path)
    target_sources(enc-amf PRIVATE "${data_file}")
    source_group("Resources/${relative_path}" FILES "${data_file}")
  endforeach()

  set(target_destination "${OBS_DATA_DESTINATION}/obs-plugins/enc-amf")

  install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/"
    DESTINATION "${target_destination}"
    USE_SOURCE_PERMISSIONS
    COMPONENT Runtime)

  add_custom_command(
    TARGET enc-amf
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${OBS_OUTPUT_DIR}/$<CONFIG>/${target_destination}"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources"
            "${OBS_OUTPUT_DIR}/$<CONFIG>/${target_destination}"
    COMMENT "Copy enc-amf resources to data directory"
    VERBATIM)
endif()

set(PROJECT_VERSION_MAJOR 2)
set(PROJECT_VERSION_MINOR 8)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION_TWEAK 1)

configure_file(include/version.hpp.in version.hpp)
target_sources(enc-amf PRIVATE version.hpp)

configure_file(cmake/windows/obs-module.rc.in enc-amf.rc)
target_sources(enc-amf PRIVATE enc-amf.rc)

target_compile_options(enc-amf PRIVATE /wd4828)

target_link_libraries(enc-amf PRIVATE OBS::libobs version winmm)

set_target_properties_obs(enc-amf PROPERTIES PREFIX "" FOLDER plugins/enc-amf)

add_executable(enc-amf-test)

target_compile_definitions(
  enc-amf-test
  PRIVATE # cmake-format: sortable
          LITE_OBS
          NOATOM
          NOCLIPBOARD
          NOCOLOR
          NOCOMM
          NOCTLMGR
          NODEFERWINDOWPOS
          NODRAWTEXT
          NOGPICAPMASKS
          NOHELP
          NOICONS
          NOIME
          NOINOUT
          NOKANJI
          NOKERNEL
          NOKEYSTATES
          NOMB
          NOMCX
          NOMDI
          NOMEMMGR
          NOMENUS
          NOMETAFILE
          NOMINMAX
          NOOPENFILE
          NOPROFILER
          NORASTEROPS
          NOSCROLL
          NOSERVICE
          NOSHOWWINDOW
          NOSOUND
          NOSYSCOMMANDS
          NOSYSMETRICS
          NOVIRTUALKEYCODES
          NOWH
          NOWINOFFSETS
          NOWINSTYLES
          WIN32_LEAN_AND_MEAN)

target_sources(
  enc-amf-test
  PRIVATE amf-test/main.cpp
          source/amf.cpp
          source/amf-capabilities.cpp
          source/amf-encoder.cpp
          source/amf-encoder-h264.cpp
          source/amf-encoder-h265.cpp
          source/api-base.cpp
          source/api-d3d9.cpp
          source/api-d3d11.cpp
          source/utility.cpp
          include/amf.hpp
          include/amf-capabilities.hpp
          include/amf-encoder.hpp
          include/amf-encoder-h264.hpp
          include/amf-encoder-h265.hpp
          include/api-base.hpp
          include/api-d3d9.hpp
          include/api-d3d11.hpp
          include/utility.hpp)

target_include_directories(enc-amf-test PRIVATE amf-test include "${CMAKE_CURRENT_BINARY_DIR}/include" source
                                                AMF/amf/public/include)

target_link_libraries(enc-amf-test version winmm)

# cmake-format: off
set_target_properties_obs(
  enc-amf-test
  PROPERTIES FOLDER plugins/enc-amf
             OUTPUT_NAME enc-amf-test64)
# cmake-format: on

add_dependencies(enc-amf enc-amf-test)
