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

//////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////
#include "enc-h264-simple.h"
#include "misc-util.cpp"

#include <string>
#include <iostream>
#include <sstream>

#if (defined _WIN32) | (defined _WIN64)
#include <VersionHelpers.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////

// Simple Interface
#define AMF_H264SIMPLE_NAME							TEXT_AMF_H264SIMPLE("Name")
#define AMF_H264SIMPLE_PRESET						TEXT_AMF_H264SIMPLE("Preset")
#define AMF_H264SIMPLE_PRESET_RECORDING				TEXT_AMF_H264SIMPLE("Preset.Recording")
#define AMF_H264SIMPLE_PRESET_TWITCH				TEXT_AMF_H264SIMPLE("Preset.Twitch")
#define AMF_H264SIMPLE_PRESET_YOUTUBE				TEXT_AMF_H264SIMPLE("Preset.YouTube")
#define AMF_H264SIMPLE_PRESET_HIGHQUALTIY			TEXT_AMF_H264SIMPLE("Preset.HighQuality")
#define AMF_H264SIMPLE_PRESET_INDISTINGUISHABLE		TEXT_AMF_H264SIMPLE("Preset.Indistinguishable")
#define AMF_H264SIMPLE_PRESET_LOSSLESS				TEXT_AMF_H264SIMPLE("Preset.Lossless")
#define AMF_H264SIMPLE_KEYFRAME_INTERVAL			TEXT_AMF_H264SIMPLE("KeyframeInterval")
#define AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE		TEXT_AMF_H264SIMPLE("UseCustomBufferSize")
#define AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE			TEXT_AMF_H264SIMPLE("CustomBufferSize")
#define AMF_H264SIMPLE_CUSTOM_BUFFER_FULLNESS		TEXT_AMF_H264SIMPLE("CustomBufferFullness")
#define AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE			TEXT_AMF_H264SIMPLE("UseCustomGOPSize")
#define AMF_H264SIMPLE_ADVANCED_SHOW_PARAMETERS		TEXT_AMF_H264SIMPLE("ShowAdvancedParameters")
#define AMF_H264SIMPLE_EXPERT_SHOW_PARAMETERS		TEXT_AMF_H264SIMPLE("ShowExpertParameters")

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin;
using namespace Plugin::AMD;
using namespace Plugin::Interface;

enum Preset {
	Preset_Recording,
	Preset_Twitch,
	Preset_YouTube,
	Preset_HighQuality,
	Preset_Indistinguishable,
	Preset_Lossless,
};

void Plugin::Interface::H264SimpleInterface::encoder_register() {
	static obs_encoder_info* encoder_info = new obs_encoder_info();
	static const char* encoder_name = "amd_amf_h264_simple";
	static const char* encoder_codec = "h264";

	std::memset(encoder_info, 0, sizeof(obs_encoder_info));

	// Initialize Structure
	encoder_info->id = encoder_name;
	encoder_info->type = obs_encoder_type::OBS_ENCODER_VIDEO;
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

	obs_register_encoder(encoder_info);
}

const char* Plugin::Interface::H264SimpleInterface::get_name(void*) {
	return obs_module_text(AMF_H264SIMPLE_NAME);
}

void Plugin::Interface::H264SimpleInterface::get_defaults(obs_data_t *data) {
	// Main Properties
	/// Preset
	obs_data_set_default_int(data, AMF_H264SIMPLE_PRESET, -1);
	/// Keyframe Interval
	obs_data_set_default_int(data, AMF_H264SIMPLE_KEYFRAME_INTERVAL, 2);
	/// Quality Preset
	obs_data_set_default_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Balanced);
	/// Profile
	obs_data_set_default_int(data, AMF_H264_PROFILE, VCEProfile_Main);
	obs_data_set_default_int(data, AMF_H264_PROFILELEVEL, 0);
	/// Rate Control
	obs_data_set_default_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantBitrate);
	obs_data_set_default_int(data, AMF_H264_QP_MINIMUM, 0);
	obs_data_set_default_int(data, AMF_H264_QP_MAXIMUM, 51);
	/// Rate Control: CBR, VBR
	obs_data_set_default_int(data, AMF_H264_BITRATE_TARGET, 2500);
	obs_data_set_default_int(data, AMF_H264_BITRATE_PEAK, 2500);
	/// Rate Control: Constant QP
	obs_data_set_default_int(data, AMF_H264_QP_IFRAME, 22);
	obs_data_set_default_int(data, AMF_H264_QP_PFRAME, 22);
	obs_data_set_default_int(data, AMF_H264_QP_BFRAME, 22);
	/// VBV Buffer
	obs_data_set_default_bool(data, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE, false);
	obs_data_set_default_int(data, AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE, 2500);
	obs_data_set_default_double(data, AMF_H264SIMPLE_CUSTOM_BUFFER_FULLNESS, 100.0);
	/// Frame Skipping
	obs_data_set_default_bool(data, AMF_H264_FRAMESKIPPING, false);

	// Advanced Properties
	obs_data_set_default_bool(data, AMF_H264SIMPLE_ADVANCED_SHOW_PARAMETERS, false);
	obs_data_set_default_int(data, AMF_H264_BPICTURE_PATTERN, (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames ? 3 : 0));
	obs_data_set_default_int(data, AMF_H264_BPICTURE_REFERENCE, 0);
	obs_data_set_default_int(data, AMF_H264_QP_BPICTURE_DELTA, 0);
	obs_data_set_default_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, 0);
	obs_data_set_default_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, 0);
	obs_data_set_default_int(data, AMF_H264_DEBLOCKINGFILTER, 1);

	// Expert Properties
	obs_data_set_default_bool(data, AMF_H264SIMPLE_EXPERT_SHOW_PARAMETERS, false);
	obs_data_set_default_int(data, AMF_H264_MEMORYTYPE, VCEMemoryType_Host);
	obs_data_set_default_int(data, AMF_H264_COMPUTETYPE, VCEComputeType_None);
	obs_data_set_default_int(data, AMF_H264_SURFACEFORMAT, -1);
	/// GOP Size
	obs_data_set_default_bool(data, AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE, false);
	obs_data_set_default_int(data, AMF_H264_GOP_SIZE, 60);
	/// CABAC
	obs_data_set_default_int(data, AMF_H264_CABAC, -1);

	// Debug Mode
	obs_data_set_default_bool(data, AMF_H264_DEBUGTRACING, false);

	//////////////////////////////////////////////////////////////////////////
	// OBS - Enforce Streaming Service Restrictions
	//////////////////////////////////////////////////////////////////////////
	obs_data_set_default_int(data, "bitrate", -1);
	obs_data_set_default_int(data, "keyint_sec", -1);
	obs_data_set_default_string(data, "rate_control", "");
	obs_data_set_default_string(data, "profile", "");
	obs_data_set_int(data, "bitrate", -1);
	obs_data_set_int(data, "keyint_sec", -1);
	obs_data_set_string(data, "rate_control", "");
	obs_data_set_string(data, "profile", "");
}

