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

#define PREFIX "[H264/AVC]"

void Plugin::Interface::H264Interface::encoder_register() {
	// Test if we actually have AVC support.
	if (!AMD::CapabilityManager::Instance()->IsCodecSupported(Codec::H264AVC)) {
		AMF_LOG_WARNING(PREFIX " Not supported by any GPU, disabling...");
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
	static const char* name = "H264/AVC Encoder (" PLUGIN_NAME_AMF ")";
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
		if (enc)
			delete enc;
	}
	return nullptr;
}

void Plugin::Interface::H264Interface::destroy(void* data) {
	try {
		AMF_LOG_INFO("Shutting down...");
		Plugin::Interface::H264Interface* enc = static_cast<Plugin::Interface::H264Interface*>(data);
		delete enc;
	} catch (std::exception e) {
		AMF_LOG_ERROR("%s", e.what());
		data = nullptr;
	}
}

bool Plugin::Interface::H264Interface::encode(void *data, struct encoder_frame *frame, struct encoder_packet *packet, bool *received_packet) {
	try {
		return static_cast<Plugin::Interface::H264Interface*>(data)->encode(frame, packet, received_packet);
	} catch (std::exception e) {
		AMF_LOG_ERROR("%s", e.what());
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
	obs_data_set_default_int(data, AMF_H264_USAGE, static_cast<int32_t>(Usage::Transcoding));
	obs_data_set_default_int(data, AMF_H264_QUALITY_PRESET, static_cast<int32_t>(QualityPreset::Balanced));
	obs_data_set_default_int(data, AMF_H264_PROFILE, static_cast<int32_t>(Profile::Main));
	obs_data_set_default_int(data, AMF_H264_PROFILELEVEL, static_cast<int32_t>(ProfileLevel::Automatic));
	obs_data_set_default_frames_per_second(data, AMF_H264_ASPECTRATIO, media_frames_per_second{ 1, 1 }, "");
	obs_data_set_default_int(data, AMF_H264_CODINGTYPE, static_cast<int32_t>(CodingType::Automatic));
	obs_data_set_default_int(data, AMF_H264_MAXIMUMREFERENCEFRAMES, 4);

	// Rate Control Properties
	obs_data_set_int(data, "last" vstr(AMF_H264_RATECONTROLMETHOD), -1);
	obs_data_set_default_int(data, AMF_H264_RATECONTROLMETHOD, static_cast<int32_t>(RateControlMethod::ConstantBitrate));
	obs_data_set_default_int(data, AMF_H264_PREPASS, static_cast<int32_t>(PrePassMode::Disabled));
	obs_data_set_default_int(data, AMF_H264_BITRATE_TARGET, 3500);
	obs_data_set_default_int(data, AMF_H264_BITRATE_PEAK, 9000);
	obs_data_set_default_int(data, AMF_H264_QP_IFRAME, 22);
	obs_data_set_default_int(data, AMF_H264_QP_PFRAME, 22);
	obs_data_set_default_int(data, AMF_H264_QP_BFRAME, 22);
	obs_data_set_default_int(data, AMF_H264_QP_MINIMUM, 18);
	obs_data_set_default_int(data, AMF_H264_QP_MAXIMUM, 51);
	obs_data_set_default_int(data, AMF_H264_FILLERDATA, 1);
	obs_data_set_default_int(data, AMF_H264_FRAMESKIPPING, 0);
	obs_data_set_default_int(data, AMF_H264_VBAQ, 0);
	obs_data_set_default_int(data, AMF_H264_ENFORCEHRD, 1);

	// VBV Buffer
	obs_data_set_int(data, "last" vstr(AMF_H264_VBVBUFFER), -1);
	obs_data_set_default_int(data, AMF_H264_VBVBUFFER, 0);
	obs_data_set_default_int(data, AMF_H264_VBVBUFFER_SIZE, 3500);
	obs_data_set_default_double(data, AMF_H264_VBVBUFFER_STRICTNESS, 50);
	obs_data_set_default_double(data, AMF_H264_VBVBUFFER_FULLNESS, 100);

	// Picture Control
	obs_data_set_default_double(data, AMF_H264_KEYFRAME_INTERVAL, 2);
	obs_data_set_default_int(data, AMF_H264_IDR_PERIOD, 60);
	obs_data_set_int(data, "last" vstr(AMF_H264_BFRAME_PATTERN), -1);
	obs_data_set_default_int(data, AMF_H264_BFRAME_PATTERN, 0);
	obs_data_set_int(data, "last" vstr(AMF_H264_BFRAME_REFERENCE), -1);
	obs_data_set_default_int(data, AMF_H264_BFRAME_REFERENCE, 0);
	obs_data_set_default_int(data, AMF_H264_BFRAME_REFERENCEDELTAQP, 2);
	obs_data_set_default_int(data, AMF_H264_BFRAME_DELTAQP, 4);
	obs_data_set_default_int(data, AMF_H264_DEBLOCKINGFILTER, 1);
	obs_data_set_default_int(data, AMF_H264_MOTIONESTIMATION, 3);

	// System Properties
	obs_data_set_string(data, "last" vstr(AMF_H264_VIDEOAPI), "");
	obs_data_set_default_string(data, AMF_H264_VIDEOAPI, "");
	obs_data_set_int(data, "last" vstr(AMF_H264_VIDEOADAPTER), 0);
	obs_data_set_default_int(data, AMF_H264_VIDEOADAPTER, 0);
	obs_data_set_default_int(data, AMF_H264_OPENCL, 0);
	obs_data_set_int(data, "last" vstr(AMF_H264_VIEW), -1);
	obs_data_set_default_int(data, AMF_H264_VIEW, static_cast<int32_t>(ViewMode::Basic));
	obs_data_set_default_bool(data, AMF_H264_DEBUG, false);
	obs_data_set_default_int(data, AMF_H264_VERSION, 0x0001000400030005ull);
}

static void fill_api_list(obs_property_t* p) {
	obs_property_list_clear(p);
	auto cm = CapabilityManager::Instance();

	for (auto api : Plugin::API::EnumerateAPIs()) {
		if (cm->IsCodecSupportedByAPI(Codec::H264AVC, api->GetType()))
			obs_property_list_add_string(p, api->GetName().c_str(), api->GetName().c_str());
	}
}

static void fill_device_list(obs_property_t* p, const char* apiname) {
	obs_property_list_clear(p);

	auto cm = CapabilityManager::Instance();
	auto api = Plugin::API::GetAPI(std::string(apiname));
	for (auto adapter : api->EnumerateAdapters()) {
		union {
			int32_t id[2];
			int64_t v;
		} adapterid = { adapter.idLow, adapter.idHigh };
		if (cm->IsCodecSupportedByAPIAdapter(Codec::H264AVC, api->GetType(), adapter))
			obs_property_list_add_int(p, adapter.Name.c_str(), adapterid.v);
	}
}

obs_properties_t* Plugin::Interface::H264Interface::get_properties(void*) {
	obs_properties* props = obs_properties_create();
	obs_property_t* p;

	//////////////////////////////////////////////////////////////////////////
	// New UI Design
	//////////////////////////////////////////////////////////////////////////
	// All: Preset
	// ----------- Static Section
	// Mas: Usage
	// All: Quality Preset
	// Adv: Profile
	// Adv: Profile Level
	// Exp: Aspect Ratio
	// Exp: Coding Type
	// Exp: Maximum Reference Frames
	// ----------- Rate Control Section
	// All: Rate Control Method
	// Adv: Pre-Pass Encoding (if supported)
	// All, CBR&VBR: Target Bitrate
	// All, VBR: Peak Bitrate
	// All, CQP: QP I/P/B
	// Adv, CBR&VBR: Min/Max QP
	// CBR: Filler Data
	// Adv: Frame Skipping
	// Exp: VBAQ
	// Exp: Enforce HRD 
	// ----------- VBV Buffer
	// Adv: VBV Buffer Size
	// Exp: VBV Buffer Initial Fullness
	// ----------- Picture Control
	// All: Keyframe Interval (Float)
	// Mas: IDR Period (Overrides Keyframe Interval if non-zero)
	// Adv: B-Frames (if supported)
	// Adv: B-Frame Delta QP (if supported, not CQP)
	// Adv: B-Frame Reference (if supported and B-Frames enabled)
	// Adv: B-Frame Reference Delta QP (if supported, not CQP)
	// Exp: Deblocking Filter
	// Exp: Motion Estimation (Dropdown)
	// ----------- Intra-Refresh
	// ToDo: Master Mode only?
	// ----------- System
	// Adv: API
	// Adv: Adapter
	// Exp: OpenCL
	// All: View

	// 1.4.3.8 -> 2.0.0.0
	// PreAnalysisPass -> PrePass

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

	// Static Properties
	#pragma region Usage
	p = obs_properties_add_list(props, AMF_H264_USAGE, TEXT_T(AMF_H264_USAGE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_USAGE_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_USAGE_TRANSCODING), static_cast<int32_t>(Usage::Transcoding));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_USAGE_ULTRALOWLATENCY), static_cast<int32_t>(Usage::UltraLowLatency));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_USAGE_LOWLATENCY), static_cast<int32_t>(Usage::LowLatency));
	#pragma endregion Usage

	#pragma region Quality Preset
	p = obs_properties_add_list(props, AMF_H264_QUALITY_PRESET, TEXT_T(AMF_H264_QUALITY_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QUALITY_PRESET_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_QUALITY_PRESET_SPEED), static_cast<int32_t>(QualityPreset::Speed));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_QUALITY_PRESET_BALANCED), static_cast<int32_t>(QualityPreset::Balanced));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_QUALITY_PRESET_QUALITY), static_cast<int32_t>(QualityPreset::Quality));
	#pragma endregion Quality Preset

	#pragma region Profile, Levels
	p = obs_properties_add_list(props, AMF_H264_PROFILE, TEXT_T(AMF_H264_PROFILE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_PROFILE_DESCRIPTION));
	obs_property_list_add_int(p, "Constrained Baseline", static_cast<int32_t>(Profile::ConstrainedBaseline));
	obs_property_list_add_int(p, "Baseline", static_cast<int32_t>(Profile::Baseline));
	obs_property_list_add_int(p, "Main", static_cast<int32_t>(Profile::Main));
	obs_property_list_add_int(p, "Constrained High", static_cast<int32_t>(Profile::ConstrainedHigh));
	obs_property_list_add_int(p, "High", static_cast<int32_t>(Profile::High));

	p = obs_properties_add_list(props, AMF_H264_PROFILELEVEL, TEXT_T(AMF_H264_PROFILELEVEL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_PROFILELEVEL_DESCRIPTION));
	obs_property_list_add_int(p, AMF_UTIL_AUTOMATIC, static_cast<int32_t>(ProfileLevel::Automatic));
	obs_property_list_add_int(p, "1.0", static_cast<int32_t>(ProfileLevel::L10));
	obs_property_list_add_int(p, "1.1", static_cast<int32_t>(ProfileLevel::L11));
	obs_property_list_add_int(p, "1.2", static_cast<int32_t>(ProfileLevel::L12));
	obs_property_list_add_int(p, "1.3", static_cast<int32_t>(ProfileLevel::L13));
	obs_property_list_add_int(p, "2.0", static_cast<int32_t>(ProfileLevel::L20));
	obs_property_list_add_int(p, "2.1", static_cast<int32_t>(ProfileLevel::L21));
	obs_property_list_add_int(p, "2.2", static_cast<int32_t>(ProfileLevel::L22));
	obs_property_list_add_int(p, "3.0", static_cast<int32_t>(ProfileLevel::L30));
	obs_property_list_add_int(p, "3.1", static_cast<int32_t>(ProfileLevel::L31));
	obs_property_list_add_int(p, "3.2", static_cast<int32_t>(ProfileLevel::L32));
	obs_property_list_add_int(p, "4.0", static_cast<int32_t>(ProfileLevel::L40));
	obs_property_list_add_int(p, "4.1", static_cast<int32_t>(ProfileLevel::L41));
	obs_property_list_add_int(p, "4.2", static_cast<int32_t>(ProfileLevel::L42));
	obs_property_list_add_int(p, "5.0", static_cast<int32_t>(ProfileLevel::L50));
	obs_property_list_add_int(p, "5.1", static_cast<int32_t>(ProfileLevel::L51));
	obs_property_list_add_int(p, "5.2", static_cast<int32_t>(ProfileLevel::L52));
	#pragma endregion Profile, Levels

	#pragma region Aspect Ratio
	p = obs_properties_add_frame_rate(props, AMF_H264_ASPECTRATIO, TEXT_T(AMF_H264_ASPECTRATIO));
	obs_property_set_long_description(p, TEXT_T(AMF_H264_ASPECTRATIO_DESCRIPTION));
	#pragma endregion Aspect Ratio

	#pragma region Coding Type
	p = obs_properties_add_list(props, AMF_H264_CODINGTYPE, TEXT_T(AMF_H264_CODINGTYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_CODINGTYPE_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_AUTOMATIC), static_cast<int32_t>(CodingType::Automatic));
	obs_property_list_add_int(p, "CABAC", static_cast<int32_t>(CodingType::CABAC));
	obs_property_list_add_int(p, "CALVC", static_cast<int32_t>(CodingType::CALVC));
	#pragma endregion Coding Type

	#pragma region Maximum Reference Frames
	p = obs_properties_add_int_slider(props, AMF_H264_MAXIMUMREFERENCEFRAMES, TEXT_T(AMF_H264_MAXIMUMREFERENCEFRAMES),
		1, 16, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_MAXIMUMREFERENCEFRAMES_DESCRIPTION));
	#pragma endregion Maximum Reference Frames

	// Rate Control
	#pragma region Rate Control Method
	p = obs_properties_add_list(props, AMF_H264_RATECONTROLMETHOD, TEXT_T(AMF_H264_RATECONTROLMETHOD), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_CQP), static_cast<int32_t>(RateControlMethod::ConstantQP));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_CBR), static_cast<int32_t>(RateControlMethod::ConstantBitrate));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_VBR), static_cast<int32_t>(RateControlMethod::PeakConstrainedVariableBitrate));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_VBR_LAT), static_cast<int32_t>(RateControlMethod::LatencyConstrainedVariableBitrate));
	obs_property_set_modified_callback(p, properties_modified);
	#pragma endregion Rate Control Method

	#pragma region Pre-Pass
	p = obs_properties_add_list(props, AMF_H264_PREPASS, TEXT_T(AMF_H264_PREPASS), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_PREPASS_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), static_cast<int32_t>(PrePassMode::Disabled));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PREPASS_QUARTERSCALE), static_cast<int32_t>(PrePassMode::EnabledAtQuarterScale));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PREPASS_HALFSCALE), static_cast<int32_t>(PrePassMode::EnabledAtHalfScale));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PREPASS_FULLSCALE), static_cast<int32_t>(PrePassMode::Enabled));
	#pragma endregion Pre-Pass

	#pragma region Parameters
	/// Bitrate Constraints
	p = obs_properties_add_int(props, AMF_H264_BITRATE_TARGET, TEXT_T(AMF_H264_BITRATE_TARGET), 0, 1, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BITRATE_TARGET_DESCRIPTION));
	p = obs_properties_add_int(props, AMF_H264_BITRATE_PEAK, TEXT_T(AMF_H264_BITRATE_PEAK), 0, 1, 1);
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
	#pragma endregion Parameters

	#pragma region Filler Data
	p = obs_properties_add_list(props, AMF_H264_FILLERDATA, TEXT_T(AMF_H264_FILLERDATA), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_FILLERDATA_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	#pragma endregion Filler Data

	#pragma region Frame Skipping
	p = obs_properties_add_list(props, AMF_H264_FRAMESKIPPING, TEXT_T(AMF_H264_FRAMESKIPPING), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_FRAMESKIPPING_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	#pragma endregion Frame Skipping

	#pragma region VBAQ
	p = obs_properties_add_list(props, AMF_H264_VBAQ, TEXT_T(AMF_H264_VBAQ), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VBAQ_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	#pragma endregion VBAQ

	#pragma region Enforce Hyptothetical Reference Decoder Restrictions
	p = obs_properties_add_list(props, AMF_H264_ENFORCEHRD, TEXT_T(AMF_H264_ENFORCEHRD), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_ENFORCEHRD_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	#pragma endregion Enforce Hyptothetical Reference Decoder Restrictions

	// VBV Buffer
	#pragma region VBV Buffer Mode
	p = obs_properties_add_list(props, AMF_H264_VBVBUFFER, TEXT_T(AMF_H264_VBVBUFFER), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VBVBUFFER_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_AUTOMATIC), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_MANUAL), 1);
	obs_property_set_modified_callback(p, properties_modified);
	#pragma endregion VBV Buffer Mode

	#pragma region VBV Buffer Strictness
	p = obs_properties_add_float_slider(props, AMF_H264_VBVBUFFER_STRICTNESS, TEXT_T(AMF_H264_VBVBUFFER_STRICTNESS), 0.0, 100.0, 0.1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VBVBUFFER_STRICTNESS_DESCRIPTION));
	#pragma endregion VBV Buffer Strictness

	#pragma region VBV Buffer Size
	p = obs_properties_add_int_slider(props, AMF_H264_VBVBUFFER_SIZE, TEXT_T(AMF_H264_VBVBUFFER_SIZE), 1, 1000000, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VBVBUFFER_SIZE_DESCRIPTION));
	#pragma endregion VBV Buffer Size

	#pragma region VBV Buffer Initial Fullness
	p = obs_properties_add_float_slider(props, AMF_H264_VBVBUFFER_FULLNESS, TEXT_T(AMF_H264_VBVBUFFER_FULLNESS), 0.0, 100.0, 100.0 / 64.0);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VBVBUFFER_FULLNESS_DESCRIPTION));
	#pragma endregion VBV Buffer Initial Fullness

	// Picture Control
	#pragma region Keyframe Interval
	p = obs_properties_add_float(props, AMF_H264_KEYFRAME_INTERVAL, TEXT_T(AMF_H264_KEYFRAME_INTERVAL), 0, 100, 0.001);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_KEYFRAME_INTERVAL_DESCRIPTION));
	#pragma endregion Keyframe Interval

	#pragma region IDR Period
	p = obs_properties_add_int(props, AMF_H264_IDR_PERIOD, TEXT_T(AMF_H264_IDR_PERIOD), 1, 1000, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_IDR_PERIOD_DESCRIPTION));
	#pragma endregion IDR Period

	#pragma region B-Frames Pattern
	p = obs_properties_add_int_slider(props, AMF_H264_BFRAME_PATTERN, TEXT_T(AMF_H264_BFRAME_PATTERN), 0, 3, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BFRAME_PATTERN_DESCRIPTION));
	obs_property_set_modified_callback(p, properties_modified);
	#pragma endregion B-Frames Pattern

	#pragma region B-Frames Reference
	p = obs_properties_add_list(props, AMF_H264_BFRAME_REFERENCE, TEXT_T(AMF_H264_BFRAME_REFERENCE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BFRAME_REFERENCE_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	obs_property_set_modified_callback(p, properties_modified);
	#pragma endregion B-Frames Reference

	#pragma region B-Frames Delta QP
	p = obs_properties_add_int_slider(props, AMF_H264_BFRAME_REFERENCEDELTAQP, TEXT_T(AMF_H264_BFRAME_REFERENCEDELTAQP), -10, 10, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BFRAME_REFERENCEDELTAQP_DESCRIPTION));

	p = obs_properties_add_int_slider(props, AMF_H264_BFRAME_DELTAQP, TEXT_T(AMF_H264_BFRAME_DELTAQP), -10, 10, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BFRAME_DELTAQP_DESCRIPTION));
	#pragma endregion B-Frames Delta QP

	#pragma region Deblocking Filter
	p = obs_properties_add_list(props, AMF_H264_DEBLOCKINGFILTER, TEXT_T(AMF_H264_DEBLOCKINGFILTER), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_DEBLOCKINGFILTER_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	#pragma endregion Deblocking Filter

	#pragma region Motion Estimation
	p = obs_properties_add_list(props, AMF_H264_MOTIONESTIMATION, TEXT_T(AMF_H264_MOTIONESTIMATION), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_MOTIONESTIMATION_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_MOTIONESTIMATION_NONE), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_MOTIONESTIMATION_HALF), 1);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_MOTIONESTIMATION_QUARTER), 2);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_MOTIONESTIMATION_BOTH), 3);
	#pragma endregion Motion Estimation

	// System
	#pragma region Video APIs
	p = obs_properties_add_list(props, AMF_H264_VIDEOAPI, TEXT_T(AMF_H264_VIDEOAPI), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VIDEOAPI_DESCRIPTION));
	obs_property_set_modified_callback(p, properties_modified);
	fill_api_list(p);
	#pragma endregion Video APIs

	#pragma region Video Adapters
	p = obs_properties_add_list(props, AMF_H264_VIDEOADAPTER, TEXT_T(AMF_H264_VIDEOADAPTER), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VIDEOADAPTER_DESCRIPTION));
	obs_property_set_modified_callback(p, properties_modified);
	#pragma endregion Video Adapters

	#pragma region OpenCL
	p = obs_properties_add_list(props, AMF_H264_OPENCL, TEXT_T(AMF_H264_OPENCL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_OPENCL_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	#pragma endregion OpenCL

	#pragma region View Mode
	p = obs_properties_add_list(props, AMF_H264_VIEW, TEXT_T(AMF_H264_VIEW), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VIEW_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_VIEW_BASIC), static_cast<int32_t>(ViewMode::Basic));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_VIEW_ADVANCED), static_cast<int32_t>(ViewMode::Advanced));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_VIEW_EXPERT), static_cast<int32_t>(ViewMode::Expert));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_VIEW_MASTER), static_cast<int32_t>(ViewMode::Master));
	obs_property_set_modified_callback(p, properties_modified);
	#pragma endregion View Mode

	/// Debug
	p = obs_properties_add_bool(props, AMF_H264_DEBUG, TEXT_T(AMF_H264_DEBUG));
	obs_property_set_long_description(p, TEXT_T(AMF_H264_DEBUG_DESCRIPTION));

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
			obs_data_set_double(data, "AMF.H264.VBVBuffer.Strictness", obs_data_get_double(data, "AMF.H264.VBVBuffer.Strictness") + 50.0);
		case 0x0001000400030008ull:
			obs_data_set_int(data, "AMF.H264.PrePass", obs_data_get_int(data, "AMF.H264.PreAnalysisPass"));
		case PLUGIN_VERSION_FULL:
			obs_data_set_int(data, AMF_H264_VERSION, PLUGIN_VERSION_FULL);
			break;
	}
	#pragma endregion Version Differences
}

