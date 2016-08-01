/*
MIT License

Copyright (c) 2016 Michael Fabian Dirks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include "win-amf.h"

using namespace AMF_Encoder;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}


OBS_DECLARE_MODULE();
OBS_MODULE_AUTHOR("Michael Fabian Dirks");
OBS_MODULE_USE_DEFAULT_LOCALE("win-amf", "en-US");

/**
* Required: Called when the module is loaded.  Use this function to load all
* the sources/encoders/outputs/services for your module, or anything else that
* may need loading.
*
* @return           Return true to continue loading the module, otherwise
*                   false to indicate failure and unload the module
*/
MODULE_EXPORT bool obs_module_load(void) {
	AMF_LOG_INFO(PLUGIN_VERSION " loaded.");

	//////////////////////////////////////////////////////////////////////////
	// Report Capabilities to log file first.
	//////////////////////////////////////////////////////////////////////////
#pragma region Capability Reporting

	AMF_LOG_INFO("Gathering Capability Information...");
	h264_capabilities* caps = h264_capabilities::getInstance();

	AMF_LOG_INFO("Encoder Capabilities:");
	h264_capabilities::EncoderCaps* capsEnc[2] = { &caps->m_AVCCaps, &caps->m_SVCCaps };
	for (uint8_t i = 0; i < 2; i++) {
		if (i == 0)
			AMF_LOG_INFO("	AVC:");
		else
			AMF_LOG_INFO("	SVC:");

		switch (capsEnc[i]->acceleration_type) {
			case amf::AMF_ACCEL_NOT_SUPPORTED:
				AMF_LOG_INFO("		Acceleration Type: %s", "Not Supported");
				break;
			case amf::AMF_ACCEL_HARDWARE:
				AMF_LOG_INFO("		Acceleration Type: %s", "Hardware");
				break;
			case amf::AMF_ACCEL_GPU:
				AMF_LOG_INFO("		Acceleration Type: %s", "GPU");
				break;
			case amf::AMF_ACCEL_SOFTWARE:
				AMF_LOG_INFO("		Acceleration Type: %s", "Software");
				break;
		}
		AMF_LOG_INFO("		Maximum Bitrate: %d bits", capsEnc[i]->maxBitrate);
		AMF_LOG_INFO("		Maximum Stream Count: %d", capsEnc[i]->maxStreamCount);

		h264_capabilities::EncoderCaps::IOCaps* capsIO[2] = { &capsEnc[i]->input, &capsEnc[i]->output };
		for (uint8_t i = 0; i < 2; i++) {
			if (i == 0)
				AMF_LOG_INFO("		Input:");
			else
				AMF_LOG_INFO("		Output:");

			AMF_LOG_INFO("			Width Range: %d, %d", capsIO[i]->minWidth, capsIO[i]->maxWidth);
			AMF_LOG_INFO("			Height Range: %d, %d", capsIO[i]->minHeight, capsIO[i]->maxHeight);
			AMF_LOG_INFO("			Supports Interlaced: %s", capsIO[i]->isInterlacedSupported ? "Yes" : "No");
			AMF_LOG_INFO("			Vertical Buffer Alignment: %d bytes", capsIO[i]->verticalAlignment);

			char* surfaceFormat[] = {
				"Unknown",
				"NV12",
				"YV12",
				"BRGA",
				"ARGB",
				"RGBA",
				"GRAY8",
				"YUV420P",
				"U8V8",
				"YUY2"
			};
			AMF_LOG_INFO("			Surface Formats:");
			for (uint32_t i2 = 0; i2 < capsIO[i]->formats.size(); i2++) {
				AMF_LOG_INFO("			- %s%s", surfaceFormat[capsIO[i]->formats[i2].first], capsIO[i]->formats[i2].second ? " (Native)" : "");
			}

			char * memoryTypes[] = {
				"Unknown",
				"Host",
				"DirectX9",
				"DirectX11",
				"OpenCL",
				"OpenGL",
				"XV",
				"GrAlloc"
			};
			AMF_LOG_INFO("			Memory Types:");
			for (uint32_t i2 = 0; i2 < capsIO[i]->memoryTypes.size(); i2++) {
				AMF_LOG_INFO("			- %s%s", memoryTypes[capsIO[i]->memoryTypes[i2].first], capsIO[i]->memoryTypes[i2].second ? " (Native)" : "");
			}
		}
	}
#pragma endregion

	// Register Encoder
	AMF_Encoder::h264::encoder_register();

	return true;
}

/** Optional: Called when the module is unloaded.  */
//MODULE_EXPORT void obs_module_unload(void) {
//}


/** Optional: Returns the full name of the module */
//MODULE_EXPORT const char *obs_module_name() {
//	return "Windows AMF Encoder";
//}

/** Optional: Returns a description of the module */
//MODULE_EXPORT const char *obs_module_description() {
//	return "AMF Encoder Plugin for OBS Studio";
//}