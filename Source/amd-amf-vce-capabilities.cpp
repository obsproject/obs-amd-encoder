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

#ifdef _WIN32
#include <windows.h>
#include <VersionHelpers.h>
#endif

#include "amd-amf-vce-capabilities.h"
#include "api-d3d11.h"
#include "api-d3d9.h"
#include "misc-util.cpp"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////

std::shared_ptr<Plugin::AMD::VCECapabilities> Plugin::AMD::VCECapabilities::GetInstance() {
	static std::shared_ptr<VCECapabilities> __instance = std::make_shared<VCECapabilities>();
	static std::mutex __mutex;

	const std::lock_guard<std::mutex> lock(__mutex);
	return __instance;
}

void Plugin::AMD::VCECapabilities::ReportCapabilities() {
	auto inst = GetInstance();

	auto devs = inst->GetDevices();
	for (auto dev : devs) {
		ReportDeviceCapabilities(dev);
	}
}

void Plugin::AMD::VCECapabilities::ReportDeviceCapabilities(Plugin::API::Device device) {
	auto inst = GetInstance();

	AMF_LOG_INFO("Capabilities for Device '%s':", device.Name.c_str());

	VCEEncoderType types[] = { VCEEncoderType_AVC, VCEEncoderType_SVC };
	for (VCEEncoderType type : types) {
		auto caps = inst->GetDeviceCaps(device, type);

		AMF_LOG_INFO("  %s (Acceleration: %s)",
			(type == VCEEncoderType_AVC ? "AVC" : (type == VCEEncoderType_SVC ? "SVC" : (type == VCEEncoderType_HEVC ? "HEVC" : "Unknown"))),
			(caps.acceleration_type == amf::AMF_ACCEL_SOFTWARE ? "Software" : (caps.acceleration_type == amf::AMF_ACCEL_GPU ? "GPU" : (caps.acceleration_type == amf::AMF_ACCEL_HARDWARE ? "Hardware" : "None")))
		);

		if (caps.acceleration_type == amf::AMF_ACCEL_NOT_SUPPORTED)
			continue;

		AMF_LOG_INFO("    Limits");
		AMF_LOG_INFO("      # of Streams: %ld", caps.maxNumOfStreams);
		AMF_LOG_INFO("      # of Instances: %ld", caps.maxNumOfHwInstances);
		AMF_LOG_INFO("      Profile: %s", Plugin::Utility::ProfileAsString((VCEProfile)caps.maxProfile));
		AMF_LOG_INFO("      Level: %ld.%ld", caps.maxProfileLevel / 10, caps.maxProfileLevel % 10);
		AMF_LOG_INFO("      Bitrate: %ld", caps.maxBitrate);
		AMF_LOG_INFO("      Temporal Layers: %ld", caps.maxTemporalLayers);
		AMF_LOG_INFO("      Reference Frames: %ld (min) - %ld (max)", caps.minReferenceFrames, caps.maxReferenceFrames);
		AMF_LOG_INFO("    Features")
			AMF_LOG_INFO("      B-Frames: %s", caps.supportsBFrames ? "Supported" : "Not Supported");
		AMF_LOG_INFO("      Fixed Slice Mode: %s", caps.supportsFixedSliceMode ? "Supported" : "Not Supported");
		AMF_LOG_INFO("    Input");
		ReportDeviceIOCapabilities(device, type, false);
		AMF_LOG_INFO("    Output");
		ReportDeviceIOCapabilities(device, type, true);
	}
}

void Plugin::AMD::VCECapabilities::ReportDeviceIOCapabilities(Plugin::API::Device device, VCEEncoderType type, bool output) {
	auto amf = Plugin::AMD::AMF::GetInstance();
	auto inst = GetInstance();
	auto ioCaps = inst->GetDeviceIOCaps(device, type, output);
	AMF_LOG_INFO("      Resolution: %ldx%ld - %ldx%ld",
		ioCaps.minWidth, ioCaps.minHeight,
		ioCaps.maxWidth, ioCaps.maxHeight);
	AMF_LOG_INFO("      Vertical Alignment: %ld", ioCaps.verticalAlignment);
	AMF_LOG_INFO("      Interlaced: %s", ioCaps.isInterlacedSupported ? "Supported" : "Not Supported");
	std::stringstream formatstr;
	for (auto format : ioCaps.formats) {
		std::vector<char> buf(1024);
		wcstombs(buf.data(), amf->GetTrace()->SurfaceGetFormatName(format.first), 1024);
		formatstr
			<< buf.data()
			<< (format.second ? " (Native)" : "")
			<< ", ";
	}
	AMF_LOG_INFO("      Formats: %s", formatstr.str().c_str());
	std::stringstream memorystr;
	for (auto memory : ioCaps.memoryTypes) {
		std::vector<char> buf(1024);
		wcstombs(buf.data(), amf->GetTrace()->GetMemoryTypeName(memory.first), 1024);
		memorystr
			<< buf.data()
			<< (memory.second ? " (Native)" : "")
			<< ", ";
	}
	AMF_LOG_INFO("      Memory Types: %s", memorystr.str().c_str());
}

