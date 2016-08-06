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

		AMF_LOG_ERROR("%s, error code %d: %s.", "AMFCreateCapsManager", res, msgBuf.data());
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

