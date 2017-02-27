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
#include "enc-h264.h"
#include "misc-util.cpp"

#ifdef _WIN32
#include <VersionHelpers.h>

#include "api-d3d9.h"
#include "api-d3d11.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin;
using namespace Plugin::AMD;
using namespace Plugin::Interface;

enum class Presets : int8_t {
	None = -1,
	ResetToDefaults = 0,
	Recording,
	HighQuality,
	Indistinguishable,
	Lossless,
	Twitch,
	YouTube,
};
enum class ViewMode :uint8_t {
	Basic,
	Advanced,
	Expert,
	Master
};

void Plugin::Interface::H264Interface::encoder_register() {
	// Ensure that there is a supported AMD GPU.
	bool haveAVCsupport = false;
	for (auto api : Plugin::API::Base::EnumerateAPIs()) {
		for (auto adapter : api->EnumerateAdapters()) {
			auto caps = VCECapabilities::GetInstance()->GetAdapterCapabilities(api, adapter, H264EncoderType::AVC);
			if (caps.acceleration_type != amf::AMF_ACCEL_NOT_SUPPORTED)
				haveAVCsupport = true;
		}
	}
	if (!haveAVCsupport) {
		AMF_LOG_WARNING("No detected GPU supports H264 encoding.");
		return;
	}

	// Create structure
	static std::unique_ptr<obs_encoder_info> encoder_info = std::make_unique<obs_encoder_info>();
	std::memset(encoder_info.get(), 0, sizeof(obs_encoder_info));

	// Initialize Structure
	encoder_info->type = obs_encoder_type::OBS_ENCODER_VIDEO;
	static const char* encoder_name = "amd_amf_h264";
	encoder_info->id = encoder_name;
	static const char* encoder_codec = "h264";
	encoder_info->codec = encoder_codec;

	// Functions
	encoder_info->get_name = &get_name;
	encoder_info->get_defaults = &get_defaults;
	encoder_info->get_properties = &get_properties;
	encoder_info->create = &create;
	encoder_info->destroy = &destroy;
	encoder_info->encode = &encode;
	encoder_info->update = &update;
	encoder_info->get_video_info = &get_video_info;
	encoder_info->get_extra_data = &get_extra_data;

	obs_register_encoder(encoder_info.get());
}

const char* Plugin::Interface::H264Interface::get_name(void*) {
	static const char* name = "H264 Encoder (AMD Advanced Media Framework)";
	return name;
}

void* Plugin::Interface::H264Interface::create(obs_data_t* settings, obs_encoder_t* encoder) {
	Plugin::Interface::H264Interface* enc = nullptr;
	try {
		AMF_LOG_INFO("Starting up...");
		enc = new Plugin::Interface::H264Interface(settings, encoder);
		return enc;
	} catch (std::exception e) {
		AMF_LOG_ERROR("%s", e.what());
	} catch (std::exception* e) {
		AMF_LOG_ERROR("%s", e->what());
		delete e;
	} catch (...) {
		AMF_LOG_ERROR("Unknown Exception.");
	}
	if (enc)
		delete enc;
	return NULL;
}

void Plugin::Interface::H264Interface::destroy(void* data) {
	try {
		AMF_LOG_INFO("Shutting down...");
		Plugin::Interface::H264Interface* enc = static_cast<Plugin::Interface::H264Interface*>(data);
		delete enc;
	} catch (std::exception e) {
		AMF_LOG_ERROR("%s", e.what());
	} catch (std::exception* e) {
		AMF_LOG_ERROR("%s", e->what());
		delete e;
	} catch (...) {
		AMF_LOG_ERROR("Unknown Exception.");
	}
	data = nullptr;
}

bool Plugin::Interface::H264Interface::encode(void *data, struct encoder_frame *frame, struct encoder_packet *packet, bool *received_packet) {
	try {
		return static_cast<Plugin::Interface::H264Interface*>(data)->encode(frame, packet, received_packet);
	} catch (std::exception e) {
		AMF_LOG_ERROR("%s", e.what());
	} catch (std::exception* e) {
		AMF_LOG_ERROR("%s", e->what());
		delete e;
	} catch (...) {
		AMF_LOG_ERROR("Unknown Exception.");
		throw;
	}
	return false;
}

void Plugin::Interface::H264Interface::get_defaults(obs_data_t *data) {
	#pragma region OBS - Enforce Streaming Service Restrictions
	obs_data_set_default_int(data, "bitrate", -1);
	obs_data_set_default_int(data, "keyint_sec", -1);
	obs_data_set_default_string(data, "rate_control", "");
	obs_data_set_default_string(data, "profile", "");
	obs_data_set_default_string(data, "preset", "");
	obs_data_set_int(data, "bitrate", -1);
	obs_data_set_int(data, "keyint_sec", -1);
	obs_data_set_string(data, "rate_control", "");
	obs_data_set_string(data, "profile", "");
	obs_data_set_string(data, "preset", "");
	#pragma endregion OBS - Enforce Streaming Service Restrictions

	// Preset
	obs_data_set_default_int(data, AMF_H264_PRESET, static_cast<int32_t>(Presets::None));

	// Static Properties
	obs_data_set_default_int(data, AMF_H264_USAGE, static_cast<int32_t>(H264Usage::Transcoding));
	obs_data_set_default_int(data, AMF_H264_QUALITY_PRESET, static_cast<int32_t>(H264QualityPreset::Balanced));
	obs_data_set_default_int(data, AMF_H264_PROFILE, static_cast<int32_t>(H264Profile::Main));
	obs_data_set_default_int(data, AMF_H264_PROFILELEVEL, static_cast<int32_t>(H264ProfileLevel::Automatic));

	// Rate Control Properties
	obs_data_set_int(data, "last" vstr(AMF_H264_RATECONTROLMETHOD), -1);
	obs_data_set_default_int(data, AMF_H264_RATECONTROLMETHOD, static_cast<int32_t>(H264RateControlMethod::ConstantBitrate));
	obs_data_set_default_int(data, AMF_H264_BITRATE_TARGET, 3500);
	obs_data_set_default_int(data, AMF_H264_BITRATE_PEAK, 9000);
	obs_data_set_default_int(data, AMF_H264_QP_MINIMUM, 11);
	obs_data_set_default_int(data, AMF_H264_QP_MAXIMUM, 51);
	obs_data_set_default_int(data, AMF_H264_QP_IFRAME, 22);
	obs_data_set_default_int(data, AMF_H264_QP_PFRAME, 22);
	obs_data_set_default_int(data, AMF_H264_QP_BFRAME, 22);
	obs_data_set_int(data, "last" vstr(AMF_H264_VBVBUFFER), -1);
	obs_data_set_default_int(data, AMF_H264_VBVBUFFER, 0);
	obs_data_set_default_int(data, AMF_H264_VBVBUFFER_SIZE, 3500);
	obs_data_set_default_double(data, AMF_H264_VBVBUFFER_STRICTNESS, 50);
	obs_data_set_default_double(data, AMF_H264_VBVBUFFER_FULLNESS, 100);
	obs_data_set_default_int(data, AMF_H264_MAXIMUMACCESSUNITSIZE, 0);
	obs_data_set_default_int(data, AMF_H264_FILLERDATA, 1);
	obs_data_set_default_int(data, AMF_H264_FRAMESKIPPING, 0);
	obs_data_set_default_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, 1);

	// Frame Control Properties
	obs_data_set_default_double(data, AMF_H264_KEYFRAME_INTERVAL, 2);
	obs_data_set_default_int(data, AMF_H264_IDR_PERIOD, 60);
	obs_data_set_int(data, "last" vstr(AMF_H264_BFRAME_PATTERN), -1);
	obs_data_set_default_int(data, AMF_H264_BFRAME_PATTERN, static_cast<int32_t>(H264BFramePattern::None));
	obs_data_set_int(data, "last" vstr(AMF_H264_BFRAME_REFERENCE), -1);
	obs_data_set_default_int(data, AMF_H264_BFRAME_REFERENCE, 0);
	obs_data_set_default_int(data, AMF_H264_BFRAME_REFERENCEDELTAQP, 2);
	obs_data_set_default_int(data, AMF_H264_BFRAME_DELTAQP, 4);
	obs_data_set_default_int(data, AMF_H264_DEBLOCKINGFILTER, 1);

	// Miscellaneous Control Properties
	obs_data_set_default_int(data, AMF_H264_SCANTYPE, static_cast<int32_t>(H264ScanType::Progressive));
	obs_data_set_default_int(data, AMF_H264_MOTIONESTIMATION, 3);

	// Experimental Properties
	obs_data_set_default_int(data, AMF_H264_MAXIMUMLTRFRAMES, 0);
	obs_data_set_default_int(data, AMF_H264_CODINGTYPE, static_cast<int32_t>(H264CodingType::Default));
	obs_data_set_default_int(data, AMF_H264_HEADER_INSERTION_SPACING, 0);
	obs_data_set_default_int(data, AMF_H264_SLICESPERFRAME, 1);
	obs_data_set_default_int(data, AMF_H264_SLICEMODE, static_cast<int32_t>(H264SliceMode::Horizontal));
	obs_data_set_default_int(data, AMF_H264_MAXIMUMSLICESIZE, INT_MAX);
	obs_data_set_default_int(data, AMF_H264_SLICECONTROLMODE, static_cast<int32_t>(H264SliceControlMode::Off));
	obs_data_set_default_int(data, AMF_H264_SLICECONTROLSIZE, 0);
	obs_data_set_default_int(data, AMF_H264_INTRAREFRESH_NUMBEROFSTRIPES, 0);
	obs_data_set_default_int(data, AMF_H264_INTRAREFRESH_MACROBLOCKSPERSLOT, 0);
	obs_data_set_default_int(data, AMF_H264_WAITFORTASK, 0);
	obs_data_set_default_int(data, AMF_H264_PREANALYSISPASS, 0);
	obs_data_set_default_int(data, AMF_H264_VBAQ, 0);
	obs_data_set_default_int(data, AMF_H264_GOPSIZE, 0);
	obs_data_set_default_int(data, AMF_H264_GOPALIGNMENT, 1);
	obs_data_set_default_int(data, AMF_H264_MAXIMUMREFERENCEFRAMES, 4);

	// System Properties
	obs_data_set_string(data, "last" vstr(AMF_H264_VIDEOAPI), "");
	obs_data_set_default_string(data, AMF_H264_VIDEOAPI, "");
	obs_data_set_int(data, "last" vstr(AMF_H264_VIDEOADAPTER), 0);
	obs_data_set_default_int(data, AMF_H264_VIDEOADAPTER, 0);
	obs_data_set_default_int(data, AMF_H264_OPENCL, 0);
	obs_data_set_int(data, "last" vstr(AMF_H264_VIEW), -1);
	obs_data_set_default_int(data, AMF_H264_VIEW, static_cast<int32_t>(ViewMode::Basic));
	obs_data_set_default_bool(data, AMF_H264_DEBUG, false);
	obs_data_set_default_int(data, AMF_H264_VERSION, PLUGIN_VERSION_FULL);
}

