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
#include "amf-h264.h"

// h264 Profiles
const char* AMF_Encoder::h264::PROFILE_NAMES[AMF_Encoder::h264::PROFILES::PROFILE_COUNT_MAX] = {
	AMF_TEXT_H264("PROFILE.AVC.BP"), AMF_TEXT_H264("PROFILE.AVC.XP"), AMF_TEXT_H264("PROFILE.AVC.MP"),
	AMF_TEXT_H264("PROFILE.AVC.HiP"), AMF_TEXT_H264("PROFILE.AVC.Hi10P"), AMF_TEXT_H264("PROFILE.AVC.Hi422P"), AMF_TEXT_H264("PROFILE.AVC.Hi444P"),
	AMF_TEXT_H264("PROFILE.SVC.BP"), AMF_TEXT_H264("PROFILE.SVC.HiP")
};
const unsigned char AMF_Encoder::h264::PROFILE_VALUES[AMF_Encoder::h264::PROFILES::PROFILE_COUNT_MAX] = {
	66, 88, 77,
	100, 110, 122, 244,
	83, 86
};

// h264 Levels
const char* AMF_Encoder::h264::LEVEL_NAMES[AMF_Encoder::h264::LEVELS::LEVEL_COUNT_MAX] = {
	AMF_TEXT_H264("LEVEL.10"), AMF_TEXT_H264("LEVEL.11"), AMF_TEXT_H264("LEVEL.12"), AMF_TEXT_H264("LEVEL.13"),
	AMF_TEXT_H264("LEVEL.20"), AMF_TEXT_H264("LEVEL.21"), AMF_TEXT_H264("LEVEL.22"),
	AMF_TEXT_H264("LEVEL.30"), AMF_TEXT_H264("LEVEL.31"), AMF_TEXT_H264("LEVEL.32"),
	AMF_TEXT_H264("LEVEL.40"), AMF_TEXT_H264("LEVEL.41"), AMF_TEXT_H264("LEVEL.42"),
	AMF_TEXT_H264("LEVEL.50"), AMF_TEXT_H264("LEVEL.51"), AMF_TEXT_H264("LEVEL.52")
};
const unsigned char AMF_Encoder::h264::LEVEL_VALUES[LEVELS::LEVEL_COUNT_MAX] = {
	10, 11, 12, 13,
	20, 21, 22,
	30, 31, 32,
	40, 41, 42,
	50, 51, 52,
};

//////////////////////////////////////////////////////////////////////////
// Static Code
//////////////////////////////////////////////////////////////////////////
obs_encoder_info* AMF_Encoder::h264::encoder_info;

void AMF_Encoder::h264::encoder_register() {
	if (!AMF_Encoder::h264::encoder_info) {
		AMF_Encoder::h264::encoder_info = new obs_encoder_info();
		AMF_Encoder::h264::encoder_info->id = "amf_h264_encoder";
		AMF_Encoder::h264::encoder_info->type = obs_encoder_type::OBS_ENCODER_VIDEO;
		AMF_Encoder::h264::encoder_info->codec = "h264";

		// Functions
		AMF_Encoder::h264::encoder_info->get_name = &AMF_Encoder::h264::get_name;
		AMF_Encoder::h264::encoder_info->get_defaults = &AMF_Encoder::h264::get_defaults;
		AMF_Encoder::h264::encoder_info->get_properties = &AMF_Encoder::h264::get_properties;
		AMF_Encoder::h264::encoder_info->create = &AMF_Encoder::h264::create;
		AMF_Encoder::h264::encoder_info->destroy = &AMF_Encoder::h264::destroy;
		AMF_Encoder::h264::encoder_info->encode = &AMF_Encoder::h264::encode;
		AMF_Encoder::h264::encoder_info->update = &AMF_Encoder::h264::update;
		AMF_Encoder::h264::encoder_info->get_video_info = &AMF_Encoder::h264::get_video_info;
		AMF_Encoder::h264::encoder_info->get_extra_data = &AMF_Encoder::h264::get_extra_data;

		obs_register_encoder(AMF_Encoder::h264::encoder_info);
	}
}

const char* AMF_Encoder::h264::get_name(void* type_data) {
	return AMF_TEXT_H264_T("Name");
}

void* AMF_Encoder::h264::create(obs_data_t* settings, obs_encoder_t* encoder) {
	try {
		AMF_Encoder::h264* enc = new AMF_Encoder::h264(settings, encoder);
		return enc;
	} catch (std::exception e) {
		return NULL;
	}
}

void AMF_Encoder::h264::destroy(void* data) {
	AMF_Encoder::h264* enc = static_cast<AMF_Encoder::h264*>(data);
	delete enc;
	data = nullptr;
}

bool AMF_Encoder::h264::encode(void *data, struct encoder_frame *frame, struct encoder_packet *packet, bool *received_packet) {
	return static_cast<AMF_Encoder::h264*>(data)->encode(frame, packet, received_packet);
}

