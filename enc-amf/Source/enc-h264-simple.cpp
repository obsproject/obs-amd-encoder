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

#if (defined _WIN32) | (defined _WIN64)
#include <VersionHelpers.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////

// Simple Interface
#define AMF_H264SIMPLE_NAME						TEXT_AMF_H264SIMPLE("Name")
#define AMF_H264SIMPLE_PRESET					TEXT_AMF_H264SIMPLE("Preset")
#define AMF_H264SIMPLE_PRESET_RECORDING			TEXT_AMF_H264SIMPLE("Preset.Recording")
#define AMF_H264SIMPLE_PRESET_TWITCH			TEXT_AMF_H264SIMPLE("Preset.Twitch")
#define AMF_H264SIMPLE_PRESET_YOUTUBE			TEXT_AMF_H264SIMPLE("Preset.YouTube")
#define AMF_H264SIMPLE_KEYFRAME_INTERVAL		TEXT_AMF_H264SIMPLE("KeyframeInterval")
#define AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE	TEXT_AMF_H264SIMPLE("UseCustomBufferSize")
#define AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE		TEXT_AMF_H264SIMPLE("CustomBufferSize")
#define AMF_H264SIMPLE_CUSTOM_BUFFER_FULLNESS	TEXT_AMF_H264SIMPLE("CustomBufferFullness")
#define AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE		TEXT_AMF_H264SIMPLE("UseCustomGOPSize")
#define AMF_H264SIMPLE_ADVANCED_SHOW_PARAMETERS	TEXT_AMF_H264SIMPLE("ShowAdvancedParameters")
#define AMF_H264SIMPLE_EXPERT_SHOW_PARAMETERS	TEXT_AMF_H264SIMPLE("ShowExpertParameters")

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin;
using namespace Plugin::AMD;
using namespace Plugin::Interface;

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
	obs_data_set_default_int(data, AMF_H264SIMPLE_PRESET, 0);
	/// Keyframe Interval
	obs_data_set_default_int(data, AMF_H264SIMPLE_KEYFRAME_INTERVAL, 2);
	/// Quality Preset
	obs_data_set_default_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Balanced);
	/// Profile
	obs_data_set_default_int(data, AMF_H264_PROFILE, VCEProfile_Main);
	obs_data_set_default_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_41);
	/// Rate Control
	obs_data_set_default_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantBitrate);
	obs_data_set_default_int(data, AMF_H264_QP_MINIMUM, 18);
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
	obs_data_set_default_int(data, AMF_H264_BPICTURE_PATTERN, 0);
	obs_data_set_default_int(data, AMF_H264_BPICTURE_REFERENCE, 0);
	obs_data_set_default_int(data, AMF_H264_QP_BPICTURE_DELTA, 0);
	obs_data_set_default_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, 0);
	obs_data_set_default_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, 0);
	obs_data_set_default_int(data, AMF_H264_DEBLOCKINGFILTER, 0);

	// Expert Properties
	obs_data_set_default_bool(data, AMF_H264SIMPLE_EXPERT_SHOW_PARAMETERS, false);
	obs_data_set_default_int(data, AMF_H264_MEMORYTYPE, VCEMemoryType_Host);
	obs_data_set_default_int(data, AMF_H264_COMPUTETYPE, VCEComputeType_None);
	obs_data_set_default_int(data, AMF_H264_SURFACEFORMAT, -1);
	/// GOP Size
	obs_data_set_default_bool(data, AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE, false);
	obs_data_set_default_int(data, AMF_H264_GOP_SIZE, 60);
	/// CABAC
	obs_data_set_default_bool(data, AMF_H264_CABAC, true);

	// Debug Mode
	obs_data_set_default_bool(data, AMF_H264_DEBUGTRACING, false);

	//////////////////////////////////////////////////////////////////////////
	// OBS - Enforce Streaming Service Restrictions
	//////////////////////////////////////////////////////////////////////////
	obs_data_set_default_int(data, "bitrate", -1);
	obs_data_set_default_int(data, "keyint_sec", -1);
	obs_data_set_default_string(data, "rate_control", "");
	obs_data_set_default_string(data, "profile", "");
}

