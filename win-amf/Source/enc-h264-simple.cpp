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

#include "enc-h264-simple.h"
//////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////

// Plugin
#include "win-amf.h"
#include "amf-vce-capabilities.h"

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////
#define AMF_TEXT_H264(x) (AMF_TEXT("H264Simple." ## x))
#define AMF_TEXT_H264_T(x) obs_module_text(AMF_TEXT_H264(x))

#define AMF_VCE_H264_NAME												AMF_TEXT_H264("Name")
#define AMF_VCE_H264_RATECONTROL										AMF_TEXT_H264("RateControl")
#define AMF_VCE_H264_RATECONTROL_CQP									AMF_TEXT_H264("RateControl.CQP")
#define AMF_VCE_H264_RATECONTROL_CBR									AMF_TEXT_H264("RateControl.CBR")
#define AMF_VCE_H264_RATECONTROL_VBR_PEAK_CONSTRAINED					AMF_TEXT_H264("RateControl.VBR.Peak")
#define AMF_VCE_H264_RATECONTROL_VBR_LATENCY_CONSTRAINED				AMF_TEXT_H264("RateControl.VBR.Latency")
#define AMF_VCE_H264_KEYFRAME_INTERVAL									AMF_TEXT_H264("KeyframeInterval")
#define AMF_VCE_H264_BITRATE_TARGET										AMF_TEXT_H264("Bitrate.Target")
#define AMF_VCE_H264_BITRATE_PEAK										AMF_TEXT_H264("Bitrate.Peak")
#define AMF_VCE_H264_QP_MINIMUM											AMF_TEXT_H264("QP.Minimum")
#define AMF_VCE_H264_QP_MAXIMUM											AMF_TEXT_H264("QP.Maximum")
#define AMF_VCE_H264_QP_IFRAME											AMF_TEXT_H264("QP.IFrame")
#define AMF_VCE_H264_QP_PFRAME											AMF_TEXT_H264("QP.PFrame")
#define AMF_VCE_H264_QP_BFRAME											AMF_TEXT_H264("QP.BFrame")
#define AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE								AMF_TEXT_H264("UseCustomBufferSize")
#define AMF_VCE_H264_CUSTOM_BUFFER_SIZE									AMF_TEXT_H264("CustomBufferSize")
#define AMF_VCE_H264_QUALITY_PRESET										AMF_TEXT_H264("QualityPreset")
#define AMF_VCE_H264_QUALITY_PRESET_NONE								AMF_TEXT_H264("QualityPreset.None")
#define AMF_VCE_H264_QUALITY_PRESET_SPEED								AMF_TEXT_H264("QualityPreset.Speed")
#define AMF_VCE_H264_QUALITY_PRESET_BALANCED							AMF_TEXT_H264("QualityPreset.Balanced")
#define AMF_VCE_H264_QUALITY_PRESET_QUALITY								AMF_TEXT_H264("QualityPreset.Quality")
#define AMF_VCE_H264_PROFILE											AMF_TEXT_H264("Profile")
#define AMF_VCE_H264_PROFILE_BASELINE									AMF_TEXT_H264("Profile.Baseline")
#define AMF_VCE_H264_PROFILE_MAIN										AMF_TEXT_H264("Profile.Main")
#define AMF_VCE_H264_PROFILE_HIGH										AMF_TEXT_H264("Profile.High")

