# A Plugin that integrates the AMD AMF encoder into OBS Studio Copyright (C) 2016 - 2017 Michael Fabian Dirks
#
# This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with this program; if not, write to the Free
# Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA

# CMake Setup
cmake_minimum_required(VERSION 3.1.0)
include("cmake/util.cmake")

# Automatic Versioning
set(VERSION_MAJOR 2)
set(VERSION_MINOR 8)
set(VERSION_PATCH 0)
set(VERSION_TWEAK 0)
set(PROJECT_COMMIT "N/A")
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git")
  set(GIT_RESULT "")
  set(GIT_OUTPUT "")
  execute_process(
    COMMAND git rev-list --count --topo-order ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}..HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    RESULT_VARIABLE GIT_RESULT
    OUTPUT_VARIABLE GIT_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
  if(GIT_RESULT EQUAL 0)
    set(VERSION_TWEAK ${GIT_OUTPUT})
  endif()
  execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    RESULT_VARIABLE GIT_RESULT
    OUTPUT_VARIABLE GIT_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
  if(GIT_RESULT EQUAL 0)
    set(PROJECT_COMMIT ${GIT_OUTPUT})
  endif()
endif()

# Define Project
project(enc-amf VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK})

# ######################################################################################################################
# CMake / Compiler
# ######################################################################################################################

# Detect Build Type
if("${CMAKE_SOURCE_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
  set(PropertyPrefix "")
else()
  set(PropertyPrefix "${PROJECT_NAME}_")
endif()

# Detect Architecture
math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")
if("${BITS}" STREQUAL "32")
  set(ARCH "x86")
else()
  set(ARCH "x64")
endif()

# Configure Installer script
configure_file("${PROJECT_SOURCE_DIR}/ci/installer.in.iss" "${PROJECT_BINARY_DIR}/ci/installer.iss")

# Configure Version Header
configure_file("${PROJECT_SOURCE_DIR}/include/version.hpp.in" "${PROJECT_BINARY_DIR}/include/version.hpp")

# Windows Specific Resource Definition
if(WIN32)
  set(PROJECT_PRODUCT_NAME "OBS Studio AMD Encoder")
  set(PROJECT_DESCRIPTION "")
  set(PROJECT_COMPANY_NAME "Xaymar")
  set(PROJECT_COPYRIGHT "Xaymar © 2016 - 2018")
  set(PROJECT_LEGAL_TRADEMARKS_1
      "Advanced Micro Devices, AMD, AMD Ryzen, Ryzen, AMD Radeon and Radeon are Trademarks of Advanced Micro Devices.")
  set(PROJECT_LEGAL_TRADEMARKS_2 "")
  set(PROJECT_DESCRIPTION "AMD Encoder integration for OBS Studio")

  configure_file("${PROJECT_SOURCE_DIR}/cmake/version.rc.in" "${PROJECT_BINARY_DIR}/cmake/version.rc" @ONLY)
endif()

# All Warnings, Extra Warnings, Pedantic
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wall -Wno-missing-braces -Wmissing-field-initializers -Wno-c++98-compat-pedantic -Wold-style-cast -Wno-documentation -Wno-documentation-unknown-command -Wno-covered-switch-default -Wno-switch-enum"
  )
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # GCC: -fpermissive is required as GCC does not allow the same template to be in different namespaces.
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -fpermissive -Wno-long-long -Wno-missing-braces -Wmissing-field-initializers"
  )
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
endif()
# C++ Standard and Extensions Use C++17 and no non-standard extensions.
set(_CXX_STANDARD 17)
set(_CXX_EXTENSIONS OFF)

math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")

# ######################################################################################################################
# Options
# ######################################################################################################################
set(${PropertyPrefix}OBS_NATIVE
    FALSE
    CACHE BOOL "Use native obs-studio build" FORCE)
set(${PropertyPrefix}OBS_REFERENCE
    FALSE
    CACHE BOOL "Use referenced obs-studio build" FORCE)
set(${PropertyPrefix}OBS_PACKAGE
    FALSE
    CACHE BOOL "Use packaged obs-studio build" FORCE)
set(${PropertyPrefix}OBS_DOWNLOAD
    FALSE
    CACHE BOOL "Use downloaded obs-studio build" FORCE)
mark_as_advanced(FORCE OBS_NATIVE OBS_PACKAGE OBS_REFERENCE OBS_DOWNLOAD)

