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

#include <exception>
#include <stdexcept>
#include <memory>
#include <chrono>
#include <string> // std::string
#include <sstream> // std::stringstream
#include <queue>

// AMF
#include "AMD-Media-SDK/1.1/inc/ErrorCodes.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/CapabilityManager.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/Component.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/ComponentCaps.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderCaps.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderVCE.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderVCECaps.h"

// Plugin
#include "win-amf.h"
#include "amf-vce.h"
#include "amf-vce-capabilities.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
// h264 Profiles
const char* AMFEncoder::VCE_H264_Encoder::PROFILE_NAMES[AMFEncoder::VCE_H264_Encoder::PROFILES::PROFILE_COUNT_MAX] = {
	AMF_VCE_H264_PROFILE_BASELINE,
	AMF_VCE_H264_PROFILE_MAIN,
	AMF_VCE_H264_PROFILE_HIGH,
};
const AMFEncoder::VCE_Profile AMFEncoder::VCE_H264_Encoder::PROFILE_VALUES[AMFEncoder::VCE_H264_Encoder::PROFILES::PROFILE_COUNT_MAX] = {
	VCE_PROFILE_BASELINE,
	VCE_PROFILE_MAIN,
	VCE_PROFILE_HIGH
};

// h264 Levels
const char* AMFEncoder::VCE_H264_Encoder::LEVEL_NAMES[AMFEncoder::VCE_H264_Encoder::LEVELS::LEVEL_COUNT_MAX] = {
	AMF_VCE_H264_PROFILE_LEVEL2("10"), AMF_VCE_H264_PROFILE_LEVEL2("11"), AMF_VCE_H264_PROFILE_LEVEL2("12"), AMF_VCE_H264_PROFILE_LEVEL2("13"),
	AMF_VCE_H264_PROFILE_LEVEL2("20"), AMF_VCE_H264_PROFILE_LEVEL2("21"), AMF_VCE_H264_PROFILE_LEVEL2("22"),
	AMF_VCE_H264_PROFILE_LEVEL2("30"), AMF_VCE_H264_PROFILE_LEVEL2("31"), AMF_VCE_H264_PROFILE_LEVEL2("32"),
	AMF_VCE_H264_PROFILE_LEVEL2("40"), AMF_VCE_H264_PROFILE_LEVEL2("41"), AMF_VCE_H264_PROFILE_LEVEL2("42"),
	AMF_VCE_H264_PROFILE_LEVEL2("50"), AMF_VCE_H264_PROFILE_LEVEL2("51"), AMF_VCE_H264_PROFILE_LEVEL2("52")
};
const AMFEncoder::VCE_Profile_Level AMFEncoder::VCE_H264_Encoder::LEVEL_VALUES[LEVELS::LEVEL_COUNT_MAX] = {
	VCE_PROFILE_LEVEL_10, VCE_PROFILE_LEVEL_11, VCE_PROFILE_LEVEL_12, VCE_PROFILE_LEVEL_13,
	VCE_PROFILE_LEVEL_20, VCE_PROFILE_LEVEL_21, VCE_PROFILE_LEVEL_22,
	VCE_PROFILE_LEVEL_30, VCE_PROFILE_LEVEL_31, VCE_PROFILE_LEVEL_32,
	VCE_PROFILE_LEVEL_40, VCE_PROFILE_LEVEL_41, VCE_PROFILE_LEVEL_42,
	VCE_PROFILE_LEVEL_50, VCE_PROFILE_LEVEL_51, VCE_PROFILE_LEVEL_52,
};


//////////////////////////////////////////////////////////////////////////
// Static Code
//////////////////////////////////////////////////////////////////////////
obs_encoder_info* AMFEncoder::VCE_H264_Encoder::encoder_info;

void AMFEncoder::VCE_H264_Encoder::encoder_register() {
	if (!AMFEncoder::VCE_H264_Encoder::encoder_info) {
		AMFEncoder::VCE_H264_Encoder::encoder_info = new obs_encoder_info();
		AMFEncoder::VCE_H264_Encoder::encoder_info->id = "amf_h264_encoder";
		AMFEncoder::VCE_H264_Encoder::encoder_info->type = obs_encoder_type::OBS_ENCODER_VIDEO;
		AMFEncoder::VCE_H264_Encoder::encoder_info->codec = "h264";

		// Functions
		AMFEncoder::VCE_H264_Encoder::encoder_info->get_name = &AMFEncoder::VCE_H264_Encoder::get_name;
		AMFEncoder::VCE_H264_Encoder::encoder_info->get_defaults = &AMFEncoder::VCE_H264_Encoder::get_defaults;
		AMFEncoder::VCE_H264_Encoder::encoder_info->get_properties = &AMFEncoder::VCE_H264_Encoder::get_properties;
		AMFEncoder::VCE_H264_Encoder::encoder_info->create = &AMFEncoder::VCE_H264_Encoder::create;
		AMFEncoder::VCE_H264_Encoder::encoder_info->destroy = &AMFEncoder::VCE_H264_Encoder::destroy;
		AMFEncoder::VCE_H264_Encoder::encoder_info->encode = &AMFEncoder::VCE_H264_Encoder::encode;
		AMFEncoder::VCE_H264_Encoder::encoder_info->update = &AMFEncoder::VCE_H264_Encoder::update;
		AMFEncoder::VCE_H264_Encoder::encoder_info->get_video_info = &AMFEncoder::VCE_H264_Encoder::get_video_info;
		AMFEncoder::VCE_H264_Encoder::encoder_info->get_extra_data = &AMFEncoder::VCE_H264_Encoder::get_extra_data;

		obs_register_encoder(AMFEncoder::VCE_H264_Encoder::encoder_info);
	}
}

