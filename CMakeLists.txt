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

cmake_minimum_required(VERSION 3.1.0)
PROJECT(enc-amf VERSION 2.4.2)

################################################################################
# CMake / Compiler
################################################################################
configure_file(
	"${PROJECT_SOURCE_DIR}/ci/installer.in.iss"
	"${PROJECT_BINARY_DIR}/ci/installer.iss"
)

configure_file(
	"${PROJECT_SOURCE_DIR}/Include/Version.h.in"
	"${PROJECT_BINARY_DIR}/Include/Version.h"
)

# All Warnings, Extra Warnings, Pedantic
if(MSVC)
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
	
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	# Update if necessary
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")

################################################################################
# Options
################################################################################
SET(AMF_SDK_DIR  "${PROJECT_SOURCE_DIR}/AMF/" CACHE PATH "AMD Advanced Media Framework SDK Directory")

IF(TARGET libobs)
	MESSAGE(STATUS "enc-amf: Using native obs-studio.")
	SET(OBS_NATIVE TRUE)
	OPTION(BUILD_AMD_ENCODER "Build AMD Encoder module" ON)
	
	if (NOT BUILD_AMD_ENCODER)
		message(STATUS "Not building AMD Encoder")
		return()
	endif()
ELSE()
	SET(OBS_NATIVE FALSE)
	SET(OBS_STUDIO_DIR "" CACHE PATH "OBS Studio Source/Package Directory")	
ENDIF()

################################################################################
# Dependencies
################################################################################

# AMF SDK
if(AMF_SDK_DIR STREQUAL "")
	message(STATUS "AMF_SDK_DIR not set! enc-amf plugin disabled.")
	return()
endif()

if(NOT EXISTS "${AMF_SDK_DIR}/amf/public/include/components/VideoEncoderVCE.h")
	message(STATUS "AMF_SDK_DIR invalid! enc-amf plugin disabled.")
	return()
endif()

# Detect OBS_STUDIO_DIR type
IF(NOT OBS_NATIVE)
	IF(EXISTS "${OBS_STUDIO_DIR}/libobs/obs-module.h")
		MESSAGE(STATUS "enc-amf: Using referenced obs-studio.")
		SET(OBS_PACKAGE FALSE)
		SET(obsPath "${OBS_STUDIO_DIR}")
		INCLUDE("${OBS_STUDIO_DIR}/cmake/external/Findlibobs.cmake")
	ELSEIF(EXISTS "${OBS_STUDIO_DIR}/cmake/LibObs/LibObsConfig.cmake")
		MESSAGE(STATUS "enc-amf: Using packaged obs-studio.")
		SET(OBS_PACKAGE TRUE)
		INCLUDE("${OBS_STUDIO_DIR}/cmake/LibObs/LibObsConfig.cmake")
	ELSE()
		MESSAGE("Failed to detect OBS_STUDIO_DIR type, verify that the following is path correct '${OBS_STUDIO_DIR}'.")
		return()
	ENDIF()
ENDIF()

