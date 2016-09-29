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
#include <string>
#include <sstream>

#include "amd-amf-vce-capabilities.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////

std::shared_ptr<Plugin::AMD::VCECapabilities> Plugin::AMD::VCECapabilities::GetInstance() {
	static std::shared_ptr<VCECapabilities> __instance = std::make_shared<VCECapabilities>();
	static std::mutex __mutex;

	try {
		const std::lock_guard<std::mutex> lock(__mutex);
		return __instance;
	} catch (...) {
		return nullptr;
	}
}

void Plugin::AMD::VCECapabilities::ReportCapabilities() {
	static std::vector<char> msgBuf(8192);

	//////////////////////////////////////////////////////////////////////////
	// Report Capabilities to log file first.
	//////////////////////////////////////////////////////////////////////////
	#pragma region Capability Reporting

	AMF_LOG_INFO("Gathering Capability Information...");
	auto caps = VCECapabilities::GetInstance();

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

	VCECapabilities::EncoderCaps* capsEnc[] = { &caps->m_AVCCaps, &caps->m_SVCCaps, &caps->m_HEVCCaps };
	for (uint8_t i = 0; i < 3; i++) {
		// Encoder Acceleration
		char* accelType = "";
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
		sprintf_s(msgBuf.data(), msgBuf.size(),
			" %4s | %8s | %11d | %8d | %11d | %9d | %7s | %4d - %4d | %7d | %3s | %10d ",
			(i == 0 ? "AVC" : (i == 1 ? "SVC" : "HEVC")),
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
	}

	// Type | Flow   | Min. Res. | Max. Res. | S.I | Align | Formats | Surface Types
	//      |  Input |   64x  64 | 4096x4096 | Yes | 8     | ... | ...
	//      | Output |   64x  64 | 4096x4096 | No  | 8     | ... | ...
	AMF_LOG_INFO(" %4s | %6s | %9s | %9s | %3s | %5s | %7s | %12s",
		"Type",
		"Flow",
		"Min. Res.",
		"Max. Res.",
		"S.I",
		"Align",
		"Formats",
		"Memory Types");
	for (uint8_t i = 0; i < 3; i++) {
		VCECapabilities::EncoderCaps::IOCaps* capsIO[2] = { &capsEnc[i]->input, &capsEnc[i]->output };
		for (uint8_t j = 0; j < 2; j++) {
			std::shared_ptr<AMF> t_amf = AMF::GetInstance();
			std::stringstream formats, memtypes;

			// Surface Formats
			for (uint32_t k = 0; k < capsIO[j]->formats.size(); k++) {
				wcstombs(msgBuf.data(), t_amf->GetTrace()->SurfaceGetFormatName(capsIO[j]->formats[k].first), 1024);
				formats << msgBuf.data();
				if (capsIO[j]->formats[k].second)
					formats << " (Native)";
				if (k < capsIO[j]->formats.size() - 1)
					formats << ", ";
			}

			// Memory Types
			for (uint32_t k = 0; k < capsIO[j]->memoryTypes.size(); k++) {
				wcstombs(msgBuf.data(), t_amf->GetTrace()->GetMemoryTypeName(capsIO[j]->memoryTypes[k].first), 1024);
				memtypes << msgBuf.data();
				if (capsIO[j]->memoryTypes[k].second)
					memtypes << " (Native)";
				if (k < capsIO[j]->memoryTypes.size() - 1)
					memtypes << ", ";
			}

			// Print to log
			sprintf_s(msgBuf.data(), msgBuf.size(), 
				" %4s | %6s | %4dx%4d | %4dx%4d | %3s | %5d | %7s | %12s",
				(i == 0 ? "AVC" : (i == 1 ? "SVC" : "HEVC")),
				(j == 0 ? "Input" : "Output"),
				capsIO[j]->minWidth, capsIO[j]->minHeight,
				capsIO[j]->maxWidth, capsIO[j]->maxHeight,
				capsIO[j]->isInterlacedSupported ? "Yes" : "No",
				capsIO[j]->verticalAlignment,
				formats.str().c_str(),
				memtypes.str().c_str());
			AMF_LOG_INFO("%s", msgBuf.data());
		}
	}
	#pragma endregion
}

Plugin::AMD::VCECapabilities::VCECapabilities() {
	RefreshCapabilities();
}

Plugin::AMD::VCECapabilities::~VCECapabilities() {

}

bool Plugin::AMD::VCECapabilities::RefreshCapabilities() {
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
	EncoderCaps* caps[] = { &m_AVCCaps, &m_SVCCaps, &m_HEVCCaps };
	const wchar_t* capsString[] = { AMFVideoEncoderVCE_AVC , AMFVideoEncoderVCE_SVC, L"AMFVideoEncoderHW_HEVC" };
	for (uint8_t capsIndex = 0; capsIndex < 3; capsIndex++) {
		// Null Values
		caps[capsIndex]->acceleration_type = amf::AMF_ACCEL_NOT_SUPPORTED;
		caps[capsIndex]->maxBitrate =
			caps[capsIndex]->maxNumOfStreams =
			caps[capsIndex]->maxProfile =
			caps[capsIndex]->maxProfileLevel =
			caps[capsIndex]->minReferenceFrames =
			caps[capsIndex]->maxReferenceFrames =
			caps[capsIndex]->maxTemporalLayers =
			caps[capsIndex]->maxNumOfHwInstances =
			caps[capsIndex]->input.minWidth =
			caps[capsIndex]->input.maxWidth =
			caps[capsIndex]->input.minHeight =
			caps[capsIndex]->input.maxHeight =
			caps[capsIndex]->input.verticalAlignment =
			caps[capsIndex]->output.minWidth =
			caps[capsIndex]->output.maxWidth =
			caps[capsIndex]->output.minHeight =
			caps[capsIndex]->output.maxHeight =
			caps[capsIndex]->output.verticalAlignment = 0;
		caps[capsIndex]->supportsBFrames =
			caps[capsIndex]->supportsFixedSliceMode =
			caps[capsIndex]->input.isInterlacedSupported =
			caps[capsIndex]->output.isInterlacedSupported = false;
		caps[capsIndex]->input.formats.clear();
		caps[capsIndex]->output.formats.clear();
		caps[capsIndex]->input.memoryTypes.clear();
		caps[capsIndex]->output.memoryTypes.clear();

		// Attempt to create stuff
		amf::AMFComponentPtr l_AMFComponent;
		res = l_AMFFactory->CreateComponent(l_AMFContext, capsString[capsIndex], &l_AMFComponent);
		if (res != AMF_OK) {
			AMF_LOG_ERROR("Failed to gather Capabilities for Encoder Type %s, error code %d.", (capsIndex == 0 ? "AVC" : (capsIndex == 1 ? "AVC" : "HEVC")), res);
			continue;
		}
		amf::AMFCapsPtr encCaps;
		res = l_AMFComponent->GetCaps(&encCaps);
		if (res != AMF_OK) {
			AMF_LOG_ERROR("Failed to gather Capabilities for Encoder Type %s, error code %d.", (capsIndex == 0 ? "AVC" : (capsIndex == 1 ? "AVC" : "HEVC")), res);
			l_AMFComponent->Terminate();
			continue;
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

Plugin::AMD::VCECapabilities::EncoderCaps* Plugin::AMD::VCECapabilities::GetEncoderCaps(VCEEncoderType type) {
	EncoderCaps* caps[2] = { &m_AVCCaps, &m_SVCCaps };
	return caps[type];
}

Plugin::AMD::VCECapabilities::EncoderCaps::IOCaps* Plugin::AMD::VCECapabilities::GetIOCaps(VCEEncoderType type, bool output) {
	EncoderCaps* caps[2] = { &m_AVCCaps, &m_SVCCaps };
	if (output)
		return &caps[type]->output;
	else
		return &caps[type]->input;
}