const char* AMFEncoder::VCE_H264_Encoder::get_name(void* type_data) {
	return AMF_TEXT_H264_T("Name");
}

void* AMFEncoder::VCE_H264_Encoder::create(obs_data_t* settings, obs_encoder_t* encoder) {
	try {
		AMFEncoder::VCE_H264_Encoder* enc = new AMFEncoder::VCE_H264_Encoder(settings, encoder);
		return enc;
	} catch (std::exception e) {
		return NULL;
	}
}

void AMFEncoder::VCE_H264_Encoder::destroy(void* data) {
	AMFEncoder::VCE_H264_Encoder* enc = static_cast<AMFEncoder::VCE_H264_Encoder*>(data);
	delete enc;
	data = nullptr;
}

bool AMFEncoder::VCE_H264_Encoder::encode(void *data, struct encoder_frame *frame, struct encoder_packet *packet, bool *received_packet) {
	return static_cast<AMFEncoder::VCE_H264_Encoder*>(data)->encode(frame, packet, received_packet);
}

void AMFEncoder::VCE_H264_Encoder::get_defaults(obs_data_t *data) {
	// Controls
	obs_data_set_default_bool(data, AMF_VCE_H264_RESET, false);
	obs_data_set_default_bool(data, AMF_VCE_H264_UPDATE, false);

	// Static Properties
	/// Type, Usage & Quality Preset
	obs_data_set_default_int(data, AMF_VCE_H264_TYPE, VCE_ENCODER_TYPE_AVC);
	obs_data_set_default_int(data, AMF_VCE_H264_USAGE, VCE_USAGE_TRANSCODING);
	obs_data_set_default_int(data, AMF_VCE_H264_QUALITY_PRESET, -1);
	/// Profile & Level
	obs_data_set_default_int(data, AMF_VCE_H264_PROFILE, -1);
	obs_data_set_default_int(data, AMF_VCE_H264_PROFILE_LEVEL, -1);
	/// Other
	obs_data_set_default_int(data, AMF_VCE_H264_MAX_LTR_FRAMES, -1);
	obs_data_set_default_int(data, AMF_VCE_H264_SCAN_TYPE, -1);

	// Dynamic Properties
	/// Rate Control
	obs_data_set_default_int(data, AMF_VCE_H264_RATECONTROL_METHOD, -1);
	obs_data_set_default_int(data, AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING, -1);
	/// Other
	obs_data_set_default_int(data, AMF_VCE_H264_FILLERDATA, -1);
	obs_data_set_default_int(data, AMF_VCE_H264_ENFORCEHRD, -1);
	/// Video Coding Settings
	obs_data_set_default_int(data, AMF_VCE_H264_GOP_SIZE, -1);
	obs_data_set_default_int(data, AMF_VCE_H264_VBVBUFFER_SIZE, -1);
	obs_data_set_default_double(data, AMF_VCE_H264_VBVBUFFER_FULLNESS, 1.0);
	obs_data_set_default_int(data, AMF_VCE_H264_MAX_AU_SIZE, -1);
	/// B-Picture Stuff
	obs_data_set_default_int(data, AMF_VCE_H264_BPIC_DELTA_QP, -1);
	obs_data_set_default_int(data, AMF_VCE_H264_REF_BPIC_DELTA_QP, -1);
	/// Rate Control: Constrained QP
	obs_data_set_default_int(data, AMF_VCE_H264_QP_MINIMUM, -1);
	obs_data_set_default_int(data, AMF_VCE_H264_QP_MAXIMUM, -1);
	obs_data_set_default_int(data, AMF_VCE_H264_QP_IFRAME, -1);
	obs_data_set_default_int(data, AMF_VCE_H264_QP_PFRAME, -1);
	obs_data_set_default_int(data, AMF_VCE_H264_QP_BFRAME, -1);
	/// Rate Control:  CBR, VBR
	obs_data_set_default_int(data, AMF_VCE_H264_BITRATE_TARGET, -1);
	obs_data_set_default_int(data, AMF_VCE_H264_BITRATE_PEAK, -1);

	//// Picture Control Properties
	//obs_data_set_default_int(data, "AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING", -1);
	//obs_data_set_default_int(data, "AMF_VIDEO_ENCODER_B_PIC_PATTERN", -1);
	//obs_data_set_default_int(data, "AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER", -1);
	//obs_data_set_default_int(data, "AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE", -1);
	//obs_data_set_default_int(data, "AMF_VIDEO_ENCODER_IDR_PERIOD", -1);
	//obs_data_set_default_int(data, "AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT", -1);
	//obs_data_set_default_int(data, "AMF_VIDEO_ENCODER_SLICES_PER_FRAME", -1);

	//// Motion Estimation
	//obs_data_set_default_bool(data, "AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL", true);
	//obs_data_set_default_bool(data, "AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL", true);

	//// SVC (Scalable Profiles)
	//obs_data_set_default_int(data, "AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS", -1);
}

