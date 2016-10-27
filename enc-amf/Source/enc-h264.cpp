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

#if (defined _WIN32) | (defined _WIN64)
#include <VersionHelpers.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin;
using namespace Plugin::AMD;
using namespace Plugin::Interface;

void Plugin::Interface::H264Interface::encoder_register() {
	static obs_encoder_info* encoder_info = new obs_encoder_info();
	static const char* encoder_name = "amd_amf_h264";
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

const char* Plugin::Interface::H264Interface::get_name(void*) {
	static const char* name = "H264 Encoder (AMD Advanced Media Framework)";
	return name;
}

void* Plugin::Interface::H264Interface::create(obs_data_t* settings, obs_encoder_t* encoder) {
	try {
		Plugin::Interface::H264Interface* enc = new Plugin::Interface::H264Interface(settings, encoder);
		return enc;
	} catch (...) {
		AMF_LOG_ERROR("Unable to create Encoder, see log for more information.");
		return NULL;
	}
}

#pragma warning( push )
#pragma warning( disable: 4702 )
void Plugin::Interface::H264Interface::destroy(void* data) {
	try {
		Plugin::Interface::H264Interface* enc = static_cast<Plugin::Interface::H264Interface*>(data);
		delete enc;
		data = nullptr;
	} catch (...) {
		AMF_LOG_ERROR("Unable to destroy Encoder, see log for more information.");
	}
}
#pragma warning( pop )

bool Plugin::Interface::H264Interface::encode(void *data, struct encoder_frame *frame, struct encoder_packet *packet, bool *received_packet) {
	try {
		return static_cast<Plugin::Interface::H264Interface*>(data)->encode(frame, packet, received_packet);
	} catch (...) {
		AMF_LOG_ERROR("Unable to encode, see log for more information.");
		return false;
	}
}

void Plugin::Interface::H264Interface::get_defaults(obs_data_t *data) {
	#pragma region OBS - Enforce Streaming Service Restrictions
	// OBS - Enforce Streaming Service Restrictions
	obs_data_set_default_int(data, "bitrate", -1);
	obs_data_set_default_int(data, "keyint_sec", -1);
	obs_data_set_default_string(data, "rate_control", "");
	obs_data_set_default_string(data, "profile", "");
	obs_data_set_int(data, "bitrate", -1);
	obs_data_set_int(data, "keyint_sec", -1);
	obs_data_set_string(data, "rate_control", "");
	obs_data_set_string(data, "profile", "");
	#pragma endregion OBS - Enforce Streaming Service Restrictions

	// Static Properties
	obs_data_set_default_int(data, AMF_H264_USAGE, VCEUsage_Transcoding);
	obs_data_set_default_int(data, AMF_H264_PROFILE, VCEProfile_Main);
	obs_data_set_default_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_Automatic);
	obs_data_set_default_int(data, AMF_H264_MAXIMUMLTRFRAMES, 0);

	// Rate Control Properties
	obs_data_set_default_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantBitrate);
	obs_data_set_default_int(data, AMF_H264_BITRATE_TARGET, 3500);
	obs_data_set_default_int(data, AMF_H264_BITRATE_PEAK, 9000);
	obs_data_set_default_int(data, AMF_H264_QP_MINIMUM, 1);
	obs_data_set_default_int(data, AMF_H264_QP_MAXIMUM, 51);
	obs_data_set_default_int(data, AMF_H264_QP_IFRAME, 22);
	obs_data_set_default_int(data, AMF_H264_QP_PFRAME, 22);
	obs_data_set_default_int(data, AMF_H264_QP_BFRAME, 22);
	obs_data_set_default_int(data, AMF_H264_QP_BPICTURE_DELTA, 0);
	obs_data_set_default_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, 0);
	obs_data_set_default_int(data, AMF_H264_VBVBUFFER, 0);
	obs_data_set_default_int(data, AMF_H264_VBVBUFFER_SIZE, 0);
	obs_data_set_default_double(data, AMF_H264_VBVBUFFER_STRICTNESS, 50);
	obs_data_set_default_double(data, AMF_H264_VBVBUFFER_FULLNESS, 50);
	obs_data_set_default_int(data, AMF_H264_MAXIMUMACCESSUNITSIZE, 0);
	obs_data_set_default_bool(data, AMF_H264_FILLERDATA, true);
	obs_data_set_default_bool(data, AMF_H264_FRAMESKIPPING, false);
	obs_data_set_default_bool(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, false);

	// Picture Control Properties
	obs_data_set_default_int(data, AMF_H264_KEYFRAME_INTERVAL, 2);
	obs_data_set_default_int(data, AMF_H264_IDR_PERIOD, 60);
	obs_data_set_default_int(data, AMF_H264_HEADER_INSERTION_SPACING, 0);
	obs_data_set_default_int(data, AMF_H264_BPICTURE_PATTERN, (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames ? VCEBPicturePattern_Three : VCEBPicturePattern_None));
	obs_data_set_default_bool(data, AMF_H264_BPICTURE_REFERENCE, (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames ? true : false));
	obs_data_set_default_int(data, AMF_H264_SLICESPERFRAME, 0);
	obs_data_set_default_int(data, AMF_H264_INTRAREFRESHNUMMBSPERSLOT, 0);

	// Miscellaneous Control Properties
	obs_data_set_default_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Balanced);
	obs_data_set_default_int(data, AMF_H264_SCANTYPE, VCEScanType_Progressive);
	obs_data_set_default_int(data, AMF_H264_MOTIONESTIMATION, 3);
	obs_data_set_default_bool(data, AMF_H264_CABAC, false);

	// System Properties
	obs_data_set_default_int(data, AMF_H264_MEMORYTYPE, VCEMemoryType_Host);
	obs_data_set_default_int(data, AMF_H264_COMPUTETYPE, VCEComputeType_None);
	obs_data_set_default_int(data, AMF_H264_SURFACEFORMAT, -1);
	obs_data_set_default_int(data, AMF_H264_VIEW, 0);
	obs_data_set_default_bool(data, AMF_H264_UNLOCK_PROPERTIES, false);
	obs_data_set_default_bool(data, AMF_H264_DEBUG, false);
}

