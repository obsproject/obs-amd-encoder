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
	AMF_LOG_INFO("h264::encoder_register");
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
	AMF_LOG_INFO("h264::get_name");
	return AMF_TEXT_T("Name");
}

void* AMF_Encoder::h264::create(obs_data_t* settings, obs_encoder_t* encoder) {
	AMF_LOG_INFO("h264::create");
	std::unique_ptr<AMF_Encoder::h264> enc(new AMF_Encoder::h264(settings, encoder));
	enc->encoder = encoder;
	enc->settings = settings;
	return enc.release();
}

AMF_Encoder::h264::h264(obs_data_t* settings, obs_encoder_t* encoder) {
	AMF_LOG_INFO("Create: Initialization Request...");

	// Select Memory Type
	s_memoryType = amf::AMF_MEMORY_HOST; // Host for now.
	s_surfaceFormat = amf::AMF_SURFACE_RGBA; // RGBA

	AMF_RESULT res = AMFCreateContext(&amf_context);
	if (res != AMF_OK) {
		AMF_LOG_ERROR("Create: Failed to create AMF context, error code %d: %s.", res, amf::AMFGetResultText(res));
	}

	switch (s_memoryType) {
		case amf::AMF_MEMORY_DX11:
			res = amf_context->InitDX11(NULL);
			break;
		case amf::AMF_MEMORY_OPENGL:
			res = amf_context->InitOpenGL(NULL, NULL, NULL);
			break;
		default: // Default initializes to nothing.
			res = amf_context->InitDX11(NULL);
			break;
	}
	if (res != AMF_OK) {
		AMF_LOG_ERROR("Create: Failed to initialize AMF context, error code %d: %s.", res, amf::AMFGetResultText(res));
	}

	// Component Encoder
	//ToDo: Switch Profile (AVC, SVC)

	res = AMFCreateComponent(amf_context, AMFVideoEncoderVCE_AVC, &this->amf_encoder);
	if (res != AMF_OK) {
		AMF_LOG_ERROR("Create: Failed to create AMF context, error code %d: %s.", res, amf::AMFGetResultText(res));
	}

	// OBS Settings
	video_t *video = obs_encoder_video(encoder);
	const struct video_output_info *voi = video_output_get_info(video);
	
	s_Width = obs_encoder_get_width(encoder);
	s_Height = obs_encoder_get_height(encoder);
	s_FPS_num = voi->fps_num; s_FPS_den = voi->fps_den;

	// Pre Initialization
	///Framesize & Framerate
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(s_Width, s_Height)); // Take from OBS
	AMF_LOG_INFO("Create: AMF_VIDEO_ENCODER_FRAMESIZE = %dx%d", s_Width, s_Height);
	if (res != AMF_OK) AMF_LOG_ERROR("Create: AMF_VIDEO_ENCODER_FRAMESIZE, error code %d: %s.", res, amf::AMFGetResultText(res));

	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(s_FPS_num, s_FPS_den)); // Take from OBS
	AMF_LOG_INFO("Create: AMF_VIDEO_ENCODER_FRAMERATE = %d/%d", s_FPS_num, s_FPS_den);
	if (res != AMF_OK) AMF_LOG_ERROR("Create: AMF_VIDEO_ENCODER_FRAMERATE, error code %d: %s.", res, amf::AMFGetResultText(res));

	///Quality Preset & Usage
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, obs_data_get_int(settings, "AMF_VIDEO_ENCODER_QUALITY_PRESET"));
	AMF_LOG_INFO("Create: AMF_VIDEO_ENCODER_QUALITY_PRESET = %d", obs_data_get_int(settings, "AMF_VIDEO_ENCODER_QUALITY_PRESET"));
	if (res != AMF_OK) AMF_LOG_ERROR("Create: AMF_VIDEO_ENCODER_QUALITY_PRESET, error code %d: %s.", res, amf::AMFGetResultText(res));

	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, obs_data_get_int(settings, "AMF_VIDEO_ENCODER_USAGE_ENUM"));
	AMF_LOG_INFO("Create: AMF_VIDEO_ENCODER_USAGE = %d", obs_data_get_int(settings, "AMF_VIDEO_ENCODER_USAGE_ENUM"));
	if (res != AMF_OK) AMF_LOG_ERROR("Create: AMF_VIDEO_ENCODER_USAGE, error code %d: %s.", res, amf::AMFGetResultText(res));

	///Profile & Profile Level
	int64_t t_profile = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_PROFILE_ENUM");
	if (t_profile != -1) {
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_Encoder::h264::PROFILE_VALUES[t_profile]);
		AMF_LOG_INFO("Create: AMF_VIDEO_ENCODER_PROFILE = %d:%d (%s)", t_profile, AMF_Encoder::h264::PROFILE_VALUES[t_profile], AMF_Encoder::h264::PROFILE_NAMES[t_profile]);
		if (res != AMF_OK) AMF_LOG_ERROR("Create: AMF_VIDEO_ENCODER_PROFILE, error code %d: %s.", res, amf::AMFGetResultText(res));
	}
	int64_t t_profileLevel = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_PROFILE_LEVEL");
	if (t_profileLevel != -1) {
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, AMF_Encoder::h264::LEVEL_VALUES[t_profileLevel]);
		AMF_LOG_INFO("Create: AMF_VIDEO_ENCODER_PROFILE_LEVEL = %d:%d (%s)", t_profileLevel, AMF_Encoder::h264::LEVEL_VALUES[t_profileLevel], AMF_Encoder::h264::LEVEL_NAMES[t_profileLevel]);
		if (res != AMF_OK) AMF_LOG_ERROR("Create: AMF_VIDEO_ENCODER_PROFILE_LEVEL, error code %d: %s.", res, amf::AMFGetResultText(res));
	}

	// During Runtime (only set it here anyway)
	//res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, true);
	//res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_GOP_SIZE, 60 / 2);
	//res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, 8000);

	//res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, 8000);
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, 2000);
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 2);

	res = amf_encoder->Init(s_surfaceFormat, s_Width, s_Height);
	if (res != AMF_OK) {
		AMF_LOG_ERROR("Failed to create AMF context, error code %d: %s.", res, amf::AMFGetResultText(res));
	}

	//if (amf_surfaceIn == NULL) {
	//	amf_surfaceIn = NULL;
	//	res = amf_context->AllocSurface(s_memoryType, s_surfaceFormat, s_Width, s_Height, &amf_surfaceIn);
	//	FillSurface()
	//}
	AMF_LOG_INFO("Create: Request completed.");
}