void AMF_Encoder::h264::get_defaults(obs_data_t *settings) {
	//////////////////////////////////////////////////////////////////////////
	// Static Properties (Can't be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	// Quality Preset & Usage
	/// amf_int64(AMF_VIDEO_ENCODER_USAGE_ENUM); default = N/A; Encoder usage type. fully configures parameter set. 
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_USAGE", AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
	/// amf_int64(AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM); default = depends on USAGE; Quality Preset 
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_QUALITY_PRESET", AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY);

	// Profile & Level
	/// amf_int64(AMF_VIDEO_ENCODER_PROFILE_ENUM) ; default = AMF_VIDEO_ENCODER_PROFILE_MAIN;  H264 profile
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_PROFILE", -1);
	/// amf_int64; default = 42; H264 profile level
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_PROFILE_LEVEL", -1);

	// Other
	/// amf_int64; default = 0; Max number of LTR frames
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_MAX_LTR_FRAMES", -1);
	/// amf_int64(AMF_VIDEO_ENCODER_SCANTYPE_ENUM); default = AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE; indicates input stream type
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_SCANTYPE", 0);

	//////////////////////////////////////////////////////////////////////////
	// Dynamic Properties (Can be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	// Rate Control
	/// amf_int64(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM); default = depends on USAGE; Rate Control Method 
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD", -1);
	/// bool; default =  depends on USAGE; Rate Control Based Frame Skip 
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME", -1);
	/// bool; default = depends on USAGE; Enforce HRD
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_ENFORCE_HRD", -1);
	/// bool; default = false; Filler Data Enable
	obs_data_set_default_bool(settings, "AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE", false);
	/// amf_int64; default = 60; GOP Size, in frames
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_GOP_SIZE", -1);
	/// amf_int64; default = depends on USAGE; VBV Buffer Size in bits
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE", -1);
	/// amf_int64; default =  64; Initial VBV Buffer Fullness 0=0% 64=100%
	obs_data_set_default_double(settings, "AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS", 1.0);
	/// amf_int64; default = 60; Max AU Size in bits
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_MAX_AU_SIZE", -1);
	/// amf_int64; default = depends on USAGE; B-picture Delta
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_B_PIC_DELTA_QP", -1);
	/// amf_int64; default = depends on USAGE; Reference B-picture Delta
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP", -1);

	// Rate Control: Constrained QP
	/// amf_int64; default = depends on USAGE; Min QP; range = 0-51
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_MIN_QP", -1);
	/// amf_int64; default = depends on USAGE; Max QP; range = 0-51
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_MAX_QP", -1);
	/// amf_int64; default = 22; I-frame QP; range = 0-51
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_QP_I", -1);
	/// amf_int64; default = 22; P-frame QP; range = 0-51
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_QP_P", -1);
	/// amf_int64; default = 22; B-frame QP; range = 0-51
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_QP_B", -1);

	// Rate Control:  CBR, VBR
	/// amf_int64; default = depends on USAGE; Target bit rate in bits
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_TARGET_BITRATE", -1);
	/// amf_int64; default = depends on USAGE; Peak bit rate in bits
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_PEAK_BITRATE", -1);

	// Picture Control Properties
	/// amf_int64; default = 0; Header Insertion Spacing; range 0-1000
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING", -1);
	/// amf_int64; default = 3; B-picture Pattern (number of B-Frames)
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_B_PIC_PATTERN", -1);
	/// bool; default = depends on USAGE; De-blocking Filter
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER", -1);
	/// bool; default = true; Enable Refrence to B-frames
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE", -1);
	/// amf_int64; default = depends on USAGE; IDR Period in frames
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_IDR_PERIOD", -1);
	/// amf_int64; default = depends on USAGE; Intra Refresh MBs Number Per Slot in Macroblocks
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT", -1);
	/// amf_int64; default = 1; Number of slices Per Frame 
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_SLICES_PER_FRAME", -1);

	// Motion Estimation
	/// bool; default= true; Half Pixel 
	obs_data_set_default_bool(settings, "AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL", true);
	/// bool; default= true; Quarter Pixel
	obs_data_set_default_bool(settings, "AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL", true);

	// SVC (Scalable Profiles)
	/// amf_int64; default = 0; range = 0, min(2, caps->GetMaxNumOfTemporalLayers()) number of temporal enhancment Layers (SVC)
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS", -1);
}

