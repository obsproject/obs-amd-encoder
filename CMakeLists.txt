# A Plugin that integrates the AMD AMF encoder into OBS Studio
# Copyright (C) 2016 - 2017 Michael Fabian Dirks
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

# CMake Setup
CMake_Minimum_Required(VERSION 3.1.0)
Include("cmake/util.cmake")

# Automatic Versioning
Set(VERSION_MAJOR 2)
Set(VERSION_MINOR 7)
Set(VERSION_PATCH 0)
Set(VERSION_TWEAK 0)
Set(PROJECT_COMMIT "N/A")
If(EXISTS "${CMAKE_CURRENT_LIST_DIR}/.git")
	Set(GIT_RESULT "")
	Set(GIT_OUTPUT "")
	EXECUTE_PROCESS(
		COMMAND git rev-list --count --topo-order ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}..HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		RESULT_VARIABLE GIT_RESULT
		OUTPUT_VARIABLE GIT_OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	If(GIT_RESULT EQUAL 0)
		Set(VERSION_TWEAK ${GIT_OUTPUT})
	EndIf()
	EXECUTE_PROCESS(
		COMMAND git rev-parse HEAD
		WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
		RESULT_VARIABLE GIT_RESULT
		OUTPUT_VARIABLE GIT_OUTPUT
		OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE ERROR_QUIET
	)
	If(GIT_RESULT EQUAL 0)
		Set(PROJECT_COMMIT ${GIT_OUTPUT})
	EndIf()
EndIf()

# Define Project
PROJECT(
	enc-amf
	VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}
)

################################################################################
# CMake / Compiler
################################################################################