#define AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING							AMF_TEXT_H264("RateControl.FrameSkipping")
#define AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING_DEFAULT					AMF_TEXT_H264("RateControl.FrameSkipping.Default")
#define AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING_DISABLED				AMF_TEXT_H264("RateControl.FrameSkipping.Disabled")
#define AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING_ENABLED					AMF_TEXT_H264("RateControl.FrameSkipping.Enabled")
#define AMF_VCE_H264_FILLERDATA											AMF_TEXT_H264("FillerData")
#define AMF_VCE_H264_FILLERDATA_DEFAULT									AMF_TEXT_H264("FillerData.Default")
#define AMF_VCE_H264_FILLERDATA_DISABLED								AMF_TEXT_H264("FillerData.Disabled")
#define AMF_VCE_H264_FILLERDATA_ENABLED									AMF_TEXT_H264("FillerData.Enabled")
#define AMF_VCE_H264_ENFORCEHRD											AMF_TEXT_H264("EnforceHRD")
#define AMF_VCE_H264_ENFORCEHRD_DEFAULT									AMF_TEXT_H264("EnforceHRD.Default")
#define AMF_VCE_H264_ENFORCEHRD_DISABLED								AMF_TEXT_H264("EnforceHRD.Disabled")
#define AMF_VCE_H264_ENFORCEHRD_ENABLED									AMF_TEXT_H264("EnforceHRD.Enabled")
#define AMF_VCE_H264_GOP_SIZE											AMF_TEXT_H264("GOPSize")
#define AMF_VCE_H264_VBVBUFFER_SIZE										AMF_TEXT_H264("VBVBuffer.Size")
#define AMF_VCE_H264_VBVBUFFER_FULLNESS									AMF_TEXT_H264("VBVBuffer.Fullness")
#define AMF_VCE_H264_MAX_AU_SIZE										AMF_TEXT_H264("MaxAUSize")
#define AMF_VCE_H264_BPIC_DELTA_QP										AMF_TEXT_H264("BPicDeltaQP")
#define AMF_VCE_H264_REF_BPIC_DELTA_QP									AMF_TEXT_H264("RefBPicDeltaQP")
#define AMF_VCE_H264_HEADER_INSERTION_SPACING							AMF_TEXT_H264("HeaderInsertionSpacing")
#define AMF_VCE_H264_NUMBER_OF_BPICTURES								AMF_TEXT_H264("NumberOfBPictures")
#define AMF_VCE_H264_DEBLOCKING_FILTER									AMF_TEXT_H264("DeblockingFilter")
#define AMF_VCE_H264_DEBLOCKING_FILTER_DEFAULT							AMF_TEXT_H264("DeblockingFilter.Default")
#define AMF_VCE_H264_DEBLOCKING_FILTER_DISABLED							AMF_TEXT_H264("DeblockingFilter.Disabled")
#define AMF_VCE_H264_DEBLOCKING_FILTER_ENABLED							AMF_TEXT_H264("DeblockingFilter.Enabled")
#define AMF_VCE_H264_BREFERENCE											AMF_TEXT_H264("BReference")
#define AMF_VCE_H264_BREFERENCE_DEFAULT									AMF_TEXT_H264("BReference.Default")
#define AMF_VCE_H264_BREFERENCE_DISABLED								AMF_TEXT_H264("BReference.Disabled")
#define AMF_VCE_H264_BREFERENCE_ENABLED									AMF_TEXT_H264("BReference.Enabled")
#define AMF_VCE_H264_IDR_PERIOD											AMF_TEXT_H264("IDRPeriod")
#define AMF_VCE_H264_INTRAREFRESHNUMMBPERSLOT							AMF_TEXT_H264("IntraRefreshNumMBPerSlot")
#define AMF_VCE_H264_SLICESPERFRAME										AMF_TEXT_H264("SlicesPerFrame")
#define AMF_VCE_H264_MOTIONESTIMATION									AMF_TEXT_H264("MotionEstimation")
#define AMF_VCE_H264_MOTIONESTIMATION_DEFAULT							AMF_TEXT_H264("MotionEstimation.Default")
#define AMF_VCE_H264_MOTIONESTIMATION_NONE								AMF_TEXT_H264("MotionEstimation.None")
#define AMF_VCE_H264_MOTIONESTIMATION_HALF								AMF_TEXT_H264("MotionEstimation.Half")
#define AMF_VCE_H264_MOTIONESTIMATION_QUARTER							AMF_TEXT_H264("MotionEstimation.Quarter")
#define AMF_VCE_H264_MOTIONESTIMATION_BOTH								AMF_TEXT_H264("MotionEstimation.Both")
#define AMF_VCE_H264_NUMBEROFTEMPORALENHANCEMENTLAYERS					AMF_TEXT_H264("NumberOfTemporalEnhancementLayers")

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
obs_encoder_info* AMFEncoder::VCE_H264_Simple_Encoder::encoder_info;