static void fill_api_list(obs_property_t* p) {
	obs_property_list_clear(p);
	for (auto api : Plugin::API::Base::EnumerateAPIs()) {
		obs_property_list_add_string(p, api->GetName().c_str(), api->GetName().c_str());
	}
}

static void fill_device_list(obs_property_t* p, const char* apiname) {
	obs_property_list_clear(p);
	auto api = Plugin::API::Base::GetAPIByName(std::string(apiname));
	for (auto adapter : api->EnumerateAdapters()) {
		obs_property_list_add_int(p, adapter.Name.c_str(), ((int64_t)adapter.idHigh << 32) + (int64_t)adapter.idLow);
	}
}

obs_properties_t* Plugin::Interface::H264Interface::get_properties(void*) {
	obs_properties* props = obs_properties_create();
	obs_property_t* p;

	#pragma region Preset
	p = obs_properties_add_list(props, AMF_H264_PRESET, TEXT_T(AMF_H264_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_modified_callback(p, properties_modified);
	obs_property_list_add_int(p, "", -1);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PRESET_RESETTODEFAULTS), static_cast<int32_t>(Presets::ResetToDefaults));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PRESET_RECORDING), static_cast<int32_t>(Presets::Recording));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PRESET_HIGHQUALITY), static_cast<int32_t>(Presets::HighQuality));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PRESET_INDISTINGUISHABLE), static_cast<int32_t>(Presets::Indistinguishable));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PRESET_LOSSLESS), static_cast<int32_t>(Presets::Lossless));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PRESET_TWITCH), static_cast<int32_t>(Presets::Twitch));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PRESET_YOUTUBE), static_cast<int32_t>(Presets::YouTube));
	#pragma endregion Preset

	#pragma region Static Properties
	#pragma region Usage
	p = obs_properties_add_list(props, AMF_H264_USAGE, TEXT_T(AMF_H264_USAGE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_USAGE_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_USAGE_TRANSCODING), static_cast<int32_t>(H264Usage::Transcoding));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_USAGE_ULTRALOWLATENCY), static_cast<int32_t>(H264Usage::UltraLowLatency));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_USAGE_LOWLATENCY), static_cast<int32_t>(H264Usage::LowLatency));
	// Webcam requires SVC, which is not something OBSs properties API makes easy to support. Nor would it look like anything usable.
	//obs_property_list_add_int(list, TEXT_T(AMF_H264_USAGE_WEBCAM), VCEUsage_Webcam);
	#pragma endregion Usage

	#pragma region Quality Preset
	p = obs_properties_add_list(props, AMF_H264_QUALITY_PRESET, TEXT_T(AMF_H264_QUALITY_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QUALITY_PRESET_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_QUALITY_PRESET_SPEED), static_cast<int32_t>(H264QualityPreset::Speed));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_QUALITY_PRESET_BALANCED), static_cast<int32_t>(H264QualityPreset::Balanced));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_QUALITY_PRESET_QUALITY), static_cast<int32_t>(H264QualityPreset::Quality));
	#pragma endregion Quality Preset

	#pragma region Profile
	p = obs_properties_add_list(props, AMF_H264_PROFILE, TEXT_T(AMF_H264_PROFILE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_PROFILE_DESCRIPTION));
	#pragma endregion Profile

	#pragma region Profile Level
	p = obs_properties_add_list(props, AMF_H264_PROFILELEVEL, TEXT_T(AMF_H264_PROFILELEVEL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_PROFILELEVEL_DESCRIPTION));
	#pragma endregion Profile Levels
	#pragma endregion Static Properties

	#pragma region Rate Control Properties
	#pragma region Method
	p = obs_properties_add_list(props, AMF_H264_RATECONTROLMETHOD, TEXT_T(AMF_H264_RATECONTROLMETHOD), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_CQP), static_cast<int32_t>(H264RateControlMethod::ConstantQP));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_CBR), static_cast<int32_t>(H264RateControlMethod::ConstantBitrate));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_VBR), static_cast<int32_t>(H264RateControlMethod::VariableBitrate_PeakConstrained));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_VBR_LAT), static_cast<int32_t>(H264RateControlMethod::VariableBitrate_LatencyConstrained));
	obs_property_set_modified_callback(p, properties_modified);
	#pragma endregion Method

	#pragma region Method Parameters
	/// Bitrate Constraints
	p = obs_properties_add_int(props, AMF_H264_BITRATE_TARGET, TEXT_T(AMF_H264_BITRATE_TARGET), 0,
							   1, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BITRATE_TARGET_DESCRIPTION));
	p = obs_properties_add_int(props, AMF_H264_BITRATE_PEAK, TEXT_T(AMF_H264_BITRATE_PEAK), 0,
							   1, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BITRATE_PEAK_DESCRIPTION));

	/// Minimum QP, Maximum QP
	p = obs_properties_add_int_slider(props, AMF_H264_QP_MINIMUM, TEXT_T(AMF_H264_QP_MINIMUM), 0, 51, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QP_MINIMUM_DESCRIPTION));
	p = obs_properties_add_int_slider(props, AMF_H264_QP_MAXIMUM, TEXT_T(AMF_H264_QP_MAXIMUM), 0, 51, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QP_MAXIMUM_DESCRIPTION));

	/// Method: Constant QP
	p = obs_properties_add_int_slider(props, AMF_H264_QP_IFRAME, TEXT_T(AMF_H264_QP_IFRAME), 0, 51, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QP_IFRAME_DESCRIPTION));
	p = obs_properties_add_int_slider(props, AMF_H264_QP_PFRAME, TEXT_T(AMF_H264_QP_PFRAME), 0, 51, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QP_PFRAME_DESCRIPTION));
	p = obs_properties_add_int_slider(props, AMF_H264_QP_BFRAME, TEXT_T(AMF_H264_QP_BFRAME), 0, 51, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QP_BFRAME_DESCRIPTION));
	#pragma endregion Method Parameters

	#pragma region VBV Buffer
	p = obs_properties_add_list(props, AMF_H264_VBVBUFFER, TEXT_T(AMF_H264_VBVBUFFER), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VBVBUFFER_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_AUTOMATIC), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_MANUAL), 1);
	obs_property_set_modified_callback(p, properties_modified);
	p = obs_properties_add_float_slider(props, AMF_H264_VBVBUFFER_STRICTNESS, TEXT_T(AMF_H264_VBVBUFFER_STRICTNESS), 0.0, 100.0, 0.1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VBVBUFFER_STRICTNESS_DESCRIPTION));
	p = obs_properties_add_int_slider(props, AMF_H264_VBVBUFFER_SIZE, TEXT_T(AMF_H264_VBVBUFFER_SIZE), 1, 1000000, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VBVBUFFER_SIZE_DESCRIPTION));
	p = obs_properties_add_float_slider(props, AMF_H264_VBVBUFFER_FULLNESS, TEXT_T(AMF_H264_VBVBUFFER_FULLNESS), 0.0, 100.0, 100.0 / 64.0);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VBVBUFFER_FULLNESS_DESCRIPTION));
	#pragma endregion VBV Buffer

	/// Max Access Unit Size
	p = obs_properties_add_int_slider(props, AMF_H264_MAXIMUMACCESSUNITSIZE, TEXT_T(AMF_H264_MAXIMUMACCESSUNITSIZE), 0, 100000000, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_MAXIMUMACCESSUNITSIZE_DESCRIPTION));

	#pragma region Flags
	/// Filler Data (Only supported by CBR so far)
	p = obs_properties_add_list(props, AMF_H264_FILLERDATA, TEXT_T(AMF_H264_FILLERDATA), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_FILLERDATA_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);

	/// Frame Skipping
	p = obs_properties_add_list(props, AMF_H264_FRAMESKIPPING, TEXT_T(AMF_H264_FRAMESKIPPING), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_FRAMESKIPPING_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);

	/// Enforce Hypothetical Reference Decoder Compatibility
	p = obs_properties_add_list(props, AMF_H264_ENFORCEHRDCOMPATIBILITY, TEXT_T(AMF_H264_ENFORCEHRDCOMPATIBILITY), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_ENFORCEHRDCOMPATIBILITY_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	#pragma endregion Flags
	#pragma endregion Rate Control Properties

	#pragma region Frame Control Properties
	#pragma region IDR Period / Keyframe Interval
	p = obs_properties_add_float(props, AMF_H264_KEYFRAME_INTERVAL, TEXT_T(AMF_H264_KEYFRAME_INTERVAL), 0, 100, 0.001);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_KEYFRAME_INTERVAL_DESCRIPTION));
	p = obs_properties_add_int(props, AMF_H264_IDR_PERIOD, TEXT_T(AMF_H264_IDR_PERIOD), 1, 1000, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_IDR_PERIOD_DESCRIPTION));
	#pragma endregion IDR Period / Keyframe Interval

	#pragma region B-Frames
	/// B-Frames Pattern
	p = obs_properties_add_int_slider(props, AMF_H264_BFRAME_PATTERN, TEXT_T(AMF_H264_BFRAME_PATTERN),
									  static_cast<int32_t>(H264BFramePattern::None),
									  static_cast<int32_t>(H264BFramePattern::Three),
									  1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BFRAME_PATTERN_DESCRIPTION));
	obs_property_set_modified_callback(p, properties_modified);
	/// Enable Reference to B-Frames (2nd Generation GCN and newer)
	p = obs_properties_add_list(props, AMF_H264_BFRAME_REFERENCE, TEXT_T(AMF_H264_BFRAME_REFERENCE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BFRAME_REFERENCE_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	obs_property_set_modified_callback(p, properties_modified);
	/// B-Frame Delta QP
	p = obs_properties_add_int_slider(props, AMF_H264_BFRAME_REFERENCEDELTAQP, TEXT_T(AMF_H264_BFRAME_REFERENCEDELTAQP), -10, 10, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BFRAME_REFERENCEDELTAQP_DESCRIPTION));
	p = obs_properties_add_int_slider(props, AMF_H264_BFRAME_DELTAQP, TEXT_T(AMF_H264_BFRAME_DELTAQP), -10, 10, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BFRAME_DELTAQP_DESCRIPTION));
	#pragma endregion B-Frames

	/// De-Blocking Filter
	p = obs_properties_add_list(props, AMF_H264_DEBLOCKINGFILTER, TEXT_T(AMF_H264_DEBLOCKINGFILTER), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_DEBLOCKINGFILTER_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	#pragma endregion Frame Control Properties

	#pragma region Miscellaneous Control Properties
	/// Scan Type
	p = obs_properties_add_list(props, AMF_H264_SCANTYPE, TEXT_T(AMF_H264_SCANTYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_SCANTYPE_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_SCANTYPE_PROGRESSIVE), static_cast<int32_t>(H264ScanType::Progressive));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_SCANTYPE_INTERLACED), static_cast<int32_t>(H264ScanType::Interlaced));

	/// Motion Estimation
	p = obs_properties_add_list(props, AMF_H264_MOTIONESTIMATION, TEXT_T(AMF_H264_MOTIONESTIMATION), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_MOTIONESTIMATION_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_MOTIONESTIMATION_NONE), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_MOTIONESTIMATION_HALF), 1);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_MOTIONESTIMATION_QUARTER), 2);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_MOTIONESTIMATION_BOTH), 3);
	#pragma endregion Miscellaneous Control Properties

	#pragma region Experimental Properties
	#pragma region Coding Type
	p = obs_properties_add_list(props, AMF_H264_CODINGTYPE, TEXT_T(AMF_H264_CODINGTYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_CODINGTYPE_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_DEFAULT), static_cast<int32_t>(H264CodingType::Default));
	obs_property_list_add_int(p, "CABAC", static_cast<int32_t>(H264CodingType::CABAC));
	obs_property_list_add_int(p, "CALVC", static_cast<int32_t>(H264CodingType::CALVC));
	#pragma endregion Coding Type

	#pragma region Long Term Reference Frames
	p = obs_properties_add_int_slider(props, AMF_H264_MAXIMUMLTRFRAMES, TEXT_T(AMF_H264_MAXIMUMLTRFRAMES), 0, 2, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_MAXIMUMLTRFRAMES_DESCRIPTION));
	obs_property_set_modified_callback(p, properties_modified);
	#pragma endregion Long Term Reference Frames

	/// Header Insertion Spacing
	p = obs_properties_add_int(props, AMF_H264_HEADER_INSERTION_SPACING, TEXT_T(AMF_H264_HEADER_INSERTION_SPACING), 0, 1000, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_HEADER_INSERTION_SPACING_DESCRIPTION));

	#pragma region Slicing
	/// Number of Slices Per Frame 
	p = obs_properties_add_int_slider(props, AMF_H264_SLICESPERFRAME, TEXT_T(AMF_H264_SLICESPERFRAME), 1, 8160, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_SLICESPERFRAME_DESCRIPTION));

	/// Slice Mode
	p = obs_properties_add_list(props, AMF_H264_SLICEMODE, TEXT_T(AMF_H264_SLICEMODE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_SLICEMODE_DESCRIPTION));
	obs_property_list_add_int(p, "Horizontal", static_cast<int32_t>(H264SliceMode::Horizontal));
	obs_property_list_add_int(p, "Vertical", static_cast<int32_t>(H264SliceMode::Vertical));

	/// Maximum Slice Size
	p = obs_properties_add_int_slider(props, AMF_H264_MAXIMUMSLICESIZE, TEXT_T(AMF_H264_MAXIMUMSLICESIZE), 1, INT_MAX, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_MAXIMUMSLICESIZE_DESCRIPTION));

	/// Slice Control Mode
	p = obs_properties_add_list(props, AMF_H264_SLICECONTROLMODE, TEXT_T(AMF_H264_SLICECONTROLMODE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_SLICECONTROLMODE_DESCRIPTION));
	obs_property_list_add_int(p, Utility::SliceControlModeAsString(H264SliceControlMode::Off), static_cast<int32_t>(H264SliceControlMode::Off));
	obs_property_list_add_int(p, Utility::SliceControlModeAsString(H264SliceControlMode::Macroblock), static_cast<int32_t>(H264SliceControlMode::Macroblock));
	obs_property_list_add_int(p, Utility::SliceControlModeAsString(H264SliceControlMode::Macroblock_Row), static_cast<int32_t>(H264SliceControlMode::Macroblock_Row));

	/// Slice Control Size
	p = obs_properties_add_int_slider(props, AMF_H264_SLICECONTROLSIZE, TEXT_T(AMF_H264_SLICECONTROLSIZE), 0, 34560, 1); // 4096x2160 / 16x16
	obs_property_set_long_description(p, TEXT_T(AMF_H264_SLICECONTROLSIZE_DESCRIPTION));
	#pragma endregion Slicing

	#pragma region Intra Refresh
	/// Intra Refresh: Number of Stripes
	p = obs_properties_add_int_slider(props, AMF_H264_INTRAREFRESH_NUMBEROFSTRIPES, TEXT_T(AMF_H264_INTRAREFRESH_NUMBEROFSTRIPES), 0, INT_MAX, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_INTRAREFRESH_NUMBEROFSTRIPES_DESCRIPTION));

	/// Intra Refresh: Macroblocks Per Slot
	p = obs_properties_add_int_slider(props, AMF_H264_INTRAREFRESH_MACROBLOCKSPERSLOT, TEXT_T(AMF_H264_INTRAREFRESH_MACROBLOCKSPERSLOT), 0, 34560, 1); // 4096x2160 / 16x16
	obs_property_set_long_description(p, TEXT_T(AMF_H264_INTRAREFRESH_MACROBLOCKSPERSLOT_DESCRIPTION));
	#pragma endregion Intra Refresh

	/// Wait For Task
	p = obs_properties_add_list(props, AMF_H264_WAITFORTASK, TEXT_T(AMF_H264_WAITFORTASK), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_WAITFORTASK_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);

	/// Preanalysis Pass
	p = obs_properties_add_list(props, AMF_H264_PREANALYSISPASS, TEXT_T(AMF_H264_PREANALYSISPASS), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_PREANALYSISPASS_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);

	/// VBAQ
	p = obs_properties_add_list(props, AMF_H264_VBAQ, TEXT_T(AMF_H264_VBAQ), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VBAQ_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);

	/// GOP Size
	p = obs_properties_add_int(props, AMF_H264_GOPSIZE, TEXT_T(AMF_H264_GOPSIZE), 0, INT_MAX, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_GOPSIZE_DESCRIPTION));

	/// GOP Alignment
	p = obs_properties_add_list(props, AMF_H264_GOPALIGNMENT, TEXT_T(AMF_H264_GOPALIGNMENT), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_GOPALIGNMENT_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);

	/// GOP Size
	p = obs_properties_add_int_slider(props, AMF_H264_MAXIMUMREFERENCEFRAMES, TEXT_T(AMF_H264_MAXIMUMREFERENCEFRAMES), 1, 1, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_MAXIMUMREFERENCEFRAMES_DESCRIPTION));

	#pragma endregion Experimental Properties

	#pragma region System Properties
	/// Video API
	p = obs_properties_add_list(props, AMF_H264_VIDEOAPI, TEXT_T(AMF_H264_VIDEOAPI), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VIDEOAPI_DESCRIPTION));
	obs_property_set_modified_callback(p, properties_modified);
	fill_api_list(p);

	/// Video Adapter
	p = obs_properties_add_list(props, AMF_H264_VIDEOADAPTER, TEXT_T(AMF_H264_VIDEOADAPTER), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VIDEOADAPTER_DESCRIPTION));
	obs_property_set_modified_callback(p, properties_modified);

	/// OpenCL
	p = obs_properties_add_list(props, AMF_H264_OPENCL, TEXT_T(AMF_H264_OPENCL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_OPENCL_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);

	/// View Mode
	p = obs_properties_add_list(props, AMF_H264_VIEW, TEXT_T(AMF_H264_VIEW), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VIEW_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_VIEW_BASIC), static_cast<int32_t>(ViewMode::Basic));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_VIEW_ADVANCED), static_cast<int32_t>(ViewMode::Advanced));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_VIEW_EXPERT), static_cast<int32_t>(ViewMode::Expert));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_VIEW_MASTER), static_cast<int32_t>(ViewMode::Master));
	obs_property_set_modified_callback(p, properties_modified);

	/// Debug
	p = obs_properties_add_bool(props, AMF_H264_DEBUG, TEXT_T(AMF_H264_DEBUG));
	obs_property_set_long_description(p, TEXT_T(AMF_H264_DEBUG_DESCRIPTION));
	#pragma endregion System Properties

	return props;
}