# Detect Build Type
If("${CMAKE_SOURCE_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
	Set(PropertyPrefix "")
Else()
	Set(PropertyPrefix "${PROJECT_NAME}_")
EndIf()

# Detect Architecture
math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")
IF("${BITS}" STREQUAL "32")
	SET(ARCH "x86")
Else()
	SET(ARCH "x64")
ENDIF()

# Configure Installer script
Configure_File(
	"${PROJECT_SOURCE_DIR}/ci/installer.in.iss"
	"${PROJECT_BINARY_DIR}/ci/installer.iss"
)

# Configure Version Header
Configure_File(
	"${PROJECT_SOURCE_DIR}/include/version.hpp.in"
	"${PROJECT_BINARY_DIR}/include/version.hpp"
)

# Windows Specific Resource Definition
If(WIN32)
	Set(PROJECT_PRODUCT_NAME "OBS Studio AMD Encoder")
	Set(PROJECT_DESCRIPTION "")
	Set(PROJECT_COMPANY_NAME "Xaymar")
	Set(PROJECT_COPYRIGHT "Xaymar © 2016 - 2018")
	Set(PROJECT_LEGAL_TRADEMARKS_1 "Advanced Micro Devices, AMD, AMD Ryzen, Ryzen, AMD Radeon and Radeon are Trademarks of Advanced Micro Devices.")
	Set(PROJECT_LEGAL_TRADEMARKS_2 "")
	Set(PROJECT_DESCRIPTION "AMD Encoder integration for OBS Studio")

	Configure_File(
		"${PROJECT_SOURCE_DIR}/cmake/version.rc.in"
		"${PROJECT_BINARY_DIR}/cmake/version.rc"
		@ONLY
	)
EndIf()

# All Warnings, Extra Warnings, Pedantic
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	# using Clang
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-missing-braces -Wmissing-field-initializers -Wno-c++98-compat-pedantic -Wold-style-cast -Wno-documentation -Wno-documentation-unknown-command -Wno-covered-switch-default -Wno-switch-enum")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# GCC: -fpermissive is required as GCC does not allow the same template to be in different namespaces.
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic -fpermissive -Wno-long-long -Wno-missing-braces -Wmissing-field-initializers")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	# using Intel C++
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
endif()
# C++ Standard and Extensions
## Use C++17 and no non-standard extensions.
set(_CXX_STANDARD 17)
set(_CXX_EXTENSIONS OFF)

math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")

################################################################################
# Options
################################################################################
set(${PropertyPrefix}OBS_NATIVE FALSE CACHE BOOL "Use native obs-studio build" FORCE)
set(${PropertyPrefix}OBS_REFERENCE FALSE CACHE BOOL "Use referenced obs-studio build" FORCE)
set(${PropertyPrefix}OBS_PACKAGE FALSE CACHE BOOL "Use packaged obs-studio build" FORCE)
set(${PropertyPrefix}OBS_DOWNLOAD FALSE CACHE BOOL "Use downloaded obs-studio build" FORCE)
mark_as_advanced(FORCE OBS_NATIVE OBS_PACKAGE OBS_REFERENCE OBS_DOWNLOAD)

if(NOT TARGET libobs)
	set(${PropertyPrefix}OBS_STUDIO_DIR "" CACHE PATH "OBS Studio Source/Package Directory")
	set(${PropertyPrefix}OBS_DOWNLOAD_VERSION "24.0.1-ci" CACHE STRING "OBS Studio Version to download")
endif()

if(NOT ${PropertyPrefix}OBS_NATIVE)
	set(${PropertyPrefix}OBS_DEPENDENCIES_DIR "" CACHE PATH "Path to OBS Dependencies")
	set(CMAKE_PACKAGE_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Path for generated archives.")
	set(CMAKE_PACKAGE_NAME "${PROJECT_NAME}" CACHE STRING "Name for the generated archives.")
	set(CMAKE_PACKAGE_SUFFIX_OVERRIDE "" CACHE STRING "Override for the suffix.")
endif()

################################################################################
# Dependencies
################################################################################

# Detect OBS Studio Type
if(TARGET libobs)
	message(STATUS "${PROJECT_NAME}: Using native obs-studio.")
	CacheSet(${PropertyPrefix}OBS_NATIVE TRUE)
else()
	CacheSet(${PropertyPrefix}OBS_NATIVE FALSE)
	if(EXISTS "${OBS_STUDIO_DIR}/cmake/LibObs/LibObsConfig.cmake")
		message(STATUS "${PROJECT_NAME}: Using packaged obs-studio.")
		CacheSet(${PropertyPrefix}OBS_PACKAGE TRUE)
	elseif(EXISTS "${OBS_STUDIO_DIR}/libobs/obs-module.h")
		message(STATUS "${PROJECT_NAME}: Using referenced obs-studio.")
		CacheSet(${PropertyPrefix}OBS_REFERENCE TRUE)
	else()
		message(STATUS "${PROJECT_NAME}: No OBS Studio detected, using downloadable prebuilt binaries.")
		CacheSet(${PropertyPrefix}OBS_DOWNLOAD TRUE)
		if (WIN32)
			set(${PropertyPrefix}OBS_DOWNLOAD_URL "https://github.com/Xaymar/obs-studio/releases/download/${OBS_DOWNLOAD_VERSION}/obs-studio-${ARCH}-0.0.0.0-vs2017.7z")
		elseif(UNIX)
			set(${PropertyPrefix}OBS_DOWNLOAD_URL "https://github.com/Xaymar/obs-studio/releases/download/${OBS_DOWNLOAD_VERSION}/obs-studio-${ARCH}-0.0.0.0-gcc.7z")
		endif()
	endif()
endif()

If(NOT ${PropertyPrefix}OBS_NATIVE)
	Set(CMAKE_PACKAGE_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Path for generated archives.")
	Set(CMAKE_PACKAGE_NAME "${PROJECT_NAME}" CACHE STRING "Name for the generated archives.")
	Set(CMAKE_PACKAGE_SUFFIX_OVERRIDE "" CACHE STRING "Override for the suffix.")
EndIf()

# CMake Modules
If(${PropertyPrefix}OBS_DOWNLOAD)
	Include("cmake/DownloadProject.cmake")
EndIf()
If(NOT ${PropertyPrefix}OBS_NATIVE)
	Include("cmake/cppcheck.cmake")
EndIf()

# Load OBS Studio
If(${PropertyPrefix}OBS_NATIVE)
	Option(BUILD_AMD_ENCODER "Build AMD Encoder module" ON)
	If (NOT BUILD_AMD_ENCODER)
		Message(STATUS "Not building AMD Encoder")
		Return()
	EndIf()
ElseIf(${PropertyPrefix}OBS_PACKAGE)
	Include("${OBS_STUDIO_DIR}/cmake/LibObs/LibObsConfig.cmake")
ElseIf(${PropertyPrefix}OBS_REFERENCE)
	Set(obsPath "${OBS_STUDIO_DIR}")
	Include("${OBS_STUDIO_DIR}/cmake/external/Findlibobs.cmake")
ElseIf(${PropertyPrefix}OBS_DOWNLOAD)
	download_project(
		PROJ libobs
		URL ${OBS_DOWNLOAD_URL}
		UPDATE_DISCONNECTED 1
	)
	INCLUDE("${libobs_SOURCE_DIR}/cmake/LibObs/LibObsConfig.cmake")
Else()
	Message(CRITICAL "Impossible case reached, very system stability.")
	Return()
EndIf()

################################################################################
# Code
################################################################################
Set(PROJECT_HEADERS
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
	"${PROJECT_BINARY_DIR}/include/version.hpp"
)
Set(PROJECT_SOURCES
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
	"${PROJECT_SOURCE_DIR}/source/plugin.cpp"
)
Set(PROJECT_DATA
	"${PROJECT_SOURCE_DIR}/resources/locale/en-US.ini"
	"${PROJECT_SOURCE_DIR}/LICENSE"
)
Set(PROJECT_LIBRARIES
	version
	winmm
)

If(WIN32) # Windows Only
	LIST(APPEND PROJECT_HEADERS
		"include/api-d3d9.hpp"
		"include/api-d3d11.hpp"
	)
	LIST(APPEND PROJECT_SOURCES
		"source/api-d3d9.cpp"
		"source/api-d3d11.cpp"
		"${PROJECT_BINARY_DIR}/cmake/version.rc"
	)
EndIf()

# Source Grouping
Source_Group("Data Files" FILES ${enc-amf_DATA})

################################################################################
# Target
################################################################################

Add_Library(${PROJECT_NAME} MODULE
	${PROJECT_HEADERS}
	${PROJECT_SOURCES}
	${PROJECT_DATA}
)
If(${PropertyPrefix}OBS_NATIVE)
	Set_Target_Properties(${PROJECT_NAME} PROPERTIES FOLDER "plugins/enc-amf")
EndIf()

# Include Directories
Target_Include_Directories(${PROJECT_NAME}
	PUBLIC
		"${PROJECT_BINARY_DIR}/include"
		"${PROJECT_SOURCE_DIR}/include"
		"${PROJECT_SOURCE_DIR}/AMF/amf/public/include"
	PRIVATE
		"${PROJECT_BINARY_DIR}/source"
		"${PROJECT_SOURCE_DIR}/source"
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_SOURCE_DIR}"
		"${CMAKE_SOURCE_DIR}"
)

# OBS Studio
If(${PropertyPrefix}OBS_NATIVE)
	Target_Link_Libraries(${PROJECT_NAME}
		libobs
	)
ElseIf(${PropertyPrefix}OBS_REFERENCE)
	Target_Include_Directories(${PROJECT_NAME}
		PRIVATE
			"${OBS_STUDIO_DIR}/libobs"
	)
	Target_Link_Libraries(${PROJECT_NAME}
		"${LIBOBS_LIB}"
	)
ElseIf(${PropertyPrefix}OBS_PACKAGE)
	Target_Include_Directories(${PROJECT_NAME}
		PRIVATE
			"${OBS_STUDIO_DIR}/include"
	)
	Target_Link_Libraries(${PROJECT_NAME}
		libobs
	)
ElseIf(${PropertyPrefix}OBS_DOWNLOAD)
	Target_Link_Libraries(${PROJECT_NAME}
		libobs
	)
EndIf()

# Link Libraries
Target_Link_Libraries(${PROJECT_NAME}
	"${PROJECT_LIBRARIES}"
)

# Definitions
If (WIN32)
	Target_Compile_Definitions(${PROJECT_NAME}
		PRIVATE
			_CRT_SECURE_NO_WARNINGS
			# windows.h
			WIN32_LEAN_AND_MEAN
			NOGPICAPMASKS
			NOVIRTUALKEYCODES
			#NOWINMESSAGES
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
			#NOGDI
			NOKERNEL
			#NOUSER
			#NONLS
			NOMB
			NOMEMMGR
			NOMETAFILE
			NOMINMAX
			#NOMSG
			NOOPENFILE
			NOSCROLL
			NOSERVICE
			NOSOUND
			#NOTEXTMETRIC
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
			NOINOUT
	)
EndIf()

# File Version
If(WIN32)
	Set_Target_Properties(
		${PROJECT_NAME}
		PROPERTIES
		VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
		SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
	)
Else()
	Set_Target_Properties(
		${PROJECT_NAME}
		PROPERTIES
		VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
		SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}
	)
EndIf()

# CPPCheck
If(NOT ${PropertyPrefix}OBS_NATIVE)
	SET(excludes )
	LIST(APPEND excludes "${PROJECT_SOURCE_DIR}/AMF")
	If(${PropertyPrefix}OBS_REFERENCE)
		LIST(APPEND excludes "${OBS_STUDIO_DIR}/libobs")
	ElseIf(${PropertyPrefix}OBS_PACKAGE)
		LIST(APPEND excludes "${OBS_STUDIO_DIR}/libobs")
	ElseIf(${PropertyPrefix}OBS_DOWNLOAD)
		LIST(APPEND excludes "${libobs_SOURCE_DIR}")
	EndIf()

	CppCheck(
		EXCLUDE ${excludes}
	)
	CppCheck_Add_Project(${PROJECT_NAME})
EndIf()

################################################################################
# Installation
################################################################################

If(${PropertyPrefix}OBS_NATIVE)
	install_obs_plugin_with_data(${PROJECT_NAME} resources)
Else()
	Install(
		TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION "./obs-plugins/${BITS}bit/" COMPONENT Runtime
		LIBRARY DESTINATION "./obs-plugins/${BITS}bit/" COMPONENT Runtime
	)
	IF(MSVC)
		Install(
			FILES $<TARGET_PDB_FILE:${PROJECT_NAME}>
			DESTINATION "./obs-plugins/${BITS}bit/"
			OPTIONAL
		)
	ENDIF()

	Install(
		DIRECTORY "${PROJECT_SOURCE_DIR}/resources/"
		DESTINATION "./data/obs-plugins/${PROJECT_NAME}/"
	)

	If("${CMAKE_PACKAGE_SUFFIX_OVERRIDE}" STREQUAL "")
		Set(PackageFullName "${CMAKE_PACKAGE_PREFIX}/${CMAKE_PACKAGE_NAME}-${PROJECT_VERSION}")
	Else()
		Set(PackageFullName "${CMAKE_PACKAGE_PREFIX}/${CMAKE_PACKAGE_NAME}-${CMAKE_PACKAGE_SUFFIX_OVERRIDE}")
	EndIf()

	Add_Custom_Target(
		PACKAGE_7Z
		${CMAKE_COMMAND} -E tar cfv "${PackageFullName}.7z" --format=7zip --
			"${CMAKE_INSTALL_PREFIX}/obs-plugins"
			"${CMAKE_INSTALL_PREFIX}/data"
		WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
	)
	Add_Custom_Target(
		PACKAGE_ZIP
		${CMAKE_COMMAND} -E tar cfv "${PackageFullName}.zip" --format=zip --
			"${CMAKE_INSTALL_PREFIX}/obs-plugins"
			"${CMAKE_INSTALL_PREFIX}/data"
		WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}"
	)
EndIf()

################################################################################
# Child Projects
################################################################################

# Sub Project
Add_SubDirectory(amf-test)
