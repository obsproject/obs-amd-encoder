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
#include "amd-amf-h264-capabilities.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
Plugin::AMD::H264Capabilities* Plugin::AMD::H264Capabilities::instance;

Plugin::AMD::H264Capabilities* Plugin::AMD::H264Capabilities::getInstance() {
	if (!instance)
		instance = new H264Capabilities();

	return instance;
}

void Plugin::AMD::H264Capabilities::reportCapabilities() {
	//////////////////////////////////////////////////////////////////////////
	// Report Capabilities to log file first.
	//////////////////////////////////////////////////////////////////////////
	#pragma region Capability Reporting

	AMF_LOG_INFO("Gathering Capability Information...");
	H264Capabilities* caps = H264Capabilities::getInstance();

	AMF_LOG_INFO(" %4s | %8s | %11s | %8s | %11s | %9s | %7s | %11s | %7s | %3s | %10s ",
		"Type",
		"Acc.Type",
		"Max Bitrate",
		"Stream #",
		"Max Profile",
		"Max Level",
		"BFrames",
		"Ref. Frames",
		"Layer #",
		"FSM",
		"Instance #");

	H264Capabilities::EncoderCaps* capsEnc[2] = { &caps->m_AVCCaps, &caps->m_SVCCaps };
	for (uint8_t i = 0; i < 2; i++) {
		// Encoder Acceleration
		char* accelType;
		switch (capsEnc[i]->acceleration_type) {
			case amf::AMF_ACCEL_NOT_SUPPORTED:
				accelType = "None";
				break;
			case amf::AMF_ACCEL_HARDWARE:
				accelType = "Hardware";
				break;
			case amf::AMF_ACCEL_GPU:
				accelType = "GPU";
				break;
			case amf::AMF_ACCEL_SOFTWARE:
				accelType = "Software";
				break;
		}

		// Print to log
		std::vector<char> msgBuf(8192);
		sprintf(msgBuf.data(),
			" %4s | %8s | %11d | %8d | %11d | %9d | %7s | %4d - %4d | %7d | %3s | %10d ",
			(i == 0 ? "AVC" : "SVC"),
			accelType,
			capsEnc[i]->maxBitrate,
			capsEnc[i]->maxNumOfStreams, 
			capsEnc[i]->maxProfile,
			capsEnc[i]->maxProfileLevel,
			capsEnc[i]->supportsBFrames ? "Yes" : "No",
			capsEnc[i]->minReferenceFrames, capsEnc[i]->maxReferenceFrames,
			capsEnc[i]->maxTemporalLayers,
			capsEnc[i]->supportsFixedSliceMode ? "Yes" : "No",
			capsEnc[i]->maxNumOfHwInstances);
		AMF_LOG_INFO("%s", msgBuf.data());

		/*H264Capabilities::EncoderCaps::IOCaps* capsIO[2] = { &capsEnc[i]->input, &capsEnc[i]->output };
		for (uint8_t j = 0; j < 2; j++) {
			if (j == 0)
				AMF_LOG_INFO("<Plugin::AMD::H264Capabilities::reportCapabilities> 		Input:");
			else
				AMF_LOG_INFO("<Plugin::AMD::H264Capabilities::reportCapabilities> 		Output:");

			AMF_LOG_INFO("<Plugin::AMD::H264Capabilities::reportCapabilities> 			Width Range: %d, %d", capsIO[j]->minWidth, capsIO[j]->maxWidth);
			AMF_LOG_INFO("<Plugin::AMD::H264Capabilities::reportCapabilities> 			Height Range: %d, %d", capsIO[j]->minHeight, capsIO[j]->maxHeight);
			AMF_LOG_INFO("<Plugin::AMD::H264Capabilities::reportCapabilities> 			Supports Interlaced: %s", capsIO[j]->isInterlacedSupported ? "Yes" : "No");
			AMF_LOG_INFO("<Plugin::AMD::H264Capabilities::reportCapabilities> 			Vertical Buffer Alignment: %d bytes", capsIO[j]->verticalAlignment);

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
			AMF_LOG_INFO("<Plugin::AMD::H264Capabilities::reportCapabilities> 			Surface Formats:");
			for (uint32_t k = 0; k < capsIO[j]->formats.size(); k++) {
				AMF_LOG_INFO("<Plugin::AMD::H264Capabilities::reportCapabilities> 			- %s%s", surfaceFormat[capsIO[j]->formats[k].first], capsIO[j]->formats[k].second ? " (Native)" : "");
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
			AMF_LOG_INFO("<Plugin::AMD::H264Capabilities::reportCapabilities> 			Memory Types:");
			for (uint32_t k = 0; k < capsIO[j]->memoryTypes.size(); k++) {
				AMF_LOG_INFO("<Plugin::AMD::H264Capabilities::reportCapabilities> 			- %s%s", memoryTypes[capsIO[j]->memoryTypes[k].first], capsIO[j]->memoryTypes[k].second ? " (Native)" : "");
			}
		}*/
	}
	#pragma endregion
}