obs_properties_t* AMF_Encoder::h264::get_properties(void* data) {
	obs_property_t* list;
	obs_properties* props = obs_properties_create();

	//ToDo: Reset Button?
	

	//////////////////////////////////////////////////////////////////////////
	// Static Properties (Can't be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	// Usage & Quality Preset
	/// Usage
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_USAGE", AMF_TEXT_H264_T("USAGE"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("USAGE.TRANSCODING"), AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("USAGE.ULTRALOWLATENCY"), AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("USAGE.LOWLATENCY"), AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("USAGE.WEBCAM"), AMF_VIDEO_ENCODER_USAGE_WEBCAM);
	/// Quality Preset
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_QUALITY_PRESET", AMF_TEXT_H264_T("PRESET"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("PRESET.SPEED"), AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("PRESET.BALANCED"), AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("PRESET.QUALITY"), AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY);

	// Profile & Level
	/// h264 Profile
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_PROFILE", AMF_TEXT_H264_T("PROFILE"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("PROFILE.DEFAULT"), -1);
	for (unsigned int i = 0; i < AMF_Encoder::h264::PROFILES::PROFILE_COUNT_MAX; i++) {
		obs_property_list_add_int(list, obs_module_text(AMF_Encoder::h264::PROFILE_NAMES[i]), i);
	}
	/// h264 Profile Level
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_PROFILE_LEVEL", AMF_TEXT_H264_T("LEVEL"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("LEVEL.DEFAULT"), -1);
	for (unsigned int i = 0; i < AMF_Encoder::h264::LEVELS::LEVEL_COUNT_MAX; i++) {
		obs_property_list_add_int(list, obs_module_text(AMF_Encoder::h264::LEVEL_NAMES[i]), i);
	}

	// Other
	/// Maximum LTR Frames
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_MAX_LTR_FRAMES", AMF_TEXT_H264_T("MAXOFLTRFRAMES"), -1, 65535, 1);
	/// Scan Type
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_SCANTYPE", AMF_TEXT_H264_T("SCANTYPE"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("SCANTYPE.PROGRESSIVE"), AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("SCANTYPE.INTERLACED"), AMF_VIDEO_ENCODER_SCANTYPE_INTERLACED);

	//////////////////////////////////////////////////////////////////////////
	// Dynamic Properties (Can be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	// Rate Control
	/// Method
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD", AMF_TEXT_H264_T("RATE_CONTROL"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("RATE_CONTROL.DEFAULT"), -1);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("RATE_CONTROL.CONSTRAINEDQP"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTRAINED_QP);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("RATE_CONTROL.CBR"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("RATE_CONTROL.PEAK_CONSTRAINED_VBR"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("RATE_CONTROL.LATENCY_CONSTRAINED_VBR"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR);
	/// Skip Frames if necessary
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME", AMF_TEXT_H264_T("SKIP_FRAME"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("SKIP_FRAME.DEFAULT"), -1);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("SKIP_FRAME.DISABLE"), 0);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("SKIP_FRAME.ENABLE"), 1);
	/// Enforce HRD (?)
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_ENFORCE_HRD", AMF_TEXT_H264_T("ENFORCE_HRD"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("ENFORCE_HRD.DEFAULT"), -1);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("ENFORCE_HRD.DISABLE"), 0);
	obs_property_list_add_int(list, AMF_TEXT_H264_T("ENFORCE_HRD.ENABLE"), 1);
	/// Filler Data
	obs_properties_add_bool(props, "AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE", AMF_TEXT_H264_T("FILLER_DATA"));
	/// GOP Size
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_GOP_SIZE", AMF_TEXT_H264_T("GOP_SIZE"), -1, 8192, 1);
	/// VBV Buffer
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE", AMF_TEXT_H264_T("VBV_BUFFER_SIZE"), -1, INT_MAX, 1);
	obs_properties_add_float_slider(props, "AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS", AMF_TEXT_H264_T("INITIAL_VBV_BUFFER_FULLNESS"), 0.0, 1.0, 0.015625);
	/// Max AU Size
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_MAX_AU_SIZE", AMF_TEXT_H264_T("MAX_AU_SIZE"), -1, 1024, 1);
	/// B-Picture Delta QP
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_B_PIC_DELTA_QP", AMF_TEXT_H264_T("B_PIC_DELTA_QP"), -1, 51, 1);
	/// Reference B-Picture Delta QP
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP", AMF_TEXT_H264_T("REF_B_PIC_DELTA_QP"), -1, 51, 1);

	// Rate Control: Constrained QP
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_MIN_QP", AMF_TEXT_H264_T("QP.MIN"), -1, 51, 1);
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_MAX_QP", AMF_TEXT_H264_T("QP.MAX"), -1, 51, 1);
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_QP_I", AMF_TEXT_H264_T("QP.I"), -1, 51, 1);
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_QP_P", AMF_TEXT_H264_T("QP.P"), -1, 51, 1);
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_QP_B", AMF_TEXT_H264_T("QP.B"), -1, 51, 1);

	// Rate Control: CBR, VBR
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_TARGET_BITRATE", AMF_TEXT_H264_T("BITRATE.TARGET"), -1, INT_MAX, 1);
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_PEAK_BITRATE", AMF_TEXT_H264_T("BITRATE.PEAK"), -1, INT_MAX, 1);

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
	/// Number of Temporal Enhancment Layers (SVC)
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS", AMF_TEXT_H264_T("NUM_TEMPORAL_ENHANCEMENT_LAYERS"), -1, 1024, 1);

	//
	/// ToDo: Option to override requested Surface Format? Allows for a lot more recording types, including Grayscale.

	return props;
}

bool AMF_Encoder::h264::update(void *data, obs_data_t *settings) {
	return static_cast<AMF_Encoder::h264*>(data)->update(settings);
}

void AMF_Encoder::h264::get_video_info(void *data, struct video_scale_info *info) {
	return static_cast<AMF_Encoder::h264*>(data)->get_video_info(info);
}

bool AMF_Encoder::h264::get_extra_data(void *data, uint8_t** extra_data, size_t* size) {
	return static_cast<AMF_Encoder::h264*>(data)->get_extra_data(extra_data, size);
}