obs_properties_t* Plugin::Interface::H264Interface::get_properties(void*) {
	obs_properties* props = obs_properties_create();
	obs_property_t* p;

	#pragma region Static Properties
	#pragma region Usage
	p = obs_properties_add_list(props, AMF_H264_USAGE, obs_module_text(AMF_H264_USAGE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, obs_module_text(AMF_H264_USAGE_DESCRIPTION));
	obs_property_list_add_int(p, obs_module_text(AMF_H264_USAGE_TRANSCODING), VCEUsage_Transcoding);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_USAGE_ULTRALOWLATENCY), VCEUsage_UltraLowLatency);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_USAGE_LOWLATENCY), VCEUsage_LowLatency);
	//obs_property_list_add_int(list, obs_module_text(AMF_H264_USAGE_WEBCAM), VCEUsage_Webcam); // Requires SVC? SVC is not implemented by default.
	#pragma endregion Usage
	#pragma region Profile
	p = obs_properties_add_list(props, AMF_H264_PROFILE, obs_module_text(AMF_H264_PROFILE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, obs_module_text(AMF_H264_PROFILE_DESCRIPTION));
	switch (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxProfile) {
		case 100:
			obs_property_list_add_int(p, "High", VCEProfile_High);
		case 77:
			obs_property_list_add_int(p, "Main", VCEProfile_Main);
		case 66:
			obs_property_list_add_int(p, "Baseline", VCEProfile_Baseline);
			break;
	}
	#pragma endregion Profile
	#pragma region Profile Level
	p = obs_properties_add_list(props, AMF_H264_PROFILELEVEL, obs_module_text(AMF_H264_PROFILELEVEL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, obs_module_text(AMF_H264_PROFILELEVEL_DESCRIPTION));
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_AUTOMATIC), VCEProfileLevel_Automatic);
	switch (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxProfileLevel) {
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
	#pragma endregion Profile Levels
	#pragma region Long Term Reference Frames
	obs_properties_add_int_slider(props, AMF_H264_MAXIMUMLTRFRAMES, obs_module_text(AMF_H264_MAXIMUMLTRFRAMES), 0, 2, 1);
	obs_property_set_long_description(p, obs_module_text(AMF_H264_MAXIMUMLTRFRAMES_DESCRIPTION));
	#pragma endregion Long Term Reference Frames
	#pragma endregion Static Properties

	#pragma region Rate Control Properties
	//p = obs_properties_add_bool(props, "rcp_delimiter", "------ Rate Control Properties ------");
	#pragma region Method
	p = obs_properties_add_list(props, AMF_H264_RATECONTROLMETHOD, obs_module_text(AMF_H264_RATECONTROLMETHOD), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_RATECONTROLMETHOD_CQP), VCERateControlMethod_ConstantQP);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_RATECONTROLMETHOD_CBR), VCERateControlMethod_ConstantBitrate);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_RATECONTROLMETHOD_VBR), VCERateControlMethod_VariableBitrate_PeakConstrained);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_RATECONTROLMETHOD_VBR_LAT), VCERateControlMethod_VariableBitrate_LatencyConstrained);
	obs_property_set_modified_callback(p, rate_control_method_modified);
	#pragma endregion Method
	#pragma region Method Parameters
	/// Bitrate Constraints
	obs_properties_add_int(props, AMF_H264_BITRATE_TARGET, obs_module_text(AMF_H264_BITRATE_TARGET), 0, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate, 1);
	obs_properties_add_int(props, AMF_H264_BITRATE_PEAK, obs_module_text(AMF_H264_BITRATE_PEAK), 0, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate, 1);
	/// Minimum QP, Maximum QP
	obs_properties_add_int_slider(props, AMF_H264_QP_MINIMUM, obs_module_text(AMF_H264_QP_MINIMUM), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_H264_QP_MAXIMUM, obs_module_text(AMF_H264_QP_MAXIMUM), 0, 51, 1);
	/// Method: Constant QP
	obs_properties_add_int_slider(props, AMF_H264_QP_IFRAME, obs_module_text(AMF_H264_QP_IFRAME), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_H264_QP_PFRAME, obs_module_text(AMF_H264_QP_PFRAME), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_H264_QP_BFRAME, obs_module_text(AMF_H264_QP_BFRAME), 0, 51, 1);
	/// B-Picture Related
	obs_properties_add_int_slider(props, AMF_H264_QP_BPICTURE_DELTA, obs_module_text(AMF_H264_QP_BPICTURE_DELTA), -10, 10, 1);
	obs_properties_add_int_slider(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, obs_module_text(AMF_H264_QP_REFERENCE_BPICTURE_DELTA), -10, 10, 1);
	#pragma endregion Method Parameters
	#pragma region VBV Buffer
	p = obs_properties_add_list(props, AMF_H264_VBVBUFFER, obs_module_text(AMF_H264_VBVBUFFER), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_AUTOMATIC), 0);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_MANUAL), 1);
	obs_properties_add_int_slider(props, AMF_H264_VBVBUFFER_SIZE, obs_module_text(AMF_H264_VBVBUFFER_SIZE), -1, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate, 1);
	obs_properties_add_float_slider(props, AMF_H264_VBVBUFFER_FULLNESS, obs_module_text(AMF_H264_VBVBUFFER_FULLNESS), 0.0, 100.0, 100.0 / 64.0);
	obs_properties_add_float_slider(props, AMF_H264_VBVBUFFER_STRICTNESS, obs_module_text(AMF_H264_VBVBUFFER_STRICTNESS), 0.0, 100.0, 0.1);
	#pragma endregion VBV Buffer
	/// Max Access Unit Size
	obs_properties_add_int_slider(props, AMF_H264_MAXIMUMACCESSUNITSIZE, obs_module_text(AMF_H264_MAXIMUMACCESSUNITSIZE), 0, 100000000, 1);
	#pragma region Flags
	/// Filler Data (Only supported by CBR so far)
	p = obs_properties_add_list(props, AMF_H264_FILLERDATA, obs_module_text(AMF_H264_FILLERDATA), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	/// Frame Skipping
	p = obs_properties_add_list(props, AMF_H264_FRAMESKIPPING, obs_module_text(AMF_H264_FRAMESKIPPING), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	/// Enforce Hypothetical Reference Decoder Compatibility
	p = obs_properties_add_list(props, AMF_H264_ENFORCEHRDCOMPATIBILITY, obs_module_text(AMF_H264_ENFORCEHRDCOMPATIBILITY), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	#pragma endregion Flags
	#pragma endregion Rate Control Properties

	#pragma region Picture Control Properties
	//p = obs_properties_add_bool(props, "pcp_delimiter", "------ Picture Control Properties ------");
	#pragma region IDR Period / Keyframe Interval / Header Insertion Spacing
	obs_properties_add_int(props, AMF_H264_KEYFRAME_INTERVAL, obs_module_text(AMF_H264_KEYFRAME_INTERVAL), 1, 100, 1);
	obs_properties_add_int(props, AMF_H264_IDR_PERIOD, obs_module_text(AMF_H264_IDR_PERIOD), 1, 1000, 1);
	obs_properties_add_int(props, AMF_H264_HEADER_INSERTION_SPACING, obs_module_text(AMF_H264_HEADER_INSERTION_SPACING), 0, 1000, 1);
	#pragma endregion IDR Period / Keyframe Interval / Header Insertion Spacing
	#pragma region B-Pictures
	/// B-Pictures Pattern
	p = obs_properties_add_int_slider(props, AMF_H264_BPICTURE_PATTERN, obs_module_text(AMF_H264_BPICTURE_PATTERN), 0, (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames ? 3 : 0), 1);
	obs_property_set_modified_callback(p, bpictures_modified);
	/// Enable Reference to B-Frames (2nd Generation GCN and newer)
	p = obs_properties_add_list(props, AMF_H264_BPICTURE_REFERENCE, obs_module_text(AMF_H264_BPICTURE_REFERENCE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	#pragma endregion B-Pictures
	/// De-Blocking Filter
	p = obs_properties_add_list(props, AMF_H264_DEBLOCKINGFILTER, obs_module_text(AMF_H264_DEBLOCKINGFILTER), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	/// Number of Slices Per Frame 
	obs_properties_add_int_slider(props, AMF_H264_SLICESPERFRAME, obs_module_text(AMF_H264_SLICESPERFRAME), 0, 65535, 1);
	/// Intra Refresh Number of Macro Blocks per Slot
	obs_properties_add_int_slider(props, AMF_H264_INTRAREFRESHNUMMBSPERSLOT, obs_module_text(AMF_H264_INTRAREFRESHNUMMBSPERSLOT), 0, 65535, 1);
	#pragma endregion Picture Control Properties

	#pragma region Miscellaneous Control Properties
	//p = obs_properties_add_bool(props, "msc_delimiter", "------ Miscellaneous Properties ------");
	/// Quality Preset
	p = obs_properties_add_list(props, AMF_H264_QUALITY_PRESET, obs_module_text(AMF_H264_QUALITY_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_QUALITY_PRESET_SPEED), VCEQualityPreset_Speed);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_QUALITY_PRESET_BALANCED), VCEQualityPreset_Balanced);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_QUALITY_PRESET_QUALITY), VCEQualityPreset_Quality);
	/// Scan Type
	p = obs_properties_add_list(props, AMF_H264_SCANTYPE, obs_module_text(AMF_H264_SCANTYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_SCANTYPE_PROGRESSIVE), VCEScanType_Progressive);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_SCANTYPE_INTERLACED), VCEScanType_Interlaced);
	/// Motion Estimation
	p = obs_properties_add_list(props, AMF_H264_MOTIONESTIMATION, obs_module_text(AMF_H264_MOTIONESTIMATION), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_MOTIONESTIMATION_NONE), 0);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_MOTIONESTIMATION_HALF), 1);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_MOTIONESTIMATION_QUARTER), 2);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_MOTIONESTIMATION_BOTH), 3);

	/// CABAC
	p = obs_properties_add_list(props, AMF_H264_CABAC, obs_module_text(AMF_H264_CABAC), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);
	#pragma endregion Miscellaneous Control Properties

	#pragma region System Properties
	//p = obs_properties_add_bool(props, "sys_delimiter", "------ System Properties ------");
	/// Memory Type
	p = obs_properties_add_list(props, AMF_H264_MEMORYTYPE, obs_module_text(AMF_H264_MEMORYTYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, obs_module_text(AMF_UTIL_AUTOMATIC), VCEMemoryType_Auto);
	obs_property_list_add_int(p, "Host", VCEMemoryType_Host);
	#if defined(WIN32) || defined(WIN64)
	if (IsWindowsXPOrGreater()) {
		obs_property_list_add_int(p, "DirectX 9", VCEMemoryType_DirectX9);
		if (IsWindows8OrGreater()) {
			obs_property_list_add_int(p, "DirectX 11", VCEMemoryType_DirectX11);
		}
	}
	#endif
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

	/// View Mode
	p = obs_properties_add_list(props, AMF_H264_VIEW, obs_module_text(AMF_H264_VIEW), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_VIEW_BASIC), 0);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_VIEW_ADVANCED), 1);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_VIEW_EXPERT), 2);
	obs_property_list_add_int(p, obs_module_text(AMF_H264_VIEW_MASTER), 3);
	obs_property_set_modified_callback(p, view_modified);
	/// Unlock Properties to full range.
	p = obs_properties_add_bool(props, AMF_H264_UNLOCK_PROPERTIES, obs_module_text(AMF_H264_UNLOCK_PROPERTIES));
	obs_property_set_modified_callback(p, unlock_properties_modified);

	/// Debug
	p = obs_properties_add_bool(props, AMF_H264_DEBUG, obs_module_text(AMF_H264_DEBUG));
	#pragma endregion System Properties

	return props;
}

