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
#define AMF_H264_ENFORCEHRDCOMPATIBILITY		TEXT_AMF_H264("EnforceHRDCompatibility")
#define AMF_H264_DEBLOCKINGFILTER				TEXT_AMF_H264("DeBlockingFilter")
#define AMF_H264_SCANTYPE						TEXT_AMF_H264("ScanType")
#define AMF_H264_SCANTYPE_PROGRESSIVE			TEXT_AMF_H264("ScanType.Progressive")
#define AMF_H264_SCANTYPE_INTERLACED			TEXT_AMF_H264("ScanType.Interlaced")
#define AMF_H264_BPICTURE_PATTERN				TEXT_AMF_H264("BPicture.Pattern")
#define AMF_H264_BPICTURE_REFERENCE				TEXT_AMF_H264("BPicture.Reference")
#define AMF_H264_DEBUGTRACING					TEXT_AMF_H264("DebugTracing")

// Advanced Interface
#define AMF_H264ADVANCED_NAME						TEXT_AMF_H264ADVANCED("Name")
#define AMF_H264ADVANCED_RESET						TEXT_AMF_H264ADVANCED("Reset")
#define AMF_H264ADVANCED_UPDATE						TEXT_AMF_H264ADVANCED("Update")
#define AMF_H264ADVANCED_MAX_LTR_FRAMES				TEXT_AMF_H264ADVANCED("MaxLTRFrames")
#define AMF_H264ADVANCED_VBVBUFFER_SIZE				TEXT_AMF_H264ADVANCED("VBVBuffer.Size")
#define AMF_H264ADVANCED_VBVBUFFER_FULLNESS			TEXT_AMF_H264ADVANCED("VBVBuffer.Fullness")
#define AMF_H264ADVANCED_MAX_AU_SIZE				TEXT_AMF_H264ADVANCED("MaxAUSize")
#define AMF_H264ADVANCED_HEADER_INSERTION_SPACING	TEXT_AMF_H264ADVANCED("HeaderInsertionSpacing")
#define AMF_H264ADVANCED_IDR_PERIOD					TEXT_AMF_H264ADVANCED("IDRPeriod")
#define AMF_H264ADVANCED_INTRAREFRESHNUMMBPERSLOT	TEXT_AMF_H264ADVANCED("IntraRefreshNumMBPerSlot")
#define AMF_H264ADVANCED_SLICESPERFRAME				TEXT_AMF_H264ADVANCED("SlicesPerFrame")
#define AMF_H264ADVANCED_MOTIONESTIMATION			TEXT_AMF_H264ADVANCED("MotionEstimation")
#define AMF_H264ADVANCED_MOTIONESTIMATION_NONE		TEXT_AMF_H264ADVANCED("MotionEstimation.None")
#define AMF_H264ADVANCED_MOTIONESTIMATION_HALF		TEXT_AMF_H264ADVANCED("MotionEstimation.Half")
#define AMF_H264ADVANCED_MOTIONESTIMATION_QUARTER	TEXT_AMF_H264ADVANCED("MotionEstimation.Quarter")
#define AMF_H264ADVANCED_MOTIONESTIMATION_BOTH		TEXT_AMF_H264ADVANCED("MotionEstimation.Both")

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