static void obs_data_default_single(obs_properties_t *props, obs_data_t *data, const char* name) {
	obs_property_t *p = obs_properties_get(props, name);
	switch (obs_property_get_type(p)) {
		case OBS_PROPERTY_INVALID:
			break;
		case OBS_PROPERTY_BOOL:
			obs_data_set_bool(data, name, obs_data_get_default_bool(data, name));
			break;
		case OBS_PROPERTY_INT:
			obs_data_set_int(data, name, obs_data_get_default_int(data, name));
			break;
		case OBS_PROPERTY_FLOAT:
			obs_data_set_double(data, name, obs_data_get_default_double(data, name));
			break;
		case OBS_PROPERTY_TEXT:
		case OBS_PROPERTY_PATH:
			obs_data_set_string(data, name, obs_data_get_default_string(data, name));
			break;
		case OBS_PROPERTY_LIST:
		case OBS_PROPERTY_EDITABLE_LIST:
			switch (obs_property_list_format(p)) {
				case OBS_COMBO_FORMAT_INT:
					obs_data_set_int(data, name, obs_data_get_default_int(data, name));
					break;
				case OBS_COMBO_FORMAT_FLOAT:
					obs_data_set_double(data, name, obs_data_get_default_double(data, name));
					break;
				case OBS_COMBO_FORMAT_STRING:
					obs_data_set_string(data, name, obs_data_get_default_string(data, name));
					break;
			}
			break;
		case OBS_PROPERTY_COLOR:
			break;
		case OBS_PROPERTY_BUTTON:
			break;
		case OBS_PROPERTY_FONT:
			break;
		case OBS_PROPERTY_FRAME_RATE:
			break;
	}
}