void AMF_Encoder::h264::wa_log_amf_error(AMF_RESULT amfResult, char* sMessage) {
	std::vector<char> msgBuf(1024);
	wcstombs(msgBuf.data(), amf::AMFGetResultText(amfResult), msgBuf.size());

	AMF_LOG_ERROR("%s, error code %d: %s.", sMessage, amfResult, msgBuf.data());
}
void AMF_Encoder::h264::wa_log_property_int(AMF_RESULT amfResult, char* sProperty, int64_t value) {
	char* format = "[AMF_Encoder::h264] Attempted to set property '%s' to '%d', result: %s (%d).";

	// Log AMF Error
	char* amfErrorBuffer = new char[1024];
	wcstombs(amfErrorBuffer, amf::AMFGetResultText(amfResult), 1024);

	blog(LOG_INFO, format, sProperty, value, amfErrorBuffer, amfResult);// , args);

	delete[] amfErrorBuffer;
}
void AMF_Encoder::h264::wa_log_property_bool(AMF_RESULT amfResult, char* sProperty, bool value) {
	char* format = "[AMF_Encoder::h264] Attempted to set property '%s' to '%s', result: %s (%d).";

	// Log AMF Error
	char* amfErrorBuffer = new char[1024];
	wcstombs(amfErrorBuffer, amf::AMFGetResultText(amfResult), 1024);

	blog(LOG_INFO, format, sProperty, value ? "true" : "false", amfErrorBuffer, amfResult);// , args);

	delete[] amfErrorBuffer;
}

//////////////////////////////////////////////////////////////////////////
// Module Code
//////////////////////////////////////////////////////////////////////////
AMF_Encoder::h264::h264(obs_data_t* settings, obs_encoder_t* encoder) {
	AMF_LOG_INFO("Create: Initialization Request...");

	// OBS Settings
	video_t *video = obs_encoder_video(encoder);
	const struct video_output_info *voi = video_output_get_info(video);

	m_cfgWidth = obs_encoder_get_width(encoder);
	m_cfgHeight = obs_encoder_get_height(encoder);
	m_cfgFPSnum = voi->fps_num; m_cfgFPSden = voi->fps_den;
	switch (voi->format) {
		case VIDEO_FORMAT_RGBA:
			m_AMFSurfaceFormat = amf::AMF_SURFACE_RGBA;
			break;
		case VIDEO_FORMAT_I420:
			m_AMFSurfaceFormat = amf::AMF_SURFACE_YUV420P;
			break;
		case VIDEO_FORMAT_NV12:
		default:
			m_AMFSurfaceFormat = amf::AMF_SURFACE_NV12;
			break;
	}

	int64_t t_profile = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_PROFILE_ENUM");

	// Select Memory Type
	m_AMFMemoryType = amf::AMF_MEMORY_HOST; // Host for now.

	AMF_RESULT res = AMFCreateContext(&m_AMFContext);
	if (res != AMF_OK) {
		wa_log_amf_error(res, "Create: Failed to create AMF context");
	}

	// Encoder Component
	switch (t_profile) {
		case h264::PROFILES::PROFILE_SVC_BP:
		case h264::PROFILES::PROFILE_SVC_HiP:
			if (t_profile == h264::PROFILES::PROFILE_SVC_BP)
				t_profile = h264::PROFILE_AVC_BP;
			else
				t_profile = h264::PROFILE_AVC_HiP;
			res = AMFCreateComponent(m_AMFContext, AMFVideoEncoderVCE_SVC, &this->m_AMFEncoder);
		default:
			res = AMFCreateComponent(m_AMFContext, AMFVideoEncoderVCE_AVC, &this->m_AMFEncoder);
	}
	if (res != AMF_OK) {
		wa_log_amf_error(res, "Create: Failed to create AMF component");
	}

	//////////////////////////////////////////////////////////////////////////
	// Static Properties (Can't be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	// Quality Preset & Usage
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, obs_data_get_int(settings, "AMF_VIDEO_ENCODER_USAGE"));
	wa_log_property_int(res, "AMF_VIDEO_ENCODER_USAGE", obs_data_get_int(settings, "AMF_VIDEO_ENCODER_USAGE"));

	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, obs_data_get_int(settings, "AMF_VIDEO_ENCODER_QUALITY_PRESET"));
	wa_log_property_int(res, "AMF_VIDEO_ENCODER_QUALITY_PRESET", obs_data_get_int(settings, "AMF_VIDEO_ENCODER_QUALITY_PRESET"));

	// Framesize & Framerate
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(m_cfgWidth, m_cfgHeight)); // Take from OBS
	wa_log_property_int(res, "AMF_VIDEO_ENCODER_FRAMESIZE.X", m_cfgWidth);
	wa_log_property_int(res, "AMF_VIDEO_ENCODER_FRAMESIZE.Y", m_cfgHeight);

	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(m_cfgFPSnum, m_cfgFPSden)); // Take from OBS
	wa_log_property_int(res, "AMF_VIDEO_ENCODER_FRAMERATE.Num", m_cfgFPSnum);
	wa_log_property_int(res, "AMF_VIDEO_ENCODER_FRAMERATE.Den", m_cfgFPSden);

	// Profile & Level
	t_profile = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_PROFILE");
	if (t_profile != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_Encoder::h264::PROFILE_VALUES[t_profile]);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_PROFILE", AMF_Encoder::h264::PROFILE_VALUES[t_profile]);
	}

	int64_t t_profileLevel = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_PROFILE_LEVEL");
	if (t_profileLevel != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, AMF_Encoder::h264::LEVEL_VALUES[t_profileLevel]);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_PROFILE_LEVEL", AMF_Encoder::h264::LEVEL_VALUES[t_profileLevel]);
	}

	// Other
	int64_t t_MaxLTRFrames = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_MAX_LTR_FRAMES");
	if (t_MaxLTRFrames != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, t_MaxLTRFrames);
		AMF_LOG_INFO("Create: AMF_VIDEO_ENCODER_MAX_LTR_FRAMES = %d", t_MaxLTRFrames);
		//if (res != AMF_OK) AMF_LOG_ERROR("Create: AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, error code %d: %s.", res, amf::AMFGetResultText(res));
	}

	int64_t t_ScanType = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_SCANTYPE");
	if (t_ScanType != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_SCANTYPE, t_ScanType);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_SCANTYPE", t_ScanType);
	}

	//////////////////////////////////////////////////////////////////////////
	// Dynamic Properties (Can be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	update_properties(settings);

	//////////////////////////////////////////////////////////////////////////
	// Initialize (locks static properties)
	//////////////////////////////////////////////////////////////////////////
	res = m_AMFEncoder->Init(m_AMFSurfaceFormat, m_cfgWidth, m_cfgHeight);
	if (res != AMF_OK) {
		wa_log_amf_error(res, "Create: Failed to initialize AMF encoder");
		throw std::exception("Failed to initialize AMF encoder");
	}

	AMF_LOG_INFO("Create: Request completed.");
}