void Plugin::Interface::H264Interface::encoder_register() {
	static obs_encoder_info* encoder_info = new obs_encoder_info();
	std::memset(encoder_info, 0, sizeof(obs_encoder_info));
	encoder_info->id = "amd_amf_h264";
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

const char* Plugin::Interface::H264Interface::get_name(void* type_data) {
	return obs_module_text(AMF_H264ADVANCED_NAME);
}

void* Plugin::Interface::H264Interface::create(obs_data_t* settings, obs_encoder_t* encoder) {
	try {
		Plugin::Interface::H264Interface* enc = new Plugin::Interface::H264Interface(settings, encoder);
		return enc;
	} catch (std::exception e) {
		return NULL;
	}
}

void Plugin::Interface::H264Interface::destroy(void* data) {
	Plugin::Interface::H264Interface* enc = static_cast<Plugin::Interface::H264Interface*>(data);
	delete enc;
	data = nullptr;
}

bool Plugin::Interface::H264Interface::encode(void *data, struct encoder_frame *frame, struct encoder_packet *packet, bool *received_packet) {
	return static_cast<Plugin::Interface::H264Interface*>(data)->encode(frame, packet, received_packet);
}

void Plugin::Interface::H264Interface::get_defaults(obs_data_t *data) {
	// Controls
	obs_data_set_default_bool(data, AMF_H264ADVANCED_RESET, false);
	obs_data_set_default_bool(data, AMF_H264ADVANCED_UPDATE, false);

	// Static Properties
	/// Usage & Quality Preset
	obs_data_set_default_int(data, AMF_H264_USAGE, VCEUsage_Transcoding);
	obs_data_set_default_int(data, AMF_H264_QUALITY_PRESET, -1);
	/// Profile & Level
	obs_data_set_default_int(data, AMF_H264_PROFILE, -1);
	obs_data_set_default_int(data, AMF_H264_PROFILELEVEL, -1);
	/// Other
	obs_data_set_default_int(data, AMF_H264ADVANCED_MAX_LTR_FRAMES, -1);
	obs_data_set_default_int(data, AMF_H264_SCANTYPE, -1);

	// Dynamic Properties
	/// Rate Control
	obs_data_set_default_int(data, AMF_H264_RATECONTROLMETHOD, -1);
	/// Other
	obs_data_set_default_int(data, AMF_H264_FILLERDATA, -1);
	obs_data_set_default_int(data, AMF_H264_FRAMESKIPPING, -1);
	obs_data_set_default_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, -1);
	/// Video Coding Settings
	obs_data_set_default_int(data, AMF_H264ADVANCED_VBVBUFFER_SIZE, -1);
	obs_data_set_default_double(data, AMF_H264ADVANCED_VBVBUFFER_FULLNESS, 1.0);
	obs_data_set_default_int(data, AMF_H264ADVANCED_MAX_AU_SIZE, -1);
	/// Rate Control:  CBR, VBR
	obs_data_set_default_int(data, AMF_H264_BITRATE_TARGET, -1);
	obs_data_set_default_int(data, AMF_H264_BITRATE_PEAK, -1);
	/// Todo: Unknown Rate Control Method, possibly just used for internal storage. I believe there is a Constrained QP setting that is yet undiscovered.
	obs_data_set_default_int(data, AMF_H264_QP_MINIMUM, -1);
	obs_data_set_default_int(data, AMF_H264_QP_MAXIMUM, -1);
	/// Rate Control: Constrained QP
	obs_data_set_default_int(data, AMF_H264_QP_IFRAME, -1);
	obs_data_set_default_int(data, AMF_H264_QP_PFRAME, -1);
	obs_data_set_default_int(data, AMF_H264_QP_BFRAME, -1);
	/// B-Picture Stuff
	obs_data_set_default_int(data, AMF_H264_QP_BPICTURE_DELTA, -11);
	obs_data_set_default_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, -11);

	//// Picture Control Properties
	obs_data_set_default_int(data, AMF_H264_BPICTURE_PATTERN, -1);
	obs_data_set_default_int(data, AMF_H264_BPICTURE_REFERENCE, -1);
	obs_data_set_default_int(data, AMF_H264ADVANCED_HEADER_INSERTION_SPACING, -1);
	obs_data_set_default_int(data, AMF_H264_DEBLOCKINGFILTER, -1);
	obs_data_set_default_int(data, AMF_H264ADVANCED_IDR_PERIOD, -1);
	obs_data_set_default_int(data, AMF_H264ADVANCED_INTRAREFRESHNUMMBPERSLOT, -1);
	obs_data_set_default_int(data, AMF_H264ADVANCED_SLICESPERFRAME, -1);

	//// Motion Estimation
	obs_data_set_default_int(data, AMF_H264ADVANCED_MOTIONESTIMATION, -1);

	/// Debug Mode
	obs_data_set_default_bool(data, "Debug", false);
}