static void obs_data_transfer_settings(obs_data_t * data) {
	#pragma region Version Differences
	uint64_t version = obs_data_get_int(data, AMF_H264_VERSION);
	switch (version) {
		case 0x0001000400030005ull:
			obs_data_set_double(data, AMF_H264_VBVBUFFER_STRICTNESS, obs_data_get_double(data, AMF_H264_VBVBUFFER_STRICTNESS) + 50.0);
		case PLUGIN_VERSION_FULL:
			obs_data_set_int(data, AMF_H264_VERSION, PLUGIN_VERSION_FULL);
			break;
	}
	#pragma endregion Version Differences
}

bool Plugin::Interface::H264Interface::properties_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	bool result = false;
	obs_property_t* p;

	obs_data_transfer_settings(data);

	#pragma region Presets
	Presets lastPreset = static_cast<Presets>(obs_data_get_int(data, "last" vstr(AMF_H264_PRESET))),
		preset = static_cast<Presets>(obs_data_get_int(data, AMF_H264_PRESET));
	if (lastPreset != preset) { // Reset State
		obs_property_t* pn = obs_properties_first(props);
		do {
			obs_property_set_enabled(pn, true);
		} while (obs_property_next(&pn));

		result = true;
	}
	if (preset != Presets::None)
		result = true;

	switch (preset) {
		case Presets::ResetToDefaults:
			#pragma region Default
			{
				obs_property_t* pn = obs_properties_first(props);
				do {
					const char* name = obs_property_name(pn);

					// Do not reset Video Adapter or API.
					if ((strcmp(name, AMF_H264_VIDEOAPI) == 0) || (strcmp(name, AMF_H264_VIDEOADAPTER) == 0))
						continue;

					switch (obs_property_get_type(pn)) {
						case obs_property_type::OBS_PROPERTY_BOOL:
							obs_data_set_bool(data, name, obs_data_get_default_bool(data, name));
							break;
						case obs_property_type::OBS_PROPERTY_FLOAT:
							obs_data_set_double(data, name, obs_data_get_default_double(data, name));
							break;
						case obs_property_type::OBS_PROPERTY_INT:
							obs_data_set_int(data, name, obs_data_get_default_int(data, name));
							break;
						case obs_property_type::OBS_PROPERTY_TEXT:
							obs_data_set_string(data, name, obs_data_get_default_string(data, name));
							break;
						case obs_property_type::OBS_PROPERTY_LIST:
							switch (obs_property_list_format(pn)) {
								case obs_combo_format::OBS_COMBO_FORMAT_INT:
									obs_data_set_int(data, name, obs_data_get_default_int(data, name));
									break;
								case obs_combo_format::OBS_COMBO_FORMAT_FLOAT:
									obs_data_set_double(data, name, obs_data_get_default_double(data, name));
									break;
								case obs_combo_format::OBS_COMBO_FORMAT_STRING:
									obs_data_set_string(data, name, obs_data_get_default_string(data, name));
									break;
							}
							break;
					}
					obs_property_set_enabled(pn, true);
				} while (obs_property_next(&pn));
			}
			break;
			#pragma endregion Default
		case Presets::Recording:
			#pragma region Recording
			// Static Properties
			//obs_data_set_int(data, AMF_H264_USAGE, VCEUsage_Transcoding);
			obs_data_set_int(data, AMF_H264_PROFILE, static_cast<int32_t>(H264Profile::High));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, static_cast<int32_t>(H264ProfileLevel::Automatic));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);
			//obs_data_set_int(data, AMF_H264_MAXIMUMLTRFRAMES, obs_data_get_default_int(data, AMF_H264_MAXIMUMLTRFRAMES));

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, static_cast<int32_t>(H264RateControlMethod::VariableBitrate_LatencyConstrained));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			if (obs_data_get_int(data, AMF_H264_BITRATE_TARGET) < 10000)
				obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 10000);
			obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_TARGET), 10000, 100000, 1);
			obs_data_default_single(props, data, AMF_H264_QP_MINIMUM);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_MINIMUM), false);
			obs_data_default_single(props, data, AMF_H264_QP_MAXIMUM);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_MAXIMUM), false);
			obs_data_set_int(data, AMF_H264_BFRAME_DELTAQP, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_BFRAME_DELTAQP), false);
			obs_data_set_int(data, AMF_H264_BFRAME_REFERENCEDELTAQP, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_BFRAME_REFERENCEDELTAQP), false);
			obs_data_set_int(data, AMF_H264_FILLERDATA, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_FILLERDATA), false);

			// Frame Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 1);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);

			// Miscellaneous Properties
			obs_data_set_int(data, AMF_H264_SCANTYPE, static_cast<int32_t>(H264ScanType::Progressive));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_SCANTYPE), false);
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			break;
			#pragma endregion Recording
		case Presets::HighQuality:
			#pragma region High Quality
			// Static Properties
			obs_data_set_int(data, AMF_H264_PROFILE, static_cast<int32_t>(H264Profile::High));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, static_cast<int32_t>(H264ProfileLevel::Automatic));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, static_cast<int32_t>(H264RateControlMethod::ConstantQP));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			obs_data_set_int(data, AMF_H264_QP_IFRAME, 26);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_IFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_PFRAME, 24);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_PFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_BFRAME, 22);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_BFRAME), false);
			obs_data_set_int(data, AMF_H264_BFRAME_DELTAQP, -2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_BFRAME_DELTAQP), false);
			obs_data_set_int(data, AMF_H264_BFRAME_REFERENCEDELTAQP, -2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_BFRAME_REFERENCEDELTAQP), false);

			// Frame Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 1);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);

			// Miscellaneous Properties
			obs_data_set_int(data, AMF_H264_SCANTYPE, static_cast<int32_t>(H264ScanType::Progressive));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_SCANTYPE), false);
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			break;
			#pragma endregion High Quality
		case Presets::Indistinguishable:
			#pragma region Indistinguishable
			// Static Properties
			obs_data_set_int(data, AMF_H264_PROFILE, static_cast<int32_t>(H264Profile::High));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, static_cast<int32_t>(H264ProfileLevel::Automatic));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, static_cast<int32_t>(H264RateControlMethod::ConstantQP));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			obs_data_set_int(data, AMF_H264_QP_IFRAME, 21);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_IFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_PFRAME, 19);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_PFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_BFRAME, 17);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_BFRAME), false);
			obs_data_set_int(data, AMF_H264_BFRAME_DELTAQP, -2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_BFRAME_DELTAQP), false);
			obs_data_set_int(data, AMF_H264_BFRAME_REFERENCEDELTAQP, -2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_BFRAME_REFERENCEDELTAQP), false);

			// Frame Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 1);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);

			// Miscellaneous Properties
			obs_data_set_int(data, AMF_H264_SCANTYPE, static_cast<int32_t>(H264ScanType::Progressive));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_SCANTYPE), false);
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			break;
			#pragma endregion Indistinguishable
		case Presets::Lossless:
			#pragma region Lossless
			// Static Properties
			obs_data_set_int(data, AMF_H264_PROFILE, static_cast<int32_t>(H264Profile::High));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, static_cast<int32_t>(H264ProfileLevel::Automatic));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, static_cast<int32_t>(H264RateControlMethod::ConstantQP));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			obs_data_set_int(data, AMF_H264_QP_IFRAME, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_IFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_PFRAME, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_PFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_BFRAME, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_BFRAME), false);
			obs_data_set_int(data, AMF_H264_BFRAME_DELTAQP, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_BFRAME_DELTAQP), false);
			obs_data_set_int(data, AMF_H264_BFRAME_REFERENCEDELTAQP, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_BFRAME_REFERENCEDELTAQP), false);

			// Frame Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 1);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);
			obs_data_set_int(data, AMF_H264_BFRAME_PATTERN, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_BFRAME_PATTERN), false);
			obs_data_set_int(data, AMF_H264_BFRAME_REFERENCE, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_BFRAME_REFERENCE), false);

			// Miscellaneous Properties
			obs_data_set_int(data, AMF_H264_SCANTYPE, static_cast<int32_t>(H264ScanType::Progressive));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_SCANTYPE), false);
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			break;
			#pragma endregion Lossless
		case Presets::Twitch:
			#pragma region Twitch
			// Static Properties
			obs_data_set_int(data, AMF_H264_USAGE, static_cast<int32_t>(H264Usage::Transcoding));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_USAGE), false);
			obs_data_set_int(data, AMF_H264_PROFILE, static_cast<int32_t>(H264Profile::Main));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, static_cast<int32_t>(H264ProfileLevel::Automatic));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, static_cast<int32_t>(H264RateControlMethod::ConstantBitrate));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			if (obs_data_get_int(data, AMF_H264_BITRATE_TARGET) < 500)
				obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 500);
			obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_TARGET), 500, 100000, 1);
			obs_data_default_single(props, data, AMF_H264_QP_MINIMUM);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_MINIMUM), false);
			obs_data_default_single(props, data, AMF_H264_QP_MAXIMUM);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_MAXIMUM), false);
			obs_data_set_int(data, AMF_H264_FILLERDATA, 1);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_FILLERDATA), false);

			// Frame Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);

			// Miscellaneous Properties
			obs_data_set_int(data, AMF_H264_SCANTYPE, static_cast<int32_t>(H264ScanType::Progressive));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_SCANTYPE), false);
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			break;
			#pragma endregion Twitch
		case Presets::YouTube:
			#pragma region YouTube
			// Static Properties
			obs_data_set_int(data, AMF_H264_USAGE, static_cast<int32_t>(H264Usage::Transcoding));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_USAGE), false);
			obs_data_set_int(data, AMF_H264_PROFILE, static_cast<int32_t>(H264Profile::Main));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, static_cast<int32_t>(H264ProfileLevel::Automatic));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, static_cast<int32_t>(H264RateControlMethod::ConstantBitrate));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			if (obs_data_get_int(data, AMF_H264_BITRATE_TARGET) < 500)
				obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 500);
			obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_TARGET), 500, 100000, 1);
			obs_data_default_single(props, data, AMF_H264_QP_MINIMUM);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_MINIMUM), false);
			obs_data_default_single(props, data, AMF_H264_QP_MAXIMUM);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_MAXIMUM), false);
			obs_data_set_int(data, AMF_H264_FILLERDATA, 1);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_FILLERDATA), false);

			// Frame Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);

			// Miscellaneous Properties
			obs_data_set_int(data, AMF_H264_SCANTYPE, static_cast<int32_t>(H264ScanType::Progressive));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_SCANTYPE), false);
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			break;
			#pragma endregion YouTube
	}
	#pragma endregion Presets

	#pragma region Video API
	const char *lastVideoAPI = obs_data_get_string(data, "last" vstr(AMF_H264_VIDEOAPI)),
		*curVideoAPI = obs_data_get_string(data, AMF_H264_VIDEOAPI);
	if (strcmp(curVideoAPI, "") == 0) {
		p = obs_properties_get(props, AMF_H264_VIDEOAPI);
		//fill_api_list(p);

		obs_data_set_string(data, AMF_H264_VIDEOAPI,
							obs_property_list_item_string(p, 0));
		curVideoAPI = obs_data_get_string(data, AMF_H264_VIDEOAPI);
	}
	if ((strcmp(lastVideoAPI, curVideoAPI) != 0)
		|| (strcmp(curVideoAPI, "") == 0)) {
		obs_data_set_string(data, "last" vstr(AMF_H264_VIDEOAPI), curVideoAPI);
		fill_device_list(obs_properties_get(props, AMF_H264_VIDEOADAPTER), curVideoAPI);

		// Reset Video Adapter to first in list.
		obs_data_set_int(data, AMF_H264_VIDEOADAPTER,
						 obs_property_list_item_int(obs_properties_get(props, AMF_H264_VIDEOADAPTER), 0));
	}
	#pragma endregion Video API

	#pragma region Video Adapter & Capabilities
	VCEDeviceCapabilities devCaps;
	int64_t lastAdapterId = obs_data_get_int(data, "last" vstr(AMF_H264_VIDEOADAPTER)),
		curAdapterId = obs_data_get_int(data, AMF_H264_VIDEOADAPTER);
	{
		auto api = Plugin::API::Base::GetAPIByName(obs_data_get_string(data, AMF_H264_VIDEOAPI));
		auto adapter = api->GetAdapterById(curAdapterId & UINT_MAX, (curAdapterId >> 32) & UINT_MAX);
		devCaps = Plugin::AMD::VCECapabilities::GetInstance()->GetAdapterCapabilities(api, adapter, H264EncoderType::AVC);
	}
	if (lastAdapterId != curAdapterId) {
		obs_data_set_int(data, "last" vstr(AMF_H264_VIDEOADAPTER), curAdapterId);

		#pragma region Profile
		p = obs_properties_get(props, AMF_H264_PROFILE);
		obs_property_list_clear(p);
		switch (devCaps.maxProfile) {
			case 100:
				obs_property_list_add_int(p, "High", (int32_t)H264Profile::High);
				obs_property_list_add_int(p, "Constrained High", (int32_t)H264Profile::ConstrainedHigh);
			case 77:
				obs_property_list_add_int(p, "Main", (int32_t)H264Profile::Main);
			case 66:
				obs_property_list_add_int(p, "Baseline", (int32_t)H264Profile::Baseline);
				obs_property_list_add_int(p, "Constrained Baseline", (int32_t)H264Profile::ConstrainedBaseline);
				break;
		}
		#pragma endregion Profile

		#pragma region Profile Level
		p = obs_properties_get(props, AMF_H264_PROFILELEVEL);
		obs_property_list_clear(p);
		obs_property_list_add_int(p, TEXT_T(AMF_UTIL_AUTOMATIC), (int32_t)H264ProfileLevel::Automatic);
		switch (devCaps.maxProfileLevel) {
			case 52:
				obs_property_list_add_int(p, "5.2", (int32_t)H264ProfileLevel::L52);
			case 51:
				obs_property_list_add_int(p, "5.1", (int32_t)H264ProfileLevel::L51);
			case 50:
				obs_property_list_add_int(p, "5.0", (int32_t)H264ProfileLevel::L50);
			case 42: // Some VCE 2.0 Cards.
				obs_property_list_add_int(p, "4.2", (int32_t)H264ProfileLevel::L42);
			case 41: // Some APUs and VCE 1.0 Cards.
				obs_property_list_add_int(p, "4.1", (int32_t)H264ProfileLevel::L41);
			case 40: // These should in theory be supported by all VCE 1.0 devices and APUs.
				obs_property_list_add_int(p, "4.0", (int32_t)H264ProfileLevel::L40);
			case 32:
				obs_property_list_add_int(p, "3.2", (int32_t)H264ProfileLevel::L32);
			case 31:
				obs_property_list_add_int(p, "3.1", (int32_t)H264ProfileLevel::L31);
			case 30:
				obs_property_list_add_int(p, "3.0", (int32_t)H264ProfileLevel::L30);
			case 22:
				obs_property_list_add_int(p, "2.2", (int32_t)H264ProfileLevel::L22);
			case 21:
				obs_property_list_add_int(p, "2.1", (int32_t)H264ProfileLevel::L21);
			case 20:
				obs_property_list_add_int(p, "2.0", (int32_t)H264ProfileLevel::L20);
			case 13:
				obs_property_list_add_int(p, "1.3", (int32_t)H264ProfileLevel::L13);
			case 12:
				obs_property_list_add_int(p, "1.2", (int32_t)H264ProfileLevel::L12);
			case 11:
				obs_property_list_add_int(p, "1.1", (int32_t)H264ProfileLevel::L11);
			case 10:
			default:
				obs_property_list_add_int(p, "1.0", (int32_t)H264ProfileLevel::L10);
		}
		#pragma endregion Profile Level

		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_TARGET),
									10, devCaps.maxBitrate / 1000, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_PEAK),
									10, devCaps.maxBitrate / 1000, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_VBVBUFFER_SIZE),
									1, 100000, 1);
		obs_property_float_set_limits(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL),
									  1.0 / 144.0, 30, 1.0 / 144.0);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_IDR_PERIOD),
									1, 1000, 1);

								// Experimental
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_MAXIMUMREFERENCEFRAMES),
									devCaps.minReferenceFrames, devCaps.maxReferenceFrames, 1);
	}
	#pragma endregion Video Adapter

	#pragma region View Mode
	ViewMode lastView = static_cast<ViewMode>(obs_data_get_int(data, "last" vstr(AMF_H264_VIEW))),
		curView = static_cast<ViewMode>(obs_data_get_int(data, AMF_H264_VIEW));
	if (lastView != curView) {
		obs_data_set_int(data, "last" vstr(AMF_H264_VIEW), static_cast<int32_t>(curView));
		result = true;
	}

	bool vis_basic = curView >= ViewMode::Basic,
		vis_advanced = curView >= ViewMode::Advanced,
		vis_expert = curView >= ViewMode::Expert,
		vis_master = curView >= ViewMode::Master;

	#pragma region Basic
	const char* basicProps[] = {
		AMF_H264_PRESET,
		AMF_H264_QUALITY_PRESET,
		AMF_H264_PROFILE,
		AMF_H264_RATECONTROLMETHOD,
		AMF_H264_VIEW,
		AMF_H264_DEBUG,
	};
	for (auto prop : basicProps) {
		obs_property_set_visible(obs_properties_get(props, prop), vis_basic);
		if (!vis_basic)
			obs_data_default_single(props, data, prop);
	}
	#pragma endregion Basic

	#pragma region Advanced
	const char* advancedProps[] = {
		AMF_H264_VBVBUFFER,
		AMF_H264_FRAMESKIPPING,
		AMF_H264_ENFORCEHRDCOMPATIBILITY,
		AMF_H264_DEBLOCKINGFILTER,
		AMF_H264_VIDEOAPI,
	};
	for (auto prop : advancedProps) {
		obs_property_set_visible(obs_properties_get(props, prop), vis_advanced);
		if (!vis_advanced)
			obs_data_default_single(props, data, prop);
	}
	#pragma endregion Advanced

	#pragma region Expert
	const char* expertProps[] = {
		AMF_H264_PROFILELEVEL,
		AMF_H264_VBVBUFFER_FULLNESS,
		AMF_H264_MOTIONESTIMATION,
	};
	for (auto prop : expertProps) {
		obs_property_set_visible(obs_properties_get(props, prop), vis_expert);
		if (!vis_expert)
			obs_data_default_single(props, data, prop);
	}
	#pragma endregion Expert

	#pragma region Master
	const char* masterProps[] = {
		AMF_H264_USAGE,
		AMF_H264_MAXIMUMACCESSUNITSIZE,
		AMF_H264_IDR_PERIOD,
		AMF_H264_HEADER_INSERTION_SPACING,
		AMF_H264_SCANTYPE,
		AMF_H264_MAXIMUMLTRFRAMES,
		AMF_H264_CODINGTYPE,
		AMF_H264_SLICESPERFRAME,
		AMF_H264_SLICEMODE,
		AMF_H264_MAXIMUMSLICESIZE,
		AMF_H264_SLICECONTROLMODE,
		AMF_H264_SLICECONTROLSIZE,
		AMF_H264_INTRAREFRESH_NUMBEROFSTRIPES,
		AMF_H264_INTRAREFRESH_MACROBLOCKSPERSLOT,
		AMF_H264_WAITFORTASK,
		AMF_H264_PREANALYSISPASS,
		AMF_H264_VBAQ,
		AMF_H264_GOPSIZE,
		AMF_H264_GOPALIGNMENT,
		AMF_H264_MAXIMUMREFERENCEFRAMES,
	};
	for (auto prop : masterProps) {
		obs_property_set_visible(obs_properties_get(props, prop), vis_master);
		if (!vis_master)
			obs_data_default_single(props, data, prop);
	}
	#pragma endregion Master

	#pragma region Special Logic
	uint32_t ltrFrames = static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_MAXIMUMLTRFRAMES));
	bool usingLTRFrames = ltrFrames > 0;

	// Key-frame Interval
	obs_property_set_visible(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), !vis_master);
	if (vis_master)
		obs_data_default_single(props, data, AMF_H264_KEYFRAME_INTERVAL);

	#pragma region B-Frames
	/// Pattern
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BFRAME_PATTERN), vis_advanced && !usingLTRFrames && devCaps.supportsBFrames);
	if (!vis_advanced || usingLTRFrames || !devCaps.supportsBFrames)
		obs_data_default_single(props, data, AMF_H264_BFRAME_PATTERN);
	bool lastUsingBFrames = obs_data_get_int(data, "last" vstr(AMF_H264_BFRAME_PATTERN)) != 0,
		usingBFrames = obs_data_get_int(data, AMF_H264_BFRAME_PATTERN) != 0;
	if (usingBFrames != lastUsingBFrames) {
		obs_data_set_int(data, "last" vstr(AMF_H264_BFRAME_PATTERN), obs_data_get_int(data, AMF_H264_BFRAME_PATTERN));
		result = true;
	}

	/// Reference
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BFRAME_REFERENCE), vis_advanced && !usingLTRFrames && usingBFrames && devCaps.supportsBFrames);
	if (!vis_advanced || usingLTRFrames || !usingBFrames || !devCaps.supportsBFrames)
		obs_data_default_single(props, data, AMF_H264_BFRAME_REFERENCE);
	bool lastUsingBFrameReference = obs_data_get_int(data, "last" vstr(AMF_H264_BFRAME_REFERENCE)) != 0,
		usingBFrameReference = obs_data_get_int(data, AMF_H264_BFRAME_REFERENCE) == 1;
	if (usingBFrameReference != lastUsingBFrameReference) {
		obs_data_set_int(data, "last" vstr(AMF_H264_BFRAME_REFERENCE), obs_data_get_int(data, AMF_H264_BFRAME_REFERENCE));
		result = true;
	}

	/// QP Delta
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BFRAME_DELTAQP), vis_advanced && usingBFrames && devCaps.supportsBFrames);
	if (!vis_advanced || !usingBFrames || !devCaps.supportsBFrames)
		obs_data_default_single(props, data, AMF_H264_BFRAME_DELTAQP);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BFRAME_REFERENCEDELTAQP), vis_advanced && usingBFrames && usingBFrameReference && devCaps.supportsBFrames);
	if (!vis_advanced || !usingBFrames || !usingBFrameReference || !devCaps.supportsBFrames)
		obs_data_default_single(props, data, AMF_H264_BFRAME_REFERENCEDELTAQP);
	#pragma endregion B-Frames

	#pragma region Rate Control
	bool vis_rcm_bitrate_target = false,
		vis_rcm_bitrate_peak = false,
		vis_rcm_qp = false,
		vis_rcm_qp_b = false,
		vis_rcm_fillerdata = false;

	H264RateControlMethod lastRCM = static_cast<H264RateControlMethod>(obs_data_get_int(data, "last" vstr(AMF_H264_RATECONTROLMETHOD))),
		curRCM = static_cast<H264RateControlMethod>(obs_data_get_int(data, AMF_H264_RATECONTROLMETHOD));
	if (lastRCM != curRCM) {
		obs_data_set_int(data, "last" vstr(AMF_H264_RATECONTROLMETHOD), static_cast<int32_t>(curRCM));
		result = true;
	}
	switch (curRCM) {
		case H264RateControlMethod::ConstantBitrate:
			vis_rcm_bitrate_target = true;
			vis_rcm_fillerdata = true;
			break;
		case H264RateControlMethod::VariableBitrate_PeakConstrained:
			vis_rcm_bitrate_target = true;
			vis_rcm_bitrate_peak = true;
			break;
		case H264RateControlMethod::VariableBitrate_LatencyConstrained:
			vis_rcm_bitrate_target = true;
			vis_rcm_bitrate_peak = true;
			break;
		case H264RateControlMethod::ConstantQP:
			vis_rcm_qp = true;
			vis_rcm_qp_b = (!usingLTRFrames) && devCaps.supportsBFrames && usingBFrames;
			break;
	}

	/// Bitrate
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_TARGET), vis_rcm_bitrate_target);
	if (!vis_rcm_bitrate_target)
		obs_data_default_single(props, data, AMF_H264_BITRATE_TARGET);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_PEAK), vis_rcm_bitrate_peak);
	if (!vis_rcm_bitrate_peak)
		obs_data_default_single(props, data, AMF_H264_BITRATE_PEAK);

	/// QP
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_IFRAME), vis_rcm_qp);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_PFRAME), vis_rcm_qp);
	if (!vis_rcm_qp) {
		obs_data_default_single(props, data, AMF_H264_QP_IFRAME);
		obs_data_default_single(props, data, AMF_H264_QP_PFRAME);
	}
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BFRAME), vis_rcm_qp_b);
	if (!vis_rcm_qp_b)
		obs_data_default_single(props, data, AMF_H264_QP_BFRAME);

	/// QP Min/Max
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_MINIMUM), vis_advanced && !vis_rcm_qp);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_MAXIMUM), vis_advanced && !vis_rcm_qp);
	if (!vis_advanced || vis_rcm_qp) {
		obs_data_default_single(props, data, AMF_H264_QP_MINIMUM);
		obs_data_default_single(props, data, AMF_H264_QP_MAXIMUM);
	}

	/// Filler Data (CBR only at the moment)
	obs_property_set_visible(obs_properties_get(props, AMF_H264_FILLERDATA), vis_rcm_fillerdata);
	if (!vis_rcm_fillerdata)
		obs_data_default_single(props, data, AMF_H264_FILLERDATA);
	#pragma endregion Rate Control

	#pragma region VBV Buffer
	uint32_t vbvBufferMode = static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_VBVBUFFER));
	bool vbvBufferVisible = vis_advanced;

	uint32_t lastVBVBufferMode = static_cast<uint32_t>(obs_data_get_int(data, "last" vstr(AMF_H264_VBVBUFFER)));
	if (lastVBVBufferMode != vbvBufferMode) {
		obs_data_set_int(data, "last" vstr(AMF_H264_VBVBUFFER), vbvBufferMode);
		result = true;
	}

	obs_property_set_visible(obs_properties_get(props, AMF_H264_VBVBUFFER_STRICTNESS), vbvBufferVisible && (vbvBufferMode == 0));
	obs_property_set_visible(obs_properties_get(props, AMF_H264_VBVBUFFER_SIZE), vbvBufferVisible && (vbvBufferMode == 1));
	if (!vbvBufferVisible || vbvBufferMode == 0)
		obs_data_default_single(props, data, AMF_H264_VBVBUFFER_SIZE);
	if (!vbvBufferVisible || vbvBufferMode == 1)
		obs_data_default_single(props, data, AMF_H264_VBVBUFFER_STRICTNESS);
	#pragma endregion VBV Buffer

	bool isnothostmode = strcmp(obs_data_get_string(data, AMF_H264_VIDEOAPI), "Host") != 0;
	/// Video Adapter
	obs_property_set_visible(obs_properties_get(props, AMF_H264_VIDEOADAPTER), vis_advanced && isnothostmode);
	if (!vis_advanced || !isnothostmode)
		obs_data_default_single(props, data, AMF_H264_VIDEOADAPTER);
	/// OpenCL
	obs_property_set_visible(obs_properties_get(props, AMF_H264_OPENCL), vis_advanced && isnothostmode);
	if (!vis_advanced || !isnothostmode)
		obs_data_default_single(props, data, AMF_H264_OPENCL);

	#pragma endregion Special Logic
	#pragma endregion View Mode

	return result;
}