obs_properties_t* Plugin::Interface::H264SimpleInterface::get_properties(void*) {
	obs_properties* props = obs_properties_create();
	obs_property_t* p;

	/// Preset
	p = obs_properties_add_list(props, AMF_H264SIMPLE_PRESET, obs_module_text(AMF_H264SIMPLE_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, "", -1);
	obs_property_list_add_int(p, obs_module_text(AMF_H264SIMPLE_PRESET_TWITCH), Preset_Twitch);
	obs_property_list_add_int(p, obs_module_text(AMF_H264SIMPLE_PRESET_YOUTUBE), Preset_YouTube);
	obs_property_list_add_int(p, obs_module_text(AMF_H264SIMPLE_PRESET_RECORDING), Preset_Recording);
	obs_property_list_add_int(p, obs_module_text(AMF_H264SIMPLE_PRESET_HIGHQUALTIY), Preset_HighQuality);
	obs_property_list_add_int(p, obs_module_text(AMF_H264SIMPLE_PRESET_INDISTINGUISHABLE), Preset_Indistinguishable);
	obs_property_list_add_int(p, obs_module_text(AMF_H264SIMPLE_PRESET_LOSSLESS), Preset_Lossless);
	obs_property_set_modified_callback(p, &modified_preset);

	{ // Basic Properties
		/// Key-Frame Interval
		obs_properties_add_int(props, AMF_H264SIMPLE_KEYFRAME_INTERVAL, obs_module_text(AMF_H264SIMPLE_KEYFRAME_INTERVAL), 1, 60, 1);

		/// Quality Preset
		p = obs_properties_add_list(props, AMF_H264_QUALITY_PRESET, obs_module_text(AMF_H264_QUALITY_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(p, obs_module_text(AMF_H264_QUALITY_PRESET_SPEED), Plugin::AMD::VCEQualityPreset::VCEQualityPreset_Speed);
		obs_property_list_add_int(p, obs_module_text(AMF_H264_QUALITY_PRESET_BALANCED), Plugin::AMD::VCEQualityPreset::VCEQualityPreset_Balanced);
		obs_property_list_add_int(p, obs_module_text(AMF_H264_QUALITY_PRESET_QUALITY), Plugin::AMD::VCEQualityPreset::VCEQualityPreset_Quality);

		/// Profile
		p = obs_properties_add_list(props, AMF_H264_PROFILE, obs_module_text(AMF_H264_PROFILE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(p, "Baseline", Plugin::AMD::VCEProfile::VCEProfile_Baseline);
		obs_property_list_add_int(p, "Main", Plugin::AMD::VCEProfile::VCEProfile_Main);
		obs_property_list_add_int(p, "High", Plugin::AMD::VCEProfile::VCEProfile_High);

		/// Profile Level
		p = obs_properties_add_list(props, AMF_H264_PROFILELEVEL, obs_module_text(AMF_H264_PROFILELEVEL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(p, AMF_UTIL_AUTOMATIC, VCEProfileLevel_Automatic);
		switch (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxProfileLevel) {
			case 62:
				obs_property_list_add_int(p, "6.2", VCEProfileLevel_62);
			case 61:
				obs_property_list_add_int(p, "6.1", VCEProfileLevel_61);
			case 60:
				obs_property_list_add_int(p, "6.0", VCEProfileLevel_60);
			case 52:
				obs_property_list_add_int(p, "5.2", VCEProfileLevel_52);
			case 51:
				obs_property_list_add_int(p, "5.1", VCEProfileLevel_51);
			case 50:
				obs_property_list_add_int(p, "5.0", VCEProfileLevel_50);
			case 42: // Some VCE 2.0 Cards.
				obs_property_list_add_int(p, "4.2", VCEProfileLevel_42);
			case 41: // Some APUs and VCE 1.0 Cards.
				obs_property_list_add_int(p, "4.1", VCEProfileLevel_41);
			case 40: // These should in theory be supported by all VCE 1.0 devices and APUs.
				obs_property_list_add_int(p, "4.0", VCEProfileLevel_40);
			case 32:
				obs_property_list_add_int(p, "3.2", VCEProfileLevel_32);
			case 31:
				obs_property_list_add_int(p, "3.1", VCEProfileLevel_31);
			case 30:
				obs_property_list_add_int(p, "3.0", VCEProfileLevel_30);
			case 22:
				obs_property_list_add_int(p, "2.2", VCEProfileLevel_22);
			case 21:
				obs_property_list_add_int(p, "2.1", VCEProfileLevel_21);
			case 20:
				obs_property_list_add_int(p, "2.0", VCEProfileLevel_20);
			case 13:
				obs_property_list_add_int(p, "1.3", VCEProfileLevel_13);
			case 12:
				obs_property_list_add_int(p, "1.2", VCEProfileLevel_12);
			case 11:
				obs_property_list_add_int(p, "1.1", VCEProfileLevel_11);
			case 10:
			default:
				obs_property_list_add_int(p, "1.0", VCEProfileLevel_10);
		}
	}

	{ // Rate Control Properties
	  /// Rate Control
		p = obs_properties_add_list(props, AMF_H264_RATECONTROLMETHOD, obs_module_text(AMF_H264_RATECONTROLMETHOD), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(p, obs_module_text(AMF_H264_RATECONTROLMETHOD_CQP), Plugin::AMD::VCERateControlMethod::VCERateControlMethod_ConstantQP);
		obs_property_list_add_int(p, obs_module_text(AMF_H264_RATECONTROLMETHOD_CBR), Plugin::AMD::VCERateControlMethod::VCERateControlMethod_ConstantBitrate);
		obs_property_list_add_int(p, obs_module_text(AMF_H264_RATECONTROLMETHOD_VBR), Plugin::AMD::VCERateControlMethod::VCERateControlMethod_VariableBitrate_PeakConstrained);
		obs_property_list_add_int(p, obs_module_text(AMF_H264_RATECONTROLMETHOD_VBR_LAT), Plugin::AMD::VCERateControlMethod::VCERateControlMethod_VariableBitrate_LatencyConstrained);
		obs_property_set_modified_callback(p, &modified_rate_control);

		/// Rate Control: CBR, VBR
		obs_properties_add_int(props, AMF_H264_BITRATE_TARGET, obs_module_text(AMF_H264_BITRATE_TARGET), 10, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / 1000, 1);
		obs_properties_add_int(props, AMF_H264_BITRATE_PEAK, obs_module_text(AMF_H264_BITRATE_PEAK), 10, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / 1000, 1);

		/// Rate Control: Latency Constrained Variable Bitrate
		obs_properties_add_int_slider(props, AMF_H264_QP_MINIMUM, obs_module_text(AMF_H264_QP_MINIMUM), 0, 51, 1);
		obs_properties_add_int_slider(props, AMF_H264_QP_MAXIMUM, obs_module_text(AMF_H264_QP_MAXIMUM), 0, 51, 1);

		/// Rate Control: Constrained QP
		obs_properties_add_int_slider(props, AMF_H264_QP_IFRAME, obs_module_text(AMF_H264_QP_IFRAME), 0, 51, 1);
		obs_properties_add_int_slider(props, AMF_H264_QP_PFRAME, obs_module_text(AMF_H264_QP_PFRAME), 0, 51, 1);
		obs_properties_add_int_slider(props, AMF_H264_QP_BFRAME, obs_module_text(AMF_H264_QP_BFRAME), 0, 51, 1);

		/// VBV Buffer
		p = obs_properties_add_bool(props, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE, obs_module_text(AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE));
		obs_properties_add_int(props, AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE, obs_module_text(AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE), 1, 100000, 1);
		obs_properties_add_float_slider(props, AMF_H264SIMPLE_CUSTOM_BUFFER_FULLNESS, obs_module_text(AMF_H264SIMPLE_CUSTOM_BUFFER_FULLNESS), 0, 100, 1.5625);
		obs_property_set_modified_callback(p, &modified_rate_control);

		/// Other Options
		p = obs_properties_add_list(props, AMF_H264_FILLERDATA, obs_module_text(AMF_H264_FILLERDATA), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
		p = obs_properties_add_list(props, AMF_H264_FRAMESKIPPING, obs_module_text(AMF_H264_FRAMESKIPPING), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	}

	{ // Advanced Properties
		p = obs_properties_add_bool(props, AMF_H264SIMPLE_ADVANCED_SHOW_PARAMETERS, obs_module_text(AMF_H264SIMPLE_ADVANCED_SHOW_PARAMETERS));
		obs_property_set_modified_callback(p, &modified_show_advanced);

		/// B-Pictures
		obs_properties_add_int_slider(props, AMF_H264_BPICTURE_PATTERN, obs_module_text(AMF_H264_BPICTURE_PATTERN), 0, 3, 1);
		p = obs_properties_add_list(props, AMF_H264_BPICTURE_REFERENCE, obs_module_text(AMF_H264_BPICTURE_REFERENCE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
		obs_properties_add_int_slider(props, AMF_H264_QP_BPICTURE_DELTA, obs_module_text(AMF_H264_QP_BPICTURE_DELTA), -10, 10, 1);
		obs_properties_add_int_slider(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, obs_module_text(AMF_H264_QP_REFERENCE_BPICTURE_DELTA), -10, 10, 1);

		/// De-Blocking Filter
		p = obs_properties_add_list(props, AMF_H264_DEBLOCKINGFILTER, obs_module_text(AMF_H264_DEBLOCKINGFILTER), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_DEFAULT), -1);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);

		/// HRD Restrictions
		p = obs_properties_add_list(props, AMF_H264_ENFORCEHRDCOMPATIBILITY, obs_module_text(AMF_H264_ENFORCEHRDCOMPATIBILITY), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_DEFAULT), -1);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	}

	{ // Expert Properties
		p = obs_properties_add_bool(props, AMF_H264SIMPLE_EXPERT_SHOW_PARAMETERS, obs_module_text(AMF_H264SIMPLE_EXPERT_SHOW_PARAMETERS));
		obs_property_set_modified_callback(p, &modified_show_expert);

		/// Memory Type
		p = obs_properties_add_list(props, AMF_H264_MEMORYTYPE, obs_module_text(AMF_H264_MEMORYTYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_AUTOMATIC), VCEMemoryType_Auto);
		obs_property_list_add_int(p, "Host", VCEMemoryType_Host);
		if (IsWindowsXPOrGreater()) {
			obs_property_list_add_int(p, "DirectX 9", VCEMemoryType_DirectX9);
			if (IsWindows8OrGreater()) {
				obs_property_list_add_int(p, "DirectX 11", VCEMemoryType_DirectX11);
			}
		}
		obs_property_list_add_int(p, "OpenGL", VCEMemoryType_OpenGL);

		/// Compute Type
		p = obs_properties_add_list(props, AMF_H264_COMPUTETYPE, obs_module_text(AMF_H264_COMPUTETYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), VCEComputeType_None);
		obs_property_list_add_int(p, "OpenCL", VCEComputeType_OpenCL);

		/// Surface Format
		p = obs_properties_add_list(props, AMF_H264_SURFACEFORMAT, obs_module_text(AMF_H264_SURFACEFORMAT), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_AUTOMATIC), -1);
		auto formats = Plugin::AMD::VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->input.formats;
		for (auto format = formats.begin(); format != formats.end(); format++) {
			switch (format->first) {
				case amf::AMF_SURFACE_NV12:
					obs_property_list_add_int(p, "NV12 (4:2:0)", VCESurfaceFormat_NV12);
					break;
				case amf::AMF_SURFACE_YUV420P:
					obs_property_list_add_int(p, "I420 (4:2:0)", VCESurfaceFormat_I420);
					break;
				case amf::AMF_SURFACE_YUY2:
					obs_property_list_add_int(p, "YUY2 (4:2:2)", VCESurfaceFormat_YUY2);
					break;
				case amf::AMF_SURFACE_BGRA:
					obs_property_list_add_int(p, "BGRA (Uncompressed)", VCESurfaceFormat_BGRA);
					break;
				case amf::AMF_SURFACE_RGBA:
					obs_property_list_add_int(p, "RGBA (Uncompressed)", VCESurfaceFormat_RGBA);
					break;
				case amf::AMF_SURFACE_GRAY8:
					obs_property_list_add_int(p, "Y800 (Gray)", VCESurfaceFormat_GRAY);
					break;
			}
		}

		/// GOP Size
		p = obs_properties_add_bool(props, AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE, obs_module_text(AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE));
		obs_property_set_modified_callback(p, &modified_show_expert);
		obs_properties_add_int(props, AMF_H264_GOP_SIZE, obs_module_text(AMF_H264_GOP_SIZE), 1, 1000, 1);

		/// CABAC
		p = obs_properties_add_list(props, AMF_H264_CABAC, obs_module_text(AMF_H264_CABAC), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_DEFAULT), -1);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
		obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	}

	// Debug Mode
	obs_properties_add_bool(props, AMF_H264_DEBUGTRACING, obs_module_text(AMF_H264_DEBUGTRACING));

	return props;
}

bool Plugin::Interface::H264SimpleInterface::modified_preset(obs_properties_t*, obs_property_t*, obs_data_t *data) {
	switch (obs_data_get_int(data, AMF_H264SIMPLE_PRESET)) {
		case Preset_Twitch: // Twitch
			#pragma region Preset: Twitch
			obs_data_set_autoselect_int(data, AMF_H264SIMPLE_KEYFRAME_INTERVAL, 2);
			obs_data_set_autoselect_int(data, AMF_H264_PROFILE, VCEProfile_Main);
			obs_data_set_autoselect_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_Automatic); // Automatic
			obs_data_set_autoselect_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantBitrate);
			obs_data_set_autoselect_int(data, AMF_H264_FILLERDATA, 1);
			break;
			#pragma endregion Preset: Twitch
		case Preset_YouTube:
			#pragma region Preset: YouTube
			obs_data_set_autoselect_int(data, AMF_H264SIMPLE_KEYFRAME_INTERVAL, 2);
			obs_data_set_autoselect_int(data, AMF_H264_PROFILE, VCEProfile_High);
			obs_data_set_autoselect_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_Automatic); // Automatic
			obs_data_set_autoselect_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantBitrate);
			obs_data_set_autoselect_int(data, AMF_H264_FILLERDATA, 1);
			break;
			#pragma endregion Preset: YouTube
		case Preset_Recording:
			#pragma region Preset: Recording
			obs_data_set_autoselect_int(data, AMF_H264SIMPLE_KEYFRAME_INTERVAL, 1);
			obs_data_set_autoselect_int(data, AMF_H264_PROFILE, VCEProfile_High);
			obs_data_set_autoselect_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_Automatic);
			obs_data_set_autoselect_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_VariableBitrate_LatencyConstrained);
			obs_data_set_autoselect_int(data, AMF_H264_QP_MINIMUM, 0);
			obs_data_set_autoselect_int(data, AMF_H264_QP_MAXIMUM, 51);
			break;
			#pragma endregion Preset: Recording
		case Preset_HighQuality:
			#pragma region Preset: High Quality
			obs_data_set_autoselect_int(data, AMF_H264SIMPLE_KEYFRAME_INTERVAL, 1);
			obs_data_set_autoselect_int(data, AMF_H264_PROFILE, VCEProfile_High);
			obs_data_set_autoselect_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_Automatic);
			obs_data_set_autoselect_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantQP);
			obs_data_set_autoselect_int(data, AMF_H264_QP_IFRAME, 16);
			obs_data_set_autoselect_int(data, AMF_H264_QP_PFRAME, 21);
			obs_data_set_autoselect_int(data, AMF_H264_QP_BFRAME, 26);
			break;
			#pragma endregion Preset: High Quality
		case Preset_Indistinguishable:
			#pragma region Preset: Indistinguishable
			obs_data_set_autoselect_int(data, AMF_H264SIMPLE_KEYFRAME_INTERVAL, 1);
			obs_data_set_autoselect_int(data, AMF_H264_PROFILE, VCEProfile_High);
			obs_data_set_autoselect_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_Automatic);
			obs_data_set_autoselect_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantQP);
			obs_data_set_autoselect_int(data, AMF_H264_QP_IFRAME, 11);
			obs_data_set_autoselect_int(data, AMF_H264_QP_PFRAME, 16);
			obs_data_set_autoselect_int(data, AMF_H264_QP_BFRAME, 21);
			break;
			#pragma endregion Preset: Indistinguishable
		case Preset_Lossless:
			#pragma region Preset: Lossless
			obs_data_set_autoselect_int(data, AMF_H264SIMPLE_KEYFRAME_INTERVAL, 1);
			obs_data_set_autoselect_int(data, AMF_H264_PROFILE, VCEProfile_High);
			obs_data_set_autoselect_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_Automatic);
			obs_data_set_autoselect_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantQP);
			obs_data_set_autoselect_int(data, AMF_H264_QP_IFRAME, 0);
			obs_data_set_autoselect_int(data, AMF_H264_QP_PFRAME, 0);
			obs_data_set_autoselect_int(data, AMF_H264_QP_BFRAME, 0);
			break;
			#pragma endregion Preset: Lossless
	}

	return false;
}