void AMF_Encoder::h264::destroy(void* data) {
	AMF_LOG_INFO("h264::destroy");
	AMF_Encoder::h264* enc = static_cast<AMF_Encoder::h264*>(data);
	delete enc;
}

AMF_Encoder::h264::~h264() {
	AMF_LOG_INFO("h264::~h264");
}

bool AMF_Encoder::h264::encode(void *data, struct encoder_frame *frame, struct encoder_packet *packet, bool *received_packet) {
	return static_cast<AMF_Encoder::h264*>(data)->encode(frame, packet, received_packet);
}

bool AMF_Encoder::h264::encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	AMF_RESULT res;
	amf::AMFSurfacePtr surfaceIn;

	AMF_LOG_INFO("Encode: Processing Request...");
	// Default to false.
	*received_packet = false;

	/// Input Handling
	if (frame != nullptr) {
		AMF_LOG_INFO("Encode: Input available, attempting to submit...");

		// Submit all Planes
		switch (s_surfaceFormat) {
			case amf::AMF_SURFACE_RGBA:
				// RGBA, Single Plane
				switch (s_memoryType) {
					case amf::AMF_MEMORY_DX11: // Not yet Implemented in OBS
						break;
					case amf::AMF_MEMORY_OPENGL: // Not yet Implemented in OBS
						break;
					default: // Host: RAM.
						res = amf_context->CreateSurfaceFromHostNative(s_surfaceFormat, s_Width, s_Height, frame->linesize[0], 0, frame->data[0], &surfaceIn, &amf_surfaceObserver);
				}
				break;
			case amf::AMF_SURFACE_NV12:
				// Y:U+V, Two Plane
				switch (s_memoryType) {
					case amf::AMF_MEMORY_DX11: // Not yet Implemented in OBS
						break;
					case amf::AMF_MEMORY_OPENGL: // Not yet Implemented in OBS
						break;
					default: // Host: RAM.
						res = amf_context->CreateSurfaceFromHostNative(s_surfaceFormat, s_Width, s_Height, frame->linesize[0], 2, frame->data[0], &surfaceIn, &amf_surfaceObserver);
				}
		}

		if (res != AMF_OK) {
			const wchar_t* errormsg = amf::AMFGetResultText(res);
			char* outbuf = new char[1024];
			wcstombs(outbuf, errormsg, 1024);
			AMF_LOG_ERROR("Encode: Failed to copy to AMF Surface, error code %d: %s.", res, outbuf);
			delete outbuf;
			return false;
		}
		AMF_LOG_INFO("Encode: Input copied to AMF Surface.");

		surfaceIn->SetPts(frame->pts);

		AMF_LOG_INFO("Encode: Submitting Surface to Encoder...");
		try {
			res = amf_encoder->SubmitInput(surfaceIn);
		} catch (...) {
			AMF_LOG_ERROR("Encode: Unknown Exception occured.");
		}
		if (res == AMF_OK) {
			AMF_LOG_INFO("Encode: Successfully sent to Encoder.");
			surfaceIn = NULL; // Automatically deletes surface?
		} else {
			const wchar_t* errormsg = amf::AMFGetResultText(res);
			char* outbuf = new char[1024];
			wcstombs(outbuf, errormsg, 1024);
			AMF_LOG_ERROR("Encode: Failed to send to Encoder, error code %d: %s.", res, outbuf);
			delete outbuf;
			return false;
		}
		//ToDo: Queue incoming frames if queue is full (should never happen anyway).
	}

	/// Output Handling
	amf::AMFDataPtr pData;
	res = amf_encoder->QueryOutput(&pData);
	if (res == AMF_REPEAT) {
		return true;
	} else if (res != AMF_OK) {
		const wchar_t* errormsg = amf::AMFGetResultText(res);
		char* outbuf = new char[1024];
		wcstombs(outbuf, errormsg, 1024);
		AMF_LOG_ERROR("Encode: Failed to query Output, %d: %s.", res, outbuf);
		delete outbuf;
		return false;
	}
	AMF_LOG_INFO("Encode: Queried output, sending to OBS...");

	// Query Buffer
	amf::AMFBufferPtr pBuffer(pData);
	packet->data = new uint8_t[pBuffer->GetSize()];
	packet->size = pBuffer->GetSize();
	void* pNative = pBuffer->GetNative();
	std::memcpy(packet->data, pNative, packet->size);
	packet->pts = pData->GetPts();

	*received_packet = true;

	AMF_LOG_INFO("Encode: Request processed.");
	return true;
}

