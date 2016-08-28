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

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////

// Shared
#define AMF_H264_USAGE							TEXT_AMF_H264("Usage")
#define AMF_H264_USAGE_TRANSCODING				TEXT_AMF_H264("Usage.Transcoding")
#define AMF_H264_USAGE_ULTRALOWLATENCY			TEXT_AMF_H264("Usage.UltraLowLatency")
#define AMF_H264_USAGE_LOWLATENCY				TEXT_AMF_H264("Usage.LowLatency")
#define AMF_H264_USAGE_WEBCAM					TEXT_AMF_H264("Usage.Webcam")
#define AMF_H264_QUALITY_PRESET					TEXT_AMF_H264("QualityPreset")
#define AMF_H264_QUALITY_PRESET_SPEED			TEXT_AMF_H264("QualityPreset.Speed")
#define AMF_H264_QUALITY_PRESET_BALANCED		TEXT_AMF_H264("QualityPreset.Balanced")
#define AMF_H264_QUALITY_PRESET_QUALITY			TEXT_AMF_H264("QualityPreset.Quality")
#define AMF_H264_PROFILE						TEXT_AMF_H264("Profile")
#define AMF_H264_PROFILE_BASELINE				TEXT_AMF_H264("Profile.Baseline")
#define AMF_H264_PROFILE_MAIN					TEXT_AMF_H264("Profile.Main")
#define AMF_H264_PROFILE_HIGH					TEXT_AMF_H264("Profile.High")
#define AMF_H264_PROFILELEVEL					TEXT_AMF_H264("ProfileLevel")
#define AMF_H264_PROFILELEVEL2(x)				TEXT_AMF_H264("ProfileLevel." # x)
#define AMF_H264_RATECONTROLMETHOD				TEXT_AMF_H264("RateControlMethod")
#define AMF_H264_RATECONTROLMETHOD_CQP			TEXT_AMF_H264("RateControlMethod.CQP")
#define AMF_H264_RATECONTROLMETHOD_CBR			TEXT_AMF_H264("RateControlMethod.CBR")
#define AMF_H264_RATECONTROLMETHOD_VBR			TEXT_AMF_H264("RateControlMethod.VBR.Peak")
#define AMF_H264_RATECONTROLMETHOD_VBR_LAT		TEXT_AMF_H264("RateControlMethod.VBR.Latency")
#define AMF_H264_BITRATE_TARGET					TEXT_AMF_H264("Bitrate.Target")
#define AMF_H264_BITRATE_PEAK					TEXT_AMF_H264("Bitrate.Peak")
#define AMF_H264_QP_MINIMUM						TEXT_AMF_H264("QP.Minimum")
#define AMF_H264_QP_MAXIMUM						TEXT_AMF_H264("QP.Maximum")
#define AMF_H264_QP_IFRAME						TEXT_AMF_H264("QP.IFrame")
#define AMF_H264_QP_PFRAME						TEXT_AMF_H264("QP.PFrame")
#define AMF_H264_QP_BFRAME						TEXT_AMF_H264("QP.BFrame")
#define AMF_H264_QP_BPICTURE_DELTA				TEXT_AMF_H264("QP.BPictureDelta")
#define AMF_H264_QP_REFERENCE_BPICTURE_DELTA	TEXT_AMF_H264("QP.RefeferenceBPictureDelta")
#define AMF_H264_FILLERDATA						TEXT_AMF_H264("FillerData")
#define AMF_H264_FRAMESKIPPING					TEXT_AMF_H264("FrameSkipping")
#define AMF_H264_SCANTYPE						TEXT_AMF_H264("ScanType")
#define AMF_H264_SCANTYPE_PROGRESSIVE			TEXT_AMF_H264("ScanType.Progressive")
#define AMF_H264_SCANTYPE_INTERLACED			TEXT_AMF_H264("ScanType.Interlaced")
#define AMF_H264_BPICTURE_PATTERN				TEXT_AMF_H264("BPicture.Pattern")
#define AMF_H264_BPICTURE_REFERENCE				TEXT_AMF_H264("BPicture.Reference")
#define AMF_H264_DEBUGTRACING					TEXT_AMF_H264("DebugTracing")