obs_properties_t* AMFEncoder::VCE_H264_Encoder::get_properties(void* data) {
	obs_properties* props = obs_properties_create();
	obs_property_t* list;
	obs_property_t* p;

	//////////////////////////////////////////////////////////////////////////
	// Controls
	//////////////////////////////////////////////////////////////////////////
	p = obs_properties_add_bool(props, AMF_VCE_H264_RESET, obs_module_text(AMF_VCE_H264_RESET));
	obs_property_set_modified_callback(p, &reset_callback);
	p = obs_properties_add_bool(props, AMF_VCE_H264_UPDATE, obs_module_text(AMF_VCE_H264_UPDATE));
	obs_property_set_modified_callback(p, &update_from_amf);

	//////////////////////////////////////////////////////////////////////////
	// Static Properties (Can't be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	// Type, Usage & Quality Preset
	/// Type
	list = obs_properties_add_list(props, AMF_VCE_H264_TYPE, obs_module_text(AMF_VCE_H264_TYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_TYPE_AVC), VCE_ENCODER_TYPE_AVC);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_TYPE_SVC), VCE_ENCODER_TYPE_SVC);
	//obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_TYPE_HEVC), VCE_ENCODER_TYPE_HEVC);
	/// Usage
	list = obs_properties_add_list(props, AMF_VCE_H264_USAGE, obs_module_text(AMF_VCE_H264_USAGE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_USAGE_TRANSCODING), VCE_USAGE_TRANSCODING);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_USAGE_ULTRALOWLATENCY), VCE_USAGE_ULTRA_LOW_LATENCY);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_USAGE_LOWLATENCY), VCE_USAGE_LOW_LATENCY);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_USAGE_WEBCAM), VCE_USAGE_WEBCAM);
	/// Quality Preset
	list = obs_properties_add_list(props, AMF_VCE_H264_QUALITY_PRESET, obs_module_text(AMF_VCE_H264_QUALITY_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_QUALITY_PRESET_NONE), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_QUALITY_PRESET_SPEED), VCE_QUALITY_PRESET_SPEED);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_QUALITY_PRESET_BALANCED), VCE_QUALITY_PRESET_BALANCED);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_QUALITY_PRESET_QUALITY), VCE_QUALITY_PRESET_QUALITY);

	// Profile & Level
	/// h264 Profile
	list = obs_properties_add_list(props, AMF_VCE_H264_PROFILE, obs_module_text(AMF_VCE_H264_PROFILE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_DEFAULT), -1);
	for (unsigned int i = 0; i < AMFEncoder::VCE_H264_Encoder::PROFILES::PROFILE_COUNT_MAX; i++) {
		obs_property_list_add_int(list, obs_module_text(AMFEncoder::VCE_H264_Encoder::PROFILE_NAMES[i]), i);
	}
	/// h264 Profile Level
	list = obs_properties_add_list(props, AMF_VCE_H264_PROFILE_LEVEL, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_LEVEL2("Default")), -1);
	for (unsigned int i = 0; i < AMFEncoder::VCE_H264_Encoder::LEVELS::LEVEL_COUNT_MAX; i++) {
		obs_property_list_add_int(list, obs_module_text(AMFEncoder::VCE_H264_Encoder::LEVEL_NAMES[i]), i);
	}

	// Other
	/// Maximum Long-Term-Reference Frames
	obs_properties_add_int_slider(props, AMF_VCE_H264_MAX_LTR_FRAMES, obs_module_text(AMF_VCE_H264_MAX_LTR_FRAMES), -1, 255, 1);
	/// Scan Type
	list = obs_properties_add_list(props, AMF_VCE_H264_SCAN_TYPE, obs_module_text(AMF_VCE_H264_SCAN_TYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_SCAN_TYPE_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_SCAN_TYPE_PROGRESSIVE), VCE_SCANTYPE_PROGRESSIVE);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_SCAN_TYPE_INTERLACED), VCE_SCANTYPE_INTERLACED);

	//////////////////////////////////////////////////////////////////////////
	// Dynamic Properties (Can be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	// Rate Control
	/// Method
	list = obs_properties_add_list(props, AMF_VCE_H264_RATECONTROL_METHOD, obs_module_text(AMF_VCE_H264_RATECONTROL_METHOD), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_METHOD_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_METHOD_CQP), VCE_RATE_CONTROL_CONSTRAINED_QUANTIZATION_PARAMETER);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_METHOD_CBR), VCE_RATE_CONTROL_CONSTANT_BITRATE);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_METHOD_VBR_PEAK_CONSTRAINED), VCE_RATE_CONTROL_VARIABLE_BITRATE_PEAK_CONSTRAINED);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_METHOD_VBR_LATENCY_CONSTRAINED), VCE_RATE_CONTROL_VARIABLE_BITRATE_LATENCY_CONSTRAINED);
	/// Skip Frames if necessary
	list = obs_properties_add_list(props, AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING, obs_module_text(AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING_DISABLED), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING_ENABLED), 1);
	// Rate Control - Other
	/// Filler Data
	list = obs_properties_add_list(props, AMF_VCE_H264_FILLERDATA, obs_module_text(AMF_VCE_H264_FILLERDATA), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_FILLERDATA_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_FILLERDATA_DISABLED), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_FILLERDATA_ENABLED), 1);
	/// Enforce Hyptohecial Reference Decoder Compatability
	list = obs_properties_add_list(props, AMF_VCE_H264_ENFORCEHRD, obs_module_text(AMF_VCE_H264_ENFORCEHRD), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_ENFORCEHRD_DEFAULT), -1);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_ENFORCEHRD_DISABLED), 0);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_ENFORCEHRD_ENABLED), 1);
	// Video Coding Settings
	/// GOP Size
	obs_properties_add_int_slider(props, AMF_VCE_H264_GOP_SIZE, obs_module_text(AMF_VCE_H264_GOP_SIZE), -1, 8192, 1);
	/// VBV Buffer
	obs_properties_add_int_slider(props, AMF_VCE_H264_VBVBUFFER_SIZE, obs_module_text(AMF_VCE_H264_VBVBUFFER_SIZE), -1, VCE_Capabilities::getInstance()->getEncoderCaps(VCE_ENCODER_TYPE_AVC)->maxBitrate, 1);
	obs_properties_add_float_slider(props, AMF_VCE_H264_VBVBUFFER_FULLNESS, obs_module_text(AMF_VCE_H264_VBVBUFFER_FULLNESS), 0.0, 1.0, 0.015625);
	/// Max AU Size
	obs_properties_add_int_slider(props, AMF_VCE_H264_MAX_AU_SIZE, obs_module_text(AMF_VCE_H264_MAX_AU_SIZE), -1, 1024, 1);
	/// B-Picture Related
	obs_properties_add_int_slider(props, AMF_VCE_H264_BPIC_DELTA_QP, obs_module_text(AMF_VCE_H264_BPIC_DELTA_QP), -1, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_REF_BPIC_DELTA_QP, obs_module_text(AMF_VCE_H264_REF_BPIC_DELTA_QP), -1, 51, 1);
	// Rate Control: Constrained QP
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_MINIMUM, obs_module_text(AMF_VCE_H264_QP_MINIMUM), -1, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_MAXIMUM, obs_module_text(AMF_VCE_H264_QP_MAXIMUM), -1, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_IFRAME, obs_module_text(AMF_VCE_H264_QP_IFRAME), -1, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_PFRAME, obs_module_text(AMF_VCE_H264_QP_PFRAME), -1, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_BFRAME, obs_module_text(AMF_VCE_H264_QP_BFRAME), -1, 51, 1);
	// Rate Control: CBR, VBR
	obs_properties_add_int_slider(props, AMF_VCE_H264_BITRATE_TARGET, obs_module_text(AMF_VCE_H264_BITRATE_TARGET), -1, VCE_Capabilities::getInstance()->getEncoderCaps(VCE_ENCODER_TYPE_AVC)->maxBitrate, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_BITRATE_PEAK, obs_module_text(AMF_VCE_H264_BITRATE_PEAK), -1, VCE_Capabilities::getInstance()->getEncoderCaps(VCE_ENCODER_TYPE_AVC)->maxBitrate, 1);

	// Picture Control Properties
	/// Header Insertion Spacing
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING", AMF_TEXT_H264_T("HEADER_INSERTION_SPACING"), -1, 1000, 1);
	/// B-Pictures Pattern
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_B_PIC_PATTERN", AMF_TEXT_H264_T("B_PIC_PATTERN"), -1, 16, 1);
	/// De-Blocking Filter
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER", AMF_TEXT_H264_T("DEBLOCKINGFILTER"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("DEBLOCKINGFILTER.DEFAULT"), -1);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("DEBLOCKINGFILTER.DISABLE"), 0);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("DEBLOCKINGFILTER.ENABLE"), 1);
	/// Enable Reference to B-Frames (2nd Generation GCN and newer)
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE", AMF_TEXT_H264_T("BREFERENCE"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("BREFERENCE.DEFAULT"), -1);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("BREFERENCE.DISABLE"), 0);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("BREFERENCE.ENABLE"), 1);
	/// IDR Period (Is this Keyframe distance?)
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_IDR_PERIOD", AMF_TEXT_H264_T("IDR_PERIOD"), -1, 1000, 1);
	/// Intra Refresh MBs Number Per Slot in Macroblocks
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT", AMF_TEXT_H264_T("INTRA_REFRESH_NUM_MBS_PER_SLOT"), -1, 1024, 1);
	/// Number of slices Per Frame 
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_SLICES_PER_FRAME", AMF_TEXT_H264_T("SLICES_PER_FRAME"), -1, 1000, 1);

	// Motion Estimation
	/// Half Pixel 
	obs_properties_add_bool(props, "AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL", AMF_TEXT_H264_T("MOTION_HALF_PIXEL"));
	/// Quarter Pixel
	obs_properties_add_bool(props, "AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL", AMF_TEXT_H264_T("MOTION_QUARTER_PIXEL"));

	// SVC (Scalable Profiles)
	/// Number of Temporal Enhancement Layers (SVC)
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS", AMF_TEXT_H264_T("NUM_TEMPORAL_ENHANCEMENT_LAYERS"), -1, 1024, 1);

	return props;
}

