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
	/// Memory Type & Compute Type
	obs_data_set_default_int(data, AMF_H264_MEMORYTYPE, VCEMemoryType_Host);
	obs_data_set_default_int(data, AMF_H264_COMPUTETYPE, VCEComputeType_None);
	obs_data_set_default_int(data, AMF_H264_SURFACEFORMAT, -1);
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

	// Picture Control Properties
	obs_data_set_default_int(data, AMF_H264_BPICTURE_PATTERN, -1);
	obs_data_set_default_int(data, AMF_H264_BPICTURE_REFERENCE, -1);
	obs_data_set_default_int(data, AMF_H264ADVANCED_HEADER_INSERTION_SPACING, -1);
	obs_data_set_default_int(data, AMF_H264_DEBLOCKINGFILTER, -1);
	obs_data_set_default_int(data, AMF_H264ADVANCED_IDR_PERIOD, -1);
	obs_data_set_default_int(data, AMF_H264ADVANCED_INTRAREFRESHNUMMBPERSLOT, -1);
	obs_data_set_default_int(data, AMF_H264ADVANCED_SLICESPERFRAME, -1);

	// Motion Estimation
	obs_data_set_default_int(data, AMF_H264ADVANCED_MOTIONESTIMATION, -1);

	// Other
	obs_data_set_default_int(data, AMF_H264_GOP_SIZE, -1);
	obs_data_set_default_int(data, AMF_H264_CABAC, -1);

	/// Debug Mode
	obs_data_set_default_bool(data, AMF_H264_DEBUGTRACING, false);
}

obs_properties_t* Plugin::Interface::H264Interface::get_properties(void*) {
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
	/// Memory Type
	list = obs_properties_add_list(props, AMF_H264_MEMORYTYPE, obs_module_text(AMF_H264_MEMORYTYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_AUTOMATIC), VCEMemoryType_Auto);
	obs_property_list_add_int(list, "Host", VCEMemoryType_Auto);
	obs_property_list_add_int(list, "DirectX 9", VCEMemoryType_DirectX9);
	obs_property_list_add_int(list, "DirectX 11", VCEMemoryType_DirectX11);
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
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
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
	/// Enforce Hypothetical Reference Decoder Compatibility
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
	/// Intra Refresh MBs Number Per Slot in Macro-Blocks
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
	// Other Parameters
	//////////////////////////////////////////////////////////////////////////
	/// GOP Size
	obs_properties_add_int_slider(props, AMF_H264_GOP_SIZE, obs_module_text(AMF_H264_GOP_SIZE), -1, 1000, 1);
	/// CABAC
	list = obs_properties_add_list(props, AMF_H264_CABAC, obs_module_text(AMF_H264_CABAC), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_UTIL_TOGGLE_ENABLED), 1);

	//////////////////////////////////////////////////////////////////////////
	// Debug
	//////////////////////////////////////////////////////////////////////////
	/// Debug Mode
	obs_properties_add_bool(props, AMF_H264_DEBUGTRACING, obs_module_text(AMF_H264_DEBUGTRACING));

	return props;
}