bool Plugin::Interface::H264Interface::maximum_ltr_frames_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	return false;
}

bool Plugin::Interface::H264Interface::rate_control_method_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	return view_modified(props, nullptr, data);
}

bool Plugin::Interface::H264Interface::bpictures_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	return false;
}

bool Plugin::Interface::H264Interface::view_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	uint32_t last_view = (uint32_t)obs_data_get_int(data, "last_view");
	uint32_t view = (uint32_t)obs_data_get_int(data, AMF_H264_VIEW);
	bool vis_basic = view >= 0,
		vis_advanced = view >= 1,
		vis_expert = view >= 2,
		vis_master = view >= 3;

	if (last_view != view) {
		switch (last_view) {
			case 1: // Advanced

			case 2: // Expert

			case 3: // Master
				// Unlocking only works in Master View.
				obs_data_set_bool(data, AMF_H264_UNLOCK_PROPERTIES, false);


				break;
		}
	}

	bool unlock_properties = obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES);

	if (vis_master) {
		#pragma region Master View
		obs_property_set_visible(obs_properties_get(props, AMF_H264_USAGE), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_PROFILE), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_PROFILELEVEL), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_MAXIMUMLTRFRAMES), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_TARGET), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_PEAK), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_MINIMUM), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_MAXIMUM), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_IFRAME), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_PFRAME), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BFRAME), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BPICTURE_DELTA), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_VBVBUFFER), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_VBVBUFFER_SIZE), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_VBVBUFFER_FULLNESS), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_VBVBUFFER_STRICTNESS), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_FILLERDATA), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_FRAMESKIPPING), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_ENFORCEHRDCOMPATIBILITY), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_MAXIMUMACCESSUNITSIZE), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_IDR_PERIOD), unlock_properties);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), !unlock_properties);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_HEADER_INSERTION_SPACING), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_PATTERN), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_REFERENCE), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_DEBLOCKINGFILTER), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_SLICESPERFRAME), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_INTRAREFRESHNUMMBSPERSLOT), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QUALITY_PRESET), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_SCANTYPE), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_CABAC), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_DEBUG), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_MEMORYTYPE), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_COMPUTETYPE), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_SURFACEFORMAT), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_VIEW), true);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_UNLOCK_PROPERTIES), true);
		#pragma endregion Master View
	} else {
		obs_property_set_visible(obs_properties_get(props, AMF_H264_USAGE), vis_master);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_PROFILE), vis_advanced);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_PROFILELEVEL), vis_advanced);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_MAXIMUMLTRFRAMES), vis_master);

		/// Rate Control Properties
		obs_property_set_visible(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), vis_basic);
		bool vis_rcm_bitrate_target = false,
			vis_rcm_bitrate_peak = false,
			vis_rcm_qp_minmax = vis_advanced,
			vis_rcm_qp_ip = false,
			vis_rcm_qp_b = false,
			vis_rcm_fillerdata = false;
		switch ((VCERateControlMethod)obs_data_get_int(data, AMF_H264_RATECONTROLMETHOD)) {
			case VCERateControlMethod_ConstantBitrate:
				vis_rcm_bitrate_target = true;
				vis_rcm_fillerdata = true;
				break;
			case VCERateControlMethod_VariableBitrate_PeakConstrained:
				vis_rcm_bitrate_target = true;
				vis_rcm_bitrate_peak = true;
				break;
			case VCERateControlMethod_VariableBitrate_LatencyConstrained:
				vis_rcm_bitrate_target = true;
				vis_rcm_qp_minmax = true;
				break;
			case VCERateControlMethod_ConstantQP:
				vis_rcm_qp_ip = true;
				vis_rcm_qp_b = VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames
					&& (obs_data_get_int(data, AMF_H264_BPICTURE_PATTERN) > 0);
				vis_rcm_qp_minmax = false;
				break;
		}
		obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_TARGET), vis_rcm_bitrate_target);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_PEAK), vis_rcm_bitrate_peak);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_MINIMUM), vis_rcm_qp_minmax);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_MAXIMUM), vis_rcm_qp_minmax);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_IFRAME), vis_rcm_qp_ip);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_PFRAME), vis_rcm_qp_ip);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BFRAME), vis_rcm_qp_b);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BPICTURE_DELTA), vis_advanced && vis_rcm_qp_b);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA), vis_advanced && vis_rcm_qp_b);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_VBVBUFFER), vis_advanced);
		uint32_t vbvBuffer = (uint32_t)obs_data_get_int(data, AMF_H264_VBVBUFFER);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_VBVBUFFER_SIZE), vis_advanced && (vbvBuffer == 1));
		obs_property_set_visible(obs_properties_get(props, AMF_H264_VBVBUFFER_STRICTNESS), vis_advanced && (vbvBuffer == 0));
		obs_property_set_visible(obs_properties_get(props, AMF_H264_VBVBUFFER_FULLNESS), vis_expert && (vbvBuffer != -1));
		obs_property_set_visible(obs_properties_get(props, AMF_H264_FILLERDATA), vis_rcm_fillerdata);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_FRAMESKIPPING), vis_advanced);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_ENFORCEHRDCOMPATIBILITY), vis_expert);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_MAXIMUMACCESSUNITSIZE), vis_master);

		/// Picture Control Properties
		obs_property_set_visible(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), !vis_master);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_IDR_PERIOD), vis_master);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_HEADER_INSERTION_SPACING), vis_master);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_DEBLOCKINGFILTER), vis_advanced);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_INTRAREFRESHNUMMBSPERSLOT), vis_master);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_SLICESPERFRAME), vis_master);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_PATTERN), vis_advanced);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_REFERENCE), vis_advanced);

		/// Miscellaneous Properties
		obs_property_set_visible(obs_properties_get(props, AMF_H264_SCANTYPE), vis_expert);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_QUALITY_PRESET), vis_advanced);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), vis_expert);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_CABAC), vis_master);

		/// System Properties
		obs_property_set_visible(obs_properties_get(props, AMF_H264_DEBUG), vis_basic);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_MEMORYTYPE), vis_expert);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_COMPUTETYPE), vis_expert);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_SURFACEFORMAT), vis_expert);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_VIEW), vis_basic);
		obs_property_set_visible(obs_properties_get(props, AMF_H264_UNLOCK_PROPERTIES), vis_master);
	}

	return true;
}