bool AMFEncoder::VCE_H264_Encoder::reset_callback(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
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
	return false;
}

bool AMFEncoder::VCE_H264_Encoder::update_from_amf(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
	if (obs_data_get_bool(settings, AMF_VCE_H264_UPDATE) == false)
		return false;

	//////////////////////////////////////////////////////////////////////////
	// Static Properties (Can't be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	VCE* vce = nullptr;
	switch (obs_data_get_int(settings, AMF_VCE_H264_TYPE)) {
		case VCE_ENCODER_TYPE_AVC:
			vce = new VCE(VCE_ENCODER_TYPE_AVC);
			break;
		case VCE_ENCODER_TYPE_SVC:
			vce = new VCE(VCE_ENCODER_TYPE_SVC);
			break;
		case VCE_ENCODER_TYPE_HEVC:
			vce = new VCE(VCE_ENCODER_TYPE_HEVC);
			break;
	}

	// Usage & Quality Preset
	int64_t usage = obs_data_get_int(settings, AMF_VCE_H264_USAGE);
	int64_t preset = obs_data_get_int(settings, AMF_VCE_H264_QUALITY_PRESET);
	vce->SetUsage((VCE_Usage)usage);
	if (preset == -1)
		obs_data_set_int(settings, AMF_VCE_H264_QUALITY_PRESET, vce->GetQualityPreset());
	else
		vce->SetQualityPreset((VCE_Quality_Preset)preset);

	// Profile & Level
	if (obs_data_get_int(settings, AMF_VCE_H264_PROFILE) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_PROFILE, vce->GetProfile());
	if (obs_data_get_int(settings, AMF_VCE_H264_PROFILE_LEVEL) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_PROFILE_LEVEL, vce->GetProfileLevel());

	// Other
	if (obs_data_get_int(settings, AMF_VCE_H264_MAX_LTR_FRAMES) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_MAX_LTR_FRAMES, vce->GetMaxLTRFrames());
	if (obs_data_get_int(settings, AMF_VCE_H264_SCAN_TYPE) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_SCAN_TYPE, vce->GetScanType());

	//////////////////////////////////////////////////////////////////////////
	// Dynamic Properties (Can be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	// Rate Control
	/// Method
	if (obs_data_get_int(settings, AMF_VCE_H264_RATECONTROL_METHOD) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_RATECONTROL_METHOD, vce->GetRateControlMethod());
	/// Skip Frames if necessary
	if (obs_data_get_int(settings, AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING, vce->IsFrameSkippingEnabled() ? 1 : 0);
	// Rate Control - Other
	/// Filler Data
	if (obs_data_get_int(settings, AMF_VCE_H264_FILLERDATA) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_FILLERDATA, vce->IsFillerDataEnabled() ? 1 : 0);
	/// Enforce Hyptohecial Reference Decoder Compatability
	if (obs_data_get_int(settings, AMF_VCE_H264_ENFORCEHRD) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_ENFORCEHRD, vce->IsEnforceHRDEnabled() ? 1 : 0);
	// Video Coding Settings
	/// GOP Size
	if (obs_data_get_int(settings, AMF_VCE_H264_GOP_SIZE) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_GOP_SIZE, vce->GetGOPSize());
	/// VBV Buffer
	if (obs_data_get_int(settings, AMF_VCE_H264_VBVBUFFER_SIZE) == -1) {
		obs_data_set_int(settings, AMF_VCE_H264_VBVBUFFER_SIZE, vce->GetVBVBufferSize());
		obs_data_set_double(settings, AMF_VCE_H264_VBVBUFFER_FULLNESS, vce->GetInitialVBVBufferFullness());
	}
	/// Max AU Size
	if (obs_data_get_int(settings, AMF_VCE_H264_MAX_AU_SIZE) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_MAX_AU_SIZE, vce->GetMaximumAccessUnitSize());
	/// B-Picture Related
	if (obs_data_get_int(settings, AMF_VCE_H264_BPIC_DELTA_QP) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_BPIC_DELTA_QP, vce->GetBPictureDeltaQP());
	if (obs_data_get_int(settings, AMF_VCE_H264_REF_BPIC_DELTA_QP) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_REF_BPIC_DELTA_QP, vce->GetReferenceBPictureDeltaQP());
	// Rate Control: Constrained QP
	if (obs_data_get_int(settings, AMF_VCE_H264_QP_MINIMUM) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_QP_MINIMUM, vce->GetMinimumQP());
	if (obs_data_get_int(settings, AMF_VCE_H264_QP_MAXIMUM) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_QP_MAXIMUM, vce->GetMaximumQP());
	if (obs_data_get_int(settings, AMF_VCE_H264_QP_IFRAME) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_QP_IFRAME, vce->GetIFrameQP());
	if (obs_data_get_int(settings, AMF_VCE_H264_QP_PFRAME) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_QP_PFRAME, vce->GetPFrameQP());
	if (obs_data_get_int(settings, AMF_VCE_H264_QP_BFRAME) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_QP_BFRAME, vce->GetBFrameQP());
	// Rate Control: CBR, VBR
	if (obs_data_get_int(settings, AMF_VCE_H264_BITRATE_TARGET) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_BITRATE_TARGET, vce->GetTargetBitrate());
	if (obs_data_get_int(settings, AMF_VCE_H264_BITRATE_PEAK) == -1)
		obs_data_set_int(settings, AMF_VCE_H264_BITRATE_PEAK, vce->GetPeakBitrate());

	//// Picture Control Properties
	///// Header Insertion Spacing
	//obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING", AMF_TEXT_H264_T("HEADER_INSERTION_SPACING"), -1, 1000, 1);
	///// B-Pictures Pattern
	//obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_B_PIC_PATTERN", AMF_TEXT_H264_T("B_PIC_PATTERN"), -1, 16, 1);
	///// De-Blocking Filter
	//list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER", AMF_TEXT_H264_T("DEBLOCKINGFILTER"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	///// Enable Reference to B-Frames (2nd Generation GCN and newer)
	//list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE", AMF_TEXT_H264_T("BREFERENCE"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	///// IDR Period (Is this Keyframe distance?)
	//obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_IDR_PERIOD", AMF_TEXT_H264_T("IDR_PERIOD"), -1, 1000, 1);
	///// Intra Refresh MBs Number Per Slot in Macroblocks
	//obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT", AMF_TEXT_H264_T("INTRA_REFRESH_NUM_MBS_PER_SLOT"), -1, 1024, 1);
	///// Number of slices Per Frame 
	//obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_SLICES_PER_FRAME", AMF_TEXT_H264_T("SLICES_PER_FRAME"), -1, 1000, 1);

	//// Motion Estimation
	///// Half Pixel 
	//obs_properties_add_bool(props, "AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL", AMF_TEXT_H264_T("MOTION_HALF_PIXEL"));
	///// Quarter Pixel
	//obs_properties_add_bool(props, "AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL", AMF_TEXT_H264_T("MOTION_QUARTER_PIXEL"));

	//// SVC (Scalable Profiles)
	///// Number of Temporal Enhancement Layers (SVC)
	//obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS", AMF_TEXT_H264_T("NUM_TEMPORAL_ENHANCEMENT_LAYERS"), -1, 1024, 1);


	return false;
}

bool AMFEncoder::VCE_H264_Encoder::update(void *data, obs_data_t *settings) {
	return static_cast<AMFEncoder::VCE_H264_Encoder*>(data)->update(settings);
}

void AMFEncoder::VCE_H264_Encoder::get_video_info(void *data, struct video_scale_info *info) {
	return static_cast<AMFEncoder::VCE_H264_Encoder*>(data)->get_video_info(info);
}

bool AMFEncoder::VCE_H264_Encoder::get_extra_data(void *data, uint8_t** extra_data, size_t* size) {
	return static_cast<AMFEncoder::VCE_H264_Encoder*>(data)->get_extra_data(extra_data, size);
}

//////////////////////////////////////////////////////////////////////////
// Module Code
//////////////////////////////////////////////////////////////////////////
AMFEncoder::VCE_H264_Encoder::VCE_H264_Encoder(obs_data_t* settings, obs_encoder_t* encoder) {
	int64_t value;

	AMF_LOG_INFO("<AMFEncoder::VCE_H264_Encoder::VCE_H264_Encoder> Initializing...");

	// OBS Settings
	m_cfgWidth = obs_encoder_get_width(encoder);
	m_cfgHeight = obs_encoder_get_height(encoder);
	video_t *video = obs_encoder_video(encoder);
	const struct video_output_info *voi = video_output_get_info(video);
	m_cfgFPSnum = voi->fps_num;
	m_cfgFPSden = voi->fps_den;

	//////////////////////////////////////////////////////////////////////////
	// Static Properties (Can't be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	// Type, Memory Type, Surface Format
	/// Type
	m_VCE = new AMFEncoder::VCE((VCE_Encoder_Type)obs_data_get_int(settings, AMF_VCE_H264_TYPE));
	/// Memory Type & Surface Format
	m_VCE->SetMemoryType(VCE_MEMORY_TYPE_HOST);
	m_VCE->SetSurfaceFormat(VCE_SURFACE_FORMAT_NV12);
	/// Quality Preset & Usage
	m_VCE->SetUsage((VCE_Usage)obs_data_get_int(settings, AMF_VCE_H264_USAGE));
	value = obs_data_get_int(settings, AMF_VCE_H264_QUALITY_PRESET);
	if (value != -1)
		m_VCE->SetQualityPreset((VCE_Quality_Preset)value);

	// Framesize & Framerate
	m_VCE->SetFrameSize(std::pair<uint32_t, uint32_t>(m_cfgWidth, m_cfgHeight));
	m_VCE->SetFrameRate(std::pair<uint32_t, uint32_t>(m_cfgFPSnum, m_cfgFPSden));

	// Profile & Level
	/// Profile
	value = obs_data_get_int(settings, AMF_VCE_H264_PROFILE);
	if (value != -1)
		m_VCE->SetProfile(PROFILE_VALUES[value]);
	/// Profile Level
	value = obs_data_get_int(settings, AMF_VCE_H264_PROFILE_LEVEL);
	if (value != -1)
		m_VCE->SetProfileLevel(LEVEL_VALUES[value]);

	// Other
	/// Maximum Long-Term-Reference Frames
	value = obs_data_get_int(settings, AMF_VCE_H264_MAX_LTR_FRAMES);
	if (value != -1)
		m_VCE->SetMaxLTRFrames((uint32_t)value);
	/// Scan Type
	value = obs_data_get_int(settings, AMF_VCE_H264_SCAN_TYPE);
	if (value != -1)
		m_VCE->SetScanType((VCE_ScanType)value);

	////////////////////////////////////////////////////////////////////////////
	//// Dynamic Properties (Can be changed during Encoding)
	////////////////////////////////////////////////////////////////////////////
	update_properties(settings);

	//////////////////////////////////////////////////////////////////////////
	// Initialize (locks static properties)
	//////////////////////////////////////////////////////////////////////////
	m_VCE->Start();

	AMF_LOG_INFO("<AMFEncoder::VCE_H264_Encoder::VCE_H264_Encoder> Complete.");
}

AMFEncoder::VCE_H264_Encoder::~VCE_H264_Encoder() {
	AMF_LOG_INFO("<AMFEncoder::VCE_H264_Encoder::~VCE_H264_Encoder> Finalizing...");
	m_VCE->Stop();
	delete m_VCE;
	AMF_LOG_INFO("<AMFEncoder::VCE_H264_Encoder::~VCE_H264_Encoder> Complete.");
}

bool AMFEncoder::VCE_H264_Encoder::update(obs_data_t* settings) {
	return update_properties(settings);
}

bool AMFEncoder::VCE_H264_Encoder::encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	if (!frame || !packet || !received_packet)
		return false;

	m_VCE->SendInput(frame);
	m_VCE->GetOutput(packet, received_packet);

	return true;
}

