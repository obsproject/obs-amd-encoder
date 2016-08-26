project(enc-amf)

set(AMDAMF_Disable CACHE BOOL "Disable AMD Advanced Media Framework support")

if(AMDAMF_Disable)
	message(STATUS "AMD AMF support disabled")
	return()
endif()

set(AMDAMF_SDKDir CACHE "PATH" "AMD Advanced Media Framework SDK Directory")

include_directories(
	SYSTEM "${CMAKE_SOURCE_DIR}"
	SYSTEM "${CMAKE_SOURCE_DIR}/plugins/enc-amf/enc-amf/Include"
	SYSTEM "${CMAKE_SOURCE_DIR}/plugins/enc-amf/enc-amf/Source"
	SYSTEM "${AMDAMF_SDKDir}/amf/public/include"
)

set(enc-amf_HEADERS
	"enc-amf/Include/amd-amf.h"
	"enc-amf/Include/amd-amf-vce.h"
	"enc-amf/Include/amd-amf-vce-capabilities.h"
	"enc-amf/Include/enc-h264.h"
	"enc-amf/Include/enc-h264-simple.h"
	"enc-amf/Include/plugin.h"
)

set(enc-amf_SOURCES
	"enc-amf/Source/amd-amf.cpp"
	"enc-amf/Source/amd-amf-vce.cpp"
	"enc-amf/Source/amd-amf-vce-capabilities.cpp"
	"enc-amf/Source/enc-h264.cpp"
	"enc-amf/Source/enc-h264-simple.cpp"
	"enc-amf/Source/plugin.cpp"
)

add_library(enc-amf MODULE
	${enc-amf_HEADERS}
	${enc-amf_SOURCES}
)
target_link_libraries(enc-amf
	libobs
	winmm.lib
)

install_obs_plugin_with_data(enc-amf enc-amf/enc-amf/Resources)