bool Plugin::Interface::H264Interface::unlock_properties_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	bool unlocked = obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES);
	bool last_unlocked = obs_data_get_bool(data, "last_" vstr(AMF_H264_UNLOCK_PROPERTIES));

	if (last_unlocked != unlocked) {
		uint32_t multiplier = (unlocked == true ? 1000 : 1),
			divisor = (unlocked == false ? 1000 : 1);
		obs_data_set_int(data, AMF_H264_BITRATE_TARGET, (obs_data_get_int(data, AMF_H264_BITRATE_TARGET) * multiplier) / divisor);
		obs_data_set_int(data, AMF_H264_BITRATE_PEAK, (obs_data_get_int(data, AMF_H264_BITRATE_PEAK) * multiplier) / divisor);
		obs_data_set_int(data, AMF_H264_VBVBUFFER_SIZE, (obs_data_get_int(data, AMF_H264_VBVBUFFER_SIZE) * multiplier) / divisor);

		obs_data_set_bool(data, "last_" vstr(AMF_H264_UNLOCK_PROPERTIES), unlocked);
	}

	if (unlocked) {
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_TARGET), 1000, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_PEAK), 1000, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_VBVBUFFER_SIZE), 1000, 100000000, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), 0, 100, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_IDR_PERIOD), 0, 1000, 1);
	} else {
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_TARGET), 1, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / 1000, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_PEAK), 1, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / 1000, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_VBVBUFFER_SIZE), 1, 100000, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), 1, 100, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_IDR_PERIOD), 1, 1000, 1);
	}

	return true;
}

