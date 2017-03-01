cmake_minimum_required(VERSION 2.8.12)

# Shared (OBS Studio & Standalone)
PROJECT(enc-amf)
SET(enc-amf_HEADERS
	"Include/amf.h"
	"Include/amf-capabilities.h"
	"Include/amf-h264.h"
	"Include/api-base.h"
	"Include/api-d3d9.h"
	"Include/api-d3d11.h"
	"Include/api-host.h"
	"Include/api-opengl.h"
	"Include/enc-h264.h"
	"Include/plugin.h"
	"${PROJECT_BINARY_DIR}/Include/Version.h"
)
SET(enc-amf_SOURCES
	"Source/amf.cpp"
	"Source/amf-capabilities.cpp"
	"Source/amf-h264.cpp"
	"Source/api-base.cpp"
	"Source/api-host.cpp"
	"Source/api-d3d9.cpp"
	"Source/api-d3d11.cpp"
	"Source/api-opengl.cpp"
	"Source/enc-h264.cpp"
	"Source/misc-util.cpp"
	"Source/plugin.cpp"
)
SET(enc-amf_LIBRARIES
	version
	winmm
)

# Version
SET(enc-amf_VERSION_MAJOR 1)
SET(enc-amf_VERSION_MINOR 4)
SET(enc-amf_VERSION_PATCH 3)
SET(enc-amf_VERSION_BUILD 11)
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
	#SET(PATH_libobs "" CACHE PATH "Path to obs.lib from OBS Studio")
	
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
