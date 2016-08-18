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
#include "amf-vce-capabilities.h"

// AMF
#include "AMD-Media-SDK/1.1/inc/amf/components/CapabilityManager.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/ComponentCaps.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderCaps.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderVCECaps.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderVCE.h"

// Plugin
#include "win-amf.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
AMFEncoder::VCE_Capabilities* AMFEncoder::VCE_Capabilities::instance;

AMFEncoder::VCE_Capabilities* AMFEncoder::VCE_Capabilities::getInstance() {
	if (!instance)
		instance = new VCE_Capabilities();

	return instance;
}

void AMFEncoder::VCE_Capabilities::reportCapabilities() {
	//////////////////////////////////////////////////////////////////////////
	// Report Capabilities to log file first.
	//////////////////////////////////////////////////////////////////////////
	#pragma region Capability Reporting

	AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> Gathering Capability Information...");
	VCE_Capabilities* caps = VCE_Capabilities::getInstance();

	AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> Encoder Capabilities:");
	VCE_Capabilities::EncoderCaps* capsEnc[2] = { &caps->m_AVCCaps, &caps->m_SVCCaps };
	for (uint8_t i = 0; i < 2; i++) {
		if (i == 0)
			AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 	AVC:");
		else
			AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 	SVC:");

		switch (capsEnc[i]->acceleration_type) {
			case amf::AMF_ACCEL_NOT_SUPPORTED:
				AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 		Acceleration Type: %s", "Not Supported");
				break;
			case amf::AMF_ACCEL_HARDWARE:
				AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 		Acceleration Type: %s", "Hardware");
				break;
			case amf::AMF_ACCEL_GPU:
				AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 		Acceleration Type: %s", "GPU");
				break;
			case amf::AMF_ACCEL_SOFTWARE:
				AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 		Acceleration Type: %s", "Software");
				break;
		}
		AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 		Maximum Bitrate: %d bits", capsEnc[i]->maxBitrate);
		AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 		Maximum Stream Count: %d", capsEnc[i]->maxStreamCount);

		AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 		H264 Capabilities:");
		AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			B-Pictures Supported: %s", capsEnc[i]->h264.isBPictureSupported ? "Yes" : "No");
		AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			Fixed-Byte Slice Mode Supported: %s", capsEnc[i]->h264.isFixedByteSliceModeSupported ? "Yes" : "No");
		AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			Can Output 3D: %s", capsEnc[i]->h264.canOutput3D ? "Yes" : "No");
		AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			Max Num of Temporal Layers: %d", capsEnc[i]->h264.maxNumOfTemporalLayers);
		AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			Max Supported Job Priority: %d", capsEnc[i]->h264.maxSupportedJobPriority);
		AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			Num of Reference Frames: %d - %d (min - max)", capsEnc[i]->h264.minReferenceFrames, capsEnc[i]->h264.maxReferenceFrames);
		AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			Profiles:");
		for (uint32_t j = 0; j < capsEnc[i]->h264.profiles.size(); j++) {
			switch (capsEnc[i]->h264.profiles[j]) {
				case AMF_VIDEO_ENCODER_PROFILE_BASELINE:
					AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 				Baseline");
					break;
				case AMF_VIDEO_ENCODER_PROFILE_MAIN:
					AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 				Main");
					break;
				case AMF_VIDEO_ENCODER_PROFILE_HIGH:
					AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 				High");
					break;
				default:
					AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 				Unknown (%d)", capsEnc[i]->h264.profiles[j]);
					break;
			}
		}
		AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			Levels:");
		for (uint32_t j = 0; j < capsEnc[i]->h264.levels.size(); j++) {
			AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 				%d", capsEnc[i]->h264.levels[j]);
		}
		AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			Rate Control Methods:");
		for (uint32_t j = 0; j < capsEnc[i]->h264.rateControlMethods.size(); j++) {
			switch (capsEnc[i]->h264.rateControlMethods[j]) {
				case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTRAINED_QP:
					AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 				Costrained QP");
					break;
				case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR:
					AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 				CBR");
					break;
				case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR:
					AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 				VBR (Peak Constrained)");
					break;
				case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR:
					AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 				VBR (Latency Constrained)");
					break;
				default:
					AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 				Unknown (%d)", capsEnc[i]->h264.rateControlMethods[j]);
					break;
			}
		}

		VCE_Capabilities::EncoderCaps::IOCaps* capsIO[2] = { &capsEnc[i]->input, &capsEnc[i]->output };
		for (uint8_t j = 0; j < 2; j++) {
			if (j == 0)
				AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 		Input:");
			else
				AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 		Output:");

			AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			Width Range: %d, %d", capsIO[j]->minWidth, capsIO[j]->maxWidth);
			AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			Height Range: %d, %d", capsIO[j]->minHeight, capsIO[j]->maxHeight);
			AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			Supports Interlaced: %s", capsIO[j]->isInterlacedSupported ? "Yes" : "No");
			AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			Vertical Buffer Alignment: %d bytes", capsIO[j]->verticalAlignment);

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
			AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			Surface Formats:");
			for (uint32_t k = 0; k < capsIO[j]->formats.size(); k++) {
				AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			- %s%s", surfaceFormat[capsIO[j]->formats[k].first], capsIO[j]->formats[k].second ? " (Native)" : "");
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
			AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			Memory Types:");
			for (uint32_t k = 0; k < capsIO[j]->memoryTypes.size(); k++) {
				AMF_LOG_INFO("<AMFEncoder::VCE_Capabilities::reportCapabilities> 			- %s%s", memoryTypes[capsIO[j]->memoryTypes[k].first], capsIO[j]->memoryTypes[k].second ? " (Native)" : "");
			}
		}
	}
	#pragma endregion
}

AMFEncoder::VCE_Capabilities::VCE_Capabilities() {
	refreshCapabilities();
}

AMFEncoder::VCE_Capabilities::~VCE_Capabilities() {

}

bool AMFEncoder::VCE_Capabilities::refreshCapabilities() {
	AMF_RESULT res;
	amf::AMFCapabilityManagerPtr mgr;

	res = AMFCreateCapsManager(&mgr);
	if (res != AMF_OK) {
		std::vector<char> msgBuf(1024);
		wcstombs(msgBuf.data(), amf::AMFGetResultText(res), msgBuf.size());

		AMF_LOG_ERROR("<AMFEncoder::VCE_Capabilities::refreshCapabilities> %s, error code %d: %s.", "AMFCreateCapsManager", res, msgBuf.data());
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Get Encoder Capabilities
	//////////////////////////////////////////////////////////////////////////
	EncoderCaps* caps[2] = { &m_AVCCaps, &m_SVCCaps };
	const wchar_t* capsString[2] = { AMFVideoEncoderVCE_AVC , AMFVideoEncoderVCE_SVC };
	for (uint8_t capsIndex = 0; capsIndex < 2; capsIndex++) {
		amf::AMFEncoderCapsPtr encCaps;
		if (capsIndex == 0)
			res = mgr->GetEncoderCaps(AMFVideoEncoderVCE_AVC, &encCaps);
		else
			res = mgr->GetEncoderCaps(AMFVideoEncoderVCE_SVC, &encCaps);

		if (res != AMF_OK)
			break;

		// Basic Capabilities
		caps[capsIndex]->acceleration_type = encCaps->GetAccelerationType();
		caps[capsIndex]->maxStreamCount = encCaps->GetMaxNumOfStreams();
		caps[capsIndex]->maxBitrate = encCaps->GetMaxBitrate();

		// H264 Capabilities
		amf::H264EncoderCapsPtr encH264Caps = (amf::H264EncoderCapsPtr)encCaps;
		if (encH264Caps) {
			caps[capsIndex]->h264.isBPictureSupported = encH264Caps->IsBPictureSupported();
			caps[capsIndex]->h264.isFixedByteSliceModeSupported = encH264Caps->IsFixedByteSliceModeSupported();
			caps[capsIndex]->h264.canOutput3D = encH264Caps->CanOutput3D();
			caps[capsIndex]->h264.maxNumOfTemporalLayers = encH264Caps->GetMaxNumOfTemporalLayers();
			caps[capsIndex]->h264.maxSupportedJobPriority = encH264Caps->GetMaxSupportedJobPriority();
			encH264Caps->GetNumOfReferenceFrames(&(caps[capsIndex]->h264.minReferenceFrames), &(caps[capsIndex]->h264.maxReferenceFrames));

			// Profiles
			uint32_t numProfiles = encH264Caps->GetNumOfSupportedProfiles();
			caps[capsIndex]->h264.profiles.resize(numProfiles);
			for (uint32_t i = 0; i < numProfiles; i++) {
				caps[capsIndex]->h264.profiles[i] = encH264Caps->GetProfile(i);
			}

			// Profile Levels
			uint32_t numLevels = encH264Caps->GetNumOfSupportedLevels();
			caps[capsIndex]->h264.levels.resize(numLevels);
			for (uint32_t i = 0; i < numLevels; i++) {
				caps[capsIndex]->h264.levels[i] = encH264Caps->GetLevel(i);
			}

			// Rate Control Methods
			uint32_t numRCM = encH264Caps->GetNumOfRateControlMethods();
			caps[capsIndex]->h264.rateControlMethods.resize(numRCM);
			for (uint32_t i = 0; i < numRCM; i++) {
				caps[capsIndex]->h264.rateControlMethods[i] = encH264Caps->GetRateControlMethod(i);
			}
		}

		// Input Capabilities
		amf::AMFIOCapsPtr capsIO[2];
		EncoderCaps::IOCaps* capsIOS[2] = { &caps[capsIndex]->input, &caps[capsIndex]->output };

		res = encCaps->GetInputCaps(&capsIO[0]);
		res = encCaps->GetOutputCaps(&capsIO[1]);

		for (uint8_t ioIndex = 0; ioIndex < 2; ioIndex++) {
			capsIO[ioIndex]->GetWidthRange(&(capsIOS[ioIndex]->minWidth), &(capsIOS[ioIndex]->maxWidth));
			capsIO[ioIndex]->GetHeightRange(&(capsIOS[ioIndex]->minHeight), &(capsIOS[ioIndex]->maxHeight));
			capsIOS[ioIndex]->isInterlacedSupported = capsIO[ioIndex]->IsInterlacedSupported();
			capsIOS[ioIndex]->verticalAlignment = capsIO[ioIndex]->GetVertAlign();

			int32_t numFormats = capsIO[ioIndex]->GetNumOfFormats();
			capsIOS[ioIndex]->formats.resize(numFormats);
			for (int32_t formatIndex = 0; formatIndex < numFormats; formatIndex++) {
				amf::AMF_SURFACE_FORMAT format = amf::AMF_SURFACE_UNKNOWN;
				bool isNative = false;

				capsIO[ioIndex]->GetFormatAt(formatIndex, &format, &isNative);
				capsIOS[ioIndex]->formats[formatIndex].first = format;
				capsIOS[ioIndex]->formats[formatIndex].second = isNative;
			}

			int32_t numMemoryTypes = capsIO[ioIndex]->GetNumOfMemoryTypes();
			capsIOS[ioIndex]->memoryTypes.resize(numMemoryTypes);
			for (int32_t memoryTypeIndex = 0; memoryTypeIndex < numMemoryTypes; memoryTypeIndex++) {
				amf::AMF_MEMORY_TYPE type = amf::AMF_MEMORY_UNKNOWN;
				bool isNative = false;

				capsIO[ioIndex]->GetMemoryTypeAt(memoryTypeIndex, &type, &isNative);
				capsIOS[ioIndex]->memoryTypes[memoryTypeIndex].first = type;
				capsIOS[ioIndex]->memoryTypes[memoryTypeIndex].second = isNative;
			}
		}
	}

	mgr->Terminate();

	return true;
}

AMFEncoder::VCE_Capabilities::EncoderCaps* AMFEncoder::VCE_Capabilities::getEncoderCaps(VCE_Encoder_Type type) {
	EncoderCaps* caps[2] = { &m_AVCCaps, &m_SVCCaps };
	return caps[type];
}

AMFEncoder::VCE_Capabilities::EncoderCaps::IOCaps* AMFEncoder::VCE_Capabilities::getIOCaps(VCE_Encoder_Type type, bool output) {
	EncoderCaps* caps[2] = { &m_AVCCaps, &m_SVCCaps };
	if (output)
		return &caps[type]->output;
	else
		return &caps[type]->input;
}