bool Plugin::Interface::H264Interface::update(void *data, obs_data_t *settings) {
	try {
		return static_cast<Plugin::Interface::H264Interface*>(data)->update(settings);
	} catch (std::exception e) {
		AMF_LOG_ERROR("%s", e.what());
	} catch (std::exception* e) {
		AMF_LOG_ERROR("%s", e->what());
		delete e;
	} catch (...) {
		AMF_LOG_ERROR("Unknown Exception.");
	}
	return false;
}

void Plugin::Interface::H264Interface::get_video_info(void *data, struct video_scale_info *info) {
	try {
		return static_cast<Plugin::Interface::H264Interface*>(data)->get_video_info(info);
	} catch (std::exception e) {
		AMF_LOG_ERROR("%s", e.what());
	} catch (std::exception* e) {
		AMF_LOG_ERROR("%s", e->what());
		delete e;
	} catch (...) {
		AMF_LOG_ERROR("Unknown Exception.");
		return;
	}
}

bool Plugin::Interface::H264Interface::get_extra_data(void *data, uint8_t** extra_data, size_t* size) {
	try {
		return static_cast<Plugin::Interface::H264Interface*>(data)->get_extra_data(extra_data, size);
	} catch (std::exception e) {
		AMF_LOG_ERROR("%s", e.what());
	} catch (std::exception* e) {
		AMF_LOG_ERROR("%s", e->what());
		delete e;
	} catch (...) {
		AMF_LOG_ERROR("Unknown Exception.");
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// Module Code
//////////////////////////////////////////////////////////////////////////
Plugin::Interface::H264Interface::H264Interface(obs_data_t* data, obs_encoder_t* encoder) {
	AMF_LOG_DEBUG("<H264Interface::H264Interface> Initializing...");

	// OBS Settings
	uint32_t m_cfgWidth = obs_encoder_get_width(encoder);
	uint32_t m_cfgHeight = obs_encoder_get_height(encoder);
	video_t *video = obs_encoder_video(encoder);
	const struct video_output_info *voi = video_output_get_info(video);
	uint32_t m_cfgFPSnum = voi->fps_num;
	uint32_t m_cfgFPSden = voi->fps_den;

	obs_data_transfer_settings(data);

	//////////////////////////////////////////////////////////////////////////
	/// Initialize Encoder
	bool debug = obs_data_get_bool(data, AMF_H264_DEBUG);
	Plugin::AMD::AMF::GetInstance()->EnableDebugTrace(debug);

	H264ColorFormat surfFormat = H264ColorFormat::NV12;
	switch (voi->format) {
		case VIDEO_FORMAT_NV12:
			surfFormat = H264ColorFormat::NV12;
			break;
		case VIDEO_FORMAT_I420:
			surfFormat = H264ColorFormat::I420;
			break;
		case VIDEO_FORMAT_YUY2:
			surfFormat = H264ColorFormat::YUY2;
			break;
		case VIDEO_FORMAT_RGBA:
			surfFormat = H264ColorFormat::RGBA;
			break;
		case VIDEO_FORMAT_BGRA:
			surfFormat = H264ColorFormat::BGRA;
			break;
		case VIDEO_FORMAT_Y800:
			surfFormat = H264ColorFormat::GRAY;
			break;
	}
	m_VideoEncoder = new H264Encoder(H264EncoderType::AVC, obs_data_get_string(data, AMF_H264_VIDEOAPI),
									 obs_data_get_int(data, AMF_H264_VIDEOADAPTER), !!obs_data_get_int(data, AMF_H264_OPENCL), surfFormat);

								 /// Static Properties
	m_VideoEncoder->SetUsage(static_cast<H264Usage>(obs_data_get_int(data, AMF_H264_USAGE)));
	m_VideoEncoder->SetQualityPreset(static_cast<H264QualityPreset>(obs_data_get_int(data, AMF_H264_QUALITY_PRESET)));

	/// Frame
	m_VideoEncoder->SetColorProfile(voi->colorspace == VIDEO_CS_709 ? H264ColorProfile::Rec709 : H264ColorProfile::Rec601);
	try { m_VideoEncoder->SetFullRangeColorEnabled(voi->range == VIDEO_RANGE_FULL); } catch (...) {}
	m_VideoEncoder->SetResolution(m_cfgWidth, m_cfgHeight);
	m_VideoEncoder->SetFrameRate(m_cfgFPSnum, m_cfgFPSden);
	m_VideoEncoder->SetScanType(static_cast<H264ScanType>(obs_data_get_int(data, AMF_H264_SCANTYPE)));

	/// Profile & Level
	m_VideoEncoder->SetProfile(static_cast<H264Profile>(obs_data_get_int(data, AMF_H264_PROFILE)));
	m_VideoEncoder->SetProfileLevel(static_cast<H264ProfileLevel>(obs_data_get_int(data, AMF_H264_PROFILELEVEL)));

	#pragma region Experimental
	/// Long Term Reference
	if (static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_MAXIMUMLTRFRAMES) > 0))
		m_VideoEncoder->SetBFramePattern(H264BFramePattern::None);
	m_VideoEncoder->SetMaximumLongTermReferenceFrames(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_MAXIMUMLTRFRAMES)));

	#pragma endregion Experimental

	// OBS - Enforce Streaming Service Restrictions
	#pragma region OBS - Enforce Streaming Service Restrictions
	{
		// Profile
		const char* p_str = obs_data_get_string(data, "profile");
		if (strcmp(p_str, "") != 0) {
			if (strcmp(p_str, "constrained_baseline")) {
				m_VideoEncoder->SetProfile(H264Profile::ConstrainedBaseline);
			} else if (strcmp(p_str, "baseline")) {
				m_VideoEncoder->SetProfile(H264Profile::Baseline);
			} else if (strcmp(p_str, "main")) {
				m_VideoEncoder->SetProfile(H264Profile::Main);
			} else if (strcmp(p_str, "constrained_high")) {
				m_VideoEncoder->SetProfile(H264Profile::ConstrainedHigh);
			} else if (strcmp(p_str, "high")) {
				m_VideoEncoder->SetProfile(H264Profile::High);
			}
		} else {
			switch (m_VideoEncoder->GetProfile()) {
				case H264Profile::ConstrainedBaseline:
					obs_data_set_string(data, "profile", "constrained_baseline");
					break;
				case H264Profile::Baseline:
					obs_data_set_string(data, "profile", "baseline");
					break;
				case H264Profile::Main:
					obs_data_set_string(data, "profile", "main");
					break;
				case H264Profile::ConstrainedHigh:
					obs_data_set_string(data, "profile", "constrained_high");
					break;
				case H264Profile::High:
					obs_data_set_string(data, "profile", "high");
					break;
			}
		}

		// Preset
		const char* preset = obs_data_get_string(data, "preset");
		if (strcmp(preset, "") != 0) {
			if (strcmp(preset, "speed") == 0) {
				m_VideoEncoder->SetQualityPreset(H264QualityPreset::Speed);
			} else if (strcmp(preset, "balanced") == 0) {
				m_VideoEncoder->SetQualityPreset(H264QualityPreset::Balanced);
			} else if (strcmp(preset, "quality") == 0) {
				m_VideoEncoder->SetQualityPreset(H264QualityPreset::Quality);
			}
			obs_data_set_int(data, AMF_H264_QUALITY_PRESET, (int32_t)m_VideoEncoder->GetQualityPreset());
		} else {
			switch (m_VideoEncoder->GetQualityPreset()) {
				case H264QualityPreset::Speed:
					obs_data_set_string(data, "preset", "speed");
					break;
				case H264QualityPreset::Balanced:
					obs_data_set_string(data, "preset", "balanced");
					break;
				case H264QualityPreset::Quality:
					obs_data_set_string(data, "preset", "quality");
					break;
			}
		}
	}
	#pragma endregion OBS - Enforce Streaming Service Restrictions

	// Dynamic Properties (Can be changed during Encoding)
	this->update(data);

	// Initialize (locks static properties)
	try {
		m_VideoEncoder->Start();
	} catch (...) {
		throw;
	}

	// Dynamic Properties (Can be changed during Encoding)
	this->update(data);

	AMF_LOG_DEBUG("<H264Interface::H264Interface> Complete.");
}