bool Plugin::Interface::H264Interface::properties_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	bool result = false;
	obs_property_t* p;

	// Transfer settings from older Plugin versions to newer ones.
	obs_data_transfer_settings(data);

	// Video API
	const char
		*videoAPI_last = obs_data_get_string(data, "last" vstr(AMF_H264_VIDEOAPI)),
		*videoAPI_cur = obs_data_get_string(data, AMF_H264_VIDEOAPI);
	if (strlen(videoAPI_cur) == 0) {
		p = obs_properties_get(props, AMF_H264_VIDEOAPI);
		obs_data_set_string(data, AMF_H264_VIDEOAPI, obs_property_list_item_string(p, 0));
		videoAPI_cur = obs_data_get_string(data, AMF_H264_VIDEOAPI);

		result = true;
	}
	/// If a different API was selected, rebuild the device list.
	if (strcmp(videoAPI_last, videoAPI_cur) != 0) {
		obs_data_set_string(data, "last" vstr(AMF_H264_VIDEOAPI), videoAPI_cur);
		fill_device_list(obs_properties_get(props, AMF_H264_VIDEOADAPTER), videoAPI_cur);
		result = true;

		// Reset Video Adapter to first in list.
		obs_data_set_int(data, AMF_H264_VIDEOADAPTER,
			obs_property_list_item_int(obs_properties_get(props, AMF_H264_VIDEOADAPTER), 0));
	}

	// Video Adapter
	int64_t
		videoAdapter_last = obs_data_get_int(data, "last" vstr(AMF_H264_VIDEOADAPTER)),
		videoAdapter_cur = obs_data_get_int(data, AMF_H264_VIDEOADAPTER);
	if (videoAdapter_last != videoAdapter_cur) {
		obs_data_set_int(data, "last" vstr(AMF_H264_VIDEOADAPTER), videoAdapter_cur);
		result = true;

		auto api = Plugin::API::GetAPI(obs_data_get_string(data, AMF_H264_VIDEOAPI));
		union {
			int64_t v;
			uint32_t id[2];
		} adapterid = { videoAdapter_cur };
		auto adapter = api->GetAdapterById(adapterid.id[0], adapterid.id[1]);
		try {
			auto enc = EncoderH264(api, adapter, false, ColorFormat::NV12, ColorSpace::BT601, false);

			#define TEMP_LIMIT_DROPDOWN(func, enm, prop) { \
				auto tmp_p = obs_properties_get(props, prop); \
				auto tmp_l = enc.func(); \
				enm tmp_s = static_cast<enm>(obs_data_get_int(data, obs_property_name(tmp_p))); \
				for (size_t idx = 0; idx < obs_property_list_item_count(tmp_p); idx++) { \
					bool enabled = false; \
					enm tmp_v = static_cast<enm>(obs_property_list_item_int(tmp_p, idx)); \
					for (auto tmp_k : tmp_l) { \
						if (tmp_k == tmp_v) { \
							enabled = true; \
							break; \
						} \
					} \
					obs_property_list_item_disable(tmp_p, idx, !enabled); \
					if ((enabled == false) && (tmp_s == tmp_v)) \
						obs_data_default_single(props, data, obs_property_name(tmp_p)); \
				} \
			}
			#define TEMP_LIMIT_SLIDER(func, prop) { \
				auto tmp_p = obs_properties_get(props, prop); \
				auto tmp_l = enc.func(); \
				obs_property_int_set_limits(tmp_p, (int)tmp_l.first, (int)tmp_l.second, 1); \
			}
			#define TEMP_LIMIT_SLIDER_BITRATE(func, prop) { \
				auto tmp_p = obs_properties_get(props, prop); \
				auto tmp_l = enc.func(); \
				obs_property_int_set_limits(tmp_p, (int)tmp_l.first / 1000, (int)tmp_l.second / 1000, 1); \
			}

			TEMP_LIMIT_DROPDOWN(CapsUsage, AMD::Usage, AMF_H264_USAGE);
			TEMP_LIMIT_DROPDOWN(CapsQualityPreset, AMD::QualityPreset, AMF_H264_QUALITY_PRESET);
			TEMP_LIMIT_DROPDOWN(CapsProfile, AMD::Profile, AMF_H264_PROFILE);
			TEMP_LIMIT_DROPDOWN(CapsProfileLevel, AMD::ProfileLevel, AMF_H264_PROFILELEVEL);
			// Aspect Ratio - No limits, only affects players/transcoders
			TEMP_LIMIT_DROPDOWN(CapsCodingType, AMD::CodingType, AMF_H264_CODINGTYPE);
			TEMP_LIMIT_SLIDER(CapsMaximumReferenceFrames, AMF_H264_MAXIMUMREFERENCEFRAMES);
			TEMP_LIMIT_DROPDOWN(CapsRateControlMethod, AMD::RateControlMethod, AMF_H264_RATECONTROLMETHOD);
			TEMP_LIMIT_DROPDOWN(CapsPrePassMode, AMD::PrePassMode, AMF_H264_PREPASS);
			TEMP_LIMIT_SLIDER_BITRATE(CapsTargetBitrate, AMF_H264_BITRATE_TARGET);
			TEMP_LIMIT_SLIDER_BITRATE(CapsPeakBitrate, AMF_H264_BITRATE_PEAK);
			TEMP_LIMIT_SLIDER_BITRATE(CapsVBVBufferSize, AMF_H264_VBVBUFFER_SIZE);
			{
				auto p = obs_properties_get(props, AMF_H264_BFRAME_PATTERN);
				auto l = enc.CapsBFramePattern();
				obs_property_int_set_limits(p, 0, (int)l, 1);
				if (obs_data_get_int(data, obs_property_name(p)) > l) {
					obs_data_set_int(data, obs_property_name(p), l);
				}
			}
		} catch (std::exception& e) {
			AMF_LOG_ERROR("Exception occured while updating capabilities: %s",
				e.what());
		}
	}

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
			obs_data_set_int(data, AMF_H264_USAGE, static_cast<int32_t>(Usage::Transcoding));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_USAGE), false);
			obs_data_set_int(data, AMF_H264_PROFILE, static_cast<int32_t>(Profile::High));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, static_cast<int32_t>(ProfileLevel::Automatic));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, static_cast<int32_t>(RateControlMethod::LatencyConstrainedVariableBitrate));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			if (obs_data_get_int(data, AMF_H264_BITRATE_TARGET) < 10000)
				obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 10000);
			obs_data_default_single(props, data, AMF_H264_QP_MINIMUM);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_MINIMUM), false);
			obs_data_default_single(props, data, AMF_H264_QP_MAXIMUM);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_MAXIMUM), false);
			obs_data_set_int(data, AMF_H264_FILLERDATA, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_FILLERDATA), false);

			// Frame Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 1);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);

			// Miscellaneous Properties
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			break;
			#pragma endregion Recording
		case Presets::HighQuality:
			#pragma region High Quality
			// Static Properties
			obs_data_set_int(data, AMF_H264_USAGE, static_cast<int32_t>(Usage::Transcoding));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_USAGE), false);
			obs_data_set_int(data, AMF_H264_PROFILE, static_cast<int32_t>(Profile::High));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, static_cast<int32_t>(ProfileLevel::Automatic));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, static_cast<int32_t>(RateControlMethod::ConstantQP));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			obs_data_set_int(data, AMF_H264_QP_IFRAME, 26);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_IFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_PFRAME, 24);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_PFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_BFRAME, 22);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_BFRAME), false);

			// Frame Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);

			// Miscellaneous Properties
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			break;
			#pragma endregion High Quality
		case Presets::Indistinguishable:
			#pragma region Indistinguishable
			// Static Properties
			obs_data_set_int(data, AMF_H264_USAGE, static_cast<int32_t>(Usage::Transcoding));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_USAGE), false);
			obs_data_set_int(data, AMF_H264_PROFILE, static_cast<int32_t>(Profile::High));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, static_cast<int32_t>(ProfileLevel::Automatic));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, static_cast<int32_t>(RateControlMethod::ConstantQP));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			obs_data_set_int(data, AMF_H264_QP_IFRAME, 21);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_IFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_PFRAME, 19);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_PFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_BFRAME, 17);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_BFRAME), false);

			// Frame Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);

			// Miscellaneous Properties
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			break;
			#pragma endregion Indistinguishable
		case Presets::Lossless:
			#pragma region Lossless
			// Static Properties
			obs_data_set_int(data, AMF_H264_USAGE, static_cast<int32_t>(Usage::Transcoding));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_USAGE), false);
			obs_data_set_int(data, AMF_H264_PROFILE, static_cast<int32_t>(Profile::High));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, static_cast<int32_t>(ProfileLevel::Automatic));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, static_cast<int32_t>(RateControlMethod::ConstantQP));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			obs_data_set_int(data, AMF_H264_QP_IFRAME, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_IFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_PFRAME, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_PFRAME), false);

			// Frame Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);
			obs_data_set_int(data, AMF_H264_BFRAME_PATTERN, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_BFRAME_PATTERN), false);

			// Miscellaneous Properties
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			break;
			#pragma endregion Lossless
		case Presets::Twitch:
			#pragma region Twitch
			// Static Properties
			obs_data_set_int(data, AMF_H264_USAGE, static_cast<int32_t>(Usage::Transcoding));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_USAGE), false);
			obs_data_set_int(data, AMF_H264_PROFILE, static_cast<int32_t>(Profile::Main));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, static_cast<int32_t>(ProfileLevel::Automatic));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, static_cast<int32_t>(RateControlMethod::ConstantBitrate));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			if (obs_data_get_int(data, AMF_H264_BITRATE_TARGET) < 500)
				obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 500);
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
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			break;
			#pragma endregion Twitch
		case Presets::YouTube:
			#pragma region YouTube
			// Static Properties
			obs_data_set_int(data, AMF_H264_USAGE, static_cast<int32_t>(Usage::Transcoding));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_USAGE), false);
			obs_data_set_int(data, AMF_H264_PROFILE, static_cast<int32_t>(Profile::Main));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, static_cast<int32_t>(ProfileLevel::Automatic));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, static_cast<int32_t>(RateControlMethod::ConstantBitrate));
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			if (obs_data_get_int(data, AMF_H264_BITRATE_TARGET) < 500)
				obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 500);
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
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			break;
			#pragma endregion YouTube
	}
	#pragma endregion Presets

	#pragma region View Mode
	ViewMode lastView = static_cast<ViewMode>(obs_data_get_int(data, "last" vstr(AMF_H264_VIEW))),
		curView = static_cast<ViewMode>(obs_data_get_int(data, AMF_H264_VIEW));
	if (lastView != curView) {
		obs_data_set_int(data, "last" vstr(AMF_H264_VIEW), static_cast<int32_t>(curView));
		result = true;
	}

	std::vector<std::pair<const char*, ViewMode>> viewstuff = {
		std::make_pair(AMF_H264_PRESET, ViewMode::Basic),
		// ----------- Static Section
		std::make_pair(AMF_H264_USAGE, ViewMode::Master),
		std::make_pair(AMF_H264_QUALITY_PRESET, ViewMode::Basic),
		std::make_pair(AMF_H264_PROFILE, ViewMode::Advanced),
		std::make_pair(AMF_H264_PROFILELEVEL, ViewMode::Advanced),
		std::make_pair(AMF_H264_ASPECTRATIO, ViewMode::Expert),
		std::make_pair(AMF_H264_CODINGTYPE, ViewMode::Expert),
		std::make_pair(AMF_H264_MAXIMUMREFERENCEFRAMES, ViewMode::Expert),
		// ----------- Rate Control Section
		std::make_pair(AMF_H264_RATECONTROLMETHOD, ViewMode::Basic),
		std::make_pair(AMF_H264_PREPASS, ViewMode::Basic),
		//std::make_pair(AMF_H264_BITRATE_TARGET, ViewMode::Basic),
		//std::make_pair(AMF_H264_BITRATE_PEAK, ViewMode::Basic),
		//std::make_pair(AMF_H264_QP_IFRAME, ViewMode::Basic),
		//std::make_pair(AMF_H264_QP_PFRAME, ViewMode::Basic),
		//std::make_pair(AMF_H264_QP_BFRAME, ViewMode::Basic),
		//std::make_pair(AMF_H264_QP_MINIMUM, ViewMode::Advanced),
		//std::make_pair(AMF_H264_QP_MAXIMUM, ViewMode::Advanced),
		//std::make_pair(AMF_H264_FILLERDATA, ViewMode::Basic),
		std::make_pair(AMF_H264_FRAMESKIPPING, ViewMode::Advanced),
		std::make_pair(AMF_H264_VBAQ, ViewMode::Expert),
		std::make_pair(AMF_H264_ENFORCEHRD, ViewMode::Expert),
		// ----------- VBV Buffer
		std::make_pair(AMF_H264_VBVBUFFER, ViewMode::Advanced),
		//std::make_pair(AMF_H264_VBVBUFFER_STRICTNESS, ViewMode::Advanced),
		//std::make_pair(AMF_H264_VBVBUFFER_SIZE, ViewMode::Advanced),
		std::make_pair(AMF_H264_VBVBUFFER_FULLNESS, ViewMode::Expert),
		// ----------- Picture Control
		std::make_pair(AMF_H264_KEYFRAME_INTERVAL, ViewMode::Basic),
		std::make_pair(AMF_H264_IDR_PERIOD, ViewMode::Master),
		std::make_pair(AMF_H264_BFRAME_PATTERN, ViewMode::Advanced),
		std::make_pair(AMF_H264_BFRAME_DELTAQP, ViewMode::Advanced),
		std::make_pair(AMF_H264_BFRAME_REFERENCE, ViewMode::Advanced),
		std::make_pair(AMF_H264_BFRAME_REFERENCEDELTAQP, ViewMode::Advanced),
		std::make_pair(AMF_H264_DEBLOCKINGFILTER, ViewMode::Expert),
		std::make_pair(AMF_H264_MOTIONESTIMATION, ViewMode::Expert),
		// ----------- Intra-Refresh
		//std::make_pair("", ViewMode::Master),
		// ----------- System
		std::make_pair(AMF_H264_VIDEOAPI, ViewMode::Advanced),
		std::make_pair(AMF_H264_VIDEOADAPTER, ViewMode::Advanced),
		std::make_pair(AMF_H264_OPENCL, ViewMode::Expert),
		std::make_pair(AMF_H264_VIEW, ViewMode::Basic),
	};
	for (std::pair<const char*, ViewMode> kv : viewstuff) {
		bool vis = curView >= kv.second;
		obs_property_set_visible(obs_properties_get(props, kv.first), vis);
		if (!vis)
			obs_data_default_single(props, data, kv.first);
	}

	// Special Logic
	#pragma region B-Frames
	auto bframeProperty = obs_properties_get(props, AMF_H264_BFRAME_PATTERN);
	bool bframeSupported = obs_property_int_max(bframeProperty) > 0;
	bool bframeVisible = (curView >= ViewMode::Advanced) && bframeSupported;

	/// Pattern
	obs_property_set_visible(bframeProperty, (curView >= ViewMode::Advanced) && bframeSupported);
	if (!bframeVisible)
		obs_data_default_single(props, data, AMF_H264_BFRAME_PATTERN);
	bool lastUsingBFrames = obs_data_get_int(data, "last" vstr(AMF_H264_BFRAME_PATTERN)) != 0,
		usingBFrames = obs_data_get_int(data, AMF_H264_BFRAME_PATTERN) != 0;
	if (usingBFrames != lastUsingBFrames) {
		obs_data_set_int(data, "last" vstr(AMF_H264_BFRAME_PATTERN), obs_data_get_int(data, AMF_H264_BFRAME_PATTERN));
		result = true;
	}

	/// Reference
	bool bframeReferenceVisible = (curView >= ViewMode::Advanced) && bframeSupported && usingBFrames;
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BFRAME_REFERENCE), bframeReferenceVisible);
	if (!bframeReferenceVisible)
		obs_data_default_single(props, data, AMF_H264_BFRAME_REFERENCE);
	bool lastUsingBFrameReference = obs_data_get_int(data, "last" vstr(AMF_H264_BFRAME_REFERENCE)) != 0,
		usingBFrameReference = obs_data_get_int(data, AMF_H264_BFRAME_REFERENCE) == 1;
	if (usingBFrameReference != lastUsingBFrameReference) {
		obs_data_set_int(data, "last" vstr(AMF_H264_BFRAME_REFERENCE), obs_data_get_int(data, AMF_H264_BFRAME_REFERENCE));
		result = true;
	}

	/// QP Delta
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BFRAME_DELTAQP), bframeVisible && usingBFrames);
	if (!bframeVisible || !usingBFrames)
		obs_data_default_single(props, data, AMF_H264_BFRAME_DELTAQP);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BFRAME_REFERENCEDELTAQP), bframeVisible && usingBFrames && usingBFrameReference);
	if (!bframeVisible || !usingBFrames || !usingBFrameReference)
		obs_data_default_single(props, data, AMF_H264_BFRAME_REFERENCEDELTAQP);
	#pragma endregion B-Frames

	#pragma region Rate Control
	bool vis_rcm_bitrate_target = false,
		vis_rcm_bitrate_peak = false,
		vis_rcm_qp = false,
		vis_rcm_qp_b = false,
		vis_rcm_fillerdata = false;

	RateControlMethod lastRCM = static_cast<RateControlMethod>(obs_data_get_int(data, "last" vstr(AMF_H264_RATECONTROLMETHOD))),
		curRCM = static_cast<RateControlMethod>(obs_data_get_int(data, AMF_H264_RATECONTROLMETHOD));
	if (lastRCM != curRCM) {
		obs_data_set_int(data, "last" vstr(AMF_H264_RATECONTROLMETHOD), static_cast<int32_t>(curRCM));
		result = true;
	}
	switch (curRCM) {
		case RateControlMethod::ConstantQP:
			vis_rcm_qp = true;
			vis_rcm_qp_b = bframeSupported && usingBFrames;
			break;
		case RateControlMethod::ConstantBitrate:
			vis_rcm_bitrate_target = true;
			vis_rcm_fillerdata = true;
			break;
		case RateControlMethod::PeakConstrainedVariableBitrate:
			vis_rcm_bitrate_target = true;
			vis_rcm_bitrate_peak = true;
			break;
		case RateControlMethod::LatencyConstrainedVariableBitrate:
			vis_rcm_bitrate_target = true;
			vis_rcm_bitrate_peak = true;
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
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_MINIMUM), (curView >= ViewMode::Advanced) && !vis_rcm_qp);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_MAXIMUM), (curView >= ViewMode::Advanced) && !vis_rcm_qp);
	if (!(curView >= ViewMode::Advanced) || vis_rcm_qp) {
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
	bool vbvBufferVisible = (curView >= ViewMode::Advanced);

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
	obs_property_set_visible(obs_properties_get(props, AMF_H264_VIDEOADAPTER), (curView >= ViewMode::Advanced) && isnothostmode);
	if (!(curView >= ViewMode::Advanced) || !isnothostmode)
		obs_data_default_single(props, data, AMF_H264_VIDEOADAPTER);
	/// OpenCL
	obs_property_set_visible(obs_properties_get(props, AMF_H264_OPENCL), (curView >= ViewMode::Advanced) && isnothostmode);
	if (!(curView >= ViewMode::Advanced) || !isnothostmode)
		obs_data_default_single(props, data, AMF_H264_OPENCL);

	#pragma endregion Special Logic
	#pragma endregion View Mode

	return result;
}