void AMF_Encoder::h264::get_defaults(obs_data_t *settings) {
	AMF_LOG_INFO("h264::get_defaults");

	// Quality Preset & Usage
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_QUALITY_PRESET",
		AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY); // amf_int64(AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM); default = depends on USAGE; Quality Preset 
	///AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED
	///AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED
	///AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_USAGE_ENUM",
		AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY); // amf_int64(AMF_VIDEO_ENCODER_USAGE_ENUM); default = N/A; Encoder usage type. fully configures parameter set. 
	///AMF_VIDEO_ENCODER_USAGE_TRANSCONDING;
	///AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY;
	///AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY;
	///AMF_VIDEO_ENCODER_USAGE_WEBCAM;

	// Profile & Level
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_PROFILE_ENUM",
		-1); // amf_int64(AMF_VIDEO_ENCODER_PROFILE_ENUM) ; default = AMF_VIDEO_ENCODER_PROFILE_MAIN;  H264 profile
	///AMF_VIDEO_ENCODER_PROFILE_BASELINE
	///AMF_VIDEO_ENCODER_PROFILE_MAIN
	///AMF_VIDEO_ENCODER_PROFILE_HIGH
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_PROFILE_LEVEL",
		-1); // amf_int64; default = 42; H264 profile level
	///AMF_Encoder::h264::LEVELS

	// Rate Control
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD",
		-1); // amf_int64(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM); default = depends on USAGE; Rate Control Method 
	///AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTRAINED_QP
	///AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR
	///AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR
	///AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR
	obs_data_set_default_bool(settings, "AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE",
		false); // bool; default =  depends on USAGE; Rate Control Based Frame Skip 

	// CBR, VBR
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_TARGET_BITRATE",
		-1); // amf_int64; default = depends on USAGE; Target bit rate in bits
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_PEAK_BITRATE",
		-1); // amf_int64; default = depends on USAGE; Peak bit rate in bits
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE",
		-1); // amf_int64; default = depends on USAGE; VBV Buffer Size in bits

	// Constrained QP
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_MIN_QP",
		-1); // amf_int64; default = depends on USAGE; Min QP; range = 0-51
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_MAX_QP",
		-1); // amf_int64; default = depends on USAGE; Max QP; range = 0-51
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_QP_I",
		-1); // amf_int64; default = 22; I-frame QP; range = 0-51
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_QP_P",
		-1); // amf_int64; default = 22; P-frame QP; range = 0-51
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_QP_B",
		-1); // amf_int64; default = 22; B-frame QP; range = 0-51

	// Other
	obs_data_set_default_bool(settings, "AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE",
		true); // bool; default = false; Filler Data Enable
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_GOP_SIZE",
		-1); // amf_int64; default = 60; GOP Size, in frames