bool Plugin::Interface::H264Interface::update(void *data, obs_data_t *settings) {
	try {
		return static_cast<Plugin::Interface::H264Interface*>(data)->update(settings);
	} catch (...) {
		AMF_LOG_ERROR("Unable to update Encoder, see log for more information.");
		return false;
	}
}

void Plugin::Interface::H264Interface::get_video_info(void *data, struct video_scale_info *info) {
	try {
		return static_cast<Plugin::Interface::H264Interface*>(data)->get_video_info(info);
	} catch (...) {
		AMF_LOG_ERROR("Unable to get video info, see log for more information.");
	}
}

bool Plugin::Interface::H264Interface::get_extra_data(void *data, uint8_t** extra_data, size_t* size) {
	try {
		return static_cast<Plugin::Interface::H264Interface*>(data)->get_extra_data(extra_data, size);
	} catch (...) {
		AMF_LOG_ERROR("Unable to get extra data, see log for more information.");
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
// Module Code
//////////////////////////////////////////////////////////////////////////
Plugin::Interface::H264Interface::H264Interface(obs_data_t* settings, obs_encoder_t* encoder) {
	AMF_LOG_INFO("<AMFEncoder::H264Interface::H264Interface> Initializing...");

	// OBS Settings
	uint32_t m_cfgWidth = obs_encoder_get_width(encoder);
	uint32_t m_cfgHeight = obs_encoder_get_height(encoder);
	video_t *video = obs_encoder_video(encoder);
	const struct video_output_info *voi = video_output_get_info(video);
	uint32_t m_cfgFPSnum = voi->fps_num;
	uint32_t m_cfgFPSden = voi->fps_den;


	uint32_t viewmode = (uint32_t)obs_data_get_int(settings, AMF_H264_VIEW);
	bool vis_advanced = (viewmode >= 1),
		vis_master = (viewmode >= 3);
	
	//////////////////////////////////////////////////////////////////////////
	/// Initialize Encoder
	Plugin::AMD::AMF::GetInstance()->EnableDebugTrace(obs_data_get_bool(settings, AMF_H264_DEBUG));
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
	m_VideoEncoder = new VCEEncoder(VCEEncoderType_AVC, surfFormat, (VCEMemoryType)obs_data_get_int(settings, AMF_H264_MEMORYTYPE), (VCEComputeType)obs_data_get_int(settings, AMF_H264_COMPUTETYPE));

	/// Static Properties
	if (vis_master)
		m_VideoEncoder->SetUsage((VCEUsage)obs_data_get_int(settings, AMF_H264_USAGE));
	else
		m_VideoEncoder->SetUsage(VCEUsage_Transcoding);
	if (vis_advanced)
		m_VideoEncoder->SetProfile((VCEProfile)obs_data_get_int(settings, AMF_H264_PROFILE));
	else
		m_VideoEncoder->SetProfile(VCEProfile_Main);
	if (vis_advanced)
		m_VideoEncoder->SetProfileLevel((VCEProfileLevel)obs_data_get_int(settings, AMF_H264_PROFILELEVEL));
	else
		m_VideoEncoder->SetProfileLevel(VCEProfileLevel_Automatic);
	if (vis_master)
		m_VideoEncoder->SetMaximumLongTermReferenceFrames((uint32_t)obs_data_get_int(settings, AMF_H264_MAXIMUMLTRFRAMES));
	else
		m_VideoEncoder->SetMaximumLongTermReferenceFrames(0);

	/// Framesize & Framerate
	m_VideoEncoder->SetFrameSize(m_cfgWidth, m_cfgHeight);
	m_VideoEncoder->SetFrameRate(m_cfgFPSnum, m_cfgFPSden);

	// Dynamic Properties (Can be changed during Encoding)
	this->update(settings);

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
		uint32_t fpsNum = m_VideoEncoder->GetFrameRate().first;
		uint32_t fpsDen = m_VideoEncoder->GetFrameRate().second;
		if (obs_data_get_int(settings, "keyint_sec") != -1) {
			m_VideoEncoder->SetIDRPeriod((uint32_t)(obs_data_get_int(settings, "keyint_sec") * ((double_t)fpsNum / (double_t)fpsDen)));
		} else {
			obs_data_set_int(settings, "keyint_sec", (uint64_t)(m_VideoEncoder->GetIDRPeriod() / ((double_t)fpsNum / (double_t)fpsDen)));
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Verify
	//////////////////////////////////////////////////////////////////////////
	if (obs_data_get_int(settings, AMF_H264_VIEW) >= 2) {
		AMF_LOG_ERROR("View Mode is set to Expert or Master, no support will be given.");
	}
	m_VideoEncoder->LogProperties();
	if (obs_data_get_int(settings, AMF_H264_VIEW) >= 2) {
		AMF_LOG_ERROR("View Mode is set to Expert or Master, no support will be given.");
	}

	//////////////////////////////////////////////////////////////////////////
	// Initialize (locks static properties)
	//////////////////////////////////////////////////////////////////////////
	m_VideoEncoder->Start();

	AMF_LOG_INFO("<AMFEncoder::H264Interface::H264Interface> Complete.");

	if (obs_data_get_int(settings, AMF_H264_VIEW) >= 2) {
		AMF_LOG_ERROR("Expert/Master View Mode being used. Don't bother supporting.");
	}
}

Plugin::Interface::H264Interface::~H264Interface() {
	AMF_LOG_INFO("<AMFEncoder::H264Interface::~H264Interface> Finalizing...");
	m_VideoEncoder->Stop();
	delete m_VideoEncoder;
	AMF_LOG_INFO("<AMFEncoder::H264Interface::~H264Interface> Complete.");
}

bool Plugin::Interface::H264Interface::update(obs_data_t* settings) {
	int32_t viewmode = (int32_t)obs_data_get_int(settings, AMF_H264_VIEW);
	bool vis_basic = (viewmode >= 0),
		vis_advanced = (viewmode >= 1),
		vis_expert = (viewmode >= 2),
		vis_master = (viewmode >= 3);
	bool unlocked = obs_data_get_bool(settings, AMF_H264_UNLOCK_PROPERTIES);
	double_t framerate = (double_t)m_VideoEncoder->GetFrameRate().first / (double_t)m_VideoEncoder->GetFrameRate().second;
	int32_t bitratemult = unlocked ? 1 : 1000;

	// Rate Control Properties
	VCERateControlMethod rcm = (VCERateControlMethod)obs_data_get_int(settings, AMF_H264_RATECONTROLMETHOD);
	if (unlocked || vis_basic)
		m_VideoEncoder->SetRateControlMethod(rcm);
	else
		m_VideoEncoder->SetRateControlMethod(VCERateControlMethod_VariableBitrate_LatencyConstrained);
	if (unlocked || (vis_basic && (rcm == VCERateControlMethod_ConstantBitrate || rcm == VCERateControlMethod_VariableBitrate_LatencyConstrained)))
		m_VideoEncoder->SetTargetBitrate((uint32_t)obs_data_get_int(settings, AMF_H264_BITRATE_TARGET) * bitratemult);
	else
		m_VideoEncoder->SetTargetBitrate(VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate);
	if (unlocked || (vis_basic && (rcm == VCERateControlMethod_VariableBitrate_PeakConstrained || rcm == VCERateControlMethod_VariableBitrate_LatencyConstrained)))
		m_VideoEncoder->SetPeakBitrate((uint32_t)obs_data_get_int(settings, AMF_H264_BITRATE_PEAK) * bitratemult);
	else
		m_VideoEncoder->SetPeakBitrate(VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate);
	if (unlocked || (vis_basic && (rcm != VCERateControlMethod_ConstantQP)))
		m_VideoEncoder->SetMinimumQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_MINIMUM));
	else
		m_VideoEncoder->SetMinimumQP(0);
	if (unlocked || (vis_basic && (rcm != VCERateControlMethod_ConstantQP)))
		m_VideoEncoder->SetMaximumQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_MAXIMUM));
	else
		m_VideoEncoder->SetMaximumQP(51);
	if (unlocked || (vis_basic && (rcm == VCERateControlMethod_ConstantQP)))
		m_VideoEncoder->SetIFrameQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_IFRAME));
	else
		m_VideoEncoder->SetIFrameQP(0);
	if (unlocked || (vis_basic && (rcm == VCERateControlMethod_ConstantQP)))
		m_VideoEncoder->SetPFrameQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_PFRAME));
	else
		m_VideoEncoder->SetPFrameQP(0);
	try {
		if (unlocked || (vis_basic && (rcm == VCERateControlMethod_ConstantQP)))
			m_VideoEncoder->SetBFrameQP((uint8_t)obs_data_get_int(settings, AMF_H264_QP_BFRAME));
		else
			m_VideoEncoder->SetBFrameQP(0);
	} catch (...) {}
	try {
		if (unlocked || (vis_basic && (rcm == VCERateControlMethod_ConstantQP)))
			m_VideoEncoder->SetBPictureDeltaQP((int8_t)obs_data_get_int(settings, AMF_H264_QP_BPICTURE_DELTA));
		else
			m_VideoEncoder->SetBPictureDeltaQP(0);
	} catch (...) {}
	try {
		if (unlocked || (vis_basic && (rcm == VCERateControlMethod_ConstantQP)))
			m_VideoEncoder->SetReferenceBPictureDeltaQP((int8_t)obs_data_get_int(settings, AMF_H264_QP_REFERENCE_BPICTURE_DELTA));
		else
			m_VideoEncoder->SetReferenceBPictureDeltaQP(0);
	} catch (...) {}
	if (unlocked || vis_advanced) {
		if (obs_data_get_int(settings, AMF_H264_VBVBUFFER) == 0) {
			m_VideoEncoder->SetVBVBufferAutomatic(obs_data_get_double(settings, AMF_H264_VBVBUFFER_STRICTNESS) / 100.0);
		} else {
			m_VideoEncoder->SetVBVBufferSize((uint32_t)obs_data_get_int(settings, AMF_H264_VBVBUFFER_SIZE) * bitratemult);
		}
	} else {
		m_VideoEncoder->SetVBVBufferAutomatic(0.5);
	}
	if (vis_expert)
		m_VideoEncoder->SetInitialVBVBufferFullness((float_t)obs_data_get_double(settings, AMF_H264_VBVBUFFER_FULLNESS) / 100.0);
	else
		m_VideoEncoder->SetInitialVBVBufferFullness(1.0);
	if (unlocked || vis_basic && rcm == VCERateControlMethod_ConstantBitrate)
		m_VideoEncoder->SetFillerDataEnabled(obs_data_get_bool(settings, AMF_H264_FILLERDATA));
	else
		m_VideoEncoder->SetFillerDataEnabled(false);
	if (unlocked || vis_advanced)
		m_VideoEncoder->SetFrameSkippingEnabled(obs_data_get_bool(settings, AMF_H264_FRAMESKIPPING));
	else
		m_VideoEncoder->SetFrameSkippingEnabled(false);
	if (unlocked || vis_expert)
		m_VideoEncoder->SetEnforceHRDRestrictionsEnabled(obs_data_get_bool(settings, AMF_H264_ENFORCEHRDCOMPATIBILITY));
	else
		m_VideoEncoder->SetEnforceHRDRestrictionsEnabled(false);
	if (unlocked || vis_master)
		m_VideoEncoder->SetMaximumAccessUnitSize((uint32_t)obs_data_get_int(settings, AMF_H264_MAXIMUMACCESSUNITSIZE));
	else
		m_VideoEncoder->SetMaximumAccessUnitSize(0);

	// Picture Control Properties
	if (unlocked)
		m_VideoEncoder->SetIDRPeriod((uint32_t)obs_data_get_int(settings, AMF_H264_IDR_PERIOD));
	else
		m_VideoEncoder->SetIDRPeriod((uint32_t)(obs_data_get_int(settings, AMF_H264_KEYFRAME_INTERVAL) * framerate));
	if (vis_advanced)
		m_VideoEncoder->SetDeBlockingFilterEnabled(obs_data_get_bool(settings, AMF_H264_DEBLOCKINGFILTER));
	else
		m_VideoEncoder->SetDeBlockingFilterEnabled(true);
	try {
		if (unlocked || vis_basic)
			m_VideoEncoder->SetBPicturePattern((VCEBPicturePattern)obs_data_get_int(settings, AMF_H264_BPICTURE_PATTERN));
		else
			m_VideoEncoder->SetBPicturePattern(VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames ? VCEBPicturePattern_Three : VCEBPicturePattern_None);
	} catch (...) {}
	try {
		if (unlocked || vis_advanced)
			m_VideoEncoder->SetBPictureReferenceEnabled(obs_data_get_bool(settings, AMF_H264_BPICTURE_REFERENCE));
		else
			m_VideoEncoder->SetBPictureReferenceEnabled(false);
	} catch (...) {}

	// Miscellaneous Properties
	if (unlocked || vis_expert)
		m_VideoEncoder->SetScanType((VCEScanType)obs_data_get_int(settings, AMF_H264_SCANTYPE));
	else
		m_VideoEncoder->SetScanType(VCEScanType_Progressive);
	if (unlocked || vis_advanced)
		m_VideoEncoder->SetQualityPreset((VCEQualityPreset)obs_data_get_int(settings, AMF_H264_QUALITY_PRESET));
	else
		m_VideoEncoder->SetQualityPreset(VCEQualityPreset_Balanced);
	if (unlocked || vis_expert) {
		m_VideoEncoder->SetHalfPixelMotionEstimationEnabled(obs_data_get_int(settings, AMF_H264_MOTIONESTIMATION) && 1);
		m_VideoEncoder->SetQuarterPixelMotionEstimationEnabled(obs_data_get_int(settings, AMF_H264_MOTIONESTIMATION) && 2);
	} else {
		m_VideoEncoder->SetHalfPixelMotionEstimationEnabled(true);
		m_VideoEncoder->SetQuarterPixelMotionEstimationEnabled(true);
	}
	try {
		m_VideoEncoder->SetCABACEnabled(obs_data_get_bool(settings, AMF_H264_CABAC));
	} catch (...) {}


	try { m_VideoEncoder->Restart(); } catch (...) { return false; }

	return true;
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
