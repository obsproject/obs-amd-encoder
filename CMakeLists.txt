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

cmake_minimum_required(VERSION 2.8.12)
PROJECT(enc-amf)

################################################################################
# Version
################################################################################
SET(enc-amf_VERSION_MAJOR 2)
SET(enc-amf_VERSION_MINOR 2)
SET(enc-amf_VERSION_PATCH 3)
configure_file(
	"${PROJECT_SOURCE_DIR}/#Resources/package.in.bat"
	"${PROJECT_SOURCE_DIR}/#Resources/package.bat"
)
configure_file(
	"${PROJECT_SOURCE_DIR}/#Resources/Installer.in.iss"
	"${PROJECT_SOURCE_DIR}/#Resources/Installer.iss"
)
configure_file(
	"${PROJECT_SOURCE_DIR}/Include/Version.h.in"
	"${PROJECT_BINARY_DIR}/Include/Version.h"
)

################################################################################
# Code
################################################################################
SET(enc-amf_HEADERS
	"${PROJECT_SOURCE_DIR}/Include/amf.h"
	"${PROJECT_SOURCE_DIR}/Include/amf-capabilities.h"
	"${PROJECT_SOURCE_DIR}/Include/amf-encoder.h"
	"${PROJECT_SOURCE_DIR}/Include/amf-encoder-h264.h"
	"${PROJECT_SOURCE_DIR}/Include/enc-h264.h"
	"${PROJECT_SOURCE_DIR}/Include/amf-encoder-h265.h"
	"${PROJECT_SOURCE_DIR}/Include/enc-h265.h"
	"${PROJECT_SOURCE_DIR}/Include/api-base.h"
	"${PROJECT_SOURCE_DIR}/Include/api-host.h"
	"${PROJECT_SOURCE_DIR}/Include/api-opengl.h"
	"${PROJECT_SOURCE_DIR}/Include/utility.h"
	"${PROJECT_SOURCE_DIR}/Include/plugin.h"
	"${PROJECT_SOURCE_DIR}/Include/strings.h"
	"${PROJECT_BINARY_DIR}/Include/Version.h"
)
SET(enc-amf_SOURCES
	"${PROJECT_SOURCE_DIR}/Source/amf.cpp"
	"${PROJECT_SOURCE_DIR}/Source/amf-capabilities.cpp"
	"${PROJECT_SOURCE_DIR}/Source/amf-encoder.cpp"
	"${PROJECT_SOURCE_DIR}/Source/amf-encoder-h264.cpp"
	"${PROJECT_SOURCE_DIR}/Source/enc-h264.cpp"
	"${PROJECT_SOURCE_DIR}/Source/amf-encoder-h265.cpp"
	"${PROJECT_SOURCE_DIR}/Source/enc-h265.cpp"
	"${PROJECT_SOURCE_DIR}/Source/api-base.cpp"
	"${PROJECT_SOURCE_DIR}/Source/api-host.cpp"
	"${PROJECT_SOURCE_DIR}/Source/api-opengl.cpp"
	"${PROJECT_SOURCE_DIR}/Source/utility.cpp"
	"${PROJECT_SOURCE_DIR}/Source/plugin.cpp"
)
SET(enc-amf_DATA
	"${PROJECT_SOURCE_DIR}/Resources/locale/en-US.ini"
	"${PROJECT_SOURCE_DIR}/LICENSE"
	"${PROJECT_SOURCE_DIR}/AUTHORS"
)
SET(enc-amf_LIBRARIES
	version
	winmm
)

# Source Grouping
source_group("Data Files" FILES ${enc-amf_DATA})

# Windows Only
if (WIN32)
	LIST(APPEND enc-amf_HEADERS
		"Include/api-d3d9.h"
		"Include/api-d3d11.h"
	)
	LIST(APPEND enc-amf_SOURCES
		"Source/api-d3d9.cpp"
		"Source/api-d3d11.cpp"
	)
endif()

