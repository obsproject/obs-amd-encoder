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

#include "amf-capabilities.h"
#include "api-d3d11.h"
#include "api-d3d9.h"
#include "misc-util.cpp"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////

Plugin::AMD::VCEDeviceCapabilities::VCEDeviceCapabilities() {
	acceleration_type = amf::AMF_ACCEL_NOT_SUPPORTED;
	maxProfile =
		maxProfileLevel =
		maxBitrate =
		minReferenceFrames =
		maxReferenceFrames =
		maxTemporalLayers =
		maxNumOfStreams =
		maxNumOfHwInstances =
		input.minWidth =
		input.maxWidth =
		input.minHeight =
		input.maxHeight =
		input.verticalAlignment =
		output.minWidth =
		output.maxWidth =
		output.minHeight =
		output.maxHeight =
		output.verticalAlignment = 0;
	supportsBFrames =
		supportsFixedSliceMode =
		input.supportsInterlaced =
		output.supportsInterlaced = false;
	input.formats.clear();
	input.memoryTypes.clear();
	output.formats.clear();
	output.memoryTypes.clear();
}

std::shared_ptr<Plugin::AMD::VCECapabilities> Plugin::AMD::VCECapabilities::GetInstance() {
	static std::shared_ptr<VCECapabilities> __instance = std::make_shared<VCECapabilities>();
	static std::mutex __mutex;

	const std::lock_guard<std::mutex> lock(__mutex);
	return __instance;
}

void Plugin::AMD::VCECapabilities::ReportCapabilities(std::shared_ptr<Plugin::API::Base> api) {
	auto inst = GetInstance();
	auto adapters = api->EnumerateAdapters();
	for (auto adapter : adapters) {
		ReportAdapterCapabilities(api, adapter);
	}
}

void Plugin::AMD::VCECapabilities::ReportAdapterCapabilities(std::shared_ptr<Plugin::API::Base> api, Plugin::API::Adapter adapter) {
	auto inst = GetInstance();
	H264EncoderType types[] = {
		H264EncoderType::AVC,
		H264EncoderType::SVC,
	};

	AMF_LOG_INFO("Capabilities for %s adapter '%s':",
		api->GetName().c_str(),
		adapter.Name.c_str());

	for (auto type : types) {
		ReportAdapterTypeCapabilities(api, adapter, type);
	}
}

void Plugin::AMD::VCECapabilities::ReportAdapterTypeCapabilities(std::shared_ptr<Plugin::API::Base> api, Plugin::API::Adapter adapter, H264EncoderType type) {
	auto inst = GetInstance();
	auto caps = inst->GetAdapterCapabilities(api, adapter, type);

	AMF_LOG_INFO("  %s (Acceleration: %s)",
		(type == H264EncoderType::AVC ? "AVC" : (type == H264EncoderType::SVC ? "SVC" : "Unknown")),
		(caps.acceleration_type == amf::AMF_ACCEL_SOFTWARE ? "Software" : (caps.acceleration_type == amf::AMF_ACCEL_GPU ? "GPU" : (caps.acceleration_type == amf::AMF_ACCEL_HARDWARE ? "Hardware" : "None")))
	);

	if (caps.acceleration_type == amf::AMF_ACCEL_NOT_SUPPORTED)
		return;

	AMF_LOG_INFO("    Limits");
	AMF_LOG_INFO("      Profile: %s", Plugin::Utility::ProfileAsString((H264Profile)caps.maxProfile));
	AMF_LOG_INFO("      Profile Level: %ld.%ld", caps.maxProfileLevel / 10, caps.maxProfileLevel % 10);
	AMF_LOG_INFO("      Bitrate: %ld", caps.maxBitrate);
	AMF_LOG_INFO("      Reference Frames: %ld (min) - %ld (max)", caps.minReferenceFrames, caps.maxReferenceFrames);
	if (type == H264EncoderType::SVC)
		AMF_LOG_INFO("      Temporal Layers: %ld", caps.maxTemporalLayers);
	AMF_LOG_INFO("    Features");
	AMF_LOG_INFO("      B-Frames: %s", caps.supportsBFrames ? "Supported" : "Not Supported");
	AMF_LOG_INFO("      Fixed Slice Mode: %s", caps.supportsFixedSliceMode ? "Supported" : "Not Supported");
	AMF_LOG_INFO("    Instancing");
	AMF_LOG_INFO("      # of Streams: %ld", caps.maxNumOfStreams);
	AMF_LOG_INFO("      # of Instances: %ld", caps.maxNumOfHwInstances);
	AMF_LOG_INFO("    Input");
	ReportAdapterTypeIOCapabilities(api, adapter, type, false);
	AMF_LOG_INFO("    Output");
	ReportAdapterTypeIOCapabilities(api, adapter, type, true);
}