//			 // Static properties - can be set befor Init()
//#define AMF_VIDEO_ENCODER_EXTRADATA                             L"ExtraData"                // AMFInterface* - > AMFBuffer*; SPS/PPS buffer - read-only
//#define AMF_VIDEO_ENCODER_MAX_LTR_FRAMES                        L"MaxOfLTRFrames"           // amf_int64; default = 0; Max number of LTR frames
//#define AMF_VIDEO_ENCODER_SCANTYPE                              L"ScanType"                 // amf_int64(AMF_VIDEO_ENCODER_SCANTYPE_ENUM); default = AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE; indicates input stream type
//
//			 // Rate control properties
//#define AMF_VIDEO_ENCODER_B_PIC_DELTA_QP                        L"BPicturesDeltaQP"         // amf_int64; default = depends on USAGE; B-picture Delta
//#define AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP                    L"ReferenceBPicturesDeltaQP" //  amf_int64; default = depends on USAGE; Reference B-picture Delta
//#define AMF_VIDEO_ENCODER_ENFORCE_HRD                           L"EnforceHRD"               // bool; default = depends on USAGE; Enforce HRD
//#define AMF_VIDEO_ENCODER_MAX_AU_SIZE                           L"MaxAUSize"                // amf_int64; default = 60; Max AU Size in bits
//
//			 // Picture control properties - 
//#define AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING              L"HeaderInsertionSpacing"   // amf_int64; default = 0; Header Insertion Spacing; range 0-1000
//#define AMF_VIDEO_ENCODER_B_PIC_PATTERN                         L"BPicturesPattern"         // amf_int64; default = 3; B-picture Pattern (number of B-Frames)
//#define AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER                    L"DeBlockingFilter"         // bool; default = depends on USAGE; De-blocking Filter
//#define AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE                    L"BReferenceEnable"         // bool; default = true; Enable Refrence to B-frames
//#define AMF_VIDEO_ENCODER_IDR_PERIOD                            L"IDRPeriod"                // amf_int64; default = depends on USAGE; IDR Period in frames
//#define AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT        L"IntraRefreshMBsNumberPerSlot" // amf_int64; default = depends on USAGE; Intra Refresh MBs Number Per Slot in Macroblocks
//#define AMF_VIDEO_ENCODER_SLICES_PER_FRAME                      L"SlicesPerFrame"           // amf_int64; default = 1; Number of slices Per Frame 
//
//			 // Motion estimation
//#define AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL                     L"HalfPixel"                // bool; default= true; Half Pixel 
//#define AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL                   L"QuarterPixel"             // bool; default= true; Quarter Pixel
//
//			 // SVC
//#define AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS        L"NumOfTemporalEnhancmentLayers" // amf_int64; default = 0; range = 0, min(2, caps->GetMaxNumOfTemporalLayers()) number of temporal enhancment Layers (SVC)
}