// Simple Interface
#define AMF_H264SIMPLE_NAME						TEXT_AMF_H264SIMPLE("Name")
#define AMF_H264SIMPLE_PRESET					TEXT_AMF_H264SIMPLE("Preset")
#define AMF_H264SIMPLE_PRESET_RECORDING			TEXT_AMF_H264SIMPLE("Preset.Recording")
#define AMF_H264SIMPLE_PRESET_TWITCH			TEXT_AMF_H264SIMPLE("Preset.Twitch")
#define AMF_H264SIMPLE_PRESET_YOUTUBE			TEXT_AMF_H264SIMPLE("Preset.YouTube")
#define AMF_H264SIMPLE_KEYFRAME_INTERVAL		TEXT_AMF_H264SIMPLE("KeyframeInterval")
#define AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE	TEXT_AMF_H264SIMPLE("UseCustomBufferSize")
#define AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE		TEXT_AMF_H264SIMPLE("CustomBufferSize")
#define AMF_H264SIMPLE_ADVANCED_SHOW_PARAMETERS	TEXT_AMF_H264SIMPLE("Advanced.ShowParameters")

// Utility
#define AMF_UTIL_DEFAULT						TEXT_AMF_UTIL("Default")
#define AMF_UTIL_TOGGLE_DISABLED				TEXT_AMF_UTIL("Toggle.Disabled")
#define AMF_UTIL_TOGGLE_ENABLED					TEXT_AMF_UTIL("Toggle.Enabled")


//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin;
using namespace Plugin::AMD;
using namespace Plugin::Interface;

void Plugin::Interface::H264SimpleInterface::encoder_register() {
	static obs_encoder_info* encoder_info = new obs_encoder_info();
	std::memset(encoder_info, 0, sizeof(obs_encoder_info));
	encoder_info->id = "amd_amf_h264_simple";
	encoder_info->type = obs_encoder_type::OBS_ENCODER_VIDEO;
	encoder_info->codec = "h264";

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

const char* Plugin::Interface::H264SimpleInterface::get_name(void* type_data) {
	return TEXT_AMF_H264_T("Name");
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
	/// Frame Skipping
	obs_data_set_default_bool(data, AMF_H264_FRAMESKIPPING, false);
	
	// Advanced Properties
	obs_data_set_default_bool(data, AMF_H264SIMPLE_ADVANCED_SHOW_PARAMETERS, false);
	obs_data_set_default_int(data, AMF_H264_BPICTURE_PATTERN, 0);
	obs_data_set_default_int(data, AMF_H264_QP_BPICTURE_DELTA, 0);
	obs_data_set_default_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, 0);

	// Debug Mode
	obs_data_set_default_bool(data, AMF_H264_DEBUGTRACING, false);
}