obs_properties_t* Plugin::Interface::H264Interface::get_properties(void* data) {
	obs_properties* props = obs_properties_create();
	obs_property_t* list;
	obs_property_t* p;

	//////////////////////////////////////////////////////////////////////////
	// Controls
	//////////////////////////////////////////////////////////////////////////
	p = obs_properties_add_bool(props, AMF_H264ADVANCED_RESET, obs_module_text(AMF_H264ADVANCED_RESET));
	obs_property_set_modified_callback(p, &reset_callback);
	p = obs_properties_add_bool(props, AMF_H264ADVANCED_UPDATE, obs_module_text(AMF_H264ADVANCED_UPDATE));
	obs_property_set_modified_callback(p, &update_from_amf);

	//////////////////////////////////////////////////////////////////////////
	// Encoder Static Parameters
	//////////////////////////////////////////////////////////////////////////
	/// Usage
	list = obs_properties_add_list(props, AMF_H264_USAGE, obs_module_text(AMF_H264_USAGE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_USAGE_TRANSCODING), VCEUsage_Transcoding);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_USAGE_ULTRALOWLATENCY), VCEUsage_UltraLowLatency);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_USAGE_LOWLATENCY), VCEUsage_LowLatency);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_USAGE_WEBCAM), VCEUsage_Webcam);
	/// h264 Profile
	list = obs_properties_add_list(props, AMF_H264_PROFILE, obs_module_text(AMF_H264_PROFILE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
	switch (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxProfile) {
		case 100:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILE_HIGH), VCEProfile_High);
		case 77:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILE_MAIN), VCEProfile_Main);
		case 66:
			obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILE_BASELINE), VCEProfile_Baseline);
			break;
	}
	/// h264 Profile Level
	list = obs_properties_add_list(props, AMF_H264_PROFILELEVEL, obs_module_text(AMF_H264_PROFILELEVEL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_PROFILELEVEL2(Default)), -1);
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
	/// Maximum Long-Term-Reference Frames
	obs_properties_add_int_slider(props, AMF_H264ADVANCED_MAX_LTR_FRAMES, obs_module_text(AMF_H264ADVANCED_MAX_LTR_FRAMES), -1, 2, 1);

	//////////////////////////////////////////////////////////////////////////
	// Encoder Rate Control
	//////////////////////////////////////////////////////////////////////////
	/// Method
	list = obs_properties_add_list(props, AMF_H264_RATECONTROLMETHOD, obs_module_text(AMF_H264_RATECONTROLMETHOD), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_RATECONTROLMETHOD_CQP), VCERateControlMethod_ConstantQP);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_RATECONTROLMETHOD_CBR), VCERateControlMethod_ConstantBitrate);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_RATECONTROLMETHOD_VBR), VCERateControlMethod_VariableBitrate_PeakConstrained);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_RATECONTROLMETHOD_VBR_LAT), VCERateControlMethod_VariableBitrate_LatencyConstrained);
	/// Skip Frames if necessary
	list = obs_properties_add_list(props, AMF_H264_FRAMESKIPPING, obs_module_text(AMF_H264_FRAMESKIPPING), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	/// Method: Bitrate
	obs_properties_add_int_slider(props, AMF_H264_BITRATE_TARGET, obs_module_text(AMF_H264_BITRATE_TARGET), -1, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate, 1);
	obs_properties_add_int_slider(props, AMF_H264_BITRATE_PEAK, obs_module_text(AMF_H264_BITRATE_PEAK), -1, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate, 1);
	/// Minimum QP, Maximum QP
	obs_properties_add_int_slider(props, AMF_H264_QP_MINIMUM, obs_module_text(AMF_H264_QP_MINIMUM), -1, 51, 1);
	obs_properties_add_int_slider(props, AMF_H264_QP_MAXIMUM, obs_module_text(AMF_H264_QP_MAXIMUM), -1, 51, 1);
	/// Method: Constant QP
	obs_properties_add_int_slider(props, AMF_H264_QP_IFRAME, obs_module_text(AMF_H264_QP_IFRAME), -1, 51, 1);
	obs_properties_add_int_slider(props, AMF_H264_QP_PFRAME, obs_module_text(AMF_H264_QP_PFRAME), -1, 51, 1);
	obs_properties_add_int_slider(props, AMF_H264_QP_BFRAME, obs_module_text(AMF_H264_QP_BFRAME), -1, 51, 1);
	/// B-Picture Related
	obs_properties_add_int_slider(props, AMF_H264_QP_BPICTURE_DELTA, obs_module_text(AMF_H264_QP_BPICTURE_DELTA), -11, 10, 1);
	obs_properties_add_int_slider(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, obs_module_text(AMF_H264_QP_REFERENCE_BPICTURE_DELTA), -11, 10, 1);
	/// VBV Buffer
	obs_properties_add_int_slider(props, AMF_H264ADVANCED_VBVBUFFER_SIZE, obs_module_text(AMF_H264ADVANCED_VBVBUFFER_SIZE), -1, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate, 1);
	obs_properties_add_float_slider(props, AMF_H264ADVANCED_VBVBUFFER_FULLNESS, obs_module_text(AMF_H264ADVANCED_VBVBUFFER_FULLNESS), 0.0, 1.0, 0.015625);
	/// Enforce Hypothecial Reference Decoder Compatibility
	list = obs_properties_add_list(props, AMF_H264_ENFORCEHRDCOMPATIBILITY, obs_module_text(AMF_H264_ENFORCEHRDCOMPATIBILITY), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	/// Filler Data
	list = obs_properties_add_list(props, AMF_H264_FILLERDATA, obs_module_text(AMF_H264_FILLERDATA), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	/// Max AU Size
	obs_properties_add_int_slider(props, AMF_H264ADVANCED_MAX_AU_SIZE, obs_module_text(AMF_H264ADVANCED_MAX_AU_SIZE), -1, 100000000, 1);

	//////////////////////////////////////////////////////////////////////////
	// Encoder Picture Control
	//////////////////////////////////////////////////////////////////////////
	/// Header Insertion Spacing
	obs_properties_add_int_slider(props, AMF_H264ADVANCED_HEADER_INSERTION_SPACING, obs_module_text(AMF_H264ADVANCED_HEADER_INSERTION_SPACING), -1, 1000, 1);
	/// IDR Period
	obs_properties_add_int_slider(props, AMF_H264ADVANCED_IDR_PERIOD, obs_module_text(AMF_H264ADVANCED_IDR_PERIOD), -1, 1000, 1);
	/// De-Blocking Filter
	list = obs_properties_add_list(props, AMF_H264_DEBLOCKINGFILTER, obs_module_text(AMF_H264_DEBLOCKINGFILTER), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	/// Intra Refresh MBs Number Per Slot in Macroblocks
	obs_properties_add_int_slider(props, AMF_H264ADVANCED_INTRAREFRESHNUMMBPERSLOT, obs_module_text(AMF_H264ADVANCED_INTRAREFRESHNUMMBPERSLOT), -1, 1024, 1);
	/// Number of slices Per Frame 
	obs_properties_add_int_slider(props, AMF_H264ADVANCED_SLICESPERFRAME, obs_module_text(AMF_H264ADVANCED_SLICESPERFRAME), -1, 1000, 1);
	/// B-Pictures Pattern
	obs_properties_add_int_slider(props, AMF_H264_BPICTURE_PATTERN, obs_module_text(AMF_H264_BPICTURE_PATTERN), -1, (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames ? 3 : 0), 1);
	/// Enable Reference to B-Frames (2nd Generation GCN and newer)
	list = obs_properties_add_list(props, AMF_H264_BPICTURE_REFERENCE, obs_module_text(AMF_H264_BPICTURE_REFERENCE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);

	//////////////////////////////////////////////////////////////////////////
	// Encoder Miscellaneous Parameters
	//////////////////////////////////////////////////////////////////////////
	/// Scan Type
	list = obs_properties_add_list(props, AMF_H264_SCANTYPE, obs_module_text(AMF_H264_SCANTYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_SCANTYPE_PROGRESSIVE), VCEScanType_Progressive);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_SCANTYPE_INTERLACED), VCEScanType_Interlaced);
	/// Quality Preset
	list = obs_properties_add_list(props, AMF_H264_QUALITY_PRESET, obs_module_text(AMF_H264_QUALITY_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_QUALITY_PRESET_SPEED), VCEQualityPreset_Speed);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_QUALITY_PRESET_BALANCED), VCEQualityPreset_Balanced);
	obs_property_list_add_int(list, obs_module_text(AMF_H264_QUALITY_PRESET_QUALITY), VCEQualityPreset_Quality);

	//////////////////////////////////////////////////////////////////////////
	// Encoder Motion Estimation Parameters
	//////////////////////////////////////////////////////////////////////////
	/// Motion Estimation
	list = obs_properties_add_list(props, AMF_H264ADVANCED_MOTIONESTIMATION, obs_module_text(AMF_H264ADVANCED_MOTIONESTIMATION), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_H264ADVANCED_MOTIONESTIMATION_NONE), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_H264ADVANCED_MOTIONESTIMATION_HALF), 1);
	obs_property_list_add_int(list, obs_module_text(AMF_H264ADVANCED_MOTIONESTIMATION_QUARTER), 2);
	obs_property_list_add_int(list, obs_module_text(AMF_H264ADVANCED_MOTIONESTIMATION_BOTH), 3);

	//////////////////////////////////////////////////////////////////////////
	// Debug
	//////////////////////////////////////////////////////////////////////////
	/// Debug Mode
	obs_properties_add_bool(props, AMF_H264_DEBUGTRACING, obs_module_text(AMF_H264_DEBUGTRACING));

	return props;
}