################################################################################
# Standalone and OBS Studio Build Data
################################################################################
if(BUILD_AMF_ENCODER)
	# OBS Studio Specific
	
	# Variables
	OPTION(AMDAMF_Disable "Disable AMD Advanced Media Framework support" OFF)
	SET(AMDAMF_SDKDir "" CACHE PATH "AMD Advanced Media Framework SDK Directory")

	# Tests
	if(AMDAMF_Disable)
		message(STATUS "AMD AMF support disabled")
		return()
	endif()

	if(AMDAMF_SDKDir STREQUAL "")
		message(STATUS "AMDAMF_SDKDir not set!  enc-amf plugin disabled.")
		return()
	endif()

	if(NOT EXISTS "${AMDAMF_SDKDir}/amf/public/include/components/VideoEncoderVCE.h")
		message(STATUS "AMDAMF_SDKDir invalid!  enc-amf plugin disabled.")
		return()
	endif()
	
	# Directories
	INCLUDE_DIRECTORIES(
		"${CMAKE_SOURCE_DIR}"
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_BINARY_DIR}/Include"
		"${PROJECT_BINARY_DIR}/Source"
		"${PROJECT_SOURCE_DIR}"
		"${PROJECT_SOURCE_DIR}/Include"
		"${PROJECT_SOURCE_DIR}/Source"
		"${AMDAMF_SDKDir}/amf/public/include"
	)
	SET(LIBOBS_LIBRARIES libobs)
else()
	# Standlone Specific

	# Variables
	SET(PATH_AMDAMFSDK "" CACHE PATH "AMD Advanced Media Framework SDK Directory")
	SET(PATH_OBSStudio "" CACHE PATH "OBS Studio Source Code Directory")
	
	# Tests
	if(PATH_AMDAMFSDK STREQUAL "")
		message(STATUS "PATH_AMDAMFSDK not set!")
		return()
	endif()

	if(NOT EXISTS "${PATH_AMDAMFSDK}/amf/public/include/components/VideoEncoderVCE.h")
		message(STATUS "PATH_AMDAMFSDK invalid!")
		return()
	endif()
	
	if(PATH_OBSStudio STREQUAL "")
		message(STATUS "PATH_OBSStudio not set!")
		return()
	endif()

	if(NOT EXISTS "${PATH_OBSStudio}/libobs/obs-module.h")
		message(STATUS "PATH_OBSStudio invalid!")
		return()
	endif()
	
	# Find OBS Libraries
	SET(obsPath "${PATH_OBSStudio}")
	INCLUDE("${PATH_OBSStudio}/cmake/external/Findlibobs.cmake")
	
	# Compiling
	INCLUDE_DIRECTORIES(
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_BINARY_DIR}/Include"
		"${PROJECT_BINARY_DIR}/Source"
		"${PROJECT_SOURCE_DIR}"
		"${PROJECT_SOURCE_DIR}/Include"
		"${PROJECT_SOURCE_DIR}/Source"
		"${PATH_AMDAMFSDK}/amf/public/include"
		"${PATH_OBSStudio}/"
	)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

################################################################################
# Build
################################################################################
ADD_LIBRARY(enc-amf MODULE
	${enc-amf_HEADERS}
	${enc-amf_SOURCES}
	${enc-amf_DATA}
)
TARGET_LINK_LIBRARIES(enc-amf
	${LIBOBS_LIBRARIES}
	${enc-amf_LIBRARIES}
)

# All Warnings, Extra Warnings, Pedantic
if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

if(BUILD_AMF_ENCODER)
	install_obs_plugin_with_data(enc-amf Resources)
else()
	math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")
	add_custom_command(TARGET enc-amf POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${PROJECT_SOURCE_DIR}/Resources/locale"
		"${PROJECT_SOURCE_DIR}/#Build/data/obs-plugins/enc-amf/locale"
	)	
	add_custom_command(TARGET enc-amf POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"$<TARGET_FILE:enc-amf>"
		"${PROJECT_SOURCE_DIR}/#Build/obs-plugins/${BITS}bit/$<TARGET_FILE_NAME:enc-amf>"
	)
	add_custom_command(TARGET enc-amf POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"$<TARGET_FILE_DIR:enc-amf>/enc-amf.pdb"
		"${PROJECT_SOURCE_DIR}/#Build/obs-plugins/${BITS}bit/enc-amf.pdb"
	)
endif()