bool Plugin::Interface::H264SimpleInterface::modified_rate_control(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	{ // Rate Control Settings
		bool vis_bitrateTarget = false,
			vis_bitratePeak = false,
			vis_FrameQP = false,
			vis_MinMaxQP = false,
			vis_FillerData = false;

		obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_TARGET), vis_bitrateTarget);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_PEAK), vis_bitratePeak);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_IFRAME), vis_FrameQP);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_PFRAME), vis_FrameQP);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BFRAME), vis_FrameQP && VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_MINIMUM), vis_MinMaxQP);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_MAXIMUM), vis_MinMaxQP);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_FILLERDATA), vis_FillerData);
	}

	{ // Buffer Size
		bool vis = obs_data_get_bool(data, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE);

		obs_property_set_visible(obs_properties_get(props, AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE), vis);
		obs_property_set_visible(obs_properties_get(props, AMF_H264SIMPLE_CUSTOM_BUFFER_FULLNESS), vis);
	}

	return false;
}

bool Plugin::Interface::H264SimpleInterface::modified_show_advanced(obs_properties_t *props, obs_property_t*, obs_data_t *data) {
	bool vis = obs_data_get_bool(data, AMF_H264SIMPLE_ADVANCED_SHOW_PARAMETERS);
	bool vis2 = vis && VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames;

	// B-Pictures
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_PATTERN), vis2);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_REFERENCE), vis2);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BPICTURE_DELTA), vis2);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA), vis2);

	// Other
	obs_property_set_visible(obs_properties_get(props, AMF_H264_DEBLOCKINGFILTER), vis);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_ENFORCEHRDCOMPATIBILITY), vis);

	return false;
}