bool Plugin::Interface::H264Interface::reset_callback(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
	if (obs_data_get_bool(settings, AMF_H264ADVANCED_RESET) == false)
		return false;
	obs_data_set_bool(settings, AMF_H264ADVANCED_RESET, false);

	for (obs_property_t* el = obs_properties_first(props); obs_property_next(&el);) {
		const char* name = obs_property_name(el);

		if (!obs_data_has_default_value(settings, name))
			continue;

		obs_property_type type = obs_property_get_type(el);
		switch (type) {
			case OBS_PROPERTY_BOOL:
				obs_data_set_bool(settings, name, obs_data_get_default_bool(settings, name));
				break;
			case OBS_PROPERTY_INT:
				obs_data_set_int(settings, name, obs_data_get_default_int(settings, name));
				break;
			case OBS_PROPERTY_FLOAT:
				obs_data_set_double(settings, name, obs_data_get_default_double(settings, name));
				break;
			case OBS_PROPERTY_TEXT:
			case OBS_PROPERTY_PATH:
			case OBS_PROPERTY_FONT:
				obs_data_set_string(settings, name, obs_data_get_default_string(settings, name));
				break;
			case OBS_PROPERTY_LIST:
			case OBS_PROPERTY_EDITABLE_LIST:
			{
				obs_combo_format format = obs_property_list_format(el);
				switch (format) {
					case OBS_COMBO_FORMAT_INT:
						obs_data_set_int(settings, name, obs_data_get_default_int(settings, name));
						break;
					case OBS_COMBO_FORMAT_FLOAT:
						obs_data_set_double(settings, name, obs_data_get_default_double(settings, name));
						break;
					case OBS_COMBO_FORMAT_STRING:
						obs_data_set_string(settings, name, obs_data_get_default_string(settings, name));
						break;
				}
				break;
			}
			case OBS_PROPERTY_COLOR:
				break;
			case OBS_PROPERTY_BUTTON:
				break;
			case OBS_PROPERTY_FRAME_RATE:
				break;
			default:
				break;
		}
	}
	return true;
}