Plugin::AMD::VCECapabilities::VCECapabilities() {
	this->Refresh();
}

Plugin::AMD::VCECapabilities::~VCECapabilities() {}

static AMF_RESULT GetIOCapability(bool output, amf::AMFCapsPtr amfCaps, Plugin::AMD::VCEDeviceCapabilities::IOCaps* caps) {
	AMF_RESULT res = AMF_OK;
	amf::AMFIOCapsPtr amfIOCaps;
	if (output)
		res = amfCaps->GetOutputCaps(&amfIOCaps);
	else
		res = amfCaps->GetInputCaps(&amfIOCaps);
	if (res != AMF_OK)
		return res;

	amfIOCaps->GetWidthRange(&(caps->minWidth), &(caps->maxWidth));
	amfIOCaps->GetHeightRange(&(caps->minHeight), &(caps->maxHeight));
	caps->isInterlacedSupported = amfIOCaps->IsInterlacedSupported();
	caps->verticalAlignment = amfIOCaps->GetVertAlign();

	int32_t numFormats = amfIOCaps->GetNumOfFormats();
	caps->formats.resize(numFormats);
	for (int32_t formatIndex = 0; formatIndex < numFormats; formatIndex++) {
		amf::AMF_SURFACE_FORMAT format = amf::AMF_SURFACE_UNKNOWN;
		bool isNative = false;

		amfIOCaps->GetFormatAt(formatIndex, &format, &isNative);
		caps->formats[formatIndex].first = format;
		caps->formats[formatIndex].second = isNative;
	}

	int32_t numMemoryTypes = amfIOCaps->GetNumOfMemoryTypes();
	caps->memoryTypes.resize(numMemoryTypes);
	for (int32_t memoryTypeIndex = 0; memoryTypeIndex < numMemoryTypes; memoryTypeIndex++) {
		amf::AMF_MEMORY_TYPE type = amf::AMF_MEMORY_UNKNOWN;
		bool isNative = false;

		amfIOCaps->GetMemoryTypeAt(memoryTypeIndex, &type, &isNative);
		caps->memoryTypes[memoryTypeIndex].first = type;
		caps->memoryTypes[memoryTypeIndex].second = isNative;
	}

	return AMF_OK;
}