AMF_Encoder::h264::~h264() {
	if (m_AMFEncoder)
		m_AMFEncoder->Terminate();
	if (m_AMFContext)
		m_AMFContext->Terminate();
}

void AMF_Encoder::h264::queue_frame(encoder_frame* frame) {
	AMF_RESULT res;
	amf::AMFSurfacePtr surfaceIn;

	// Early-Exit if either the frame or the contained data is invalid.
	if (!frame || !frame->data[0]) {
		//// Drain Queues.
		res = m_AMFEncoder->Drain();
		return;
	}

	// Create a new frame.
	h264_input_frame* myFrame = new h264_input_frame;

	// Create Surface depending on Memory Type.
	if (m_AMFMemoryType == amf::AMF_MEMORY_HOST) {
		// Host: RAM.
	#ifndef USE_CreateSurfaceFromHostNative
		res = m_AMFContext->AllocSurface(m_AMFMemoryType, m_AMFSurfaceFormat, m_cfgWidth, m_cfgHeight, &surfaceIn);
	#endif

		switch (m_AMFSurfaceFormat) {
			case amf::AMF_SURFACE_NV12:
			{
				// NV12, Y:U+V, Two Plane
			#ifndef USE_CreateSurfaceFromHostNative
				size_t iMax = surfaceIn->GetPlanesCount();
			#pragma loop(hint_parallel(2))
				for (uint8_t i = 0; i < iMax; i++) {
					amf::AMFPlane* plane = surfaceIn->GetPlaneAt(i);
					void* plane_nat = plane->GetNative();
					int32_t height = plane->GetHeight();
					size_t hpitch = plane->GetHPitch();

				#pragma loop(hint_parallel(8))
					for (int32_t py = 0; py < height; py++) {
						size_t plane_off = py * hpitch;
						size_t frame_off = py * frame->linesize[i];
						std::memcpy(static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off), static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
					}
				}
			#else
				myFrame->surfaceBuffer.resize(frame->linesize[0] * m_cfgHeight * 2); // It needs to be 1.5 times height, but 2 is safer for now.
				std::memcpy(myFrame->surfaceBuffer.data(), frame->data[0], frame->linesize[0] * m_cfgHeight);
				std::memcpy(myFrame->surfaceBuffer.data() + (frame->linesize[0] * m_cfgHeight), frame->data[1], frame->linesize[0] * (m_cfgHeight >> 1));
			#endif
				break;
			}
			case amf::AMF_SURFACE_BGRA:
			case amf::AMF_SURFACE_RGBA:
			{
			#ifndef USE_CreateSurfaceFromHostNative
				size_t iMax = surfaceIn->GetPlanesCount();
				for (uint8_t i = 0; i < iMax; i++) {
					amf::AMFPlane* plane = surfaceIn->GetPlaneAt(i);
					void* plane_nat = plane->GetNative();
					int32_t height = plane->GetHeight();
					size_t hpitch = plane->GetHPitch();

				#pragma loop(hint_parallel(8))
					for (int32_t py = 0; py < height; py++) {
						size_t plane_off = py * hpitch;
						size_t frame_off = py * frame->linesize[i];
						std::memcpy(static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off), static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
					}
				}
			#else
				// RGBA/BGRA, Single Plane
				myFrame->surfaceBuffer.resize(frame->linesize[0] * m_cfgHeight);
				std::memcpy(myFrame->surfaceBuffer.data(), frame->data[0], frame->linesize[0] * m_cfgHeight);
			#endif
				break;
			}
			case amf::AMF_SURFACE_GRAY8:
			{
				// Gray 8, Single Component
			#ifndef USE_CreateSurfaceFromHostNative
				size_t iMax = surfaceIn->GetPlanesCount();
				for (uint8_t i = 0; i < iMax; i++) {
					amf::AMFPlane* plane = surfaceIn->GetPlaneAt(i);
					void* plane_nat = plane->GetNative();

					for (int32_t py = 0; py < plane->GetHeight(); py++) {
						size_t plane_off = py * plane->GetHPitch();
						size_t frame_off = py * frame->linesize[i];
						std::memcpy(static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off), static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
					}
				}
			#else
				myFrame->surfaceBuffer.resize(frame->linesize[0] * m_cfgHeight);
				std::memcpy(myFrame->surfaceBuffer.data(), frame->data[0], frame->linesize[0] * m_cfgHeight);
			#endif
				break;
			}
			case amf::AMF_SURFACE_YV12:
			{
				// YVU 4:2:0, Y, subsampled V, subsampled U
			#ifndef USE_CreateSurfaceFromHostNative
				// Flip
				uint8_t* temp = frame->data[1];
				frame->data[1] = frame->data[2];
				frame->data[2] = temp;

				size_t iMax = surfaceIn->GetPlanesCount();
				for (uint8_t i = 0; i < iMax; i++) {
					amf::AMFPlane* plane = surfaceIn->GetPlaneAt(i);
					void* plane_nat = plane->GetNative();

					for (int32_t py = 0; py < plane->GetHeight(); py++) {
						size_t plane_off = py * plane->GetHPitch();
						size_t frame_off = py * frame->linesize[i];
						std::memcpy(static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off), static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
					}
				}

				// Flip back
				temp = frame->data[2];
				frame->data[2] = frame->data[1];
				frame->data[1] = temp;
			#else
				size_t halfHeight = m_cfgHeight >> 1;
				size_t fullFrame = (frame->linesize[0] * m_cfgHeight);
				size_t halfFrame = frame->linesize[2] * halfHeight;

				myFrame->surfaceBuffer.resize(frame->linesize[0] * m_cfgHeight * 2); // We actually need one full and two halved frames. Height * 1.5 should work for this.
				std::memcpy(myFrame->surfaceBuffer.data(), frame->data[0], frame->linesize[0] * m_cfgHeight);
				std::memcpy(myFrame->surfaceBuffer.data() + fullFrame, frame->data[2], frame->linesize[2] * halfHeight);
				std::memcpy(myFrame->surfaceBuffer.data() + (fullFrame + halfFrame), frame->data[1], frame->linesize[1] * halfHeight);
			#endif
				break;
			}
			case amf::AMF_SURFACE_YUV420P:
			{
				// YUV 4:2:0, Y, subsampled U, subsampled V
			#ifndef USE_CreateSurfaceFromHostNative
				size_t iMax = surfaceIn->GetPlanesCount();
			#pragma loop(hint_parallel(3))
				for (uint8_t i = 0; i < iMax; i++) {
					amf::AMFPlane* plane = surfaceIn->GetPlaneAt(i);
					void* plane_nat = plane->GetNative();
					int32_t height = plane->GetHeight();
					size_t hpitch = plane->GetHPitch();

				#pragma loop(hint_parallel(8))
					for (int32_t py = 0; py < height; py++) {
						size_t plane_off = py * hpitch;
						size_t frame_off = py * frame->linesize[i];
						std::memcpy(static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off), static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
					}
				}
			#else
				size_t halfHeight = m_cfgHeight >> 1;
				size_t fullFrame = (frame->linesize[0] * m_cfgHeight);
				size_t halfFrame = frame->linesize[1] * halfHeight;

				myFrame->surfaceBuffer.resize(frame->linesize[0] * m_cfgHeight * 2); // We actually need one full and two halved frames. Height * 1.5 should work for this.
				std::memcpy(myFrame->surfaceBuffer.data(), frame->data[0], frame->linesize[0] * m_cfgHeight);
				std::memcpy(myFrame->surfaceBuffer.data() + fullFrame, frame->data[1], frame->linesize[1] * halfHeight);
				std::memcpy(myFrame->surfaceBuffer.data() + (fullFrame + halfFrame), frame->data[2], frame->linesize[2] * halfHeight);
			#endif
				break;
			}
			case amf::AMF_SURFACE_YUY2:
			{
				// YUY2, Y0,Cb,Y1,Cr
			#ifndef USE_CreateSurfaceFromHostNative
				size_t iMax = surfaceIn->GetPlanesCount();
				for (uint8_t i = 0; i < iMax; i++) {
					amf::AMFPlane* plane = surfaceIn->GetPlaneAt(i);
					void* plane_nat = plane->GetNative();

					for (int32_t py = 0; py < plane->GetHeight(); py++) {
						size_t plane_off = py * plane->GetHPitch();
						size_t frame_off = py * frame->linesize[i];
						std::memcpy(static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off), static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
					}
				}
			#else
				myFrame->surfaceBuffer.resize(frame->linesize[0] * m_cfgHeight);
				std::memcpy(myFrame->surfaceBuffer.data(), frame->data[0], frame->linesize[0] * m_cfgHeight);
			#endif
				break;
			}
		}
	}
#ifdef USE_CreateSurfaceFromHostNative
	res = m_AMFContext->CreateSurfaceFromHostNative(m_AMFSurfaceFormat, m_cfgWidth, m_cfgHeight, m_cfgWidth, m_cfgHeight, myFrame->surfaceBuffer.data(), &surfaceIn, NULL);
#endif
	if (res != AMF_OK) { // Failed to create Surface.
		if (res == AMF_INPUT_FULL) // Drain Queue if full.
			res = m_AMFEncoder->Drain();

		wa_log_amf_error(res, "Encode: Creating AMF Surface failed");
		delete myFrame;
		return;
	}

	// Set per-Surface Data.
	surfaceIn->SetPts(frame->pts);

	// Queue into Input Queue.
	myFrame->surface = surfaceIn;
	this->m_InputQueue.push(myFrame);
}