bool Plugin::Interface::H264Interface::update_from_amf(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
	if (obs_data_get_bool(settings, AMF_H264ADVANCED_UPDATE) == false)
		return false;
	obs_data_set_bool(settings, AMF_H264ADVANCED_UPDATE, false);
	try {
	//////////////////////////////////////////////////////////////////////////
	// Static Properties (Can't be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
		VCEEncoder* vce = new VCEEncoder(VCEEncoderType_AVC, VCEMemoryType_Host, VCESurfaceFormat_NV12);

		// Usage & Quality Preset
		int64_t usage = obs_data_get_int(settings, AMF_H264_USAGE);
		vce->SetUsage((VCEUsage)usage);
		try {
			int64_t preset = obs_data_get_int(settings, AMF_H264_QUALITY_PRESET);
			if (preset == -1)
				obs_data_set_int(settings, AMF_H264_QUALITY_PRESET, vce->GetQualityPreset());
			else
				vce->SetQualityPreset((VCEQualityPreset)preset);
		} catch (...) {}

		// Profile & Level
		try {
			if (obs_data_get_int(settings, AMF_H264_PROFILE) == -1)
				obs_data_set_int(settings, AMF_H264_PROFILE, vce->GetProfile());
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264_PROFILELEVEL) == -1)
				obs_data_set_int(settings, AMF_H264_PROFILELEVEL, vce->GetProfileLevel());
		} catch (...) {}

		// Other
		try {
			if (obs_data_get_int(settings, AMF_H264ADVANCED_MAX_LTR_FRAMES) == -1)
				obs_data_set_int(settings, AMF_H264ADVANCED_MAX_LTR_FRAMES, vce->GetMaxLTRFrames());
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264_SCANTYPE) == -1)
				obs_data_set_int(settings, AMF_H264_SCANTYPE, vce->GetScanType());
		} catch (...) {}

		//////////////////////////////////////////////////////////////////////////
		// Dynamic Properties (Can be changed during Encoding)
		//////////////////////////////////////////////////////////////////////////
		// Rate Control
		/// Method
		try {
			if (obs_data_get_int(settings, AMF_H264_RATECONTROLMETHOD) == -1)
				obs_data_set_int(settings, AMF_H264_RATECONTROLMETHOD, vce->GetRateControlMethod());
		} catch (...) {}
			/// Skip Frames if necessary
		try {
			if (obs_data_get_int(settings, AMF_H264_FRAMESKIPPING) == -1)
				obs_data_set_int(settings, AMF_H264_FRAMESKIPPING, vce->IsRateControlSkipFrameEnabled() ? 1 : 0);
		} catch (...) {}
		// Rate Control - Other
		/// Filler Data
		try {
			if (obs_data_get_int(settings, AMF_H264_FILLERDATA) == -1)
				obs_data_set_int(settings, AMF_H264_FILLERDATA, vce->IsFillerDataEnabled() ? 1 : 0);
		} catch (...) {}
		/// Enforce Hyptohecial Reference Decoder Compatability
		try {
			if (obs_data_get_int(settings, AMF_H264_ENFORCEHRDCOMPATIBILITY) == -1)
				obs_data_set_int(settings, AMF_H264_ENFORCEHRDCOMPATIBILITY, vce->IsEnforceHRDRestrictionsEnabled() ? 1 : 0);
		} catch (...) {}
		// Video Coding Settings
		/// VBV Buffer
		try {
			if (obs_data_get_int(settings, AMF_H264ADVANCED_VBVBUFFER_SIZE) == -1) {
				obs_data_set_int(settings, AMF_H264ADVANCED_VBVBUFFER_SIZE, vce->GetVBVBufferSize());
				obs_data_set_double(settings, AMF_H264ADVANCED_VBVBUFFER_FULLNESS, vce->GetInitialVBVBufferFullness());
			}
		} catch (...) {}
		/// Max AU Size
		try {
			if (obs_data_get_int(settings, AMF_H264ADVANCED_MAX_AU_SIZE) == -1)
				obs_data_set_int(settings, AMF_H264ADVANCED_MAX_AU_SIZE, vce->GetMaximumAccessUnitSize());
		} catch (...) {}
		/// B-Picture Related
		try {
			if (obs_data_get_int(settings, AMF_H264_QP_BPICTURE_DELTA) == -11)
				obs_data_set_int(settings, AMF_H264_QP_BPICTURE_DELTA, vce->GetBPictureDeltaQP());
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264_QP_REFERENCE_BPICTURE_DELTA) == -11)
				obs_data_set_int(settings, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, vce->GetReferenceBPictureDeltaQP());
		} catch (...) {}
		// Rate Control: Constrained QP
		try {
			if (obs_data_get_int(settings, AMF_H264_QP_MINIMUM) == -1)
				obs_data_set_int(settings, AMF_H264_QP_MINIMUM, vce->GetMinimumQP());
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264_QP_MAXIMUM) == -1)
				obs_data_set_int(settings, AMF_H264_QP_MAXIMUM, vce->GetMaximumQP());
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264_QP_IFRAME) == -1)
				obs_data_set_int(settings, AMF_H264_QP_IFRAME, vce->GetIFrameQP());
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264_QP_PFRAME) == -1)
				obs_data_set_int(settings, AMF_H264_QP_PFRAME, vce->GetPFrameQP());
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264_QP_BFRAME) == -1)
				obs_data_set_int(settings, AMF_H264_QP_BFRAME, vce->GetBFrameQP());
		} catch (...) {}
		// Rate Control: CBR, VBR
		try {
			if (obs_data_get_int(settings, AMF_H264_BITRATE_TARGET) == -1)
				obs_data_set_int(settings, AMF_H264_BITRATE_TARGET, vce->GetTargetBitrate());
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264_BITRATE_PEAK) == -1)
				obs_data_set_int(settings, AMF_H264_BITRATE_PEAK, vce->GetPeakBitrate());
		} catch (...) {}

		// Picture Control Properties
		try {
			if (obs_data_get_int(settings, AMF_H264ADVANCED_HEADER_INSERTION_SPACING) == -1)
				obs_data_set_int(settings, AMF_H264ADVANCED_HEADER_INSERTION_SPACING, vce->GetHeaderInsertionSpacing());
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264_BPICTURE_PATTERN) == -1)
				obs_data_set_int(settings, AMF_H264_BPICTURE_PATTERN, vce->GetBPicturesPattern());
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264_DEBLOCKINGFILTER) == -1)
				obs_data_set_int(settings, AMF_H264_DEBLOCKINGFILTER, vce->IsDeBlockingFilterEnabled() ? 1 : 0);
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264_BPICTURE_REFERENCE) == -1)
				obs_data_set_int(settings, AMF_H264_BPICTURE_REFERENCE, vce->IsBReferenceEnabled() ? 1 : 0);
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264ADVANCED_IDR_PERIOD) == -1)
				obs_data_set_int(settings, AMF_H264ADVANCED_IDR_PERIOD, vce->GetIDRPeriod());
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264ADVANCED_INTRAREFRESHNUMMBPERSLOT) == -1)
				obs_data_set_int(settings, AMF_H264ADVANCED_INTRAREFRESHNUMMBPERSLOT, vce->GetIntraRefreshMBsNumberPerSlot());
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264ADVANCED_SLICESPERFRAME) == -1)
				obs_data_set_int(settings, AMF_H264ADVANCED_SLICESPERFRAME, vce->GetSlicesPerFrame());
		} catch (...) {}

		// Motion Estimation
		try {
			if (obs_data_get_int(settings, AMF_H264ADVANCED_MOTIONESTIMATION) == -1)
				obs_data_set_int(settings, AMF_H264ADVANCED_MOTIONESTIMATION, (vce->IsHalfPixelMotionEstimationEnabled() ? 1 : 0) + (vce->IsQuarterPixelMotionEstimationEnabled() ? 2 : 0));
		} catch (...) {}
		
		// Remove Instance again.
		delete vce;
	} catch (...) {
	}
	return true;
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
Plugin::Interface::H264Interface::H264Interface(obs_data_t* settings, obs_encoder_t* encoder) {
	int64_t value;

	AMF_LOG_INFO("<AMFEncoder::H264Interface::H264Interface> Initializing...");

	// OBS Settings
	uint32_t m_cfgWidth = obs_encoder_get_width(encoder);
	uint32_t m_cfgHeight = obs_encoder_get_height(encoder);
	video_t *video = obs_encoder_video(encoder);
	const struct video_output_info *voi = video_output_get_info(video);
	uint32_t m_cfgFPSnum = voi->fps_num;
	uint32_t m_cfgFPSden = voi->fps_den;

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

	//////////////////////////////////////////////////////////////////////////
	// Static Properties (Can't be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	// Type, Memory Type, Surface Format
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
	m_VideoEncoder = new VCEEncoder(VCEEncoderType_AVC, VCEMemoryType_Host, format);
	
	/// Usage & Quality Preset
	m_VideoEncoder->SetUsage((VCEUsage)obs_data_get_int(settings, AMF_H264_USAGE));
	value = obs_data_get_int(settings, AMF_H264_QUALITY_PRESET);
	if (value != -1)
		m_VideoEncoder->SetQualityPreset((VCEQualityPreset)value);

	// Profile & Level
	/// Profile
	value = obs_data_get_int(settings, AMF_H264_PROFILE);
	if (value != -1)
		m_VideoEncoder->SetProfile((VCEProfile)value);
	/// Profile Level
	value = obs_data_get_int(settings, AMF_H264_PROFILELEVEL);
	if (value != -1)
		m_VideoEncoder->SetProfileLevel((VCEProfileLevel)value);

	// Framesize & Framerate
	m_VideoEncoder->SetFrameSize(m_cfgWidth, m_cfgHeight);
	m_VideoEncoder->SetFrameRate(m_cfgFPSnum, m_cfgFPSden);

	// Other
	/// Maximum Long-Term-Reference Frames
	value = obs_data_get_int(settings, AMF_H264ADVANCED_MAX_LTR_FRAMES);
	if (value != -1)
		m_VideoEncoder->SetMaxLTRFrames((uint32_t)value);
	/// Scan Type
	value = obs_data_get_int(settings, AMF_H264_SCANTYPE);
	if (value != -1)
		m_VideoEncoder->SetScanType((VCEScanType)value);

	////////////////////////////////////////////////////////////////////////////
	//// Dynamic Properties (Can be changed during Encoding)
	////////////////////////////////////////////////////////////////////////////
	update_properties(settings);

	//////////////////////////////////////////////////////////////////////////
	// Verify
	//////////////////////////////////////////////////////////////////////////
	m_VideoEncoder->LogProperties();

	//////////////////////////////////////////////////////////////////////////
	// Initialize (locks static properties)
	//////////////////////////////////////////////////////////////////////////
	m_VideoEncoder->Start();

	AMF_LOG_INFO("<AMFEncoder::H264Interface::H264Interface> Complete.");
}