void Plugin::AMD::VCECapabilities::ReportAdapterTypeIOCapabilities(std::shared_ptr<Plugin::API::Base> api, Plugin::API::Adapter adapter, H264EncoderType type, bool output) {
	auto amf = Plugin::AMD::AMF::GetInstance();

	auto inst = GetInstance();
	VCEDeviceCapabilities::IOCaps ioCaps = output
		? inst->GetAdapterCapabilities(api, adapter, type).output
		: inst->GetAdapterCapabilities(api, adapter, type).input;
	AMF_LOG_INFO("      Resolution: %ldx%ld - %ldx%ld",
		ioCaps.minWidth, ioCaps.minHeight,
		ioCaps.maxWidth, ioCaps.maxHeight);
	AMF_LOG_INFO("      Vertical Alignment: %ld", ioCaps.verticalAlignment);
	AMF_LOG_INFO("      Interlaced: %s", ioCaps.supportsInterlaced ? "Supported" : "Not Supported");
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
	caps->supportsInterlaced = amfIOCaps->IsInterlacedSupported();
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

	auto amfInstance = AMD::AMF::GetInstance();
	auto amfFactory = amfInstance->GetFactory();

	auto APIs = Plugin::API::Base::EnumerateAPIs();
	for (auto api : APIs) {
		std::vector<Plugin::API::Adapter> adapters;
		try {
			adapters = api->EnumerateAdapters();
		} catch (std::exception e) {
			AMF_LOG_ERROR("<" __FUNCTION_NAME__ "> Exception while enumerating %s adapters: %s.",
				api->GetName().c_str(),
				e.what());
			continue;
		} catch (...) {
			AMF_LOG_ERROR("<" __FUNCTION_NAME__ "> Critical exception while enumerating %s adapters.",
				api->GetName().c_str());
			throw;
		}

		for (auto adapter : adapters) {
			// Create AMF Instance
			amf::AMFContextPtr amfContext;
			res = amfFactory->CreateContext(&amfContext);
			if (res != AMF_OK) {
				AMF_LOG_ERROR("<" __FUNCTION_NAME__ "> Unable to create context on %s adapter '%s', error %ls (code %d).",
					api->GetName().c_str(),
					adapter.Name.c_str(),
					amfInstance->GetTrace()->GetResultText(res),
					res);
				continue;
			}

			void* apiInst = nullptr;
			try {
				apiInst = api->CreateInstanceOnAdapter(adapter);
			} catch (std::exception e) {
				AMF_LOG_ERROR("<" __FUNCTION_NAME__ "> Exception while intializing %s adapter '%s': %s.",
					api->GetName().c_str(),
					adapter.Name,
					e.what());
				continue;
			} catch (...) {
				AMF_LOG_ERROR("<" __FUNCTION_NAME__ "> Critical exception while intializing %s adapter '%s'.",
					api->GetName().c_str(),
					adapter.Name);
				throw;
			}
			switch (api->GetType()) {
				case Plugin::API::Type::Direct3D11:
					res = amfContext->InitDX11(api->GetContextFromInstance(apiInst));
					break;
				case Plugin::API::Type::Direct3D9:
					res = amfContext->InitDX9(api->GetContextFromInstance(apiInst));
					break;
				case Plugin::API::Type::OpenGL:
					res = amfContext->InitOpenGL(api->GetContextFromInstance(apiInst), nullptr, nullptr);
					break;
				default:
					res = AMF_OK;
					break;
			}
			if (res != AMF_OK) {
				AMF_LOG_ERROR("<" __FUNCTION_NAME__ "> Initialization failed for %s adapter '%s', error %ls (code %d).",
					api->GetName().c_str(),
					adapter.Name.c_str(),
					amfInstance->GetTrace()->GetResultText(res),
					res);
				continue;
			}

			H264EncoderType types[] = {
				H264EncoderType::AVC,
				H264EncoderType::SVC
			};
			for (auto type : types) {
				VCEDeviceCapabilities devCaps = VCEDeviceCapabilities();

				amf::AMFComponentPtr amfComponent;
				res = amfFactory->CreateComponent(amfContext,
					Plugin::Utility::VCEEncoderTypeAsAMF(type),
					&amfComponent);
				if (res != AMF_OK) {
					AMF_LOG_ERROR("<" __FUNCTION_NAME__ "> Unable to create component for %s adapter '%s' with codec '%s', error %ls (code %d).",
						api->GetName().c_str(),
						adapter.Name.c_str(),
						Plugin::Utility::VCEEncoderTypeAsString(type),
						amfInstance->GetTrace()->GetResultText(res), res);
					continue;
				}

				amf::AMFCapsPtr amfCaps;
				res = amfComponent->GetCaps(&amfCaps);
				if (res != AMF_OK) {
					AMF_LOG_ERROR("<" __FUNCTION_NAME__ "> Unable to query capabilities for %s adapter '%s' with codec '%s', error %ls (code %d).",
						api->GetName().c_str(),
						adapter.Name.c_str(),
						Plugin::Utility::VCEEncoderTypeAsString(type),
						amfInstance->GetTrace()->GetResultText(res), res);
					amfComponent->Terminate();
					continue;
				}

				devCaps.acceleration_type = amfCaps->GetAccelerationType();
				if (devCaps.acceleration_type != amf::AMF_ACCEL_NOT_SUPPORTED) {
					amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_BITRATE, &(devCaps.maxBitrate));
					amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_NUM_OF_STREAMS, &(devCaps.maxNumOfStreams));
					amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_PROFILE, &(devCaps.maxProfile));
					amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_LEVEL, &(devCaps.maxProfileLevel));
					amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_BFRAMES, &(devCaps.supportsBFrames));
					amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MIN_REFERENCE_FRAMES, &(devCaps.minReferenceFrames));
					amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_REFERENCE_FRAMES, &(devCaps.maxReferenceFrames));
					amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_TEMPORAL_LAYERS, &(devCaps.maxTemporalLayers));
					amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_FIXED_SLICE_MODE, &(devCaps.supportsFixedSliceMode));
					amfCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_NUM_OF_HW_INSTANCES, &(devCaps.maxNumOfHwInstances));

					res = GetIOCapability(false, amfCaps, &(devCaps.input));
					if (res != AMF_OK)
						AMF_LOG_ERROR("<" __FUNCTION_NAME__ "> Unable to query input capabilities for %s adapter '%s' with codec '%s', error %ls (code %d).",
							api->GetName().c_str(),
							adapter.Name.c_str(),
							Plugin::Utility::VCEEncoderTypeAsString(type),
							amfInstance->GetTrace()->GetResultText(res), res);

					res = GetIOCapability(true, amfCaps, &(devCaps.output));
					if (res != AMF_OK)
						AMF_LOG_ERROR("<" __FUNCTION_NAME__ "> Unable to query output capabilities for %s adapter '%s' with codec '%s', error %ls (code %d).",
							api->GetName().c_str(),
							adapter.Name.c_str(),
							Plugin::Utility::VCEEncoderTypeAsString(type),
							amfInstance->GetTrace()->GetResultText(res), res);
				}

				amfComponent->Terminate();

				// Insert
				capabilityMap.insert(std::make_pair(
					std::make_tuple(api->GetName(), adapter, type),
					devCaps)
				);
			}
			api->DestroyInstance(apiInst);
			amfContext->Terminate();
		}
	}
	return true;
}

std::vector<std::pair<H264EncoderType, VCEDeviceCapabilities>> Plugin::AMD::VCECapabilities::GetAllAdapterCapabilities(std::shared_ptr<Plugin::API::Base> api, Plugin::API::Adapter adapter) {
	std::vector<std::pair<H264EncoderType, VCEDeviceCapabilities>> caps;
	for (auto kv : capabilityMap) {
		auto apiName = std::get<0>(kv.first);
		auto curAdapter = std::get<1>(kv.first);

		if (apiName != api->GetName())
			continue;
		if (curAdapter != adapter)
			continue;

		auto type = std::get<2>(kv.first);
		caps.push_back(std::make_pair(type, kv.second));
	}
	return caps;
}

Plugin::AMD::VCEDeviceCapabilities Plugin::AMD::VCECapabilities::GetAdapterCapabilities(std::shared_ptr<Plugin::API::Base> api, Plugin::API::Adapter adapter, H264EncoderType type) {
	auto key = std::make_tuple(api->GetName(), adapter, type);

	if (capabilityMap.count(key) == 0)
		return VCEDeviceCapabilities();

	return capabilityMap.find(key)->second;
}