Plugin::Interface::H264Interface::~H264Interface() {
	AMF_LOG_DEBUG("<H264Interface::~H264Interface> Finalizing...");
	if (m_VideoEncoder) {
		m_VideoEncoder->Stop();
		delete m_VideoEncoder;
	}
	AMF_LOG_DEBUG("<H264Interface::~H264Interface> Complete.");
}

bool Plugin::Interface::H264Interface::update(obs_data_t* data) {
	#pragma region Device Capabilities
	auto api = Plugin::API::Base::GetAPIByName(obs_data_get_string(data, AMF_H264_VIDEOAPI));
	int64_t adapterId = obs_data_get_int(data, AMF_H264_VIDEOADAPTER);
	auto adapter = api->GetAdapterById(adapterId & UINT_MAX, (adapterId >> 32) & UINT_MAX);
	auto devCaps = Plugin::AMD::VCECapabilities::GetInstance()->GetAdapterCapabilities(api, adapter, H264EncoderType::AVC);
	#pragma endregion Device Capabilities

	#pragma region Rate Control
	// Rate Control Properties
	if (m_VideoEncoder->GetUsage() != H264Usage::UltraLowLatency) {
		m_VideoEncoder->SetRateControlMethod(static_cast<H264RateControlMethod>(obs_data_get_int(data, AMF_H264_RATECONTROLMETHOD)));
		m_VideoEncoder->SetMinimumQP(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_MINIMUM)));
		m_VideoEncoder->SetMaximumQP(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_MAXIMUM)));
		switch (static_cast<H264RateControlMethod>(obs_data_get_int(data, AMF_H264_RATECONTROLMETHOD))) {
			case H264RateControlMethod::ConstantBitrate:
				m_VideoEncoder->SetTargetBitrate(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_BITRATE_TARGET) * 1000));
				m_VideoEncoder->SetPeakBitrate(m_VideoEncoder->GetTargetBitrate());
				break;
			case H264RateControlMethod::VariableBitrate_PeakConstrained:
			case H264RateControlMethod::VariableBitrate_LatencyConstrained:
				m_VideoEncoder->SetTargetBitrate(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_BITRATE_TARGET) * 1000));
				m_VideoEncoder->SetPeakBitrate(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_BITRATE_PEAK) * 1000));
				break;
			case H264RateControlMethod::ConstantQP:
				m_VideoEncoder->SetIFrameQP(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_IFRAME)));
				m_VideoEncoder->SetPFrameQP(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_PFRAME)));
				if (devCaps.supportsBFrames && m_VideoEncoder->GetUsage() != H264Usage::UltraLowLatency)
					try { m_VideoEncoder->SetBFrameQP(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_BFRAME))); } catch (...) {}
				break;
		}
		if (obs_data_get_int(data, AMF_H264_VBVBUFFER) == 0) {
			m_VideoEncoder->SetVBVBufferAutomatic(obs_data_get_double(data, AMF_H264_VBVBUFFER_STRICTNESS) / 100.0);
		} else {
			m_VideoEncoder->SetVBVBufferSize(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_VBVBUFFER_SIZE) * 1000));
		}
		m_VideoEncoder->SetInitialVBVBufferFullness(obs_data_get_double(data, AMF_H264_VBVBUFFER_FULLNESS) / 100.0);
		m_VideoEncoder->SetFillerDataEnabled(!!obs_data_get_int(data, AMF_H264_FILLERDATA));
		m_VideoEncoder->SetFrameSkippingEnabled(!!obs_data_get_int(data, AMF_H264_FRAMESKIPPING));
	} else {
		m_VideoEncoder->SetMinimumQP(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_MINIMUM)));
		m_VideoEncoder->SetMaximumQP(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_MAXIMUM)));
		m_VideoEncoder->SetTargetBitrate(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_BITRATE_TARGET) * 1000));

		m_VideoEncoder->SetIFrameQP(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_IFRAME)));
		m_VideoEncoder->SetPFrameQP(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_PFRAME)));

		if (obs_data_get_int(data, AMF_H264_VBVBUFFER) == 0) {
			m_VideoEncoder->SetVBVBufferSize(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_BITRATE_TARGET) * 1000));
		} else {
			m_VideoEncoder->SetVBVBufferSize(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_VBVBUFFER_SIZE) * 1000));
		}
	}
	m_VideoEncoder->SetEnforceHRDRestrictionsEnabled(obs_data_get_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY) == 1);
	#pragma endregion Rate Control

	// Key-frame Interval
	double_t framerate = (double_t)m_VideoEncoder->GetFrameRate().first / (double_t)m_VideoEncoder->GetFrameRate().second;
	if (static_cast<ViewMode>(obs_data_get_int(data, AMF_H264_VIEW)) == ViewMode::Master)
		m_VideoEncoder->SetIDRPeriod(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_IDR_PERIOD)));
	else
		m_VideoEncoder->SetIDRPeriod(max(static_cast<uint32_t>(obs_data_get_double(data, AMF_H264_KEYFRAME_INTERVAL) * framerate), 1));

	#pragma region B-Frames
	if (devCaps.supportsBFrames) {
		try {
			m_VideoEncoder->SetBFramePattern(static_cast<H264BFramePattern>(obs_data_get_int(data, AMF_H264_BFRAME_PATTERN)));
			if (obs_data_get_int(data, AMF_H264_BFRAME_PATTERN) != 0)
				m_VideoEncoder->SetBFrameDeltaQP(static_cast<int8_t>(obs_data_get_int(data, AMF_H264_BFRAME_DELTAQP)));
		} catch (...) {}

		// B-Frame Reference can't be used with anything else but Transcoding.
		if (m_VideoEncoder->GetUsage() == H264Usage::Transcoding) {
			try {
				m_VideoEncoder->SetBFrameReferenceEnabled(!!obs_data_get_int(data, AMF_H264_BFRAME_REFERENCE));
				if (!!obs_data_get_int(data, AMF_H264_BFRAME_REFERENCE))
					m_VideoEncoder->SetBFrameReferenceDeltaQP(static_cast<int8_t>(obs_data_get_int(data, AMF_H264_BFRAME_REFERENCEDELTAQP)));
			} catch (...) {}
		}
	}
	#pragma endregion B-Frames

	if (m_VideoEncoder->GetUsage() == H264Usage::Transcoding)
		m_VideoEncoder->SetDeblockingFilterEnabled(!!obs_data_get_int(data, AMF_H264_DEBLOCKINGFILTER));

	#pragma region Motion Estimation
	m_VideoEncoder->SetHalfPixelMotionEstimationEnabled(!!(obs_data_get_int(data, AMF_H264_MOTIONESTIMATION) & 1));
	m_VideoEncoder->SetQuarterPixelMotionEstimationEnabled(!!(obs_data_get_int(data, AMF_H264_MOTIONESTIMATION) & 2));
	#pragma endregion Motion Estimation

	#pragma region Experimental
	try { m_VideoEncoder->SetCodingType(static_cast<H264CodingType>(obs_data_get_int(data, AMF_H264_CODINGTYPE))); } catch (...) {}
	try { m_VideoEncoder->SetWaitForTaskEnabled(!!obs_data_get_int(data, AMF_H264_WAITFORTASK)); } catch (...) {}
	if (m_VideoEncoder->GetUsage() == H264Usage::Transcoding || m_VideoEncoder->GetUsage() == H264Usage::Webcam) {
		try { m_VideoEncoder->SetPreAnalysisPassEnabled(!!obs_data_get_int(data, AMF_H264_PREANALYSISPASS)); } catch (...) {}
		try { m_VideoEncoder->SetVBAQEnabled(!!obs_data_get_int(data, AMF_H264_VBAQ)); } catch (...) {}
	}

	try { m_VideoEncoder->SetHeaderInsertionSpacing(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_HEADER_INSERTION_SPACING))); } catch (...) {}
	if (m_VideoEncoder->GetUsage() == H264Usage::Transcoding || m_VideoEncoder->GetUsage() == H264Usage::Webcam) {
		try { m_VideoEncoder->SetMaximumAccessUnitSize(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_MAXIMUMACCESSUNITSIZE))); } catch (...) {}
	}
	try { m_VideoEncoder->SetMaximumReferenceFrames(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_MAXIMUMREFERENCEFRAMES))); } catch (...) {}

	if (m_VideoEncoder->GetUsage() == H264Usage::Transcoding || m_VideoEncoder->GetUsage() == H264Usage::Webcam) {
		try { m_VideoEncoder->SetGOPSize(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_GOPSIZE))); } catch (...) {}
	}
	try { m_VideoEncoder->SetGOPAlignmentEnabled(!!obs_data_get_int(data, AMF_H264_GOPALIGNMENT)); } catch (...) {}

	try { m_VideoEncoder->SetIntraRefreshNumberOfStripes(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_INTRAREFRESH_NUMBEROFSTRIPES))); } catch (...) {}
	try { m_VideoEncoder->SetIntraRefreshMacroblocksPerSlot(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_INTRAREFRESH_MACROBLOCKSPERSLOT))); } catch (...) {}

	try { m_VideoEncoder->SetSlicesPerFrame(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_SLICESPERFRAME))); } catch (...) {}
	try { m_VideoEncoder->SetSliceMode(static_cast<H264SliceMode>(obs_data_get_int(data, AMF_H264_SLICEMODE))); } catch (...) {}
	try { m_VideoEncoder->SetMaximumSliceSize(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_MAXIMUMSLICESIZE))); } catch (...) {}
	try { m_VideoEncoder->SetSliceControlMode(static_cast<H264SliceControlMode>(obs_data_get_int(data, AMF_H264_SLICECONTROLMODE))); } catch (...) {}
	try { m_VideoEncoder->SetSliceControlSize(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_SLICECONTROLSIZE))); } catch (...) {}
	#pragma endregion Experimental

	if (m_VideoEncoder->IsStarted()) {
		// OBS - Enforce Streaming Service Stuff
		#pragma region OBS Enforce Streaming Service Settings
		{
			// Rate Control Method
			const char* t_str = obs_data_get_string(data, "rate_control");
			if (strcmp(t_str, "") != 0) {
				if (strcmp(t_str, "CBR") == 0) {
					m_VideoEncoder->SetRateControlMethod(H264RateControlMethod::ConstantBitrate);
					m_VideoEncoder->SetFillerDataEnabled(true);
				} else if (strcmp(t_str, "VBR") == 0) {
					m_VideoEncoder->SetRateControlMethod(H264RateControlMethod::VariableBitrate_PeakConstrained);
				} else if (strcmp(t_str, "VBR_LAT") == 0) {
					m_VideoEncoder->SetRateControlMethod(H264RateControlMethod::VariableBitrate_LatencyConstrained);
				} else if (strcmp(t_str, "CQP") == 0) {
					m_VideoEncoder->SetRateControlMethod(H264RateControlMethod::ConstantQP);
				}

				obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, (int32_t)m_VideoEncoder->GetRateControlMethod());
			} else {
				if (m_VideoEncoder->GetUsage() != H264Usage::UltraLowLatency)
					switch (m_VideoEncoder->GetRateControlMethod()) {
						case H264RateControlMethod::ConstantBitrate:
							obs_data_set_string(data, "rate_control", "CBR");
							break;
						case H264RateControlMethod::VariableBitrate_PeakConstrained:
							obs_data_set_string(data, "rate_control", "VBR");
							break;
						case H264RateControlMethod::VariableBitrate_LatencyConstrained:
							obs_data_set_string(data, "rate_control", "VBR_LAT");
							break;
						case H264RateControlMethod::ConstantQP:
							obs_data_set_string(data, "rate_control", "CQP");
							break;
					}
			}

			// Bitrate
			uint64_t bitrateOvr = obs_data_get_int(data, "bitrate") * 1000;
			if (bitrateOvr != -1) {
				if (m_VideoEncoder->GetTargetBitrate() > bitrateOvr)
					m_VideoEncoder->SetTargetBitrate(static_cast<uint32_t>(bitrateOvr));

				if (m_VideoEncoder->GetUsage() != H264Usage::UltraLowLatency)
					if (m_VideoEncoder->GetPeakBitrate() > bitrateOvr)
						m_VideoEncoder->SetPeakBitrate(static_cast<uint32_t>(bitrateOvr));

				obs_data_set_int(data, "bitrate", m_VideoEncoder->GetTargetBitrate() / 1000);

				obs_data_set_int(data, AMF_H264_BITRATE_TARGET, m_VideoEncoder->GetTargetBitrate() / 1000);
				if (m_VideoEncoder->GetUsage() != H264Usage::UltraLowLatency)
					obs_data_set_int(data, AMF_H264_BITRATE_PEAK, m_VideoEncoder->GetPeakBitrate() / 1000);
			} else {
				obs_data_set_int(data, "bitrate", m_VideoEncoder->GetTargetBitrate() / 1000);
			}

			// IDR-Period (Keyframes)
			uint32_t fpsNum = m_VideoEncoder->GetFrameRate().first;
			uint32_t fpsDen = m_VideoEncoder->GetFrameRate().second;
			if (obs_data_get_int(data, "keyint_sec") != -1) {
				m_VideoEncoder->SetIDRPeriod(static_cast<uint32_t>(obs_data_get_int(data, "keyint_sec") * (static_cast<double_t>(fpsNum) / static_cast<double_t>(fpsDen))));

				obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, static_cast<double_t>(obs_data_get_int(data, "keyint_sec")));
				obs_data_set_int(data, AMF_H264_IDR_PERIOD, static_cast<uint32_t>(obs_data_get_int(data, "keyint_sec") *  (static_cast<double_t>(fpsNum) / static_cast<double_t>(fpsDen))));
			} else {
				obs_data_set_int(data, "keyint_sec", static_cast<uint64_t>(m_VideoEncoder->GetIDRPeriod() / (static_cast<double_t>(fpsNum) / static_cast<double_t>(fpsDen))));
			}
		}
		#pragma endregion OBS Enforce Streaming Service Settings

		// Verify
		m_VideoEncoder->LogProperties();
		if (static_cast<ViewMode>(obs_data_get_int(data, AMF_H264_VIEW)) >= ViewMode::Master)
			AMF_LOG_ERROR("View Mode 'Master' is active, avoid giving anything but basic support. Error is most likely caused by user settings themselves.");
	}

	return true;
}

bool Plugin::Interface::H264Interface::encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	if (!frame || !packet || !received_packet)
		return false;

	bool retVal = true;

	retVal = m_VideoEncoder->SendInput(frame);
	retVal = retVal && m_VideoEncoder->GetOutput(packet, received_packet);

	return retVal;
}

void Plugin::Interface::H264Interface::get_video_info(struct video_scale_info* info) {
	m_VideoEncoder->GetVideoInfo(info);
}

bool Plugin::Interface::H264Interface::get_extra_data(uint8_t** extra_data, size_t* size) {
	return m_VideoEncoder->GetExtraData(extra_data, size);
}