void AMF_Encoder::h264::update_queues() {
	AMF_RESULT res;
	amf::AMFDataPtr pData;

	// Input.
	if (!m_InputQueue.empty()) {
		do {
			h264_input_frame* myFrame = m_InputQueue.front();
			res = m_AMFEncoder->SubmitInput(myFrame->surface);
			if (res == AMF_OK) {
				m_InputQueue.pop();
				//myFrame->surface->Release(); // Does SubmitInput do this for me?
				delete myFrame;
			}
		} while ((!m_InputQueue.empty()) && (res == AMF_OK));
		if (res != AMF_OK && res != AMF_INPUT_FULL) {
			wa_log_amf_error(res, "Encode: Sending to Encoder failed");
		}
	}

	// Output.
	do {
		res = m_AMFEncoder->QueryOutput(&pData);
		if (res == AMF_OK) {
			h264_output_frame* myFrame = new h264_output_frame();
			myFrame->data = pData;
			m_OutputQueue.push(myFrame);
		}
	} while (res == AMF_OK);
	if (res != AMF_OK && res != AMF_REPEAT) {
		wa_log_amf_error(res, "Encode: Querying output failed");
	}
}

void AMF_Encoder::h264::dequeue_frame(encoder_packet* packet, bool* received_packet) {
	if (m_OutputQueue.empty())
		return;

	h264_output_frame* myFrame = m_OutputQueue.front();
	if (myFrame) {
		m_OutputQueue.pop();

		amf::AMFBufferPtr pBuffer(myFrame->data);
		size_t bufferSize = pBuffer->GetSize();

		if (m_LargeBuffer.size() < bufferSize) {
			size_t newSize = (size_t)exp2(ceil(log2(bufferSize)));
			m_LargeBuffer.resize(newSize);
			AMF_LOG_INFO("Dequeue_Frame: Resized Frame Buffer to %d (incoming data is %d big)...", newSize, bufferSize);
		}
		if ((bufferSize > 0) && (m_LargeBuffer.data()))
			std::memcpy(m_LargeBuffer.data(), pBuffer->GetNative(), bufferSize);

		packet->data = m_LargeBuffer.data();
		packet->size = bufferSize;
		packet->type = OBS_ENCODER_VIDEO;
		packet->pts = myFrame->data->GetPts(); // So far works, but I'm not sure if this is actually correct.
		packet->dts = myFrame->data->GetPts(); // Jackuns VCE fork divided this by ... 10000?

		{ // If it is a Keyframe or not, the light will tell you... the light being this integer here.
			int t_frameDataType = -1;
			pBuffer->GetProperty(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, &t_frameDataType);
			packet->keyframe = (t_frameDataType == AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_IDR);
		}

		// Free AMF Memory
		delete myFrame;

		*received_packet = true;
	}
}