################################################################################
# Code
################################################################################
SET(PROJECT_HEADERS
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
SET(PROJECT_SOURCES
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
SET(PROJECT_DATA
	"${PROJECT_SOURCE_DIR}/Resources/locale/en-US.ini"
	"${PROJECT_SOURCE_DIR}/LICENSE"
	"${PROJECT_SOURCE_DIR}/AUTHORS"
)
SET(PROJECT_LIBRARIES
	version
	winmm
)

if (WIN32)
	# Windows Only
	LIST(APPEND PROJECT_HEADERS
		"Include/api-d3d9.h"
		"Include/api-d3d11.h"
	)
	LIST(APPEND PROJECT_SOURCES
		"Source/api-d3d9.cpp"
		"Source/api-d3d11.cpp"
	)
	
	# windows.h
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-DNOGPICAPMASKS)
	add_definitions(-DNOVIRTUALKEYCODES)
	#add_definitions(-DNOWINMESSAGES)
	add_definitions(-DNOWINSTYLES)
	add_definitions(-DNOSYSMETRICS)
	add_definitions(-DNOMENUS)
	add_definitions(-DNOICONS)
	add_definitions(-DNOKEYSTATES)
	add_definitions(-DNOSYSCOMMANDS)
	add_definitions(-DNORASTEROPS)
	add_definitions(-DNOSHOWWINDOW)
	add_definitions(-DNOATOM)
	add_definitions(-DNOCLIPBOARD)
	add_definitions(-DNOCOLOR)
	add_definitions(-DNOCTLMGR)
	add_definitions(-DNODRAWTEXT)
	#add_definitions(-DNOGDI)
	add_definitions(-DNOKERNEL)
	#add_definitions(-DNOUSER)
	#add_definitions(-DNONLS)
	add_definitions(-DNOMB)
	add_definitions(-DNOMEMMGR)
	add_definitions(-DNOMETAFILE)
	add_definitions(-DNOMINMAX)
	#add_definitions(-DNOMSG)
	add_definitions(-DNOOPENFILE)
	add_definitions(-DNOSCROLL)
	add_definitions(-DNOSERVICE)
	add_definitions(-DNOSOUND)
	#add_definitions(-DNOTEXTMETRIC)
	add_definitions(-DNOWH)
	add_definitions(-DNOWINOFFSETS)
	add_definitions(-DNOCOMM)
	add_definitions(-DNOKANJI)
	add_definitions(-DNOHELP)
	add_definitions(-DNOPROFILER)
	add_definitions(-DNODEFERWINDOWPOS)
	add_definitions(-DNOMCX)
	add_definitions(-DNOIME)
	add_definitions(-DNOMDI)
	add_definitions(-DNOINOUT)
endif()

# Source Grouping
source_group("Data Files" FILES ${enc-amf_DATA})

################################################################################
# Target
################################################################################

ADD_LIBRARY(enc-amf MODULE
	${PROJECT_HEADERS}
	${PROJECT_SOURCES}
	${PROJECT_DATA}
)

# Include Directories
TARGET_INCLUDE_DIRECTORIES(enc-amf
	PUBLIC
		"${PROJECT_BINARY_DIR}/Include"
		"${PROJECT_SOURCE_DIR}/Include"
		"${AMF_SDK_DIR}/amf/public/include"
	PRIVATE
		"${PROJECT_BINARY_DIR}/Source"
		"${PROJECT_SOURCE_DIR}/Source"
		"${PROJECT_BINARY_DIR}"
		"${PROJECT_SOURCE_DIR}"
		"${CMAKE_SOURCE_DIR}"
)
IF(OBS_NATIVE)
	TARGET_LINK_LIBRARIES(enc-amf
		libobs
	)
ELSEIF(OBS_PACKAGE)
	TARGET_INCLUDE_DIRECTORIES(enc-amf
		PRIVATE
			"${OBS_STUDIO_DIR}/include"
	)
	TARGET_LINK_LIBRARIES(enc-amf
		libobs
	)
ELSE()
	TARGET_INCLUDE_DIRECTORIES(enc-amf
		PRIVATE
			"${OBS_STUDIO_DIR}/libobs"
	)
	TARGET_LINK_LIBRARIES(enc-amf
		"${LIBOBS_LIB}"
	)
ENDIF()

# Link Libraries
TARGET_LINK_LIBRARIES(enc-amf
	"${PROJECT_LIBRARIES}"
)

################################################################################
# Installation
################################################################################

IF(OBS_NATIVE)
	install_obs_plugin_with_data(enc-amf Resources)
ELSE()
	INSTALL(TARGETS enc-amf
		RUNTIME DESTINATION "./obs-plugins/${BITS}bit/" COMPONENT Runtime
		LIBRARY DESTINATION "./obs-plugins/${BITS}bit/" COMPONENT Runtime
	)
	INSTALL(FILES $<TARGET_PDB_FILE:enc-amf> DESTINATION "./obs-plugins/${BITS}bit/" OPTIONAL)
	
	INSTALL(
		DIRECTORY "${PROJECT_SOURCE_DIR}/Resources/"
		DESTINATION "./data/obs-plugins/enc-amf/"
	)
ENDIF()

################################################################################
# Child Projects
################################################################################

# Sub Project
add_subdirectory(amf-test)