obs_properties_t* Plugin::Interface::H264SimpleInterface::get_properties(void*) {
	obs_properties* props = obs_properties_create();
	obs_property_t* list;
	obs_property_t* p;

	// Main Properties
	/// Preset
	list = obs_properties_add_list(props, AMF_H264SIMPLE_PRESET, obs_module_text(AMF_H264SIMPLE_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, "", -1);
	obs_property_list_add_int(list, obs_module_text(AMF_H264SIMPLE_PRESET_RECORDING), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_H264SIMPLE_PRESET_TWITCH), 1);
	obs_property_list_add_int(list, obs_module_text(AMF_H264SIMPLE_PRESET_YOUTUBE), 2);
	obs_property_set_modified_callback(list, &ui_modified);

	/// Key-Frame Interval
	obs_properties_add_int(props, AMF_H264SIMPLE_KEYFRAME_INTERVAL, obs_module_text(AMF_H264SIMPLE_KEYFRAME_INTERVAL), 1, 60, 1);

	/// Quality Preset
	list = obs_properties_add_list(props, AMF_H264_QUALITY_PRESET, obs_module_text(AMF_H264_QUALITY_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_QUALITY_PRESET_SPEED), Plugin::AMD::VCEQualityPreset::VCEQualityPreset_Speed);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_QUALITY_PRESET_BALANCED), Plugin::AMD::VCEQualityPreset::VCEQualityPreset_Balanced);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_QUALITY_PRESET_QUALITY), Plugin::AMD::VCEQualityPreset::VCEQualityPreset_Quality);

	/// Profile
	list = obs_properties_add_list(props, AMF_H264_PROFILE, obs_module_text(AMF_H264_PROFILE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILE_BASELINE), Plugin::AMD::VCEProfile::VCEProfile_Baseline);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILE_MAIN), Plugin::AMD::VCEProfile::VCEProfile_Main);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILE_HIGH), Plugin::AMD::VCEProfile::VCEProfile_High);

	/// Profile Level
	list = obs_properties_add_list(props, AMF_H264_PROFILELEVEL, obs_module_text(AMF_H264_PROFILELEVEL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	switch (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxProfileLevel) {
		case 62:
			obs_property_list_add_int(list, "6.2", VCEProfileLevel_62);
		case 61:
			obs_property_list_add_int(list, "6.1", VCEProfileLevel_61);
		case 60:
			obs_property_list_add_int(list, "6.0", VCEProfileLevel_60);
		case 52:
			obs_property_list_add_int(list, "5.2", VCEProfileLevel_52);
		case 51:
			obs_property_list_add_int(list, "5.1", VCEProfileLevel_51);
		case 50:
			obs_property_list_add_int(list, "5.0", VCEProfileLevel_50);
		case 42: // Some VCE 2.0 Cards.
			obs_property_list_add_int(list, "4.2", VCEProfileLevel_42);
		case 41: // Some APUs and VCE 1.0 Cards.
			obs_property_list_add_int(list, "4.1", VCEProfileLevel_41);
		case 40: // These should in theory be supported by all VCE 1.0 devices and APUs.
			obs_property_list_add_int(list, "4.0", VCEProfileLevel_40);
		case 32:
			obs_property_list_add_int(list, "3.2", VCEProfileLevel_32);
		case 31:
			obs_property_list_add_int(list, "3.1", VCEProfileLevel_31);
		case 30:
			obs_property_list_add_int(list, "3.0", VCEProfileLevel_30);
		case 22:
			obs_property_list_add_int(list, "2.2", VCEProfileLevel_22);
		case 21:
			obs_property_list_add_int(list, "2.1", VCEProfileLevel_21);
		case 20:
			obs_property_list_add_int(list, "2.0", VCEProfileLevel_20);
		case 13:
			obs_property_list_add_int(list, "1.3", VCEProfileLevel_13);
		case 12:
			obs_property_list_add_int(list, "1.2", VCEProfileLevel_12);
		case 11:
			obs_property_list_add_int(list, "1.1", VCEProfileLevel_11);
		case 10:
		default:
			obs_property_list_add_int(list, "1.0", VCEProfileLevel_10);
	}

	/// Rate Control
	list = obs_properties_add_list(props, AMF_H264_RATECONTROLMETHOD, obs_module_text(AMF_H264_RATECONTROLMETHOD), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_RATECONTROLMETHOD_CQP), Plugin::AMD::VCERateControlMethod::VCERateControlMethod_ConstantQP);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_RATECONTROLMETHOD_CBR), Plugin::AMD::VCERateControlMethod::VCERateControlMethod_ConstantBitrate);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_RATECONTROLMETHOD_VBR), Plugin::AMD::VCERateControlMethod::VCERateControlMethod_VariableBitrate_PeakConstrained);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_RATECONTROLMETHOD_VBR_LAT), Plugin::AMD::VCERateControlMethod::VCERateControlMethod_VariableBitrate_LatencyConstrained);
	obs_property_set_modified_callback(list, &ui_modified);

	/// Rate Control: CBR, VBR
	obs_properties_add_int(props, AMF_H264_BITRATE_TARGET, obs_module_text(AMF_H264_BITRATE_TARGET), 10, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / 1000, 1);
	obs_properties_add_int(props, AMF_H264_BITRATE_PEAK, obs_module_text(AMF_H264_BITRATE_PEAK), 10, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / 1000, 1);

	/// Rate Control: Constrained QP
	obs_properties_add_int_slider(props, AMF_H264_QP_MINIMUM, obs_module_text(AMF_H264_QP_MINIMUM), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_H264_QP_MAXIMUM, obs_module_text(AMF_H264_QP_MAXIMUM), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_H264_QP_IFRAME, obs_module_text(AMF_H264_QP_IFRAME), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_H264_QP_PFRAME, obs_module_text(AMF_H264_QP_PFRAME), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_H264_QP_BFRAME, obs_module_text(AMF_H264_QP_BFRAME), 0, 51, 1);

	/// VBV Buffer
	p = obs_properties_add_bool(props, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE, obs_module_text(AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE));
	obs_properties_add_int(props, AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE, obs_module_text(AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE), 1, 100000, 1);
	obs_properties_add_float_slider(props, AMF_H264SIMPLE_CUSTOM_BUFFER_FULLNESS, obs_module_text(AMF_H264SIMPLE_CUSTOM_BUFFER_FULLNESS), 0, 100, 1.5625);
	obs_property_set_modified_callback(p, &ui_modified);

	/// Other Options
	list = obs_properties_add_list(props, AMF_H264_FILLERDATA, obs_module_text(AMF_H264_FILLERDATA), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	list = obs_properties_add_list(props, AMF_H264_FRAMESKIPPING, obs_module_text(AMF_H264_FRAMESKIPPING), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);

	{ // Advanced Properties
		p = obs_properties_add_bool(props, AMF_H264SIMPLE_ADVANCED_SHOW_PARAMETERS, obs_module_text(AMF_H264SIMPLE_ADVANCED_SHOW_PARAMETERS));
		obs_property_set_modified_callback(p, &ui_modified);

		/// B-Pictures
		obs_properties_add_int_slider(props, AMF_H264_BPICTURE_PATTERN, obs_module_text(AMF_H264_BPICTURE_PATTERN), 0, 3, 1);
		list = obs_properties_add_list(props, AMF_H264_BPICTURE_REFERENCE, obs_module_text(AMF_H264_BPICTURE_REFERENCE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
		obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
		obs_properties_add_int_slider(props, AMF_H264_QP_BPICTURE_DELTA, obs_module_text(AMF_H264_QP_BPICTURE_DELTA), -10, 10, 1);
		obs_properties_add_int_slider(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, obs_module_text(AMF_H264_QP_REFERENCE_BPICTURE_DELTA), -10, 10, 1);

		/// De-Blocking Filter
		list = obs_properties_add_list(props, AMF_H264_DEBLOCKINGFILTER, obs_module_text(AMF_H264_DEBLOCKINGFILTER), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
		obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
		obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);

		/// HRD Restrictions
		list = obs_properties_add_list(props, AMF_H264_ENFORCEHRDCOMPATIBILITY, obs_module_text(AMF_H264_ENFORCEHRDCOMPATIBILITY), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
		obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
		obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	}

	{ // Expert Properties
		p = obs_properties_add_bool(props, AMF_H264SIMPLE_EXPERT_SHOW_PARAMETERS, obs_module_text(AMF_H264SIMPLE_EXPERT_SHOW_PARAMETERS));
		obs_property_set_modified_callback(p, &ui_modified);

		/// Memory Type
		list = obs_properties_add_list(props, AMF_H264_MEMORYTYPE, obs_module_text(AMF_H264_MEMORYTYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(list, obs_module_text(AMF_UTIL_AUTOMATIC), VCEMemoryType_Auto);
		obs_property_list_add_int(list, "Host", VCEMemoryType_Auto);
		if (IsWindowsXPOrGreater()) {
			obs_property_list_add_int(list, "DirectX 9", VCEMemoryType_DirectX9);
			if (IsWindows8OrGreater()) {
				obs_property_list_add_int(list, "DirectX 11", VCEMemoryType_DirectX11);
			}
		}
		obs_property_list_add_int(list, "OpenGL", VCEMemoryType_OpenGL);

		/// Compute Type
		list = obs_properties_add_list(props, AMF_H264_COMPUTETYPE, obs_module_text(AMF_H264_COMPUTETYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), VCEComputeType_None);
		obs_property_list_add_int(list, "OpenCL", VCEComputeType_OpenCL);

		/// Surface Format
		list = obs_properties_add_list(props, AMF_H264_SURFACEFORMAT, obs_module_text(AMF_H264_SURFACEFORMAT), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(list, obs_module_text(AMF_UTIL_AUTOMATIC), -1);
		auto formats = Plugin::AMD::VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->input.formats;
		for (auto format = formats.begin(); format != formats.end(); format++) {
			switch (format->first) {
				case amf::AMF_SURFACE_NV12:
					obs_property_list_add_int(list, "NV12 (4:2:0)", VCESurfaceFormat_NV12);
					break;
				case amf::AMF_SURFACE_YUV420P:
					obs_property_list_add_int(list, "I420 (4:2:0)", VCESurfaceFormat_I420);
					break;
				case amf::AMF_SURFACE_YUY2:
					obs_property_list_add_int(list, "YUY2 (4:2:2)", VCESurfaceFormat_YUY2);
					break;
				case amf::AMF_SURFACE_BGRA:
					obs_property_list_add_int(list, "BGRA (Uncompressed)", VCESurfaceFormat_BGRA);
					break;
				case amf::AMF_SURFACE_RGBA:
					obs_property_list_add_int(list, "RGBA (Uncompressed)", VCESurfaceFormat_RGBA);
					break;
				case amf::AMF_SURFACE_GRAY8:
					obs_property_list_add_int(list, "Y800 (Gray)", VCESurfaceFormat_GRAY);
					break;
			}
		}

		/// GOP Size
		p = obs_properties_add_bool(props, AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE, obs_module_text(AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE));
		obs_property_set_modified_callback(p, &ui_modified);
		obs_properties_add_int(props, AMF_H264_GOP_SIZE, obs_module_text(AMF_H264_GOP_SIZE), 1, 1000, 1);

		/// CABAC
		list = obs_properties_add_list(props, AMF_H264_CABAC, obs_module_text(AMF_H264_CABAC), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
		obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
		obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
		obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);


		// Debug Mode
		obs_properties_add_bool(props, AMF_H264_DEBUGTRACING, obs_module_text(AMF_H264_DEBUGTRACING));
	}

	return props;
}

bool Plugin::Interface::H264SimpleInterface::ui_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	if (obs_data_get_int(data, AMF_H264SIMPLE_PRESET) != -1) {
		switch (obs_data_get_int(data, AMF_H264SIMPLE_PRESET)) {
			case 0: // Recording
				obs_data_set_int(data, AMF_H264SIMPLE_KEYFRAME_INTERVAL, 1);
				obs_data_set_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Quality);
				obs_data_set_int(data, AMF_H264_PROFILE, VCEProfile_High);
				obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxProfileLevel);
				obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_VariableBitrate_LatencyConstrained);
				obs_data_set_int(data, AMF_H264_QP_MINIMUM, 0);
				obs_data_set_int(data, AMF_H264_QP_MAXIMUM, 51);
				obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 10000);
				obs_data_set_int(data, AMF_H264_BITRATE_PEAK, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / 1000);
				obs_data_set_bool(data, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE, false);
				obs_data_set_int(data, AMF_H264_FILLERDATA, 0);
				obs_data_set_int(data, AMF_H264_FRAMESKIPPING, 0);

				// Advanced Properties
				obs_data_set_int(data, AMF_H264_BPICTURE_PATTERN, (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames ? 3 : 0));
				obs_data_set_int(data, AMF_H264_BPICTURE_REFERENCE, (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames ? 1 : 0));
				obs_data_set_int(data, AMF_H264_QP_BPICTURE_DELTA, 0);
				obs_data_set_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, 0);
				obs_data_set_int(data, AMF_H264_DEBLOCKINGFILTER, 1);
				obs_data_set_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, -1);

				// Expert Properties
				obs_data_set_bool(data, AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE, false);
				obs_data_set_int(data, AMF_H264_CABAC, -1);
				break;
			case 1: // Twitch
				obs_data_set_int(data, AMF_H264SIMPLE_KEYFRAME_INTERVAL, 2);
				obs_data_set_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Quality);
				obs_data_set_int(data, AMF_H264_PROFILE, VCEProfile_High);
				obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_41);
				obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantBitrate);
				obs_data_set_int(data, AMF_H264_QP_MINIMUM, 18);
				obs_data_set_int(data, AMF_H264_QP_MAXIMUM, 51);
				obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 3000);
				obs_data_set_bool(data, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE, false);
				obs_data_set_int(data, AMF_H264_FILLERDATA, 1);
				obs_data_set_int(data, AMF_H264_FRAMESKIPPING, 0);

				// Advanced Properties
				obs_data_set_int(data, AMF_H264_BPICTURE_PATTERN, 0);
				obs_data_set_int(data, AMF_H264_BPICTURE_REFERENCE, 0);
				obs_data_set_int(data, AMF_H264_QP_BPICTURE_DELTA, 0);
				obs_data_set_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, 0);
				obs_data_set_int(data, AMF_H264_DEBLOCKINGFILTER, 1);
				obs_data_set_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, -1);

				// Expert Properties
				obs_data_set_bool(data, AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE, false);
				obs_data_set_int(data, AMF_H264_CABAC, -1);
				break;
			case 2: // YouTube
				// Basic Properties
				obs_data_set_int(data, AMF_H264SIMPLE_KEYFRAME_INTERVAL, 2);
				obs_data_set_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Quality);
				obs_data_set_int(data, AMF_H264_PROFILE, VCEProfile_High);
				obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_41);
				obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantBitrate);
				obs_data_set_int(data, AMF_H264_QP_MINIMUM, 18);
				obs_data_set_int(data, AMF_H264_QP_MAXIMUM, 51);
				obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 6000);
				obs_data_set_bool(data, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE, false);
				obs_data_set_int(data, AMF_H264_FILLERDATA, 1);
				obs_data_set_int(data, AMF_H264_FRAMESKIPPING, 0);

				// Advanced Properties
				obs_data_set_int(data, AMF_H264_BPICTURE_PATTERN, 0);
				obs_data_set_int(data, AMF_H264_BPICTURE_REFERENCE, 0);
				obs_data_set_int(data, AMF_H264_QP_BPICTURE_DELTA, 0);
				obs_data_set_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, 0);
				obs_data_set_int(data, AMF_H264_DEBLOCKINGFILTER, 1);
				obs_data_set_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, -1);

				// Expert Properties
				obs_data_set_bool(data, AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE, false);
				obs_data_set_int(data, AMF_H264_CABAC, -1);
				break;
		}
		obs_data_set_int(data, AMF_H264SIMPLE_PRESET, -1);
	}

	{ // Rate Control Settings
		obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_PEAK), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_TARGET), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_IFRAME), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_PFRAME), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BFRAME), false);

		switch (obs_data_get_int(data, AMF_H264_RATECONTROLMETHOD)) {
			case Plugin::AMD::VCERateControlMethod::VCERateControlMethod_ConstantQP:
				obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_IFRAME), true);
				obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_PFRAME), true);
				if (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames) {
					obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BFRAME), true);
				}
				break;
			case Plugin::AMD::VCERateControlMethod::VCERateControlMethod_VariableBitrate_PeakConstrained:
			case Plugin::AMD::VCERateControlMethod::VCERateControlMethod_VariableBitrate_LatencyConstrained:
				obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_PEAK), true);
			case Plugin::AMD::VCERateControlMethod::VCERateControlMethod_ConstantBitrate:
				obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_TARGET), true);
				break;
		}
	}

	{ // Buffer Size
		obs_property_set_visible(obs_properties_get(props, AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264SIMPLE_CUSTOM_BUFFER_FULLNESS), false);
		if (obs_data_get_bool(data, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE)) {
			obs_property_set_visible(obs_properties_get(props, AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE), true);
			obs_property_set_visible(obs_properties_get(props, AMF_H264SIMPLE_CUSTOM_BUFFER_FULLNESS), true);
		}
	}

	{ // Advanced Properties
		obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_PATTERN), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_REFERENCE), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BPICTURE_DELTA), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_DEBLOCKINGFILTER), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_ENFORCEHRDCOMPATIBILITY), false);

		if (obs_data_get_bool(data, AMF_H264SIMPLE_ADVANCED_SHOW_PARAMETERS)) {
			if (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames) {
				obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_PATTERN), true);
				obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_REFERENCE), true);
				obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BPICTURE_DELTA), true);
				obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA), true);
			}
			obs_property_set_visible(obs_properties_get(props, AMF_H264_DEBLOCKINGFILTER), true);
			obs_property_set_visible(obs_properties_get(props, AMF_H264_ENFORCEHRDCOMPATIBILITY), true);
		}
	}

	{ // Expert Properties
		obs_property_set_visible(obs_properties_get(props, AMF_H264_MEMORYTYPE), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_COMPUTETYPE), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_SURFACEFORMAT), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_GOP_SIZE), false);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_CABAC), false);

		if (obs_data_get_bool(data, AMF_H264SIMPLE_EXPERT_SHOW_PARAMETERS)) {
			obs_property_set_visible(obs_properties_get(props, AMF_H264_MEMORYTYPE), true);
			obs_property_set_visible(obs_properties_get(props, AMF_H264_COMPUTETYPE), true);
			obs_property_set_visible(obs_properties_get(props, AMF_H264_SURFACEFORMAT), true);

			/// GOP Size
			obs_property_set_visible(obs_properties_get(props, AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE), true);
			if (obs_data_get_bool(data, AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE))
				obs_property_set_visible(obs_properties_get(props, AMF_H264_GOP_SIZE), true);

			/// CABAC
			obs_property_set_visible(obs_properties_get(props, AMF_H264_CABAC), true);
		}
	}

	return true;
}