bool AMF_Encoder::h264::encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	if (!frame || !packet || !received_packet)
		return false;

	// Input
	queue_frame(frame);

	// Work
	update_queues();

	// Output
	dequeue_frame(packet, received_packet);

	return true;
}

bool AMF_Encoder::h264::update(obs_data_t* settings) {
	//////////////////////////////////////////////////////////////////////////
	// Dynamic Properties (Can be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	return update_properties(settings);
}

void AMF_Encoder::h264::get_video_info(struct video_scale_info* info) {
	switch (m_AMFSurfaceFormat) {
		case amf::AMF_SURFACE_NV12:
			info->format = VIDEO_FORMAT_NV12;
			break;
		case amf::AMF_SURFACE_YV12: // I420 with UV swapped
			info->format = VIDEO_FORMAT_I420;
			break;
		case amf::AMF_SURFACE_BGRA:
			info->format = VIDEO_FORMAT_BGRA;
			break;
			/// ARGB has no OBS equivalent.
		case amf::AMF_SURFACE_RGBA:
			info->format = VIDEO_FORMAT_RGBA;
			break;
		case amf::AMF_SURFACE_GRAY8:
			info->format = VIDEO_FORMAT_Y800;
			break;
		case amf::AMF_SURFACE_YUV420P:
			info->format = VIDEO_FORMAT_I420;
			break;
		/*case amf::AMF_SURFACE_U8V8: // Has no OBS equivalent, could I use I444 for this?
			info->format = VIDEO_FORMAT_Y800;
			break;*/
		case amf::AMF_SURFACE_YUY2:
			info->format = VIDEO_FORMAT_YUY2;
			break;
		default: // Should never occur.
			m_AMFSurfaceFormat = amf::AMF_SURFACE_NV12;
			info->format = VIDEO_FORMAT_NV12;
			break;
	}
	//info->range = VIDEO_RANGE_FULL;
	//info->colorspace = VIDEO_CS_709;
}

