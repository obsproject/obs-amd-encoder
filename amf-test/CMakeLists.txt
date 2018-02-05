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

# All Warnings, Extra Warnings, Pedantic
if(MSVC)
	# Hide Microsofts insecurities, as they are usually not cross-platform compatible changes.
	add_definitions(-D_CRT_SECURE_NO_WARNINGS) 
	
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
	
	# Use Static Linking instead of Dynamic Linking
	string(REPLACE "/MD" "/MT" 
		"CMAKE_C_FLAGS" 
		"${CMAKE_C_FLAGS}")
	string(REPLACE "/MD" "/MT" 
		"CMAKE_CXX_FLAGS" 
		"${CMAKE_CXX_FLAGS}")
	string(TOUPPER "${CMAKE_CONFIGURATION_TYPES}" UPPER_CONFIG_TYPES)
	foreach(CONFIG_TYPE ${UPPER_CONFIG_TYPES})
		string(REPLACE "/MD" "/MT" 
			"CMAKE_C_FLAGS_${CONFIG_TYPE}" 
			"${CMAKE_C_FLAGS_${CONFIG_TYPE}}")

		string(REPLACE "/MD" "/MT" 
			"CMAKE_CXX_FLAGS_${CONFIG_TYPE}" 
			"${CMAKE_CXX_FLAGS_${CONFIG_TYPE}}")
	endforeach()  
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

# Detect Architecture (Bitness)
math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")

# Project
add_executable(enc-amf-test
	"${PROJECT_SOURCE_DIR}/main.cpp"
	"${PROJECT_SOURCE_DIR}/libobs/obs-module.h"
	"${PROJECT_SOURCE_DIR}/libobs/obs-module.c"
	"${enc-amf_SOURCE_DIR}/Source/amf.cpp"
	"${enc-amf_SOURCE_DIR}/Source/amf-capabilities.cpp"
	"${enc-amf_SOURCE_DIR}/Source/amf-encoder.cpp"
	"${enc-amf_SOURCE_DIR}/Source/amf-encoder-h264.cpp"
	"${enc-amf_SOURCE_DIR}/Source/amf-encoder-h265.cpp"
	"${enc-amf_SOURCE_DIR}/Source/api-base.cpp"
	"${enc-amf_SOURCE_DIR}/Source/api-d3d9.cpp"
	"${enc-amf_SOURCE_DIR}/Source/api-d3d11.cpp"
	"${enc-amf_SOURCE_DIR}/Source/utility.cpp"
	"${enc-amf_SOURCE_DIR}/Include/amf.h"
	"${enc-amf_SOURCE_DIR}/Include/amf-capabilities.h"
	"${enc-amf_SOURCE_DIR}/Include/amf-encoder.h"
	"${enc-amf_SOURCE_DIR}/Include/amf-encoder-h264.h"
	"${enc-amf_SOURCE_DIR}/Include/amf-encoder-h265.h"
	"${enc-amf_SOURCE_DIR}/Include/api-base.h"
	"${enc-amf_SOURCE_DIR}/Include/api-d3d9.h"
	"${enc-amf_SOURCE_DIR}/Include/api-d3d11.h"
	"${enc-amf_SOURCE_DIR}/Include/utility.h"
)
include_directories(enc-amf-test BEFORE ${PROJECT_SOURCE_DIR})	
target_link_libraries(enc-amf-test ${enc-amf_LIBRARIES})
set_target_properties(enc-amf-test
	PROPERTIES
		OUTPUT_NAME "enc-amf-test${BITS}")

if(TARGET obs)
	install_obs_datatarget(enc-amf-test "obs-plugins/enc-amf")
else()
	add_custom_command(TARGET enc-amf-test POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"$<TARGET_FILE:enc-amf-test>"
		"${enc-amf_SOURCE_DIR}/#Build/data/obs-plugins/enc-amf/$<TARGET_FILE_NAME:enc-amf-test>"
	)
	add_custom_command(TARGET enc-amf-test POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"$<TARGET_PDB_FILE:enc-amf-test>"
		"${enc-amf_SOURCE_DIR}/#Build/data/obs-plugins/enc-amf/$<TARGET_PDB_FILE_NAME:enc-amf-test>"
	)
endif()
