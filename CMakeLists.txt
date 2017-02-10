cmake_minimum_required(VERSION 2.8.12)
PROJECT(enc-amf)

# Version
SET(enc-amf_VERSION_MAJOR 1)
SET(enc-amf_VERSION_MINOR 9)
SET(enc-amf_VERSION_PATCH 9)
SET(enc-amf_VERSION_BUILD 2)
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

# Code Stuff
SET(enc-amf_HEADERS
	"Include/amf.h"
	"Include/amf-capabilities.h"
	"Include/amf-encoder.h"
	"Include/amf-encoder-h264.h"
	"Include/api-base.h"
	"Include/api-host.h"
	"Include/api-opengl.h"
	"Include/enc-h264.h"
	"Include/plugin.h"
	"${PROJECT_BINARY_DIR}/Include/Version.h"
)
SET(enc-amf_SOURCES
	"Source/amf.cpp"
	"Source/amf-capabilities.cpp"
	"Source/amf-encoder.cpp"
	"Source/amf-encoder-h264.cpp"
	"Source/api-base.cpp"
	"Source/api-host.cpp"
	"Source/api-opengl.cpp"
	"Source/enc-h264.cpp"
	"Source/misc-util.cpp"
	"Source/plugin.cpp"
)
SET(enc-amf_LIBRARIES
	version
	winmm
)

if (WIN32)
	# Direct3D support should only be compiled for Windows
	LIST(APPEND enc-amf_HEADERS
		"Include/api-d3d9.h"
		"Include/api-d3d11.h"
	)
	LIST(APPEND enc-amf_SOURCES
		"Source/api-d3d9.cpp"
		"Source/api-d3d11.cpp"
	)
endif()


# OBS Studio Specific
if(BUILD_AMF_ENCODER)
	OPTION(AMDAMF_Disable "Disable AMD Advanced Media Framework support" OFF)
	SET(AMDAMF_SDKDir "" CACHE PATH "AMD Advanced Media Framework SDK Directory")

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
	
	# Safety Check: HEVC is only built in Standalone
	add_definitions(-DWITH_HEVC)
	LIST(APPEND enc-amf_HEADERS
		"Include/amf-encoder-h265.h"
		"Include/enc-h265.h"
	)
	LIST(APPEND enc-amf_SOURCES
		"Source/amf-encoder-h265.cpp"
		"Source/enc-h265.cpp"
	)
	
	# Stuff
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
endif()

ADD_LIBRARY(enc-amf MODULE
	${enc-amf_HEADERS}
	${enc-amf_SOURCES}
)
TARGET_LINK_LIBRARIES(enc-amf
	${LIBOBS_LIBRARIES}
	${enc-amf_LIBRARIES}
)

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