bool AMF_Encoder::h264::get_extra_data(uint8_t** extra_data, size_t* size) {
	// So far I have not observer this being called.

	AMF_LOG_INFO("get_extra_data");
	if (!m_AMFContext)
		return false;
	if (!m_AMFEncoder)
		return false;

	amf::AMFVariant var;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_EXTRADATA, &var);
	if (res == AMF_OK && var.type == amf::AMF_VARIANT_INTERFACE) {
		AMF_LOG_INFO("get_extra_data: Have Extra Data of Type %d", var.type);

		amf::AMFBufferPtr buf(var.pInterface);
		void* bufnat = buf->GetNative();
		*size = buf->GetSize();
		m_ExtraData.resize(*size);
		*extra_data = m_ExtraData.data();
		std::memcpy(*extra_data, bufnat, *size);
		AMF_LOG_INFO("get_extra_data: Extra Data is %d bytes big.", *size);

		return true;
	}
	return false;
}

bool AMF_Encoder::h264::update_properties(obs_data_t* settings) {
	AMF_RESULT res;
	int64_t value;

	//////////////////////////////////////////////////////////////////////////
	// Dynamic Properties (Can be changed during Encoding)
	//////////////////////////////////////////////////////////////////////////
	// Rate Control
	/// Method
	int64_t t_rateControl = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD");
	if (t_rateControl != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, t_rateControl);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD", t_rateControl);
	}
	/// Enable Skip Frame
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, value == 1);
		wa_log_property_bool(res, "AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE", value == 1);
	}
	/// Enforce HRD
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_ENFORCE_HRD");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_ENFORCE_HRD, value == 1);
		wa_log_property_bool(res, "AMF_VIDEO_ENCODER_ENFORCE_HRD", value == 1);
	}
	/// Enable Filler Data
	{
		value = obs_data_get_bool(settings, "AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE");
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, value == 1);
		wa_log_property_bool(res, "AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE", value == 1);
	}
	/// GOP Size
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_GOP_SIZE");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_GOP_SIZE, value);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_GOP_SIZE", value);
	}
	/// VBV Buffer Size
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, value);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE", value);
	}
	/// Initial VBV Buffer Fullnes
	value = (int64_t)ceil(obs_data_get_double(settings, "AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS") * 64);
	{
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS, value);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS", value);
	}
	/// Max AU Size
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_MAX_AU_SIZE");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_AU_SIZE, value);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_MAX_AU_SIZE", value);
	}
	/// B-Picture Delta QP
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_B_PIC_DELTA_QP");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, value);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_B_PIC_DELTA_QP", value);
	}
	/// Ref B-Picture Delta QP
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, value);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP", value);
	}

	// Rate Control Parameters
	switch (t_rateControl) {
		case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTRAINED_QP:
		{
			/// Constrained QP
			value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_MIN_QP");
			if (value != -1) {
				res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MIN_QP, value);
				wa_log_property_int(res, "AMF_VIDEO_ENCODER_MIN_QP", value);
			}
			value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_MAX_QP");
			if (value != -1) {
				res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_QP, value);
				wa_log_property_int(res, "AMF_VIDEO_ENCODER_MAX_QP", value);
			}
			value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_QP_I");
			if (value != -1) {
				res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_I, value);
				wa_log_property_int(res, "AMF_VIDEO_ENCODER_QP_I", value);
			}
			value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_QP_P");
			if (value != -1) {
				res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_P, value);
				wa_log_property_int(res, "AMF_VIDEO_ENCODER_QP_P", value);
			}
			value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_QP_B");
			if (value != -1) {
				res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_B, value);
				wa_log_property_int(res, "AMF_VIDEO_ENCODER_QP_B", value);
			}
			break;
		}
		case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR:
		case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR:
		{
			/// Peak Bitrate
			value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_PEAK_BITRATE");
			if (value != -1) {
				res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, value);
				wa_log_property_int(res, "AMF_VIDEO_ENCODER_PEAK_BITRATE", value);
			}
		}
		case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR:
		{
			/// Target Bitrate
			value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_TARGET_BITRATE");
			if (value != -1) {
				res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, value);
				wa_log_property_int(res, "AMF_VIDEO_ENCODER_TARGET_BITRATE", value);
			}
			break;
		}
	}

	// Picture Control Properties
	/// Header Insertion Spacing
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING, value);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING", value);
	}
	/// B-Pictures Pattern
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_B_PIC_PATTERN");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, value);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_B_PIC_PATTERN", value);
	}
	/// De-Blocking Filter
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER, value == 1);
		wa_log_property_bool(res, "AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER", value == 1);
	}
	/// Enable Reference to B-Frames (2nd Generation GCN and newer)
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, value == 1);
		wa_log_property_bool(res, "AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE", value == 1);
	}
	/// IDR Period (Is this Keyframe distance?)
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_IDR_PERIOD");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, value);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_IDR_PERIOD", value);
	}
	/// Intra Refresh MBs Number Per Slot in Macroblocks
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, value);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT", value);
	}
	/// Number of slices Per Frame 
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_SLICES_PER_FRAME");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, value);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_SLICES_PER_FRAME", value);
	}

	// Motion Estimation
	/// Half Pixel
	{
		value = obs_data_get_bool(settings, "AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL");
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL, value == 1);
		wa_log_property_bool(res, "AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL", value == 1);
	}
	/// Quarter Pixel
	{
		value = obs_data_get_bool(settings, "AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL");
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL, value == 1);
		wa_log_property_bool(res, "AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL", value == 1);
	}

	// SVC (Scalable Profiles)
	/// Number of Temporal Enhancment Layers (SVC)
	value = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS");
	if (value != -1) {
		res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS, value);
		wa_log_property_int(res, "AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS", value);
	}

	return true;
}