Plugin::AMD::H264Capabilities::H264Capabilities() {
	refreshCapabilities();
}

Plugin::AMD::H264Capabilities::~H264Capabilities() {

}

bool Plugin::AMD::H264Capabilities::refreshCapabilities() {
	AMF_RESULT res;

	std::shared_ptr<AMD::AMF> l_AMF = AMD::AMF::GetInstance();
	amf::AMFFactory* l_AMFFactory = l_AMF->GetFactory();
	amf::AMFContextPtr l_AMFContext;
	res = l_AMFFactory->CreateContext(&l_AMFContext);
	if (res != AMF_OK) {
		AMF_LOG_ERROR("Failed to gather Capabilities, error code %d.", res);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Get Encoder Capabilities
	//////////////////////////////////////////////////////////////////////////
	EncoderCaps* caps[2] = { &m_AVCCaps, &m_SVCCaps };
	const wchar_t* capsString[2] = { AMFVideoEncoderVCE_AVC , AMFVideoEncoderVCE_SVC };
	for (uint8_t capsIndex = 0; capsIndex < 2; capsIndex++) {
		amf::AMFComponentPtr l_AMFComponent;
		res = l_AMFFactory->CreateComponent(l_AMFContext, capsString[capsIndex], &l_AMFComponent);
		if (res != AMF_OK) {
			AMF_LOG_ERROR("Failed to gather Capabilities for Encoder Type %s, error code %d.", (capsIndex ? "SVC" : "AVC"), res);
			break;
		}
		amf::AMFCapsPtr encCaps;
		res = l_AMFComponent->GetCaps(&encCaps);
		if (res != AMF_OK) {
			AMF_LOG_ERROR("Failed to gather Capabilities for Encoder Type %s, error code %d.", (capsIndex ? "SVC" : "AVC"), res);
			l_AMFComponent->Terminate();
			break;
		}

		// Basic Capabilities
		caps[capsIndex]->acceleration_type = encCaps->GetAccelerationType();
		encCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_BITRATE, &(caps[capsIndex]->maxBitrate));
		encCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_NUM_OF_STREAMS, &(caps[capsIndex]->maxNumOfStreams));
		encCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_PROFILE, &(caps[capsIndex]->maxProfile));
		encCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_LEVEL, &(caps[capsIndex]->maxProfileLevel));
		encCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_BFRAMES, &(caps[capsIndex]->supportsBFrames));
		encCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MIN_REFERENCE_FRAMES, &(caps[capsIndex]->minReferenceFrames));
		encCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_REFERENCE_FRAMES, &(caps[capsIndex]->maxReferenceFrames));
		encCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_TEMPORAL_LAYERS, &(caps[capsIndex]->maxTemporalLayers));
		encCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_FIXED_SLICE_MODE, &(caps[capsIndex]->supportsFixedSliceMode));
		encCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_NUM_OF_HW_INSTANCES, &(caps[capsIndex]->maxNumOfHwInstances));

		// Input & Output Capabilities
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

		l_AMFComponent->Terminate();
	}
	l_AMFContext->Terminate();

	return true;
}

Plugin::AMD::H264Capabilities::EncoderCaps* Plugin::AMD::H264Capabilities::getEncoderCaps(H264EncoderType type) {
	EncoderCaps* caps[2] = { &m_AVCCaps, &m_SVCCaps };
	return caps[type];
}

Plugin::AMD::H264Capabilities::EncoderCaps::IOCaps* Plugin::AMD::H264Capabilities::getIOCaps(H264EncoderType type, bool output) {
	EncoderCaps* caps[2] = { &m_AVCCaps, &m_SVCCaps };
	if (output)
		return &caps[type]->output;
	else
		return &caps[type]->input;
}

