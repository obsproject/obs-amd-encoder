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
	AMF_TEXT("PROFILE.AVC.BP"),
	AMF_TEXT("PROFILE.AVC.XP"),
	AMF_TEXT("PROFILE.AVC.MP"),
	AMF_TEXT("PROFILE.AVC.HiP"),
	AMF_TEXT("PROFILE.AVC.Hi10P"),
	AMF_TEXT("PROFILE.AVC.Hi422P"),
	AMF_TEXT("PROFILE.AVC.Hi444P"),
	AMF_TEXT("PROFILE.SVC.BP"),
	AMF_TEXT("PROFILE.SVC.HiP")
};
const unsigned char AMF_Encoder::h264::PROFILE_VALUES[AMF_Encoder::h264::PROFILES::PROFILE_COUNT_MAX] = {
	66,
	88,
	77,
	100,
	110,
	122,
	244,
	83,
	86
};

// h264 Levels
const char* AMF_Encoder::h264::LEVEL_NAMES[AMF_Encoder::h264::LEVELS::LEVEL_COUNT_MAX] = {
	AMF_TEXT("LEVEL.10"),
	AMF_TEXT("LEVEL.11"),
	AMF_TEXT("LEVEL.12"),
	AMF_TEXT("LEVEL.13"),
	AMF_TEXT("LEVEL.20"),
	AMF_TEXT("LEVEL.21"),
	AMF_TEXT("LEVEL.22"),
	AMF_TEXT("LEVEL.30"),
	AMF_TEXT("LEVEL.31"),
	AMF_TEXT("LEVEL.32"),
	AMF_TEXT("LEVEL.40"),
	AMF_TEXT("LEVEL.41"),
	AMF_TEXT("LEVEL.42"),
	AMF_TEXT("LEVEL.50"),
	AMF_TEXT("LEVEL.51"),
	AMF_TEXT("LEVEL.52")
};
const unsigned char AMF_Encoder::h264::LEVEL_VALUES[LEVELS::LEVEL_COUNT_MAX] = {
	10,
	11,
	12,
	13,
	20,
	21,
	22,
	30,
	31,
	32,
	40,
	41,
	42,
	50,
	51,
	52,
};

void AMF_Encoder::h264::encoder_register() {
	obs_encoder_info encoderInfo = { 0 };

	encoderInfo.id = "amf_h264_encoder";
	encoderInfo.type = obs_encoder_type::OBS_ENCODER_VIDEO;
	encoderInfo.codec = "h264";

	// Functions
	encoderInfo.get_name = &AMF_Encoder::h264::get_name;
	encoderInfo.create = &AMF_Encoder::h264::create;
	encoderInfo.destroy = &AMF_Encoder::h264::destroy;
	encoderInfo.encode = &AMF_Encoder::h264::encode;
	encoderInfo.get_defaults = &AMF_Encoder::h264::get_defaults;
	encoderInfo.get_properties = &AMF_Encoder::h264::get_properties;
	encoderInfo.update = &AMF_Encoder::h264::update;

	obs_register_encoder(&encoderInfo);
}

const char* AMF_Encoder::h264::get_name(void* type_data) {
	return AMF_TEXT_T("Name");
}

void* AMF_Encoder::h264::create(obs_data_t* settings, obs_encoder_t* encoder) {
	std::unique_ptr<AMF_Encoder::h264> enc(new AMF_Encoder::h264(settings, encoder));
	return enc.release();
}