if(NOT TARGET libobs)
  set(${PropertyPrefix}OBS_STUDIO_DIR
      ""
      CACHE PATH "OBS Studio Source/Package Directory")
  set(${PropertyPrefix}OBS_DOWNLOAD_VERSION
      "24.0.1-ci"
      CACHE STRING "OBS Studio Version to download")
endif()

if(NOT ${PropertyPrefix}OBS_NATIVE)
  set(${PropertyPrefix}OBS_DEPENDENCIES_DIR
      ""
      CACHE PATH "Path to OBS Dependencies")
  set(CMAKE_PACKAGE_PREFIX
      "${CMAKE_BINARY_DIR}"
      CACHE PATH "Path for generated archives.")
  set(CMAKE_PACKAGE_NAME
      "${PROJECT_NAME}"
      CACHE STRING "Name for the generated archives.")
  set(CMAKE_PACKAGE_SUFFIX_OVERRIDE
      ""
      CACHE STRING "Override for the suffix.")
endif()

# ######################################################################################################################
# Dependencies
# ######################################################################################################################

# Detect OBS Studio Type
if(TARGET libobs)
  message(STATUS "${PROJECT_NAME}: Using native obs-studio.")
  cacheset(${PropertyPrefix}OBS_NATIVE TRUE)
else()
  cacheset(${PropertyPrefix}OBS_NATIVE FALSE)
  if(EXISTS "${OBS_STUDIO_DIR}/cmake/LibObs/LibObsConfig.cmake")
    message(STATUS "${PROJECT_NAME}: Using packaged obs-studio.")
    cacheset(${PropertyPrefix}OBS_PACKAGE TRUE)
  elseif(EXISTS "${OBS_STUDIO_DIR}/libobs/obs-module.h")
    message(STATUS "${PROJECT_NAME}: Using referenced obs-studio.")
    cacheset(${PropertyPrefix}OBS_REFERENCE TRUE)
  else()
    message(STATUS "${PROJECT_NAME}: No OBS Studio detected, using downloadable prebuilt binaries.")
    cacheset(${PropertyPrefix}OBS_DOWNLOAD TRUE)
    if(WIN32)
      set(${PropertyPrefix}OBS_DOWNLOAD_URL
          "https://github.com/Xaymar/obs-studio/releases/download/${OBS_DOWNLOAD_VERSION}/obs-studio-${ARCH}-0.0.0.0-vs2017.7z"
      )
    elseif(UNIX)
      set(${PropertyPrefix}OBS_DOWNLOAD_URL
          "https://github.com/Xaymar/obs-studio/releases/download/${OBS_DOWNLOAD_VERSION}/obs-studio-${ARCH}-0.0.0.0-gcc.7z"
      )
    endif()
  endif()
endif()

if(NOT ${PropertyPrefix}OBS_NATIVE)
  set(CMAKE_PACKAGE_PREFIX
      "${CMAKE_BINARY_DIR}"
      CACHE PATH "Path for generated archives.")
  set(CMAKE_PACKAGE_NAME
      "${PROJECT_NAME}"
      CACHE STRING "Name for the generated archives.")
  set(CMAKE_PACKAGE_SUFFIX_OVERRIDE
      ""
      CACHE STRING "Override for the suffix.")
endif()

# CMake Modules
if(${PropertyPrefix}OBS_DOWNLOAD)
  include("cmake/DownloadProject.cmake")
endif()
if(NOT ${PropertyPrefix}OBS_NATIVE)
  include("cmake/cppcheck.cmake")
endif()

# Load OBS Studio
if(${PropertyPrefix}OBS_NATIVE)
  option(BUILD_AMD_ENCODER "Build AMD Encoder module" ON)
  if(NOT BUILD_AMD_ENCODER)
    message(STATUS "Not building AMD Encoder")
    return()
  endif()
elseif(${PropertyPrefix}OBS_PACKAGE)
  include("${OBS_STUDIO_DIR}/cmake/LibObs/LibObsConfig.cmake")
elseif(${PropertyPrefix}OBS_REFERENCE)
  set(obsPath "${OBS_STUDIO_DIR}")
  include("${OBS_STUDIO_DIR}/cmake/external/Findlibobs.cmake")
elseif(${PropertyPrefix}OBS_DOWNLOAD)
  download_project(PROJ libobs URL ${OBS_DOWNLOAD_URL} UPDATE_DISCONNECTED 1)
  include("${libobs_SOURCE_DIR}/cmake/LibObs/LibObsConfig.cmake")