bool Plugin::Interface::H264SimpleInterface::modified_show_expert(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	bool vis = obs_data_get_bool(data, AMF_H264SIMPLE_EXPERT_SHOW_PARAMETERS);

	// Memory, Compute And Surface Format
	obs_property_set_visible(obs_properties_get(props, AMF_H264_MEMORYTYPE), vis);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_COMPUTETYPE), vis);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_SURFACEFORMAT), vis);

	// GOP Size
	obs_property_set_visible(obs_properties_get(props, AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE), vis);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_GOP_SIZE), vis && obs_data_get_bool(data, AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE));

	// CABAC
	obs_property_set_visible(obs_properties_get(props, AMF_H264_CABAC), vis);

	return false;
}

void* Plugin::Interface::H264SimpleInterface::create(obs_data_t* settings, obs_encoder_t* encoder) {
	try {
		return new Plugin::Interface::H264SimpleInterface(settings, encoder);
	} catch (...) {
		AMF_LOG_ERROR("Unable to create Encoder, see log for more information.");
		return NULL;
	}
}

#pragma warning( push )
#pragma warning( disable: 4702 )
void Plugin::Interface::H264SimpleInterface::destroy(void* data) {
	try {
		delete (static_cast<Plugin::Interface::H264SimpleInterface*>(data));
		data = nullptr;
	} catch (...) {
		AMF_LOG_ERROR("Unable to destroy Encoder, see log for more information.");
	}
}

