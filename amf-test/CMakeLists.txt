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
PROJECT(enc-amf-test)

################################################################################
# CMake / Compiler
################################################################################

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

# Detect Architecture (Bitness)
math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")

add_definitions(-DLITE_OBS)

################################################################################
# Configuration
################################################################################

# From Parent:
#   OBS_STUDIO_DIR
#   ${PropertyPrefix}OBS_NATIVE
#   ${PropertyPrefix}OBS_PACKAGE
#   AMF_SDK_DIR

IF(WIN32)
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
ENDIF()


################################################################################
# Dependencies
################################################################################

# Project
add_executable(enc-amf-test
	"${PROJECT_SOURCE_DIR}/main.cpp"
	"${enc-amf_SOURCE_DIR}/source/amf.cpp"
	"${enc-amf_SOURCE_DIR}/source/amf-capabilities.cpp"
	"${enc-amf_SOURCE_DIR}/source/amf-encoder.cpp"
	"${enc-amf_SOURCE_DIR}/source/amf-encoder-h264.cpp"
	"${enc-amf_SOURCE_DIR}/source/amf-encoder-h265.cpp"
	"${enc-amf_SOURCE_DIR}/source/api-base.cpp"
	"${enc-amf_SOURCE_DIR}/source/api-d3d9.cpp"
	"${enc-amf_SOURCE_DIR}/source/api-d3d11.cpp"
	"${enc-amf_SOURCE_DIR}/source/utility.cpp"
	"${enc-amf_SOURCE_DIR}/include/amf.hpp"
	"${enc-amf_SOURCE_DIR}/include/amf-capabilities.hpp"
	"${enc-amf_SOURCE_DIR}/include/amf-encoder.hpp"
	"${enc-amf_SOURCE_DIR}/include/amf-encoder-h264.hpp"
	"${enc-amf_SOURCE_DIR}/include/amf-encoder-h265.hpp"
	"${enc-amf_SOURCE_DIR}/include/api-base.hpp"
	"${enc-amf_SOURCE_DIR}/include/api-d3d9.hpp"
	"${enc-amf_SOURCE_DIR}/include/api-d3d11.hpp"
	"${enc-amf_SOURCE_DIR}/include/utility.hpp"
)
target_include_directories(enc-amf-test
	PUBLIC
		"${PROJECT_SOURCE_DIR}"
		"${enc-amf_SOURCE_DIR}/include"
		"${enc-amf_BINARY_DIR}/include"
		"${enc-amf_SOURCE_DIR}/source"
		"${enc-amf_SOURCE_DIR}/AMF/amf/public/include"
)
IF(${PropertyPrefix}OBS_NATIVE)
	target_include_directories(enc-amf-test
		PUBLIC
	)
ELSEIF(${PropertyPrefix}OBS_PACKAGE)
	target_include_directories(enc-amf-test
		PUBLIC
	)
ELSE()
	target_include_directories(enc-amf-test
		PUBLIC
	)
ENDIF()

IF(WIN32)
	target_link_libraries(enc-amf-test
		version
		winmm
	)
ENDIF()

set_target_properties(enc-amf-test
	PROPERTIES
		OUTPUT_NAME "enc-amf-test${BITS}")

if(${PropertyPrefix}OBS_NATIVE)
	install_obs_datatarget(enc-amf-test "obs-plugins/enc-amf")
	Set_Target_Properties(enc-amf-test PROPERTIES FOLDER "plugins/enc-amf")
else()
	INSTALL(TARGETS enc-amf-test
		RUNTIME DESTINATION "./data/obs-plugins/enc-amf/" COMPONENT Runtime
		LIBRARY DESTINATION "./data/obs-plugins/enc-amf/" COMPONENT Runtime
	)
	INSTALL(FILES $<TARGET_PDB_FILE:enc-amf-test> DESTINATION "./data/obs-plugins/enc-amf/" OPTIONAL)
endif()