else()
  message(CRITICAL "Impossible case reached, very system stability.")
  return()
endif()

# ######################################################################################################################
# Code
# ######################################################################################################################
set(PROJECT_HEADERS
    "${PROJECT_SOURCE_DIR}/include/amf.hpp"
    "${PROJECT_SOURCE_DIR}/include/amf-capabilities.hpp"
    "${PROJECT_SOURCE_DIR}/include/amf-encoder.hpp"
    "${PROJECT_SOURCE_DIR}/include/amf-encoder-h264.hpp"
    "${PROJECT_SOURCE_DIR}/include/enc-h264.hpp"
    "${PROJECT_SOURCE_DIR}/include/amf-encoder-h265.hpp"
    "${PROJECT_SOURCE_DIR}/include/enc-h265.hpp"
    "${PROJECT_SOURCE_DIR}/include/api-base.hpp"
    "${PROJECT_SOURCE_DIR}/include/api-host.hpp"
    "${PROJECT_SOURCE_DIR}/include/api-opengl.hpp"
    "${PROJECT_SOURCE_DIR}/include/utility.hpp"
    "${PROJECT_SOURCE_DIR}/include/plugin.hpp"
    "${PROJECT_SOURCE_DIR}/include/strings.hpp"
    "${PROJECT_BINARY_DIR}/include/version.hpp")
set(PROJECT_SOURCES
    "${PROJECT_SOURCE_DIR}/source/amf.cpp"
    "${PROJECT_SOURCE_DIR}/source/amf-capabilities.cpp"
    "${PROJECT_SOURCE_DIR}/source/amf-encoder.cpp"
    "${PROJECT_SOURCE_DIR}/source/amf-encoder-h264.cpp"
    "${PROJECT_SOURCE_DIR}/source/enc-h264.cpp"
    "${PROJECT_SOURCE_DIR}/source/amf-encoder-h265.cpp"
    "${PROJECT_SOURCE_DIR}/source/enc-h265.cpp"
    "${PROJECT_SOURCE_DIR}/source/api-base.cpp"
    "${PROJECT_SOURCE_DIR}/source/api-host.cpp"
    "${PROJECT_SOURCE_DIR}/source/api-opengl.cpp"
    "${PROJECT_SOURCE_DIR}/source/utility.cpp"
    "${PROJECT_SOURCE_DIR}/source/plugin.cpp")
set(PROJECT_DATA "${PROJECT_SOURCE_DIR}/resources/locale/en-US.ini" "${PROJECT_SOURCE_DIR}/LICENSE")
set(PROJECT_LIBRARIES version winmm)

if(WIN32) # Windows Only
  list(APPEND PROJECT_HEADERS "include/api-d3d9.hpp" "include/api-d3d11.hpp")
  list(APPEND PROJECT_SOURCES "source/api-d3d9.cpp" "source/api-d3d11.cpp" "${PROJECT_BINARY_DIR}/cmake/version.rc")
endif()

# Source Grouping
source_group("Data Files" FILES ${enc-amf_DATA})

# ######################################################################################################################
# Target
# ######################################################################################################################

add_library(${PROJECT_NAME} MODULE ${PROJECT_HEADERS} ${PROJECT_SOURCES} ${PROJECT_DATA})
if(${PropertyPrefix}OBS_NATIVE)
  set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "plugins/enc-amf")
endif()

# Include Directories
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC "${PROJECT_BINARY_DIR}/include" "${PROJECT_SOURCE_DIR}/include" "${PROJECT_SOURCE_DIR}/AMF/amf/public/include"
  PRIVATE "${PROJECT_BINARY_DIR}/source" "${PROJECT_SOURCE_DIR}/source" "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}"
          "${CMAKE_SOURCE_DIR}")

# OBS Studio
if(${PropertyPrefix}OBS_NATIVE)
  target_link_libraries(${PROJECT_NAME} libobs)
elseif(${PropertyPrefix}OBS_REFERENCE)
  target_include_directories(${PROJECT_NAME} PRIVATE "${OBS_STUDIO_DIR}/libobs")
  target_link_libraries(${PROJECT_NAME} "${LIBOBS_LIB}")
elseif(${PropertyPrefix}OBS_PACKAGE)
  target_include_directories(${PROJECT_NAME} PRIVATE "${OBS_STUDIO_DIR}/include")
  target_link_libraries(${PROJECT_NAME} libobs)