bool Plugin::Interface::H264SimpleInterface::update(void *data, obs_data_t *settings) {
	try {
		return static_cast<Plugin::Interface::H264SimpleInterface*>(data)->update(settings);
	} catch (...) {
		AMF_LOG_ERROR("Unable to update Encoder, see log for more information.");
		return false;
	}
}
#pragma warning( pop )

bool Plugin::Interface::H264SimpleInterface::encode(void *data, struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	try {
		return static_cast<Plugin::Interface::H264SimpleInterface*>(data)->encode(frame, packet, received_packet);
	} catch (...) {
		AMF_LOG_ERROR("Unable to encode, see log for more information.");
		return false;
	}
}

void Plugin::Interface::H264SimpleInterface::get_video_info(void *data, struct video_scale_info *info) {
	try {
		static_cast<Plugin::Interface::H264SimpleInterface*>(data)->get_video_info(info);
	} catch (...) {
		AMF_LOG_ERROR("Unable to get video info, see log for more information.");
	}
}

bool Plugin::Interface::H264SimpleInterface::get_extra_data(void *data, uint8_t** extra_data, size_t* size) {
	try {
		return static_cast<Plugin::Interface::H264SimpleInterface*>(data)->get_extra_data(extra_data, size);
	} catch (...) {
		AMF_LOG_ERROR("Unable to get extra data, see log for more information.");
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
// Module Code
//////////////////////////////////////////////////////////////////////////
Plugin::Interface::H264SimpleInterface::H264SimpleInterface(obs_data_t* settings, obs_encoder_t* encoder) {
	int32_t width, height, fpsNum, fpsDen;

	AMF_LOG_INFO("<AMFEncoder::H264SimpleInterface::H264SimpleInterface> Initializing...");

	// OBS Settings
	width = obs_encoder_get_width(encoder);
	height = obs_encoder_get_height(encoder);
	video_t *video = obs_encoder_video(encoder);
	const struct video_output_info *voi = video_output_get_info(video);
	fpsNum = voi->fps_num;
	fpsDen = voi->fps_den;

	// AMF Setup
	Plugin::AMD::AMF::GetInstance()->EnableDebugTrace(obs_data_get_bool(settings, AMF_H264_DEBUGTRACING));

	// Set up Encoder
	VCESurfaceFormat surfFormat = (VCESurfaceFormat)obs_data_get_int(settings, AMF_H264_SURFACEFORMAT);
	if (surfFormat == -1) {
		switch (voi->format) {
			case VIDEO_FORMAT_NV12:
				surfFormat = VCESurfaceFormat_NV12;
				break;
			case VIDEO_FORMAT_I420:
				surfFormat = VCESurfaceFormat_I420;
				break;
			case VIDEO_FORMAT_RGBA:
				surfFormat = VCESurfaceFormat_RGBA;
				break;
			case VIDEO_FORMAT_BGRA:
				surfFormat = VCESurfaceFormat_BGRA;
				break;
			case VIDEO_FORMAT_Y800:
				surfFormat = VCESurfaceFormat_GRAY;
				break;
			default:
				surfFormat = VCESurfaceFormat_NV12;
				break;
		}
	}
	m_VideoEncoder = new Plugin::AMD::VCEEncoder(VCEEncoderType_AVC, surfFormat, (VCEMemoryType)obs_data_get_int(settings, AMF_H264_MEMORYTYPE), (VCEComputeType)obs_data_get_int(settings, AMF_H264_COMPUTETYPE));

	// Static Parameters
	/// Usage - Always Transcoding
	m_VideoEncoder->SetUsage(VCEUsage_Transcoding);

	/// Quality Preset - User Defined
	m_VideoEncoder->SetQualityPreset((VCEQualityPreset)obs_data_get_int(settings, AMF_H264_QUALITY_PRESET));

	/// Frame Size, Frame Rate - Taken from OBS
	m_VideoEncoder->SetFrameSize(width, height);
	m_VideoEncoder->SetFrameRate(fpsNum, fpsDen);

	/// Profile, Profile Level - User Defined
	m_VideoEncoder->SetProfile((VCEProfile)obs_data_get_int(settings, AMF_H264_PROFILE));
	VCEProfileLevel level = (VCEProfileLevel)obs_data_get_int(settings, AMF_H264_PROFILELEVEL);
	if (level != VCEProfileLevel_Automatic) {
		VCEProfileLevel minLevel = Plugin::Utility::GetMinimumProfileLevel(std::pair<uint32_t, uint32_t>(width, height), std::pair<uint32_t, uint32_t>(fpsNum, fpsDen));
		if (level < minLevel) {
			AMF_LOG_WARNING("[WARNING] Profile Level %d.%d is too low for %dx%d at %d/%d fps, set to %d.%d.", ((int32_t)level) / 10, ((int32_t)level % 10), width, height, fpsNum, fpsDen, ((int32_t)minLevel / 10), ((int32_t)minLevel % 10));
			level = minLevel;
			obs_data_set_int(settings, AMF_H264_PROFILELEVEL, level);
		}
	}
	m_VideoEncoder->SetProfileLevel(level);

	// Dynamic Parameters
	this->update(settings);

	/// IDR Period - User Defined (Full Second Granularity)
	m_VideoEncoder->SetIDRPeriod((uint32_t)((double_t)obs_data_get_int(settings, AMF_H264SIMPLE_KEYFRAME_INTERVAL) * ((double_t)fpsNum / (double_t)fpsDen)));

	/// Encoder Miscellaneous Parameters
	m_VideoEncoder->SetScanType(VCEScanType_Progressive);

	/// Encoder Motion Estimation Parameters
	m_VideoEncoder->SetHalfPixelMotionEstimationEnabled(true);
	m_VideoEncoder->SetQuarterPixelMotionEstimationEnabled(true);

	//////////////////////////////////////////////////////////////////////////
	// OBS - Enforce Streaming Service Restrictions
	//////////////////////////////////////////////////////////////////////////
	{
		// Profile
		const char* p_str = obs_data_get_string(settings, "profile");
		if (strcmp(p_str, "") != 0) {
			if (strcmp(p_str, "baseline")) {
				m_VideoEncoder->SetProfile(VCEProfile_Baseline);
			} else if (strcmp(p_str, "main")) {
				m_VideoEncoder->SetProfile(VCEProfile_Main);
			} else if (strcmp(p_str, "high")) {
				m_VideoEncoder->SetProfile(VCEProfile_High);
			}
		} else {
			switch (m_VideoEncoder->GetProfile()) {
				case VCEProfile_Baseline:
					obs_data_set_string(settings, "profile", "baseline");
					break;
				case VCEProfile_Main:
					obs_data_set_string(settings, "profile", "main");
					break;
				case VCEProfile_High:
					obs_data_set_string(settings, "profile", "high");
					break;
			}
		}

		// Rate Control Method
		const char* t_str = obs_data_get_string(settings, "rate_control");
		if (strcmp(t_str, "") != 0) {
			if (strcmp(t_str, "CBR") == 0) {
				m_VideoEncoder->SetRateControlMethod(VCERateControlMethod_ConstantBitrate);
				m_VideoEncoder->SetFillerDataEnabled(true);
			} else if (strcmp(t_str, "VBR") == 0) {
				m_VideoEncoder->SetRateControlMethod(VCERateControlMethod_VariableBitrate_PeakConstrained);
			} else if (strcmp(t_str, "VBR_LAT") == 0) {
				m_VideoEncoder->SetRateControlMethod(VCERateControlMethod_VariableBitrate_LatencyConstrained);
			} else if (strcmp(t_str, "CQP") == 0) {
				m_VideoEncoder->SetRateControlMethod(VCERateControlMethod_ConstantQP);
			}
		} else {
			switch (m_VideoEncoder->GetRateControlMethod()) {
				case VCERateControlMethod_ConstantBitrate:
					obs_data_set_string(settings, "rate_control", "CBR");
					break;
				case VCERateControlMethod_VariableBitrate_PeakConstrained:
					obs_data_set_string(settings, "rate_control", "VBR");
					break;
				case VCERateControlMethod_VariableBitrate_LatencyConstrained:
					obs_data_set_string(settings, "rate_control", "VBR_LAT");
					break;
				case VCERateControlMethod_ConstantQP:
					obs_data_set_string(settings, "rate_control", "CQP");
					break;
			}
		}

		// Bitrate
		uint64_t bitrateOvr = obs_data_get_int(settings, "bitrate") * 1000;
		if (bitrateOvr != -1) {
			if (m_VideoEncoder->GetTargetBitrate() > bitrateOvr)
				m_VideoEncoder->SetTargetBitrate((uint32_t)bitrateOvr);

			if (m_VideoEncoder->GetPeakBitrate() > bitrateOvr)
				m_VideoEncoder->SetPeakBitrate((uint32_t)bitrateOvr);

			obs_data_set_int(settings, "bitrate", m_VideoEncoder->GetTargetBitrate() / 1000);
		} else {
			obs_data_set_int(settings, "bitrate", m_VideoEncoder->GetTargetBitrate() / 1000);
		}

		// IDR-Period (Keyframes)
		if (obs_data_get_int(settings, "keyint_sec") != -1) {
			m_VideoEncoder->SetIDRPeriod((uint32_t)(obs_data_get_int(settings, "keyint_sec") * ((double_t)fpsNum / (double_t)fpsDen)));
		} else {
			obs_data_set_int(settings, "keyint_sec", (uint64_t)(m_VideoEncoder->GetIDRPeriod() / ((double_t)fpsNum / (double_t)fpsDen)));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Verify
	//////////////////////////////////////////////////////////////////////////
	m_VideoEncoder->LogProperties();

	//////////////////////////////////////////////////////////////////////////
	// Start Encoding
	//////////////////////////////////////////////////////////////////////////
	m_VideoEncoder->Start();

	AMF_LOG_INFO("<AMFEncoder::H264SimpleInterface::H264SimpleInterface> Initialized.");
}

Plugin::Interface::H264SimpleInterface::~H264SimpleInterface() {
	AMF_LOG_INFO("<AMFEncoder::H264SimpleInterface::~H264SimpleInterface> Finalizing...");
	m_VideoEncoder->Stop();
	delete m_VideoEncoder;
	AMF_LOG_INFO("<AMFEncoder::H264SimpleInterface::~H264SimpleInterface> Complete.");
}

bool Plugin::Interface::H264SimpleInterface::update(obs_data_t* settings) {
	// Rate Control Method
	m_VideoEncoder->SetRateControlMethod((VCERateControlMethod)obs_data_get_int(settings, AMF_H264_RATECONTROLMETHOD));

	/// Parameters
	m_VideoEncoder->SetTargetBitrate((uint32_t)obs_data_get_int(settings, AMF_H264_BITRATE_TARGET) * 1000);
	m_VideoEncoder->SetPeakBitrate((uint32_t)obs_data_get_int(settings, AMF_H264_BITRATE_PEAK) * 1000);
	m_VideoEncoder->SetMinimumQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_MINIMUM));
	m_VideoEncoder->SetMaximumQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_MAXIMUM));
	m_VideoEncoder->SetIFrameQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_IFRAME));
	m_VideoEncoder->SetPFrameQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_PFRAME));
	try { m_VideoEncoder->SetBFrameQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_BFRAME)); } catch (...) {}
	try { m_VideoEncoder->SetBPictureDeltaQP((int8_t)obs_data_get_int(settings, AMF_H264_QP_BPICTURE_DELTA)); } catch (...) {}
	try { m_VideoEncoder->SetReferenceBPictureDeltaQP((int8_t)obs_data_get_int(settings, AMF_H264_QP_REFERENCE_BPICTURE_DELTA)); } catch (...) {}

	/// Buffer Size, Initial Fullness
	if (obs_data_get_bool(settings, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE)) {
		m_VideoEncoder->SetVBVBufferSize((uint32_t)obs_data_get_int(settings, AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE) * 1000);
		m_VideoEncoder->SetInitialVBVBufferFullness(obs_data_get_double(settings, AMF_H264SIMPLE_CUSTOM_BUFFER_FULLNESS) / 100.0);
	} else {
		m_VideoEncoder->SetVBVBufferSize(0);
		m_VideoEncoder->SetInitialVBVBufferFullness(1.0);
	}

	/// Filler Data and Frame Skipping
	m_VideoEncoder->SetFillerDataEnabled(!!obs_data_get_int(settings, AMF_H264_FILLERDATA));
	m_VideoEncoder->SetRateControlSkipFrameEnabled(!!obs_data_get_int(settings, AMF_H264_FRAMESKIPPING));

	/// Deblocking Filter - User Defined or Default
	if (obs_data_get_int(settings, AMF_H264_DEBLOCKINGFILTER) != -1)
		m_VideoEncoder->SetDeBlockingFilterEnabled(!!obs_data_get_int(settings, AMF_H264_DEBLOCKINGFILTER));

	/// Enforce HRD Restrictions
	if (obs_data_get_int(settings, AMF_H264_ENFORCEHRDCOMPATIBILITY) != -1)
		m_VideoEncoder->SetEnforceHRDRestrictionsEnabled(!!obs_data_get_int(settings, AMF_H264_ENFORCEHRDCOMPATIBILITY));

	/// B-Pictures
	try {
		m_VideoEncoder->SetBPicturePattern((VCEBPicturePattern)obs_data_get_int(settings, AMF_H264_BPICTURE_PATTERN));
	} catch (...) {}
	try {
		m_VideoEncoder->SetBPictureReferenceEnabled(!!obs_data_get_int(settings, AMF_H264_BPICTURE_REFERENCE));
	} catch (...) {}

	/// Expert Stuff
	if (obs_data_get_bool(settings, AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE))
		m_VideoEncoder->SetGOPSize((uint32_t)obs_data_get_int(settings, AMF_H264_GOP_SIZE));
	if (obs_data_get_int(settings, AMF_H264_CABAC) != -1)
		m_VideoEncoder->SetCABACEnabled(!!obs_data_get_bool(settings, AMF_H264_CABAC));

	return false;
}

bool Plugin::Interface::H264SimpleInterface::encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	bool retVal = true;
	retVal = m_VideoEncoder->SendInput(frame);
	m_VideoEncoder->GetOutput(packet, received_packet);
	return retVal;
}

void Plugin::Interface::H264SimpleInterface::get_video_info(struct video_scale_info* info) {
	m_VideoEncoder->GetVideoInfo(info);
}

bool Plugin::Interface::H264SimpleInterface::get_extra_data(uint8_t** extra_data, size_t* size) {
	return m_VideoEncoder->GetExtraData(extra_data, size);
}