obs_properties_t* Plugin::Interface::H264SimpleInterface::get_properties(void* data) {
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
	obs_property_set_modified_callback(list, &preset_modified);

	/// Keyframe Interval
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
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(62)), VCEProfileLevel_62);
		case 61:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(61)), VCEProfileLevel_61);
		case 60:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(60)), VCEProfileLevel_60);
		case 52:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(52)), VCEProfileLevel_52);
		case 51:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(51)), VCEProfileLevel_51);
		case 50:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(50)), VCEProfileLevel_50);
		case 42: // Some VCE 2.0 Cards.
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(42)), VCEProfileLevel_42);
		case 41: // Some APUs and VCE 1.0 Cards.
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(41)), VCEProfileLevel_41);
		case 40: // These should in theory be supported by all VCE 1.0 devices and APUs.
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(40)), VCEProfileLevel_40);
		case 32:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(32)), VCEProfileLevel_32);
		case 31:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(31)), VCEProfileLevel_31);
		case 30:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(30)), VCEProfileLevel_30);
		case 22:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(22)), VCEProfileLevel_22);
		case 21:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(21)), VCEProfileLevel_21);
		case 20:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(20)), VCEProfileLevel_20);
		case 13:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(13)), VCEProfileLevel_13);
		case 12:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(12)), VCEProfileLevel_12);
		case 11:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(11)), VCEProfileLevel_11);
		case 10:
		default:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(10)), VCEProfileLevel_10);
	}

	/// Rate Control
	list = obs_properties_add_list(props, AMF_H264_RATECONTROLMETHOD, obs_module_text(AMF_H264_RATECONTROLMETHOD), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_RATECONTROLMETHOD_CQP), Plugin::AMD::VCERateControlMethod::VCERateControlMethod_ConstantQP);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_RATECONTROLMETHOD_CBR), Plugin::AMD::VCERateControlMethod::VCERateControlMethod_ConstantBitrate);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_RATECONTROLMETHOD_VBR), Plugin::AMD::VCERateControlMethod::VCERateControlMethod_VariableBitrate_PeakConstrained);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_RATECONTROLMETHOD_VBR_LAT), Plugin::AMD::VCERateControlMethod::VCERateControlMethod_VariableBitrate_LatencyConstrained);
	obs_property_set_modified_callback(list, &ratecontrolmethod_modified);

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
	obs_property_set_modified_callback(p, &custombuffer_modified);

	/// Other Options
	obs_properties_add_bool(props, AMF_H264_FILLERDATA, obs_module_text(AMF_H264_FILLERDATA));
	obs_properties_add_bool(props, AMF_H264_FRAMESKIPPING, obs_module_text(AMF_H264_FRAMESKIPPING));

	// Advanced Properties
	p = obs_properties_add_bool(props, AMF_H264SIMPLE_ADVANCED_SHOW_PARAMETERS, obs_module_text(AMF_H264SIMPLE_ADVANCED_SHOW_PARAMETERS));
	obs_property_set_modified_callback(p, &advanced_modified);
	/// B-Pictures
	obs_properties_add_int_slider(props, AMF_H264_BPICTURE_PATTERN, obs_module_text(AMF_H264_BPICTURE_PATTERN), 0, 3, 1);
	obs_properties_add_bool(props, AMF_H264_BPICTURE_REFERENCE, obs_module_text(AMF_H264_BPICTURE_REFERENCE));
	obs_properties_add_int_slider(props, AMF_H264_QP_BPICTURE_DELTA, obs_module_text(AMF_H264_QP_BPICTURE_DELTA), -10, 10, 1);
	obs_properties_add_int_slider(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, obs_module_text(AMF_H264_QP_REFERENCE_BPICTURE_DELTA), -10, 10, 1);
	
	// Debug Mode
	obs_properties_add_bool(props, AMF_H264_DEBUGTRACING, obs_module_text(AMF_H264_DEBUGTRACING));

	return props;
}

