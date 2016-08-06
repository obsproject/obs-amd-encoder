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
//////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////
#include "win-amf.h"

// Plugin
#include "amf-vce-capabilities.h"
#include "enc-h264.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace AMFEncoder;

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
	AMF_LOG_INFO("Loading...");

	//////////////////////////////////////////////////////////////////////////
	// Report Capabilities to log file first.
	//////////////////////////////////////////////////////////////////////////
	#pragma region Capability Reporting

	AMF_LOG_INFO("Gathering Capability Information...");
	VCE_Capabilities* caps = VCE_Capabilities::getInstance();

	AMF_LOG_INFO("Encoder Capabilities:");
	VCE_Capabilities::EncoderCaps* capsEnc[2] = { &caps->m_AVCCaps, &caps->m_SVCCaps };
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

		AMF_LOG_INFO("		H264 Capabilities:");
		AMF_LOG_INFO("			B-Pictures Supported: %s", capsEnc[i]->h264.isBPictureSupported ? "Yes" : "No");
		AMF_LOG_INFO("			Fixed-Byte Slice Mode Supported: %s", capsEnc[i]->h264.isFixedByteSliceModeSupported ? "Yes" : "No");
		AMF_LOG_INFO("			Can Output 3D: %s", capsEnc[i]->h264.canOutput3D ? "Yes" : "No");
		AMF_LOG_INFO("			Max Num of Temporal Layers: %d", capsEnc[i]->h264.maxNumOfTemporalLayers);
		AMF_LOG_INFO("			Max Supported Job Priority: %d", capsEnc[i]->h264.maxSupportedJobPriority);
		AMF_LOG_INFO("			Num of Reference Frames: %d - %d (min - max)", capsEnc[i]->h264.minReferenceFrames, capsEnc[i]->h264.maxReferenceFrames);
		AMF_LOG_INFO("			Profiles:");
		for (uint32_t j = 0; j < capsEnc[i]->h264.profiles.size(); j++) {
			switch (capsEnc[i]->h264.profiles[j]) {
				case AMF_VIDEO_ENCODER_PROFILE_BASELINE:
					AMF_LOG_INFO("				Baseline");
					break;
				case AMF_VIDEO_ENCODER_PROFILE_MAIN:
					AMF_LOG_INFO("				Main");
					break;
				case AMF_VIDEO_ENCODER_PROFILE_HIGH:
					AMF_LOG_INFO("				High");
					break;
				default:
					AMF_LOG_INFO("				Unknown (%d)", capsEnc[i]->h264.profiles[j]);
					break;
			}
		}
		AMF_LOG_INFO("			Levels:");
		for (uint32_t j = 0; j < capsEnc[i]->h264.levels.size(); j++) {
			AMF_LOG_INFO("				%d", capsEnc[i]->h264.levels[j]);
		}
		AMF_LOG_INFO("			Rate Control Methods:");
		for (uint32_t j = 0; j < capsEnc[i]->h264.rateControlMethods.size(); j++) {
			switch (capsEnc[i]->h264.rateControlMethods[j]) {
				case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTRAINED_QP:
					AMF_LOG_INFO("				Costrained QP");
					break;
				case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR:
					AMF_LOG_INFO("				CBR");
					break;
				case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR:
					AMF_LOG_INFO("				VBR (Peak Constrained)");
					break;
				case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR:
					AMF_LOG_INFO("				VBR (Latency Constrained)");
					break;
				default:
					AMF_LOG_INFO("				Unknown (%d)", capsEnc[i]->h264.rateControlMethods[j]);
					break;
			}
		}

		VCE_Capabilities::EncoderCaps::IOCaps* capsIO[2] = { &capsEnc[i]->input, &capsEnc[i]->output };
		for (uint8_t j = 0; j < 2; j++) {
			if (j == 0)
				AMF_LOG_INFO("		Input:");
			else
				AMF_LOG_INFO("		Output:");

			AMF_LOG_INFO("			Width Range: %d, %d", capsIO[j]->minWidth, capsIO[j]->maxWidth);
			AMF_LOG_INFO("			Height Range: %d, %d", capsIO[j]->minHeight, capsIO[j]->maxHeight);
			AMF_LOG_INFO("			Supports Interlaced: %s", capsIO[j]->isInterlacedSupported ? "Yes" : "No");
			AMF_LOG_INFO("			Vertical Buffer Alignment: %d bytes", capsIO[j]->verticalAlignment);

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
			for (uint32_t k = 0; k < capsIO[j]->formats.size(); k++) {
				AMF_LOG_INFO("			- %s%s", surfaceFormat[capsIO[j]->formats[k].first], capsIO[j]->formats[k].second ? " (Native)" : "");
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
			for (uint32_t k = 0; k < capsIO[j]->memoryTypes.size(); k++) {
				AMF_LOG_INFO("			- %s%s", memoryTypes[capsIO[j]->memoryTypes[k].first], capsIO[j]->memoryTypes[k].second ? " (Native)" : "");
			}
		}
	}
	#pragma endregion

	// Register Encoder
	AMFEncoder::VCE_H264_Encoder::encoder_register();

	AMF_LOG_INFO("Loaded.");
	return true;
}

/** Optional: Called when the module is unloaded.  */
MODULE_EXPORT void obs_module_unload(void) {}


/** Optional: Returns the full name of the module */
MODULE_EXPORT const char *obs_module_name() {
	return "AMD Media Framework Plugin";
}

/** Optional: Returns a description of the module */
MODULE_EXPORT const char *obs_module_description() {
	return "AMD Media Framework Plugin";
}