AMF_Encoder::h264::h264(obs_data_t* settings, obs_encoder_t* encoder) {
	AMF_LOG_INFO("h264::h264");

	// Select Memory Type
	s_memoryType = amf::AMF_MEMORY_HOST; // Host for now.
	s_surfaceFormat = amf::AMF_SURFACE_NV12; // NV12 for now

	AMF_RESULT res = AMFCreateContext(&amf_context);
	if (res != AMF_OK) {
		std::stringstream myStream;
		myStream << "Failed to create AMF context, error code " << amf::AMFGetResultText(res) << ".";
		AMF_LOG_ERROR(myStream.str().c_str());
	}

	switch (s_memoryType) {
		case amf::AMF_MEMORY_DX11:
			res = amf_context->InitDX11(NULL);
			break;
		case amf::AMF_MEMORY_OPENGL:
			res = amf_context->InitOpenGL(NULL, NULL, NULL);
			break;
		default: // Default initializes to nothing.
			break;
	}

	// Component Encoder
	//ToDo: Switch Profile (AVC, SVC)
	res = AMFCreateComponent(amf_context, AMFVideoEncoderVCE_AVC, &this->amf_encoder);
	if (res != AMF_OK) {
		std::stringstream myStream;
		myStream << "Failed to create AMF encoder, error code " << amf::AMFGetResultText(res) << ".";
		AMF_LOG_ERROR(myStream.str().c_str());
	}

	// pre Init
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(1920, 1080));
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(60, 1));
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY); // ToDo: Configurable
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_HIGH);
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, LEVEL_VALUES[LEVELS::LEVEL_5_2]);
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);

	// Any Time
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, true); // Fill up to Target Bitrate with padding.
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_GOP_SIZE, 60 / 2); // GOP of half the framerate
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, 8000); // VBV Buffer Size

	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, 8000);
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, 2000);
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 2);

	res = amf_encoder->Init(amf::AMF_SURFACE_NV12, 1920, 1080);
	if (res != AMF_OK) {
		std::stringstream myStream;
		myStream << "Failed to initialize AMF encoder, error code " << amf::AMFGetResultText(res) << ".";
		AMF_LOG_ERROR(myStream.str().c_str());
	}

	//if (amf_surfaceIn == NULL) {
	//	amf_surfaceIn = NULL;
	//	res = amf_context->AllocSurface(s_memoryType, s_surfaceFormat, s_Width, s_Height, &amf_surfaceIn);
	//	FillSurface()
	//}
}

void AMF_Encoder::h264::destroy(void* data) {
	delete (static_cast<AMF_Encoder::h264*>(data));
}

AMF_Encoder::h264::~h264() {
	AMF_LOG_INFO("h264::~h264");
}

bool AMF_Encoder::h264::encode(void *data, struct encoder_frame *frame, struct encoder_packet *packet, bool *received_packet) {
	return static_cast<AMF_Encoder::h264*>(data)->encode(frame, packet, received_packet);
}

bool AMF_Encoder::h264::encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	AMF_LOG_INFO("h264::encode");
	AMF_RESULT res;

	/// Input Handling
	switch (s_memoryType) {
		case amf::AMF_MEMORY_DX11:
			// Not yet Implemented in OBS
			break;
		case amf::AMF_MEMORY_OPENGL:
			// Not yet Implemented in OBS
			break;
		default: // Host: RAM.
			res = amf_context->CreateSurfaceFromHostNative(s_surfaceFormat, s_Width, s_Height, s_Width, s_Height, (void*)frame->data, &amf_surfaceIn, NULL);
	}

	res = amf_encoder->SubmitInput(amf_surfaceIn);
	if (res == AMF_OK) {
		amf_surfaceIn = NULL; // Unsure if I can 
		*received_packet = true;
	} else {
		*received_packet = false;
	}
	
	/// Output Handling
	res = amf_encoder->Drain();
	if (res == AMF_OK) {
		amf::AMFData** ppData;
		res = amf_encoder->QueryOutput(ppData);

	}

	return true;
}

void AMF_Encoder::h264::get_defaults(obs_data_t *settings) {
	AMF_LOG_INFO("h264::get_defaults");
	
}

obs_properties_t* AMF_Encoder::h264::get_properties(void* data) {
	return static_cast<AMF_Encoder::h264*>(data)->get_properties();
}

obs_properties_t* AMF_Encoder::h264::get_properties() {
	AMF_LOG_INFO("h264::get_properties");
	return NULL;
}

bool AMF_Encoder::h264::update(void *data, obs_data_t *settings) {
	return static_cast<AMF_Encoder::h264*>(data)->update(settings);
}

bool AMF_Encoder::h264::update(obs_data_t* settings) {
	AMF_LOG_INFO("h264::update");
	return false;
}

void AMF_Encoder::h264::get_video_info(void *data, struct video_scale_info *info) {
	return static_cast<AMF_Encoder::h264*>(data)->get_video_info(info);
}

void AMF_Encoder::h264::get_video_info(struct video_scale_info* info) {
	AMF_LOG_INFO("h264::get_video_info");
	info->format = VIDEO_FORMAT_NV12;
}