bool Plugin::Interface::H264Interface::update(void *data, obs_data_t *settings) {
	return static_cast<Plugin::Interface::H264Interface*>(data)->update(settings);
}

void Plugin::Interface::H264Interface::get_video_info(void *data, struct video_scale_info *info) {
	return static_cast<Plugin::Interface::H264Interface*>(data)->get_video_info(info);
}

bool Plugin::Interface::H264Interface::get_extra_data(void *data, uint8_t** extra_data, size_t* size) {
	return static_cast<Plugin::Interface::H264Interface*>(data)->get_extra_data(extra_data, size);
}

//////////////////////////////////////////////////////////////////////////
// Module Code
//////////////////////////////////////////////////////////////////////////
Plugin::Interface::H264Interface::H264Interface(obs_data_t* data, obs_encoder_t* encoder) {
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Initializing...");

	// OBS Settings
	uint32_t obsWidth = obs_encoder_get_width(encoder);
	uint32_t obsHeight = obs_encoder_get_height(encoder);
	video_t *obsVideoInfo = obs_encoder_video(encoder);
	const struct video_output_info *voi = video_output_get_info(obsVideoInfo);
	uint32_t obsFPSnum = voi->fps_num;
	uint32_t obsFPSden = voi->fps_den;

	//////////////////////////////////////////////////////////////////////////
	/// Initialize Encoder
	bool debug = obs_data_get_bool(data, AMF_H264_DEBUG);
	Plugin::AMD::AMF::Instance()->EnableDebugTrace(debug);

	ColorFormat colorFormat = ColorFormat::NV12;
	switch (voi->format) {
		case VIDEO_FORMAT_NV12:
			colorFormat = ColorFormat::NV12;
			break;
		case VIDEO_FORMAT_I420:
			colorFormat = ColorFormat::I420;
			break;
		case VIDEO_FORMAT_YUY2:
			colorFormat = ColorFormat::YUY2;
			break;
		case VIDEO_FORMAT_RGBA:
			colorFormat = ColorFormat::RGBA;
			break;
		case VIDEO_FORMAT_BGRA:
			colorFormat = ColorFormat::BGRA;
			break;
		case VIDEO_FORMAT_Y800:
			colorFormat = ColorFormat::GRAY;
			break;
	}
	ColorSpace colorSpace = ColorSpace::BT601;
	switch (voi->colorspace) {
		case VIDEO_CS_601:
			colorSpace = ColorSpace::BT601;
			break;
		case VIDEO_CS_DEFAULT:
		case VIDEO_CS_709:
			colorSpace = ColorSpace::BT709;
			break;
	}

	auto api = API::GetAPI(obs_data_get_string(data, AMF_H264_VIDEOAPI));
	union {
		int64_t v;
		uint32_t id[2];
	} adapterid = { obs_data_get_int(data, AMF_H264_VIDEOADAPTER) };
	auto adapter = api->GetAdapterById(adapterid.id[0], adapterid.id[1]);

	m_VideoEncoder = new EncoderH264(api, adapter, !!obs_data_get_int(data, AMF_H264_OPENCL),
		colorFormat, colorSpace, voi->range == VIDEO_RANGE_FULL);

	/// Static Properties
	m_VideoEncoder->SetUsage(static_cast<Usage>(obs_data_get_int(data, AMF_H264_USAGE)));
	m_VideoEncoder->SetQualityPreset(static_cast<QualityPreset>(obs_data_get_int(data, AMF_H264_QUALITY_PRESET)));

	/// Frame
	m_VideoEncoder->SetResolution(std::make_pair(obsWidth, obsHeight));
	m_VideoEncoder->SetFrameRate(std::make_pair(obsFPSnum, obsFPSden));

	/// Profile & Level
	m_VideoEncoder->SetProfile(static_cast<Profile>(obs_data_get_int(data, AMF_H264_PROFILE)));
	m_VideoEncoder->SetProfileLevel(static_cast<ProfileLevel>(obs_data_get_int(data, AMF_H264_PROFILELEVEL)));

	try {
		m_VideoEncoder->SetCodingType(static_cast<CodingType>(obs_data_get_int(data, AMF_H264_CODINGTYPE)));
	} catch (...) {}
	try {
		m_VideoEncoder->SetMaximumReferenceFrames(obs_data_get_int(data, AMF_H264_MAXIMUMREFERENCEFRAMES));
	} catch (...) {}

	// OBS - Enforce Streaming Service Restrictions
	#pragma region OBS - Enforce Streaming Service Restrictions
	{
		// Profile
		const char* p_str = obs_data_get_string(data, "profile");
		if (strcmp(p_str, "") != 0) {
			if (strcmp(p_str, "constrained_baseline")) {
				m_VideoEncoder->SetProfile(Profile::ConstrainedBaseline);
			} else if (strcmp(p_str, "baseline")) {
				m_VideoEncoder->SetProfile(Profile::Baseline);
			} else if (strcmp(p_str, "main")) {
				m_VideoEncoder->SetProfile(Profile::Main);
			} else if (strcmp(p_str, "constrained_high")) {
				m_VideoEncoder->SetProfile(Profile::ConstrainedHigh);
			} else if (strcmp(p_str, "high")) {
				m_VideoEncoder->SetProfile(Profile::High);
			}
		} else {
			switch (m_VideoEncoder->GetProfile()) {
				case Profile::ConstrainedBaseline:
					obs_data_set_string(data, "profile", "constrained_baseline");
					break;
				case Profile::Baseline:
					obs_data_set_string(data, "profile", "baseline");
					break;
				case Profile::Main:
					obs_data_set_string(data, "profile", "main");
					break;
				case Profile::ConstrainedHigh:
					obs_data_set_string(data, "profile", "constrained_high");
					break;
				case Profile::High:
					obs_data_set_string(data, "profile", "high");
					break;
			}
		}

		// Preset
		const char* preset = obs_data_get_string(data, "preset");
		if (strcmp(preset, "") != 0) {
			if (strcmp(preset, "speed") == 0) {
				m_VideoEncoder->SetQualityPreset(QualityPreset::Speed);
			} else if (strcmp(preset, "balanced") == 0) {
				m_VideoEncoder->SetQualityPreset(QualityPreset::Balanced);
			} else if (strcmp(preset, "quality") == 0) {
				m_VideoEncoder->SetQualityPreset(QualityPreset::Quality);
			}
			obs_data_set_int(data, AMF_H264_QUALITY_PRESET, (int32_t)m_VideoEncoder->GetQualityPreset());
		} else {
			switch (m_VideoEncoder->GetQualityPreset()) {
				case QualityPreset::Speed:
					obs_data_set_string(data, "preset", "speed");
					break;
				case QualityPreset::Balanced:
					obs_data_set_string(data, "preset", "balanced");
					break;
				case QualityPreset::Quality:
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

	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Complete.");
}

Plugin::Interface::H264Interface::~H264Interface() {
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Finalizing...");
	if (m_VideoEncoder) {
		m_VideoEncoder->Stop();
		delete m_VideoEncoder;
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Complete.");
}

bool Plugin::Interface::H264Interface::update(obs_data_t* data) {
	Usage myUsage = static_cast<Usage>(obs_data_get_int(data, AMF_H264_USAGE));

	if (myUsage == Usage::Transcoding) {
		// Rate Control
		m_VideoEncoder->SetRateControlMethod(static_cast<RateControlMethod>(obs_data_get_int(data, AMF_H264_RATECONTROLMETHOD)));
		m_VideoEncoder->SetPrePassMode(static_cast<PrePassMode>(obs_data_get_int(data, AMF_H264_PREPASS)));
		m_VideoEncoder->SetVariableBitrateAverageQualityEnabled(!!obs_data_get_int(data, AMF_H264_VBAQ));
		m_VideoEncoder->SetFrameSkippingEnabled(!!obs_data_get_int(data, AMF_H264_FRAMESKIPPING));
		m_VideoEncoder->SetEnforceHRDEnabled(!!obs_data_get_int(data, AMF_H264_ENFORCEHRD));
		m_VideoEncoder->SetFillerDataEnabled(!!obs_data_get_int(data, AMF_H264_FILLERDATA));

		m_VideoEncoder->SetQPMinimum(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_MINIMUM)));
		m_VideoEncoder->SetQPMaximum(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_MAXIMUM)));
		m_VideoEncoder->SetTargetBitrate(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_BITRATE_TARGET) * 1000));
		m_VideoEncoder->SetPeakBitrate(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_BITRATE_PEAK) * 1000));
		m_VideoEncoder->SetIFrameQP(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_IFRAME)));
		m_VideoEncoder->SetPFrameQP(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_PFRAME)));
		try { m_VideoEncoder->SetBFrameQP(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_BFRAME))); } catch (...) {}

		m_VideoEncoder->SetVBVBufferSize(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_VBVBUFFER_SIZE) * 1000));
		m_VideoEncoder->SetVBVBufferInitialFullness((float)obs_data_get_double(data, AMF_H264_VBVBUFFER_FULLNESS) / 100.0f);
		//if (obs_data_get_int(data, AMF_H264_VBVBUFFER) == 0)
		//	m_VideoEncoder->SetVBVBufferAutomatic(obs_data_get_double(data, AMF_H264_VBVBUFFER_STRICTNESS) / 100.0);

		// Picture Control
		double_t framerate = (double_t)m_VideoEncoder->GetFrameRate().first / (double_t)m_VideoEncoder->GetFrameRate().second;
		m_VideoEncoder->SetIDRPeriod(max(static_cast<uint32_t>(obs_data_get_double(data, AMF_H264_KEYFRAME_INTERVAL) * framerate), 1));
		if (static_cast<ViewMode>(obs_data_get_int(data, AMF_H264_VIEW)) == ViewMode::Master) {
			uint64_t idrperiod = static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_IDR_PERIOD));
			if (idrperiod != 0)
				m_VideoEncoder->SetIDRPeriod(idrperiod);
		}
		m_VideoEncoder->SetDeblockingFilterEnabled(!!obs_data_get_int(data, AMF_H264_DEBLOCKINGFILTER));

		#pragma region B-Frames
		if (m_VideoEncoder->CapsBFramePattern() > 0) {
			try {
				m_VideoEncoder->SetBFramePattern(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_BFRAME_PATTERN)));
				if (obs_data_get_int(data, AMF_H264_BFRAME_PATTERN) != 0)
					m_VideoEncoder->SetBFrameDeltaQP(static_cast<int8_t>(obs_data_get_int(data, AMF_H264_BFRAME_DELTAQP)));
				m_VideoEncoder->SetBFrameReferenceEnabled(!!obs_data_get_int(data, AMF_H264_BFRAME_REFERENCE));
				if (!!obs_data_get_int(data, AMF_H264_BFRAME_REFERENCE))
					m_VideoEncoder->SetBFrameReferenceDeltaQP(static_cast<int8_t>(obs_data_get_int(data, AMF_H264_BFRAME_REFERENCEDELTAQP)));
			} catch (...) {}
		}
		#pragma endregion B-Frames

		// Motion Estimation
		m_VideoEncoder->SetMotionEstimationHalfPixelEnabled(!!(obs_data_get_int(data, AMF_H264_MOTIONESTIMATION) & 1));
		m_VideoEncoder->SetMotionEstimationQuarterPixelEnabled(!!(obs_data_get_int(data, AMF_H264_MOTIONESTIMATION) & 2));
	} else if (m_VideoEncoder->GetUsage() == Usage::UltraLowLatency) {
		m_VideoEncoder->SetQPMinimum(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_MINIMUM)));
		m_VideoEncoder->SetQPMaximum(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_MAXIMUM)));
		m_VideoEncoder->SetTargetBitrate(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_BITRATE_TARGET) * 1000));

		m_VideoEncoder->SetIFrameQP(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_IFRAME)));
		m_VideoEncoder->SetPFrameQP(static_cast<uint8_t>(obs_data_get_int(data, AMF_H264_QP_PFRAME)));

		if (obs_data_get_int(data, AMF_H264_VBVBUFFER) == 0) {
			m_VideoEncoder->SetVBVBufferSize(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_BITRATE_TARGET) * 1000));
		} else {
			m_VideoEncoder->SetVBVBufferSize(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_VBVBUFFER_SIZE) * 1000));
		}
	}

	//#pragma region Experimental
	//try { m_VideoEncoder->SetWaitForTaskEnabled(!!obs_data_get_int(data, AMF_H264_WAITFORTASK)); } catch (...) {}
	//if (m_VideoEncoder->GetUsage() == H264Usage::Transcoding || m_VideoEncoder->GetUsage() == H264Usage::Webcam) {
	//	try { m_VideoEncoder->SetGOPSize(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_GOPSIZE))); } catch (...) {}
	//}
	//try { m_VideoEncoder->SetGOPAlignmentEnabled(!!obs_data_get_int(data, AMF_H264_GOPALIGNMENT)); } catch (...) {}
	//try { m_VideoEncoder->SetIntraRefreshNumberOfStripes(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_INTRAREFRESH_NUMBEROFSTRIPES))); } catch (...) {}
	//try { m_VideoEncoder->SetIntraRefreshMacroblocksPerSlot(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_INTRAREFRESH_MACROBLOCKSPERSLOT))); } catch (...) {}
	//try { m_VideoEncoder->SetSlicesPerFrame(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_SLICESPERFRAME))); } catch (...) {}
	//try { m_VideoEncoder->SetSliceMode(static_cast<H264SliceMode>(obs_data_get_int(data, AMF_H264_SLICEMODE))); } catch (...) {}
	//try { m_VideoEncoder->SetMaximumSliceSize(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_MAXIMUMSLICESIZE))); } catch (...) {}
	//try { m_VideoEncoder->SetSliceControlMode(static_cast<H264SliceControlMode>(obs_data_get_int(data, AMF_H264_SLICECONTROLMODE))); } catch (...) {}
	//try { m_VideoEncoder->SetSliceControlSize(static_cast<uint32_t>(obs_data_get_int(data, AMF_H264_SLICECONTROLSIZE))); } catch (...) {}
	//#pragma endregion Experimental

	#pragma region OBS Enforce Streaming Service Settings
	{
		// Rate Control Method
		const char* t_str = obs_data_get_string(data, "rate_control");
		if (strcmp(t_str, "") != 0) {
			if (strcmp(t_str, "CBR") == 0) {
				m_VideoEncoder->SetRateControlMethod(RateControlMethod::ConstantBitrate);
				m_VideoEncoder->SetFillerDataEnabled(true);
			} else if (strcmp(t_str, "VBR") == 0) {
				m_VideoEncoder->SetRateControlMethod(RateControlMethod::PeakConstrainedVariableBitrate);
			} else if (strcmp(t_str, "VBR_LAT") == 0) {
				m_VideoEncoder->SetRateControlMethod(RateControlMethod::LatencyConstrainedVariableBitrate);
			} else if (strcmp(t_str, "CQP") == 0) {
				m_VideoEncoder->SetRateControlMethod(RateControlMethod::ConstantQP);
			}

			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, (int32_t)m_VideoEncoder->GetRateControlMethod());
		} else {
			if (m_VideoEncoder->GetUsage() != Usage::UltraLowLatency)
				switch (m_VideoEncoder->GetRateControlMethod()) {
					case RateControlMethod::ConstantBitrate:
						obs_data_set_string(data, "rate_control", "CBR");
						break;
					case RateControlMethod::PeakConstrainedVariableBitrate:
						obs_data_set_string(data, "rate_control", "VBR");
						break;
					case RateControlMethod::LatencyConstrainedVariableBitrate:
						obs_data_set_string(data, "rate_control", "VBR_LAT");
						break;
					case RateControlMethod::ConstantQP:
						obs_data_set_string(data, "rate_control", "CQP");
						break;
				}
		}

		// Bitrate
		uint64_t bitrateOvr = obs_data_get_int(data, "bitrate") * 1000;
		if (bitrateOvr != -1) {
			if (m_VideoEncoder->GetTargetBitrate() > bitrateOvr)
				m_VideoEncoder->SetTargetBitrate(static_cast<uint32_t>(bitrateOvr));

			if (m_VideoEncoder->GetUsage() != Usage::UltraLowLatency)
				if (m_VideoEncoder->GetPeakBitrate() > bitrateOvr)
					m_VideoEncoder->SetPeakBitrate(static_cast<uint32_t>(bitrateOvr));

			obs_data_set_int(data, "bitrate", m_VideoEncoder->GetTargetBitrate() / 1000);

			obs_data_set_int(data, AMF_H264_BITRATE_TARGET, m_VideoEncoder->GetTargetBitrate() / 1000);
			if (m_VideoEncoder->GetUsage() != Usage::UltraLowLatency)
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

	if (m_VideoEncoder->IsStarted()) {
		AMF_LOG_INFO("-- Encoder Parameters --");

		AMF_LOG_INFO("Initialization:");
		AMF_LOG_INFO("  Unqiue Id: %d", m_VideoEncoder->GetUniqueId());
		AMF_LOG_INFO("  Codec: AVC");
		AMF_LOG_INFO("  Video API: %s", m_VideoEncoder->GetVideoAPI()->GetName().c_str());
		AMF_LOG_INFO("  Video Adapter: %s", m_VideoEncoder->GetVideoAdapter().Name.c_str());
		AMF_LOG_INFO("  OpenCL: %s", m_VideoEncoder->IsOpenCLEnabled() ? "Enabled" : "Disabled");

		AMF_LOG_INFO("Frame:");
		AMF_LOG_INFO("  Format: %s %s %s ",
			Utility::ColorFormatToString(m_VideoEncoder->GetColorFormat()),
			Utility::ColorSpaceToString(m_VideoEncoder->GetColorSpace()),
			m_VideoEncoder->IsFullRangeColor() ? "Full" : "Partial");
		auto frres = m_VideoEncoder->GetResolution();
		AMF_LOG_INFO("  Resolution: %ldx%ld", frres.first, frres.second);
		auto frrate = m_VideoEncoder->GetFrameRate();
		AMF_LOG_INFO("  Rate: %ld/%ld (%f fps)", frrate.first, frrate.second, (float)frrate.first / (float)frrate.second);
		auto fraspect = m_VideoEncoder->GetAspectRatio();
		AMF_LOG_INFO("  Aspect Ratio: %d:%d", fraspect.first, fraspect.second);

		AMF_LOG_INFO("Static:");
		AMF_LOG_INFO("  Usage: %s",
			Utility::UsageToString(m_VideoEncoder->GetUsage()));
		AMF_LOG_INFO("  Quality Preset: %s",
			Utility::QualityPresetToString(m_VideoEncoder->GetQualityPreset()));
		auto plvl = static_cast<int32_t>(m_VideoEncoder->GetProfileLevel());
		AMF_LOG_INFO("  Profile: %s %d.%d",
			Utility::ProfileToString(m_VideoEncoder->GetProfile()), plvl / 10, plvl % 10);
		AMF_LOG_INFO("  Max. Reference Frames: %d Frames",
			m_VideoEncoder->GetMaximumReferenceFrames());
		AMF_LOG_INFO("  Coding Type: %s",
			Utility::CodingTypeToString(m_VideoEncoder->GetCodingType()));

		AMF_LOG_INFO("Dynamic:");
		AMF_LOG_INFO("  Control Method: %s",
			Utility::RateControlMethodToString(m_VideoEncoder->GetRateControlMethod()));
		AMF_LOG_INFO("  Pre-Pass: %s",
			Utility::PrePassModeToString(m_VideoEncoder->GetPrePassMode()));
		AMF_LOG_INFO("  QP Limits: %d - %d",
			m_VideoEncoder->GetQPMinimum(), m_VideoEncoder->GetQPMaximum());
		AMF_LOG_INFO("  Fixed QP:");
		AMF_LOG_INFO("    I-Frame: %d",
			m_VideoEncoder->GetIFrameQP());
		AMF_LOG_INFO("    P-Frame: %d",
			m_VideoEncoder->GetPFrameQP());
		try {
			AMF_LOG_INFO("    B-Frame: %d",
				m_VideoEncoder->GetBFrameQP());
		} catch (...) {}
		AMF_LOG_INFO("  Bitrate:");
		AMF_LOG_INFO("    Target: %ld bps",
			m_VideoEncoder->GetTargetBitrate());
		AMF_LOG_INFO("    Peak: %ld bps",
			m_VideoEncoder->GetPeakBitrate());
		AMF_LOG_INFO("  VBV Buffer:");
		AMF_LOG_INFO("    Size: %ld bps",
			m_VideoEncoder->GetVBVBufferSize());
		AMF_LOG_INFO("    Initial Fullness: %f %%",
			m_VideoEncoder->GetInitialVBVBufferFullness() * 100.0);
		AMF_LOG_INFO("  Flags:");
		AMF_LOG_INFO("    Frame Skipping: %s",
			m_VideoEncoder->IsFrameSkippingEnabled() ? "Enabled" : "Disabled");
		AMF_LOG_INFO("    Enforce HRD: %s",
			m_VideoEncoder->IsEnforceHRDEnabled() ? "Enabled" : "Disabled");
		AMF_LOG_INFO("    Filler Data: %s",
			m_VideoEncoder->IsFillerDataEnabled() ? "Enabled" : "Disabled");
		AMF_LOG_INFO("    VBAQ: %s",
			m_VideoEncoder->IsVariableBitrateAverageQualityEnabled() ? "Enabled" : "Disabled");
		AMF_LOG_INFO("    Deblocking Filter: %s",
			m_VideoEncoder->IsDeblockingFilterEnabled() ? "Enabled" : "Disabled");
		AMF_LOG_INFO("  IDR Period: %d Frames",
			m_VideoEncoder->GetIDRPeriod());
		AMF_LOG_INFO("  B-Frames:");
		try {
			AMF_LOG_INFO("    Pattern: %d",
				m_VideoEncoder->GetBFramePattern());
		} catch (...) {
			AMF_LOG_INFO("    Pattern: N/A");
		}
		try {
			AMF_LOG_INFO("    Delta QP: %d",
				m_VideoEncoder->GetBFrameDeltaQP());
		} catch (...) {
			AMF_LOG_INFO("    Delta QP: N/A");
		}
		try {
			AMF_LOG_INFO("    Reference: %s",
				m_VideoEncoder->IsBFrameReferenceEnabled() ? "Enabled" : "Disabled");
		} catch (...) {
			AMF_LOG_INFO("    Reference: N/A");
		}
		try {
			AMF_LOG_INFO("    Reference Delta QP: %d",
				m_VideoEncoder->GetBFrameReferenceDeltaQP());
		} catch (...) {
			AMF_LOG_INFO("    Reference Delta QP: N/A");
		}
		AMF_LOG_INFO("  Motion Estimation: %s %s",
			m_VideoEncoder->IsMotionEstimationQuarterPixelEnabled() ? "Quarter " : "",
			m_VideoEncoder->IsMotionEstimationHalfPixelEnabled() ? "Half" : "");

		AMF_LOG_INFO("-- Encoder Parameters --");


	//	// Verify
	//	m_VideoEncoder->LogProperties();
	//	if (static_cast<ViewMode>(obs_data_get_int(data, AMF_H264_VIEW)) >= ViewMode::Master)
	//		AMF_LOG_ERROR("View Mode 'Master' is active, avoid giving anything but basic support. Error is most likely caused by user settings themselves.");
	//}
	}


	return true;
}

bool Plugin::Interface::H264Interface::encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	if (!frame || !packet || !received_packet)
		return false;

	bool retVal = false;

	try {
		retVal = m_VideoEncoder->Encode(frame, packet, received_packet);
	} catch (std::exception e) {
		AMF_LOG_ERROR("Exception during encoding: %s", e.what());
	} catch (...) {
		AMF_LOG_ERROR("Unknown exception during encoding.");
	}

	return retVal;
}

void Plugin::Interface::H264Interface::get_video_info(struct video_scale_info* info) {
	m_VideoEncoder->GetVideoInfo(info);
}

bool Plugin::Interface::H264Interface::get_extra_data(uint8_t** extra_data, size_t* size) {
	return m_VideoEncoder->GetExtraData(extra_data, size);
}