bool Plugin::AMD::VCECapabilities::Refresh() {
	AMF_RESULT res;

	// Build a list of Devices
	#ifdef _WIN32
	if (IsWindows8OrGreater()) {
		devices = Plugin::API::Direct3D11::EnumerateDevices();
	} else if (IsWindowsXPOrGreater()) {
		devices = Plugin::API::Direct3D9::EnumerateDevices();
	} else
		#endif 
	{ // OpenGL
		//devices = Plugin::API::OpenGL::EnumerateDevices();
	}
	//devices.insert(devices.begin(), Plugin::API::Device());

	// Query Information for each Device
	std::shared_ptr<AMD::AMF> amfInstance = AMD::AMF::GetInstance();
	amf::AMFFactory* amfFactory = amfInstance->GetFactory();
	for (Plugin::API::Device device : devices) {
		amf::AMFContextPtr amfContext;
		res = amfFactory->CreateContext(&amfContext);
		if (res != AMF_OK) {
			AMF_LOG_ERROR("Unable to gather capabilities for device '%s', error %ls (code %d).",
				device.Name.c_str(), amfInstance->GetTrace()->GetResultText(res), res);
			continue;
		}

		auto apiDev = Plugin::API::APIBase::CreateBestAvailableAPI(device);
		switch (apiDev->GetType()) {
			case Plugin::API::APIType_Direct3D11:
				res = amfContext->InitDX11(apiDev->GetContext());
				break;
			case Plugin::API::APIType_Direct3D9:
				res = amfContext->InitDX9(apiDev->GetContext());
				break;
			case Plugin::API::APIType_OpenGL:
				res = amfContext->InitOpenGL(apiDev->GetContext(), nullptr, nullptr);
				break;
		}
		if (res != AMF_OK) {
			AMF_LOG_ERROR("Unable to gather capabilities for device '%s' after initialization, error %ls (code %d).",
				device.Name.c_str(), amfInstance->GetTrace()->GetResultText(res), res);
			continue;
		}

		VCEDeviceCapabilities devAVCCaps, devSVCCaps;
		const wchar_t* capsString[] = {
			AMFVideoEncoderVCE_AVC,
			AMFVideoEncoderVCE_SVC,
		};
		VCEDeviceCapabilities* caps[] = {
			&devAVCCaps,
			&devSVCCaps,
		};
		VCEEncoderType capsType[] = {
			VCEEncoderType_AVC,
			VCEEncoderType_SVC,
		};

		for (uint8_t capsIndex = 0; capsIndex < _countof(caps); capsIndex++) {
			#pragma region Null Structure
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
			#pragma endregion Null Structure

			#pragma region Initialization
			amf::AMFComponentPtr amfComponent;
			res = amfFactory->CreateComponent(amfContext, capsString[capsIndex], &amfComponent);
			if (res != AMF_OK) {
				AMF_LOG_ERROR("<" __FUNCTION_NAME__ "> Failed to create component for device '%s' with codec '%ls', error %ls (code %d).",
					device.Name.c_str(), capsString[capsIndex],
					amfInstance->GetTrace()->GetResultText(res), res);
				continue;
			}

			amf::AMFCapsPtr amfCaps;
			res = amfComponent->GetCaps(&amfCaps);
			if (res != AMF_OK) {
				AMF_LOG_ERROR("<" __FUNCTION_NAME__ "> Failed to gather capabilities for device '%s' with codec '%ls', error %ls (code %d).",
					device.Name.c_str(), capsString[capsIndex],
					amfInstance->GetTrace()->GetResultText(res), res);
				amfComponent->Terminate();
				continue;
			}
			#pragma endregion Initialization

			#pragma region Basic Capabilities
			caps[capsIndex]->acceleration_type = amfCaps->GetAccelerationType();
			amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_BITRATE, &(caps[capsIndex]->maxBitrate));
			amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_NUM_OF_STREAMS, &(caps[capsIndex]->maxNumOfStreams));
			amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_PROFILE, &(caps[capsIndex]->maxProfile));
			amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_LEVEL, &(caps[capsIndex]->maxProfileLevel));
			amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_BFRAMES, &(caps[capsIndex]->supportsBFrames));
			amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MIN_REFERENCE_FRAMES, &(caps[capsIndex]->minReferenceFrames));
			amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_REFERENCE_FRAMES, &(caps[capsIndex]->maxReferenceFrames));
			amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_TEMPORAL_LAYERS, &(caps[capsIndex]->maxTemporalLayers));
			amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_FIXED_SLICE_MODE, &(caps[capsIndex]->supportsFixedSliceMode));
			amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_NUM_OF_HW_INSTANCES, &(caps[capsIndex]->maxNumOfHwInstances));
			#pragma endregion Basic Capabilities

			if (GetIOCapability(false, amfCaps, &(caps[capsIndex]->input))) {
				AMF_LOG_ERROR("<" __FUNCTION_NAME__ "> Failed to gather input capabilities for device '%s' with codec '%ls', error %ls (code %d).",
					device.Name.c_str(), capsString[capsIndex],
					amfInstance->GetTrace()->GetResultText(res), res);
			}
			if (GetIOCapability(true, amfCaps, &(caps[capsIndex]->output))) {
				AMF_LOG_ERROR("<" __FUNCTION_NAME__ "> Failed to gather output capabilities for device '%s' with codec '%ls', error %ls (code %d).",
					device.Name.c_str(), capsString[capsIndex],
					amfInstance->GetTrace()->GetResultText(res), res);
			}

			amfComponent->Terminate();

			// Register
			deviceToCapabilities.insert_or_assign(
				std::pair<Plugin::API::Device, Plugin::AMD::VCEEncoderType>(device, capsType[capsIndex]),
				*caps[capsIndex]);
		}

		amfContext->Terminate();
	}

	return true;
}

std::vector<Plugin::API::Device> Plugin::AMD::VCECapabilities::GetDevices() {
	return std::vector<Plugin::API::Device>(devices);
}

Plugin::AMD::VCEDeviceCapabilities Plugin::AMD::VCECapabilities::GetDeviceCaps(Plugin::API::Device device, VCEEncoderType type) {
	if (device.UniqueId == "")
		return deviceToCapabilities.begin()->second;

	auto dt = std::pair<Plugin::API::Device, Plugin::AMD::VCEEncoderType>(device, type);
	if (deviceToCapabilities.count(dt) == 0)
		return Plugin::AMD::VCEDeviceCapabilities();

	return deviceToCapabilities.find(dt)->second;
}

Plugin::AMD::VCEDeviceCapabilities::IOCaps Plugin::AMD::VCECapabilities::GetDeviceIOCaps(Plugin::API::Device device, VCEEncoderType type, bool output) {
	if (device.UniqueId == "") {
		if (output)
			return deviceToCapabilities.begin()->second.output;
		else
			return deviceToCapabilities.begin()->second.input;
	}

	auto dt = std::pair<Plugin::API::Device, Plugin::AMD::VCEEncoderType>(device, type);
	if (deviceToCapabilities.count(dt) == 0)
		return Plugin::AMD::VCEDeviceCapabilities::IOCaps();

	if (output)
		return deviceToCapabilities.find(dt)->second.output;
	else
		return deviceToCapabilities.find(dt)->second.input;
}