bool Plugin::Interface::H264Interface::reset_callback(obs_properties_t *props, obs_property_t *, obs_data_t *settings) {
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

bool Plugin::Interface::H264Interface::update_from_amf(obs_properties_t *, obs_property_t *, obs_data_t *settings) {
	if (obs_data_get_bool(settings, AMF_H264ADVANCED_UPDATE) == false)
		return false;
	obs_data_set_bool(settings, AMF_H264ADVANCED_UPDATE, false);

	try {
		VCEEncoder* vce = new VCEEncoder(VCEEncoderType_AVC);

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
				obs_data_set_int(settings, AMF_H264ADVANCED_MAX_LTR_FRAMES, vce->GetMaximumLongTermReferenceFrames());
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
				obs_data_set_int(settings, AMF_H264_BPICTURE_PATTERN, vce->GetBPicturePattern());
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264_DEBLOCKINGFILTER) == -1)
				obs_data_set_int(settings, AMF_H264_DEBLOCKINGFILTER, vce->IsDeBlockingFilterEnabled() ? 1 : 0);
		} catch (...) {}
		try {
			if (obs_data_get_int(settings, AMF_H264_BPICTURE_REFERENCE) == -1)
				obs_data_set_int(settings, AMF_H264_BPICTURE_REFERENCE, vce->IsBPictureReferenceEnabled() ? 1 : 0);
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
	Plugin::AMD::AMF::GetInstance()->EnableDebugTrace(obs_data_get_bool(settings, AMF_H264_DEBUGTRACING));

	//////////////////////////////////////////////////////////////////////////
	// Static Properties (Can't be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	// Encoder
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

	// Other
	/// Maximum Long-Term-Reference Frames
	value = obs_data_get_int(settings, AMF_H264ADVANCED_MAX_LTR_FRAMES);
	if (value != -1)
		m_VideoEncoder->SetMaximumLongTermReferenceFrames((uint32_t)value);
	/// Scan Type
	value = obs_data_get_int(settings, AMF_H264_SCANTYPE);
	if (value != -1)
		m_VideoEncoder->SetScanType((VCEScanType)value);

	// Framesize & Framerate
	m_VideoEncoder->SetFrameSize(m_cfgWidth, m_cfgHeight);
	m_VideoEncoder->SetFrameRate(m_cfgFPSnum, m_cfgFPSden);

	////////////////////////////////////////////////////////////////////////////
	//// Dynamic Properties (Can be changed during Encoding)
	////////////////////////////////////////////////////////////////////////////
	update_properties(settings);

	// Framesize & Framerate
	m_VideoEncoder->SetFrameSize(m_cfgWidth, m_cfgHeight);
	m_VideoEncoder->SetFrameRate(m_cfgFPSnum, m_cfgFPSden);

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

	/// Bitrate
	try {
		value = obs_data_get_int(settings, AMF_H264_BITRATE_TARGET);
		if (value != -1)
			m_VideoEncoder->SetTargetBitrate((uint32_t)value);
	} catch (...) {}
	try {
		value = obs_data_get_int(settings, AMF_H264_BITRATE_PEAK);
		if (value != -1)
			m_VideoEncoder->SetPeakBitrate((uint32_t)value);
	} catch (...) {}

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

	/// Rate Control
	try {
		value = obs_data_get_int(settings, AMF_H264_RATECONTROLMETHOD);
		if (value != -1)
			m_VideoEncoder->SetRateControlMethod((VCERateControlMethod)value);
	} catch (...) {}
	try {
		value = obs_data_get_int(settings, AMF_H264_FILLERDATA);
		if (value != -1)
			m_VideoEncoder->SetFillerDataEnabled(value == 1);
	} catch (...) {}
	try {
		value = obs_data_get_int(settings, AMF_H264_FRAMESKIPPING);
		if (value != -1)
			m_VideoEncoder->SetRateControlSkipFrameEnabled(value == 1);
	} catch (...) {}
	/// CABAC
	try {
		value = obs_data_get_int(settings, AMF_H264_CABAC);
		if (value != -1)
			m_VideoEncoder->SetCABACEnabled(value != 0);
	} catch (...) {}

	/// VBV Buffer
	try {
		value = obs_data_get_int(settings, AMF_H264ADVANCED_VBVBUFFER_SIZE);
		if (value != -1)
			m_VideoEncoder->SetVBVBufferSize((uint32_t)value);
	} catch (...) {}
	try {
		valued = obs_data_get_double(settings, AMF_H264ADVANCED_VBVBUFFER_FULLNESS);
		m_VideoEncoder->SetInitialVBVBufferFullness(valued);
	} catch (...) {}

	/// Header Insertion Spacing
	try {
		value = obs_data_get_int(settings, AMF_H264ADVANCED_HEADER_INSERTION_SPACING);
		if (value != -1)
			m_VideoEncoder->SetHeaderInsertionSpacing((uint32_t)value);
	} catch (...) {}
	/// IDR Period / Keyframe Period
	try {
		value = obs_data_get_int(settings, AMF_H264ADVANCED_IDR_PERIOD);
		if (value != -1)
			m_VideoEncoder->SetIDRPeriod((uint32_t)value);
	} catch (...) {}
	/// GOP Size
	try {
		value = obs_data_get_int(settings, AMF_H264_GOP_SIZE);
		if (value != -1)
			m_VideoEncoder->SetGOPSize((uint32_t)value);
	} catch (...) {}
	/// Max AU Size
	try {
		value = obs_data_get_int(settings, AMF_H264ADVANCED_MAX_AU_SIZE);
		if (value != -1)
			m_VideoEncoder->SetMaximumAccessUnitSize((uint32_t)value);
	} catch (...) {}

	/// De-Blocking Filter
	try {
		value = obs_data_get_int(settings, AMF_H264_DEBLOCKINGFILTER);
		if (value != -1)
			m_VideoEncoder->SetDeBlockingFilterEnabled(value == 1);
	} catch (...) {}
	/// Enforce HRD Compatibility Restrictions
	try {
		value = obs_data_get_int(settings, AMF_H264_ENFORCEHRDCOMPATIBILITY);
		if (value != -1)
			m_VideoEncoder->SetEnforceHRDRestrictionsEnabled(value == 1);
	} catch (...) {}

	/// B-Pictures Pattern
	try {
		value = obs_data_get_int(settings, AMF_H264_BPICTURE_PATTERN);
		if (value != -1)
			m_VideoEncoder->SetBPicturePattern((VCEBPicturePattern)value);
	} catch (...) {}
	/// B-Picture Reference
	try {
		value = obs_data_get_int(settings, AMF_H264_BPICTURE_REFERENCE);
		if (value != -1)
			m_VideoEncoder->SetBPictureReferenceEnabled(value == 1);
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

	/// Motion Estimation
	try {
		value = obs_data_get_int(settings, AMF_H264ADVANCED_MOTIONESTIMATION);
		if (value != -1) {
			m_VideoEncoder->SetHalfPixelMotionEstimationEnabled(!!(value & 0x1));
			m_VideoEncoder->SetQuarterPixelMotionEstimationEnabled(!!(value & 0x2));
		}
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


	uint32_t fpsNum = m_VideoEncoder->GetFrameRate().first;
	uint32_t fpsDen = m_VideoEncoder->GetFrameRate().second;

	// OBS: Enforce streaming service encoder settings
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
	if (obs_data_get_int(settings, "bitrate") != -1) {
		m_VideoEncoder->SetTargetBitrate((uint32_t)obs_data_get_int(settings, "bitrate") * 1000);
		m_VideoEncoder->SetPeakBitrate((uint32_t)obs_data_get_int(settings, "bitrate") * 1000);
		m_VideoEncoder->SetVBVBufferSize((uint32_t)obs_data_get_int(settings, "bitrate") * 1000);
		obs_data_set_int(settings, "bitrate", -1);
	} else {
		obs_data_set_int(settings, "bitrate", m_VideoEncoder->GetTargetBitrate() / 1000);
	}
	if (obs_data_get_int(settings, "keyint_sec") != -1) {
		m_VideoEncoder->SetIDRPeriod((uint32_t)(obs_data_get_int(settings, "keyint_sec") * ((double_t)fpsNum / (double_t)fpsDen)));
		obs_data_set_int(settings, "keyint_sec", -1);
	} else {
		obs_data_set_int(settings, "keyint_sec", (uint64_t)(m_VideoEncoder->GetIDRPeriod() / ((double_t)fpsNum / (double_t)fpsDen)));
	}

	return true;
}