bool Plugin::Interface::H264SimpleInterface::preset_modified(obs_properties_t *props, obs_property_t *property, obs_data_t *data) {
	switch (obs_data_get_int(data, AMF_H264SIMPLE_PRESET)) {
		case 0: // Recording
			obs_data_set_int(data, AMF_H264SIMPLE_KEYFRAME_INTERVAL, 1);
			obs_data_set_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Balanced);
			obs_data_set_int(data, AMF_H264_PROFILE, VCEProfile_High);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxProfileLevel);
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_VariableBitrate_LatencyConstrained);
			obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 10000);
			obs_data_set_int(data, AMF_H264_BITRATE_PEAK, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / 1000);
			obs_data_set_int(data, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE, false);
			obs_data_set_int(data, AMF_H264_FILLERDATA, false);
			obs_data_set_int(data, AMF_H264_FRAMESKIPPING, false);
			break;
		case 1: // Twitch
			obs_data_set_int(data, AMF_H264SIMPLE_KEYFRAME_INTERVAL, 2);
			obs_data_set_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Speed);
			obs_data_set_int(data, AMF_H264_PROFILE, VCEProfile_Main);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_41);
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantBitrate);
			obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 2500);
			obs_data_set_int(data, AMF_H264_BITRATE_PEAK, 3500);
			obs_data_set_int(data, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE, true);
			obs_data_set_int(data, AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE, 3000);
			obs_data_set_int(data, AMF_H264_FILLERDATA, true);
			obs_data_set_int(data, AMF_H264_FRAMESKIPPING, true);
			break;
		case 2: // YouTube
			obs_data_set_int(data, AMF_H264SIMPLE_KEYFRAME_INTERVAL, 2);
			obs_data_set_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Speed);
			obs_data_set_int(data, AMF_H264_PROFILE, VCEProfile_High);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_42);
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_VariableBitrate_PeakConstrained);
			obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 3000);
			obs_data_set_int(data, AMF_H264_BITRATE_PEAK, 6000);
			obs_data_set_int(data, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE, true);
			obs_data_set_int(data, AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE, 3000);
			obs_data_set_int(data, AMF_H264_FILLERDATA, true);
			obs_data_set_int(data, AMF_H264_FRAMESKIPPING, true);
			break;
	}
	obs_data_set_int(data, AMF_H264SIMPLE_PRESET, -1);
	return true;
}

bool Plugin::Interface::H264SimpleInterface::ratecontrolmethod_modified(obs_properties_t *props, obs_property_t *property, obs_data_t *data) {
	// Reset State
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_PEAK), false);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_TARGET), false);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_IFRAME), false);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_PFRAME), false);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BFRAME), false);

	switch (obs_data_get_int(data, AMF_H264_RATECONTROLMETHOD)) {
		case Plugin::AMD::VCERateControlMethod::VCERateControlMethod_ConstantQP:
			obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_IFRAME), true);
			obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_PFRAME), true);
			obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BFRAME), true);
			break;
		case Plugin::AMD::VCERateControlMethod::VCERateControlMethod_ConstantBitrate:
		case Plugin::AMD::VCERateControlMethod::VCERateControlMethod_VariableBitrate_PeakConstrained:
		case Plugin::AMD::VCERateControlMethod::VCERateControlMethod_VariableBitrate_LatencyConstrained:
			obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_PEAK), true);
			obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_TARGET), true);
			break;
	}
	return true;
}

bool Plugin::Interface::H264SimpleInterface::custombuffer_modified(obs_properties_t *props, obs_property_t *property, obs_data_t *data) {
	obs_property_set_visible(obs_properties_get(props, AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE), false);
	if (obs_data_get_bool(data, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE))
		obs_property_set_visible(obs_properties_get(props, AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE), true);

	return true;
}

bool Plugin::Interface::H264SimpleInterface::advanced_modified(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_PATTERN), false);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_REFERENCE), false);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BPICTURE_DELTA), false);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA), false);

	if (obs_data_get_bool(settings, AMF_H264SIMPLE_ADVANCED_SHOW_PARAMETERS)) {
		if (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames) {
			obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_PATTERN), true);
			obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_REFERENCE), true);
			obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BPICTURE_DELTA), true);
			obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA), true);
		}
	}

	return true;
}