obs_properties_t* AMF_Encoder::h264::get_properties(void* data) {
	return static_cast<AMF_Encoder::h264*>(data)->get_properties();
}

obs_properties_t* AMF_Encoder::h264::get_properties() {
	AMF_LOG_INFO("h264::get_properties");

	obs_properties* props = obs_properties_create();
	obs_property_t *list;
	obs_property_t *p;

	// Quality Preset & Usage
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_QUALITY_PRESET", AMF_TEXT_T("PRESET"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_T("PRESET.SPEED"), AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
	obs_property_list_add_int(list, AMF_TEXT_T("PRESET.BALANCED"), AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED);
	obs_property_list_add_int(list, AMF_TEXT_T("PRESET.QUALITY"), AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY);

	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_USAGE_ENUM", AMF_TEXT_T("USAGE"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_T("USAGE.TRANSCODING"), AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
	obs_property_list_add_int(list, AMF_TEXT_T("USAGE.ULTRALOWLATENCY"), AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
	obs_property_list_add_int(list, AMF_TEXT_T("USAGE.LOWLATENCY"), AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY);
	obs_property_list_add_int(list, AMF_TEXT_T("USAGE.WEBCAM"), AMF_VIDEO_ENCODER_USAGE_WEBCAM);

	// Profile & Level
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_PROFILE_ENUM", AMF_TEXT_T("PROFILE"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_T("PROFILE.DEFAULT"), -1);
	for (unsigned int i = 0; i < AMF_Encoder::h264::PROFILES::PROFILE_COUNT_MAX; i++) {
		obs_property_list_add_int(list, obs_module_text(AMF_Encoder::h264::PROFILE_NAMES[i]), i);
	}

	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_PROFILE_LEVEL", AMF_TEXT_T("LEVEL"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_T("LEVEL.DEFAULT"), -1);
	for (unsigned int i = 0; i < AMF_Encoder::h264::LEVELS::LEVEL_COUNT_MAX; i++) {
		obs_property_list_add_int(list, obs_module_text(AMF_Encoder::h264::LEVEL_NAMES[i]), i);
	}

	return props;
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
	switch (s_surfaceFormat) {
		case amf::AMF_SURFACE_NV12:
			info->format = VIDEO_FORMAT_NV12;
			return;
		/*case amf::AMF_SURFACE_YV12:
			info->format = VIDEO_FORMAT_;
			return;*/
		case amf::AMF_SURFACE_BGRA:
			info->format = VIDEO_FORMAT_BGRA;
			return;
		/*case amf::AMF_SURFACE_ARGB:
			info->format = VIDEO_FORMAT_;
			return;*/
		case amf::AMF_SURFACE_RGBA:
			info->format = VIDEO_FORMAT_RGBA;
			return;
		case amf::AMF_SURFACE_GRAY8:
			info->format = VIDEO_FORMAT_Y800;
			return;
		case amf::AMF_SURFACE_YUV420P:
			info->format = VIDEO_FORMAT_I420;
			return;
			/*case amf::AMF_SURFACE_U8V8:
			info->format = VIDEO_FORMAT_Y800;
			return;*/
		case amf::AMF_SURFACE_YUY2:
			info->format = VIDEO_FORMAT_YUY2;
			return;
	}
}

void AMF_STD_CALL AMF_Encoder::h264_SurfaceObserver::OnSurfaceDataRelease(amf::AMFSurface* pSurface) {
	//pSurface->
}