Plugin::Interface::H264Interface::~H264Interface() {
	AMF_LOG_INFO("<AMFEncoder::H264Interface::~H264Interface> Finalizing...");
	m_VideoEncoder->Stop();
	delete m_VideoEncoder;
	AMF_LOG_INFO("<AMFEncoder::H264Interface::~H264Interface> Complete.");
}

bool Plugin::Interface::H264Interface::update(obs_data_t* settings) {
	return update_properties(settings);
}

bool Plugin::Interface::H264Interface::encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	if (!frame || !packet || !received_packet)
		return false;

	bool retVal = true;

	retVal = m_VideoEncoder->SendInput(frame);
	m_VideoEncoder->GetOutput(packet, received_packet);

	return retVal;
}

void Plugin::Interface::H264Interface::get_video_info(struct video_scale_info* info) {
	m_VideoEncoder->GetVideoInfo(info);
}

bool Plugin::Interface::H264Interface::get_extra_data(uint8_t** extra_data, size_t* size) {
	return m_VideoEncoder->GetExtraData(extra_data, size);
}

bool Plugin::Interface::H264Interface::update_properties(obs_data_t* settings) {
	int64_t value; double_t valued;

	//////////////////////////////////////////////////////////////////////////
	// Dynamic Properties (Can be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	// Rate Control
	/// Method
	try {
		value = obs_data_get_int(settings, AMF_H264_RATECONTROLMETHOD);
		if (value != -1)
			m_VideoEncoder->SetRateControlMethod((VCERateControlMethod)value);
	} catch (...) {}
	/// Frame Skipping
	try {
		value = obs_data_get_int(settings, AMF_H264_FRAMESKIPPING);
		if (value != -1)
			m_VideoEncoder->SetRateControlSkipFrameEnabled(value == 1);
	} catch (...) {}

	// Rate Control - Other
	/// Enable Filler Data
	try {
		value = obs_data_get_int(settings, AMF_H264_FILLERDATA);
		if (value != -1)
			m_VideoEncoder->SetFillerDataEnabled(value == 1);
	} catch (...) {}
	/// Enforce HRD
	try {
		value = obs_data_get_int(settings, AMF_H264_ENFORCEHRDCOMPATIBILITY);
		if (value != -1)
			m_VideoEncoder->SetEnforceHRDRestrictionsEnabled(value == 1);
	} catch (...) {}

	// Video Coding Settings
	/// VBV Buffer Size
	try {
		value = obs_data_get_int(settings, AMF_H264ADVANCED_VBVBUFFER_SIZE);
		if (value != -1)
			m_VideoEncoder->SetVBVBufferSize((uint32_t)value);
	} catch (...) {}
	/// Initial VBV Buffer Fullness
	try {
		valued = obs_data_get_double(settings, AMF_H264ADVANCED_VBVBUFFER_FULLNESS);
		m_VideoEncoder->SetInitialVBVBufferFullness(valued);
	} catch (...) {}
	/// Max AU Size
	try {
		value = obs_data_get_int(settings, AMF_H264ADVANCED_MAX_AU_SIZE);
		if (value != -1)
			m_VideoEncoder->SetMaximumAccessUnitSize((uint32_t)value);
	} catch (...) {}

	// Unknown Rate Control
	/// Minimum & Maximum QP
	try {
		value = obs_data_get_int(settings, AMF_H264_QP_MINIMUM);
		if (value != -1)
			m_VideoEncoder->SetMinimumQP((uint8_t)value);
	} catch (...) {}
	try {
		value = obs_data_get_int(settings, AMF_H264_QP_MAXIMUM);
		if (value != -1)
			m_VideoEncoder->SetMaximumQP((uint8_t)value);
	} catch (...) {}

	// Rate Control: CBR, VBR
	/// Target Bitrate
	try {
		value = obs_data_get_int(settings, AMF_H264_BITRATE_TARGET);
		if (value != -1)
			m_VideoEncoder->SetTargetBitrate((uint32_t)value);
	} catch (...) {}
	/// Peak Bitrate
	try {
		value = obs_data_get_int(settings, AMF_H264_BITRATE_PEAK);
		if (value != -1)
			m_VideoEncoder->SetPeakBitrate((uint32_t)value);
	} catch (...) {}
	// Rate Control: CQP
	/// I-, P-, B-Frame QP
	try {
		value = obs_data_get_int(settings, AMF_H264_QP_IFRAME);
		if (value != -1)
			m_VideoEncoder->SetIFrameQP((uint8_t)value);
	} catch (...) {}
	try {
		value = obs_data_get_int(settings, AMF_H264_QP_PFRAME);
		if (value != -1)
			m_VideoEncoder->SetPFrameQP((uint8_t)value);
	} catch (...) {}
	try {
		value = obs_data_get_int(settings, AMF_H264_QP_BFRAME);
		if (value != -1)
			m_VideoEncoder->SetBFrameQP((uint8_t)value);
	} catch (...) {}
	/// B-Picture Delta QP
	try {
		value = obs_data_get_int(settings, AMF_H264_QP_BPICTURE_DELTA);
		if (value != -11)
			m_VideoEncoder->SetBPictureDeltaQP((int8_t)value);
	} catch (...) {}
	/// Ref B-Picture Delta QP
	try {
		value = obs_data_get_int(settings, AMF_H264_QP_REFERENCE_BPICTURE_DELTA);
		if (value != -11)
			m_VideoEncoder->SetReferenceBPictureDeltaQP((int8_t)value);
	} catch (...) {}

	// Picture Control Properties
	/// Header Insertion Spacing
	try {
		value = obs_data_get_int(settings, AMF_H264ADVANCED_HEADER_INSERTION_SPACING);
		if (value != -1)
			m_VideoEncoder->SetHeaderInsertionSpacing((uint32_t)value);
	} catch (...) {}
	/// B-Pictures Pattern
	try {
		value = obs_data_get_int(settings, AMF_H264_BPICTURE_PATTERN);
		if (value != -1)
			m_VideoEncoder->SetBPicturePattern((VCEBPicturePattern)value);
	} catch (...) {}
	/// De-Blocking Filter
	try {
		value = obs_data_get_int(settings, AMF_H264_DEBLOCKINGFILTER);
		if (value != -1)
			m_VideoEncoder->SetDeBlockingFilterEnabled(value == 1);
	} catch (...) {}
	/// Enable Reference to B-Frames (2nd Generation GCN and newer)
	try {
		value = obs_data_get_int(settings, AMF_H264_BPICTURE_REFERENCE);
		if (value != -1)
			m_VideoEncoder->SetBPictureReferenceEnabled(value == 1);
	} catch (...) {}
	/// IDR Period (Is this Keyframe distance?)
	try {
		value = obs_data_get_int(settings, AMF_H264ADVANCED_IDR_PERIOD);
		if (value != -1)
			m_VideoEncoder->SetIDRPeriod((uint32_t)value);
	} catch (...) {}
	/// Intra Refresh MBs Number Per Slot in Macroblocks
	try {
		value = obs_data_get_int(settings, AMF_H264ADVANCED_INTRAREFRESHNUMMBPERSLOT);
		if (value != -1)
			m_VideoEncoder->SetIntraRefreshMBsNumberPerSlot((uint32_t)value);
	} catch (...) {}
	/// Number of slices Per Frame
	try {
		value = obs_data_get_int(settings, AMF_H264ADVANCED_SLICESPERFRAME);
		if (value != -1)
			m_VideoEncoder->SetSlicesPerFrame((uint32_t)value);
	} catch (...) {}

	// Motion EstimationerSlotInMacroblocks(value);
	/// Number of slices Per Frame
	try {
		value = obs_data_get_int(settings, AMF_H264ADVANCED_MOTIONESTIMATION);
		if (value != -1) {
			m_VideoEncoder->SetHalfPixelMotionEstimationEnabled(!!(value & 0x1));
			m_VideoEncoder->SetQuarterPixelMotionEstimationEnabled(!!(value & 0x2));
		}
	} catch (...) {}

	return true;
}