void* Plugin::Interface::H264SimpleInterface::create(obs_data_t* settings, obs_encoder_t* encoder) {
	try {
		return new Plugin::Interface::H264SimpleInterface(settings, encoder);
	} catch (std::exception e) {
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
	auto t_amf = Plugin::AMD::AMF::GetInstance();
	if (obs_data_get_bool(settings, AMF_H264_DEBUGTRACING)) {
		t_amf->GetDebug()->AssertsEnable(true);
		t_amf->GetDebug()->EnablePerformanceMonitor(true);
		t_amf->GetTrace()->TraceEnableAsync(true);
		t_amf->GetTrace()->SetGlobalLevel(AMF_TRACE_TEST);
		t_amf->GetTrace()->SetWriterLevel(L"OBSWriter", AMF_TRACE_TEST);
	} else {
		t_amf->GetDebug()->AssertsEnable(false);
		t_amf->GetDebug()->EnablePerformanceMonitor(false);
		t_amf->GetTrace()->TraceEnableAsync(false);
		t_amf->GetTrace()->SetGlobalLevel(AMF_TRACE_ERROR);
		t_amf->GetTrace()->SetWriterLevel(L"OBSWriter", AMF_TRACE_ERROR);
	}

	// Encoder Static Parameters
	VCESurfaceFormat format = VCESurfaceFormat_NV12;
	switch (voi->format) {
		case VIDEO_FORMAT_NV12:
			format = VCESurfaceFormat_NV12;
			break;
		case VIDEO_FORMAT_I420:
			format = VCESurfaceFormat_I420;
			break;
		case VIDEO_FORMAT_RGBA:
			format = VCESurfaceFormat_RGBA;
			break;
	}
	m_VideoEncoder = new Plugin::AMD::VCEEncoder(VCEEncoderType_AVC, VCEMemoryType_Host, format);
	
	/// Encoder Static Parameters
	m_VideoEncoder->SetUsage(VCEUsage_Transcoding);
	m_VideoEncoder->SetQualityPreset((VCEQualityPreset)obs_data_get_int(settings, AMF_H264_QUALITY_PRESET)); // Temporarily moved up here from down there.
	m_VideoEncoder->SetProfile((VCEProfile)obs_data_get_int(settings, AMF_H264_PROFILE));
	m_VideoEncoder->SetProfileLevel((VCEProfileLevel)obs_data_get_int(settings, AMF_H264_PROFILELEVEL));
	//m_VideoEncoder->SetMaxLTRFrames(0);

	/// Encoder Resolution Parameters
	m_VideoEncoder->SetFrameSize(width, height);
	m_VideoEncoder->SetFrameRate(fpsNum, fpsDen);

	/// Encoder Rate Control
	m_VideoEncoder->SetRateControlMethod((VCERateControlMethod)obs_data_get_int(settings, AMF_H264_RATECONTROLMETHOD));
	m_VideoEncoder->SetFillerDataEnabled(obs_data_get_bool(settings, AMF_H264_FILLERDATA));
	m_VideoEncoder->SetRateControlSkipFrameEnabled(obs_data_get_bool(settings, AMF_H264_FRAMESKIPPING));
	if (obs_data_get_bool(settings, AMF_H264SIMPLE_USE_CUSTOM_BUFFER_SIZE)) {
		m_VideoEncoder->SetVBVBufferSize((uint32_t)obs_data_get_int(settings, AMF_H264SIMPLE_CUSTOM_BUFFER_SIZE) * 1000);
	} else {
		if (obs_data_get_int(settings, AMF_H264_RATECONTROLMETHOD) != VCERateControlMethod_ConstantQP) {
			uint32_t bitrate = (uint32_t)max(obs_data_get_int(settings, AMF_H264_BITRATE_TARGET), obs_data_get_int(settings, AMF_H264_BITRATE_PEAK));
			m_VideoEncoder->SetVBVBufferSize(bitrate * 1000);
		} else {
			// Values at 1920x1080@60p
			//	I	P	B	Min	Max	Bitrate
			//	 0	 0	 0	 0	 0	~150000kbit
			//	 1	 1	 1	 1	 1	~138000kbit
			//	11	11	11	11	11	~ 58800kbit
			//	21	21	21	21	21	~ 19500kbit
			//	31	31	31	31	31	~  3500kbit 
			//	41	41	41	41	41	~   320kbit
			//	51	51	51	51	51	~    90kbit
			//	SHAPE: PARABOLA! (51-QP)*(51-QP)*bitrate = What we need.
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
			qpMult = (51 - qpMult);
			qpMult = max(qpMult * qpMult, 0.001); // Can't allow 0.
			bitrate *= qpMult;
			m_VideoEncoder->SetVBVBufferSize((uint32_t)bitrate);
		}
	}
	m_VideoEncoder->SetInitialVBVBufferFullness(1.0);
	m_VideoEncoder->SetMinimumQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_MINIMUM));
	m_VideoEncoder->SetMaximumQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_MAXIMUM));
	if (obs_data_get_int(settings, AMF_H264_RATECONTROLMETHOD) != VCERateControlMethod_ConstantQP) {
		m_VideoEncoder->SetTargetBitrate((uint32_t)obs_data_get_int(settings, AMF_H264_BITRATE_TARGET) * 1000);
		m_VideoEncoder->SetPeakBitrate((uint32_t)obs_data_get_int(settings, AMF_H264_BITRATE_PEAK) * 1000);
	}
	if (obs_data_get_int(settings, AMF_H264_RATECONTROLMETHOD) == VCERateControlMethod_ConstantQP) {
		m_VideoEncoder->SetIFrameQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_IFRAME));
		m_VideoEncoder->SetPFrameQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_PFRAME));
		m_VideoEncoder->SetBFrameQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_BFRAME));
	}
	//m_VideoEncoder->SetEnforceHRDRestrictionsEnabled(false);
	m_VideoEncoder->SetFillerDataEnabled(true);

	/// Encoder Picture Control Parameters
	//m_VideoEncoder->SetHeaderInsertionSpacing((uint32_t)obs_data_get_int(settings, AMF_VCE_H264_KEYFRAME_INTERVAL) * (uint32_t)((double_t)fpsNum / (double_t)fpsDen));
	m_VideoEncoder->SetIDRPeriod((uint32_t)obs_data_get_int(settings, AMF_H264SIMPLE_KEYFRAME_INTERVAL) * (uint32_t)((double_t)fpsNum / (double_t)fpsDen));
	//m_VideoEncoder->SetDeBlockingFilterEnabled(false);
	m_VideoEncoder->SetBPicturePattern((VCEBPicturePattern)obs_data_get_int(settings, AMF_H264_BPICTURE_PATTERN));
	m_VideoEncoder->SetBPictureReferenceEnabled(obs_data_get_bool(settings, AMF_H264_BPICTURE_REFERENCE));
	m_VideoEncoder->SetBPictureDeltaQP(obs_data_get_int(settings, AMF_H264_QP_BPICTURE_DELTA));
	m_VideoEncoder->SetReferenceBPictureDeltaQP(obs_data_get_int(settings, AMF_H264_QP_REFERENCE_BPICTURE_DELTA));

	/// Encoder Miscellaneous Parameters
	//m_VideoEncoder->SetScanType(H264ScanType_Progressive);
	//m_VideoEncoder->SetQualityPreset((VCEQualityPreset)obs_data_get_int(settings, AMF_VCE_H264_QUALITY_PRESET)); // Temporarily moved from down here to up there.

	/// Encoder Motion Estimation Parameters
	m_VideoEncoder->SetHalfPixelMotionEstimationEnabled(true);
	m_VideoEncoder->SetQuarterPixelMotionEstimationEnabled(true);

	// Do this again just to be sure (output file seems to have 1000 fps?)
	m_VideoEncoder->SetFrameSize(width, height);
	m_VideoEncoder->SetFrameRate(fpsNum, fpsDen);

	//////////////////////////////////////////////////////////////////////////
	// Verify
	//////////////////////////////////////////////////////////////////////////
	m_VideoEncoder->LogProperties();

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