elseif(${PropertyPrefix}OBS_DOWNLOAD)
  target_link_libraries(${PROJECT_NAME} libobs)
endif()

# Link Libraries
target_link_libraries(${PROJECT_NAME} "${PROJECT_LIBRARIES}")

# Definitions
if(WIN32)
  target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE _CRT_SECURE_NO_WARNINGS
            # windows.h
            WIN32_LEAN_AND_MEAN
            NOGPICAPMASKS
            NOVIRTUALKEYCODES
            # NOWINMESSAGES
            NOWINSTYLES
            NOSYSMETRICS
            NOMENUS
            NOICONS
            NOKEYSTATES
            NOSYSCOMMANDS
            NORASTEROPS
            NOSHOWWINDOW
            NOATOM
            NOCLIPBOARD
            NOCOLOR
            NOCTLMGR
            NODRAWTEXT
            # NOGDI
            NOKERNEL
            # NOUSER NONLS
            NOMB
            NOMEMMGR
            NOMETAFILE
            NOMINMAX
            # NOMSG
            NOOPENFILE
            NOSCROLL
            NOSERVICE
            NOSOUND
            # NOTEXTMETRIC
            NOWH
            NOWINOFFSETS
            NOCOMM
            NOKANJI
            NOHELP
            NOPROFILER
            NODEFERWINDOWPOS
            NOMCX
            NOIME
            NOMDI
            NOINOUT)
endif()

# File Version
if(WIN32)
  set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES VERSION
               ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
               SOVERSION
               ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK})
else()
  set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES VERSION
               ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
               SOVERSION
               ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK})
endif()

# CPPCheck
if(NOT ${PropertyPrefix}OBS_NATIVE)
  set(excludes)
  list(APPEND excludes "${PROJECT_SOURCE_DIR}/AMF")
  if(${PropertyPrefix}OBS_REFERENCE)
    list(APPEND excludes "${OBS_STUDIO_DIR}/libobs")
  elseif(${PropertyPrefix}OBS_PACKAGE)
    list(APPEND excludes "${OBS_STUDIO_DIR}/libobs")
  elseif(${PropertyPrefix}OBS_DOWNLOAD)
    list(APPEND excludes "${libobs_SOURCE_DIR}")
  endif()

  cppcheck(EXCLUDE ${excludes})
  cppcheck_add_project(${PROJECT_NAME})
endif()

# ######################################################################################################################
# Installation
# ######################################################################################################################

if(${PropertyPrefix}OBS_NATIVE)
  install_obs_plugin_with_data(${PROJECT_NAME} resources)
else()
  install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "./obs-plugins/${BITS}bit/" COMPONENT Runtime
    LIBRARY DESTINATION "./obs-plugins/${BITS}bit/" COMPONENT Runtime)
  if(MSVC)
    install(
      FILES $<TARGET_PDB_FILE:${PROJECT_NAME}>
      DESTINATION "./obs-plugins/${BITS}bit/"
      OPTIONAL)
  endif()

  install(DIRECTORY "${PROJECT_SOURCE_DIR}/resources/" DESTINATION "./data/obs-plugins/${PROJECT_NAME}/")

  if("${CMAKE_PACKAGE_SUFFIX_OVERRIDE}" STREQUAL "")
    set(PackageFullName "${CMAKE_PACKAGE_PREFIX}/${CMAKE_PACKAGE_NAME}-${PROJECT_VERSION}")
  else()
    set(PackageFullName "${CMAKE_PACKAGE_PREFIX}/${CMAKE_PACKAGE_NAME}-${CMAKE_PACKAGE_SUFFIX_OVERRIDE}")
  endif()

  add_custom_target(
    PACKAGE_7Z
    ${CMAKE_COMMAND}
    -E
    tar
    cfv
    "${PackageFullName}.7z"
    --format=7zip
    --
    "${CMAKE_INSTALL_PREFIX}/obs-plugins"
    "${CMAKE_INSTALL_PREFIX}/data"
    WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
  add_custom_target(
    PACKAGE_ZIP
    ${CMAKE_COMMAND}
    -E
    tar
    cfv
    "${PackageFullName}.zip"
    --format=zip
    --
    "${CMAKE_INSTALL_PREFIX}/obs-plugins"
    "${CMAKE_INSTALL_PREFIX}/data"
    WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}")
endif()

# ######################################################################################################################
# Child Projects
# ######################################################################################################################

# Sub Project
add_subdirectory(amf-test)