void AMFEncoder::VCE_H264_Simple_Encoder::encoder_register() {
	if (!AMFEncoder::VCE_H264_Simple_Encoder::encoder_info) {
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info = new obs_encoder_info();
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->id = "amf_h264_simple_encoder";
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->type = obs_encoder_type::OBS_ENCODER_VIDEO;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->codec = "h264";

		// Functions
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->get_name = &AMFEncoder::VCE_H264_Simple_Encoder::get_name;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->get_defaults = &AMFEncoder::VCE_H264_Simple_Encoder::get_defaults;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->get_properties = &AMFEncoder::VCE_H264_Simple_Encoder::get_properties;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->create = &AMFEncoder::VCE_H264_Simple_Encoder::create;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->destroy = &AMFEncoder::VCE_H264_Simple_Encoder::destroy;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->encode = &AMFEncoder::VCE_H264_Simple_Encoder::encode;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->update = &AMFEncoder::VCE_H264_Simple_Encoder::update;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->get_video_info = &AMFEncoder::VCE_H264_Simple_Encoder::get_video_info;
		AMFEncoder::VCE_H264_Simple_Encoder::encoder_info->get_extra_data = &AMFEncoder::VCE_H264_Simple_Encoder::get_extra_data;

		obs_register_encoder(AMFEncoder::VCE_H264_Simple_Encoder::encoder_info);
	}
}

const char* AMFEncoder::VCE_H264_Simple_Encoder::get_name(void* type_data) {
	return AMF_TEXT_H264_T("Name");
}

void AMFEncoder::VCE_H264_Simple_Encoder::get_defaults(obs_data_t *settings) {

}

obs_properties_t* AMFEncoder::VCE_H264_Simple_Encoder::get_properties(void* data) {
	obs_properties* props = obs_properties_create();
	obs_property_t* list;
	obs_property_t* p;

	// Main Properties
	/// Rate Control
	list = obs_properties_add_list(props, AMF_VCE_H264_RATECONTROL, obs_module_text(AMF_VCE_H264_RATECONTROL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_CQP), VCE_RATE_CONTROL_CONSTRAINED_QUANTIZATION_PARAMETER);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_CBR), VCE_RATE_CONTROL_CONSTANT_BITRATE);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_VBR_PEAK_CONSTRAINED), VCE_RATE_CONTROL_VARIABLE_BITRATE_PEAK_CONSTRAINED);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_RATECONTROL_VBR_LATENCY_CONSTRAINED), VCE_RATE_CONTROL_VARIABLE_BITRATE_LATENCY_CONSTRAINED);
	obs_property_set_modified_callback(list, &ratecontrolmethod_modified);

	/// Rate Control: CBR, VBR
	obs_properties_add_int(props, AMF_VCE_H264_BITRATE_TARGET, obs_module_text(AMF_VCE_H264_BITRATE_TARGET), 500, AMFEncoder::VCE_Capabilities::getInstance()->m_AVCCaps.maxBitrate / 1000, 1);
	obs_properties_add_int(props, AMF_VCE_H264_BITRATE_PEAK, obs_module_text(AMF_VCE_H264_BITRATE_PEAK), 500, AMFEncoder::VCE_Capabilities::getInstance()->m_AVCCaps.maxBitrate / 1000, 1);

	/// Rate Control: Constrained QP
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_MINIMUM, obs_module_text(AMF_VCE_H264_QP_MINIMUM), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_MAXIMUM, obs_module_text(AMF_VCE_H264_QP_MAXIMUM), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_IFRAME, obs_module_text(AMF_VCE_H264_QP_IFRAME), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_PFRAME, obs_module_text(AMF_VCE_H264_QP_PFRAME), 0, 51, 1);
	obs_properties_add_int_slider(props, AMF_VCE_H264_QP_BFRAME, obs_module_text(AMF_VCE_H264_QP_BFRAME), 0, 51, 1);

	/// VBV Buffer
	p = obs_properties_add_bool(props, AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE, obs_module_text(AMF_VCE_H264_USE_CUSTOM_BUFFER_SIZE));
	obs_properties_add_int(props, AMF_VCE_H264_CUSTOM_BUFFER_SIZE, obs_module_text(AMF_VCE_H264_CUSTOM_BUFFER_SIZE), 1, AMFEncoder::VCE_Capabilities::getInstance()->m_AVCCaps.maxBitrate / 1000, 1);
	obs_property_set_modified_callback(p, &custombuffer_modified);

	/// Keyframe Interval
	obs_properties_add_int(props, AMF_VCE_H264_KEYFRAME_INTERVAL, obs_module_text(AMF_VCE_H264_KEYFRAME_INTERVAL), 0, 60, 1);

	/// Quality Preset
	list = obs_properties_add_list(props, AMF_VCE_H264_QUALITY_PRESET, obs_module_text(AMF_VCE_H264_QUALITY_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_QUALITY_PRESET_SPEED), VCE_QUALITY_PRESET_SPEED);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_QUALITY_PRESET_BALANCED), VCE_QUALITY_PRESET_BALANCED);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_QUALITY_PRESET_QUALITY), VCE_QUALITY_PRESET_QUALITY);

	/// Profile
	list = obs_properties_add_list(props, AMF_VCE_H264_PROFILE, obs_module_text(AMF_VCE_H264_QUALITY_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_BASELINE), VCE_QUALITY_PRESET_SPEED);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_MAIN), VCE_QUALITY_PRESET_BALANCED);
	obs_property_list_add_int(list, obs_module_text(AMF_VCE_H264_PROFILE_HIGH), VCE_QUALITY_PRESET_QUALITY);

	return props;
}

