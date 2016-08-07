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

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////
#define AMF_TEXT_H264(x) (AMF_TEXT("H264.Simple." ## x))
#define AMF_TEXT_H264_T(x) obs_module_text(AMF_TEXT_H264(x))

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
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

}

bool AMFEncoder::VCE_H264_Simple_Encoder::encode(void *data, struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {

}

void AMFEncoder::VCE_H264_Simple_Encoder::get_video_info(void *data, struct video_scale_info *info) {

}

bool AMFEncoder::VCE_H264_Simple_Encoder::get_extra_data(void *data, uint8_t** extra_data, size_t* size) {

}

//////////////////////////////////////////////////////////////////////////
// Module Code
//////////////////////////////////////////////////////////////////////////
AMFEncoder::VCE_H264_Simple_Encoder::VCE_H264_Simple_Encoder(obs_data_t* settings, obs_encoder_t* encoder) {

}

AMFEncoder::VCE_H264_Simple_Encoder::~VCE_H264_Simple_Encoder() {

}

bool AMFEncoder::VCE_H264_Simple_Encoder::update(obs_data_t* settings) {

}

bool AMFEncoder::VCE_H264_Simple_Encoder::encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {

}

void AMFEncoder::VCE_H264_Simple_Encoder::get_video_info(struct video_scale_info* info) {

}

bool AMFEncoder::VCE_H264_Simple_Encoder::get_extra_data(uint8_t** extra_data, size_t* size) {

}