void AMFEncoder::VCE_H264_Encoder::get_video_info(struct video_scale_info* info) {
	m_VCE->GetVideoInfo(info);
}

bool AMFEncoder::VCE_H264_Encoder::get_extra_data(uint8_t** extra_data, size_t* size) {
	return m_VCE->GetExtraData(extra_data, size);
}

bool AMFEncoder::VCE_H264_Encoder::update_properties(obs_data_t* settings) {
	int64_t value; double_t valued;

	//////////////////////////////////////////////////////////////////////////
	// Dynamic Properties (Can be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	// Rate Control
	/// Method
	value = obs_data_get_int(settings, AMF_VCE_H264_RATECONTROL_METHOD);
	if (value != -1)
		m_VCE->SetRateControlMethod((VCE_Rate_Control_Method)value);
	/// Frame Skipping
	value = obs_data_get_int(settings, AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING);
	if (value != -1)
		m_VCE->SetFrameSkippingEnabled(value == 1);

	// Rate Control - Other
	/// Enable Filler Data
	value = obs_data_get_int(settings, AMF_VCE_H264_FILLERDATA);
	if (value != -1)
		m_VCE->SetFillerDataEnabled(value == 1);
	/// Enforce HRD
	value = obs_data_get_int(settings, AMF_VCE_H264_ENFORCEHRD);
	if (value != -1)
		m_VCE->SetEnforceHRDEnabled(value == 1);

	// Video Coding Settings
	/// GOP Size
	value = obs_data_get_int(settings, AMF_VCE_H264_GOP_SIZE);
	if (value != -1)
		m_VCE->SetGOPSize((uint32_t)value);
	/// VBV Buffer Size
	value = obs_data_get_int(settings, AMF_VCE_H264_VBVBUFFER_SIZE);
	if (value != -1)
		m_VCE->SetVBVBufferSize((uint32_t)value);
	/// Initial VBV Buffer Fullnes
	valued = obs_data_get_double(settings, AMF_VCE_H264_VBVBUFFER_FULLNESS);
	if (valued != 1.0)
		m_VCE->SetInitialVBVBufferFullness(valued);
	/// Max AU Size
	value = obs_data_get_int(settings, AMF_VCE_H264_MAX_AU_SIZE);
	if (value != -1)
		m_VCE->SetMaximumAccessUnitSize((uint32_t)value);
	/// B-Picture Delta QP
	value = obs_data_get_int(settings, AMF_VCE_H264_BPIC_DELTA_QP);
	if (value != -1)
		m_VCE->SetBPictureDeltaQP((uint8_t)value);
	/// Ref B-Picture Delta QP
	value = obs_data_get_int(settings, AMF_VCE_H264_REF_BPIC_DELTA_QP);
	if (value != -1)
		m_VCE->SetReferenceBPictureDeltaQP((uint8_t)value);

	// Rate Control: CQP
	/// Minimum & Maximum QP
	value = obs_data_get_int(settings, AMF_VCE_H264_QP_MINIMUM);
	if (value != -1)
		m_VCE->SetMinimumQP((uint8_t)value);
	value = obs_data_get_int(settings, AMF_VCE_H264_QP_MAXIMUM);
	if (value != -1)
		m_VCE->SetMaximumQP((uint8_t)value);
	/// I-, P-, B-Frame QP
	value = obs_data_get_int(settings, AMF_VCE_H264_QP_IFRAME);
	if (value != -1)
		m_VCE->SetIFrameQP((uint8_t)value);
	value = obs_data_get_int(settings, AMF_VCE_H264_QP_PFRAME);
	if (value != -1)
		m_VCE->SetPFrameQP((uint8_t)value);
	value = obs_data_get_int(settings, AMF_VCE_H264_QP_BFRAME);
	if (value != -1)
		m_VCE->SetBFrameQP((uint8_t)value);

	// Rate Control: CBR, VBR
	/// Target Bitrate
	value = obs_data_get_int(settings, AMF_VCE_H264_BITRATE_TARGET);
	if (value != -1)
		m_VCE->SetTargetBitrate((uint32_t)value);
	/// Peak Bitrate
	value = obs_data_get_int(settings, AMF_VCE_H264_BITRATE_PEAK);
	if (value != -1)
		m_VCE->SetPeakBitrate((uint32_t)value);

	//// Picture Control Properties
	///// Header Insertion Spacing
	//value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING");
	//if (value != -1) {
	//	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING, value);
	//	wa_log_property_int(res, "AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING", value);
	//}
	///// B-Pictures Pattern
	//value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_B_PIC_PATTERN");
	//if (value != -1) {
	//	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, value);
	//	wa_log_property_int(res, "AMF_VIDEO_ENCODER_B_PIC_PATTERN", value);
	//}
	///// De-Blocking Filter
	//value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER");
	//if (value != -1) {
	//	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER, value == 1);
	//	wa_log_property_bool(res, "AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER", value == 1);
	//}
	///// Enable Reference to B-Frames (2nd Generation GCN and newer)
	//value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE");
	//if (value != -1) {
	//	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, value == 1);
	//	wa_log_property_bool(res, "AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE", value == 1);
	//}
	///// IDR Period (Is this Keyframe distance?)
	//value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_IDR_PERIOD");
	//if (value != -1) {
	//	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, value);
	//	wa_log_property_int(res, "AMF_VIDEO_ENCODER_IDR_PERIOD", value);
	//}
	///// Intra Refresh MBs Number Per Slot in Macroblocks
	//value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT");
	//if (value != -1) {
	//	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, value);
	//	wa_log_property_int(res, "AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT", value);
	//}
	///// Number of slices Per Frame 
	//value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_SLICES_PER_FRAME");
	//if (value != -1) {
	//	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, value);
	//	wa_log_property_int(res, "AMF_VIDEO_ENCODER_SLICES_PER_FRAME", value);
	//}

	//// Motion Estimation
	///// Half Pixel
	//{
	//	value = obs_data_get_bool(settings, "AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL");
	//	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL, value == 1);
	//	wa_log_property_bool(res, "AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL", value == 1);
	//}
	///// Quarter Pixel
	//{
	//	value = obs_data_get_bool(settings, "AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL");
	//	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL, value == 1);
	//	wa_log_property_bool(res, "AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL", value == 1);
	//}

	//// SVC (Scalable Profiles)
	///// Number of Temporal Enhancment Layers (SVC)
	//value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS");
	//if (value != -1) {
	//	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS, value);
	//	wa_log_property_int(res, "AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS", value);
	//}

	return true;
}