void* Plugin::Interface::H264SimpleInterface::create(obs_data_t* settings, obs_encoder_t* encoder) {
	try {
		return new Plugin::Interface::H264SimpleInterface(settings, encoder);
	} catch (std::exception e) {
		AMF_LOG_ERROR("Unable to create Encoder, see log for more information.");
		return NULL;
	}
}

void Plugin::Interface::H264SimpleInterface::destroy(void* data) {
	delete (static_cast<Plugin::Interface::H264SimpleInterface*>(data));
	data = nullptr;
}

bool Plugin::Interface::H264SimpleInterface::update(void *data, obs_data_t *settings) {
	return static_cast<Plugin::Interface::H264SimpleInterface*>(data)->update(settings);
}

bool Plugin::Interface::H264SimpleInterface::encode(void *data, struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	return static_cast<Plugin::Interface::H264SimpleInterface*>(data)->encode(frame, packet, received_packet);
}

void Plugin::Interface::H264SimpleInterface::get_video_info(void *data, struct video_scale_info *info) {
	static_cast<Plugin::Interface::H264SimpleInterface*>(data)->get_video_info(info);
}

bool Plugin::Interface::H264SimpleInterface::get_extra_data(void *data, uint8_t** extra_data, size_t* size) {
	return static_cast<Plugin::Interface::H264SimpleInterface*>(data)->get_extra_data(extra_data, size);
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
	m_VideoEncoder->SetProfileLevel((VCEProfileLevel)obs_data_get_int(settings, AMF_H264_PROFILELEVEL));

	// Static Parameters
	/// Rate Control Method - User Defined
	m_VideoEncoder->SetRateControlMethod((VCERateControlMethod)obs_data_get_int(settings, AMF_H264_RATECONTROLMETHOD));

	/// Frame Skipping - User Defined
	m_VideoEncoder->SetRateControlSkipFrameEnabled(!!obs_data_get_int(settings, AMF_H264_FRAMESKIPPING));

	/// Minimum QP, Maximum QP - User Defined
	m_VideoEncoder->SetMinimumQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_MINIMUM));
	m_VideoEncoder->SetMaximumQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_MAXIMUM));

	/// Rate Control Method: CBR, VBR, VBR_LAT
	if (obs_data_get_int(settings, AMF_H264_RATECONTROLMETHOD) != VCERateControlMethod_ConstantQP) {
		if (obs_data_get_int(settings, AMF_H264_RATECONTROLMETHOD) == VCERateControlMethod_ConstantBitrate) {
			/// Target Bitrate - User Defined
			uint32_t bitrate = (uint32_t)obs_data_get_int(settings, AMF_H264_BITRATE_TARGET) * 1000;
			m_VideoEncoder->SetTargetBitrate(bitrate);
			m_VideoEncoder->SetPeakBitrate(bitrate);
		} else {
			/// Target Bitrate - User Defined
			m_VideoEncoder->SetTargetBitrate((uint32_t)obs_data_get_int(settings, AMF_H264_BITRATE_TARGET) * 1000);
			/// Peak Bitrate - User Defined
			m_VideoEncoder->SetPeakBitrate((uint32_t)obs_data_get_int(settings, AMF_H264_BITRATE_PEAK) * 1000);
		}
	}

	/// Rate Control Method: CQP
	if (obs_data_get_int(settings, AMF_H264_RATECONTROLMETHOD) == VCERateControlMethod_ConstantQP) {
		/// I-, P-, B-Frame QP - User Defined
		m_VideoEncoder->SetIFrameQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_IFRAME));
		m_VideoEncoder->SetPFrameQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_PFRAME));
		try {
			m_VideoEncoder->SetBFrameQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_BFRAME));
		} catch (...) {}
	}

	/// Buffer Size, Initial Fullness - User Defined or Automatic
	if (obs_data_get_bool(settings, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE)) {
		m_VideoEncoder->SetVBVBufferSize((uint32_t)obs_data_get_int(settings, AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE) * 1000);
		m_VideoEncoder->SetInitialVBVBufferFullness(obs_data_get_double(settings, AMF_H264SIMPLE_CUSTOM_BUFFER_FULLNESS) / 100.0);
	} else {
		if (obs_data_get_int(settings, AMF_H264_RATECONTROLMETHOD) != VCERateControlMethod_ConstantQP) {
			uint32_t bitrate = (uint32_t)max(obs_data_get_int(settings, AMF_H264_BITRATE_TARGET), obs_data_get_int(settings, AMF_H264_BITRATE_PEAK));
			m_VideoEncoder->SetVBVBufferSize(bitrate * 1000);
		} else {
			// When using Constant QP, one will have to pick a QP that is decent
			//  in both quality and bitrate. We can easily calculate both the QP
			//  required for an average bitrate and the average bitrate itself 
			//  with these formulas:
			// BITRATE = ((1 - (QP / 51)) ^ 2) * ((Width * Height) * 1.5 * (FPSNumerator / FPSDenumerator))
			// QP = (1 - sqrt(BITRATE / ((Width * Height) * 1.5 * (FPSNumerator / FPSDenumerator)))) * 51
			double_t bitrate = (width * height);
			switch (voi->format) {
				case VIDEO_FORMAT_NV12: // Y=W*H, UV=W*H/2. Total = 1.5
					bitrate *= 1.5;
					break;
				case VIDEO_FORMAT_I420: // Y=W*H, U=W*H/2, V=W*H/2. Total = 2
					bitrate *= 2;
					break;
				case VIDEO_FORMAT_RGBA: // R=W*H, G=W*H, B=W*H. Total = 3
					bitrate *= 3;
					break;
			}
			bitrate *= ((double_t)fpsNum / (double_t)fpsDen);
			double_t qpMult = (double_t)min(
				min(
					obs_data_get_int(settings, AMF_H264_QP_MINIMUM),
					obs_data_get_int(settings, AMF_H264_QP_MAXIMUM)
				), min(
					min(
						obs_data_get_int(settings, AMF_H264_QP_IFRAME),
						obs_data_get_int(settings, AMF_H264_QP_PFRAME)
					), obs_data_get_int(settings, AMF_H264_QP_BFRAME)
				)
			);
			qpMult = (51 - qpMult) / 51.0;
			qpMult = qpMult * qpMult;
			qpMult = max(qpMult, 0.001); // Needs to be at least 0.001.
			bitrate *= qpMult;
			m_VideoEncoder->SetVBVBufferSize((uint32_t)bitrate);
		}
		m_VideoEncoder->SetInitialVBVBufferFullness(1.0);
	}

	/// Filler Data - User Defined
	m_VideoEncoder->SetFillerDataEnabled(!!obs_data_get_int(settings, AMF_H264_FILLERDATA));

	/// Enforce HRD Compatibility - User Defined or Default
	if (obs_data_get_int(settings, AMF_H264_ENFORCEHRDCOMPATIBILITY) != -1)
		m_VideoEncoder->SetEnforceHRDRestrictionsEnabled(!!obs_data_get_int(settings, AMF_H264_ENFORCEHRDCOMPATIBILITY));

	/// IDR Period - User Defined (Full Second Granularity)
	m_VideoEncoder->SetIDRPeriod((uint32_t)((double_t)obs_data_get_int(settings, AMF_H264SIMPLE_KEYFRAME_INTERVAL) * ((double_t)fpsNum / (double_t)fpsDen)));

	/// Deblocking Filter - User Defined or Default
	if (obs_data_get_int(settings, AMF_H264_DEBLOCKINGFILTER) != -1)
		m_VideoEncoder->SetDeBlockingFilterEnabled(!!obs_data_get_int(settings, AMF_H264_DEBLOCKINGFILTER));

	try {
		m_VideoEncoder->SetBPicturePattern((VCEBPicturePattern)obs_data_get_int(settings, AMF_H264_BPICTURE_PATTERN));
	} catch (...) {}
	try {
		m_VideoEncoder->SetBPictureReferenceEnabled(!!obs_data_get_int(settings, AMF_H264_BPICTURE_REFERENCE));
	} catch (...) {}
	try {
		m_VideoEncoder->SetBPictureDeltaQP((int8_t)obs_data_get_int(settings, AMF_H264_QP_BPICTURE_DELTA));
	} catch (...) {}
	try {
		m_VideoEncoder->SetReferenceBPictureDeltaQP((int8_t)obs_data_get_int(settings, AMF_H264_QP_REFERENCE_BPICTURE_DELTA));
	} catch (...) {}

	/// Encoder Miscellaneous Parameters
	m_VideoEncoder->SetScanType(VCEScanType_Progressive);
	if (obs_data_get_bool(settings, AMF_H264SIMPLE_USE_CUSTOM_GOP_SIZE))
		m_VideoEncoder->SetGOPSize((uint32_t)obs_data_get_int(settings, AMF_H264_GOP_SIZE));
	if (obs_data_get_int(settings, AMF_H264_CABAC) != -1)
		m_VideoEncoder->SetCABACEnabled(!!obs_data_get_bool(settings, AMF_H264_CABAC));

	/// Encoder Motion Estimation Parameters
	m_VideoEncoder->SetHalfPixelMotionEstimationEnabled(true);
	m_VideoEncoder->SetQuarterPixelMotionEstimationEnabled(true);

	/// Encoder Resolution Parameters
	m_VideoEncoder->SetFrameSize(width, height);
	m_VideoEncoder->SetFrameRate(fpsNum, fpsDen);

	//////////////////////////////////////////////////////////////////////////
	// OBS - Enforce Streaming Service Restrictions
	//////////////////////////////////////////////////////////////////////////
	{
		// OBS: Enforce streaming service encoder settings
		const char* p_str = obs_data_get_string(settings, "profile");
		if (strcmp(p_str, "") != 0) {
			if (strcmp(p_str, "baseline")) {
				m_VideoEncoder->SetProfile(VCEProfile_Baseline);
			} else if (strcmp(p_str, "main")) {
				m_VideoEncoder->SetProfile(VCEProfile_Main);
			} else if (strcmp(p_str, "high")) {
				m_VideoEncoder->SetProfile(VCEProfile_High);
			}
			obs_data_set_string(settings, "profile", "");
		}

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
			obs_data_set_string(settings, "rate_control", "");
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

		uint64_t bitrateOvr = obs_data_get_int(settings, "bitrate") * 1000;
		if (bitrateOvr != -1) {
			if (m_VideoEncoder->GetTargetBitrate() > bitrateOvr) {
				m_VideoEncoder->SetTargetBitrate(bitrateOvr);
			}
			if (m_VideoEncoder->GetPeakBitrate() > bitrateOvr) {
				m_VideoEncoder->SetPeakBitrate(bitrateOvr);
			}
			obs_data_set_int(settings, "bitrate", obs_data_get_default_int(settings, "bitrate"));
		} else {
			obs_data_set_int(settings, "bitrate", m_VideoEncoder->GetTargetBitrate() / 1000);
		}

		uint32_t fpsNum = m_VideoEncoder->GetFrameRate().first;
		uint32_t fpsDen = m_VideoEncoder->GetFrameRate().second;
		if (obs_data_get_int(settings, "keyint_sec") != -1) {
			m_VideoEncoder->SetIDRPeriod((uint32_t)(obs_data_get_int(settings, "keyint_sec") * ((double_t)fpsNum / (double_t)fpsDen)));
			obs_data_set_int(settings, "keyint_sec", obs_data_get_default_int(settings, "keyint_sec"));
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
	// settings is not flagged as a unused here, since a future update may add support for this.
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