bool AMFEncoder::VCE_H264_Simple_Encoder::ratecontrolmethod_modified(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
	// Reset State
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_BITRATE_PEAK), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_BITRATE_TARGET), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_MINIMUM), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_MAXIMUM), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_IFRAME), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_PFRAME), false);
	obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_BFRAME), false);

	switch (obs_data_get_int(settings, AMF_VCE_H264_RATECONTROL)) {
		case VCE_RATE_CONTROL_CONSTRAINED_QUANTIZATION_PARAMETER:
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_MINIMUM), true);
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_MAXIMUM), true);
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_IFRAME), true);
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_PFRAME), true);
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_QP_BFRAME), true);
			break;
		case VCE_RATE_CONTROL_VARIABLE_BITRATE_PEAK_CONSTRAINED:
		case VCE_RATE_CONTROL_VARIABLE_BITRATE_LATENCY_CONSTRAINED:
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_BITRATE_PEAK), true);
		case VCE_RATE_CONTROL_CONSTANT_BITRATE:
			obs_property_set_visible(obs_properties_get(props, AMF_VCE_H264_BITRATE_TARGET), true);
			break;
	}
	return true;
}

bool AMFEncoder::VCE_H264_Simple_Encoder::custombuffer_modified(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
	return false;
}

void* AMFEncoder::VCE_H264_Simple_Encoder::create(obs_data_t* settings, obs_encoder_t* encoder) {
	try {
		return new AMFEncoder::VCE_H264_Simple_Encoder(settings, encoder);
	} catch (std::exception e) {
		return NULL;
	}
}

void AMFEncoder::VCE_H264_Simple_Encoder::destroy(void* data) {
	delete (static_cast<AMFEncoder::VCE_H264_Simple_Encoder*>(data));
	data = nullptr;
}

bool AMFEncoder::VCE_H264_Simple_Encoder::update(void *data, obs_data_t *settings) {
	return static_cast<AMFEncoder::VCE_H264_Simple_Encoder*>(data)->update(settings);
}

bool AMFEncoder::VCE_H264_Simple_Encoder::encode(void *data, struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	return static_cast<AMFEncoder::VCE_H264_Simple_Encoder*>(data)->encode(frame, packet, received_packet);
}

void AMFEncoder::VCE_H264_Simple_Encoder::get_video_info(void *data, struct video_scale_info *info) {
	static_cast<AMFEncoder::VCE_H264_Simple_Encoder*>(data)->get_video_info(info);
}

bool AMFEncoder::VCE_H264_Simple_Encoder::get_extra_data(void *data, uint8_t** extra_data, size_t* size) {
	return static_cast<AMFEncoder::VCE_H264_Simple_Encoder*>(data)->get_extra_data(extra_data, size);
}

//////////////////////////////////////////////////////////////////////////
// Module Code
//////////////////////////////////////////////////////////////////////////
AMFEncoder::VCE_H264_Simple_Encoder::VCE_H264_Simple_Encoder(obs_data_t* settings, obs_encoder_t* encoder) {

}

AMFEncoder::VCE_H264_Simple_Encoder::~VCE_H264_Simple_Encoder() {

}

bool AMFEncoder::VCE_H264_Simple_Encoder::update(obs_data_t* settings) {
	return false;
}

bool AMFEncoder::VCE_H264_Simple_Encoder::encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	return false;
}

void AMFEncoder::VCE_H264_Simple_Encoder::get_video_info(struct video_scale_info* info) {

}

bool AMFEncoder::VCE_H264_Simple_Encoder::get_extra_data(uint8_t** extra_data, size_t* size) {
	return false;
}
