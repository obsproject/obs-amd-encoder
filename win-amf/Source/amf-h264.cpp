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
	AMF_TEXT("PROFILE.AVC.BP"), AMF_TEXT("PROFILE.AVC.XP"), AMF_TEXT("PROFILE.AVC.MP"),
	AMF_TEXT("PROFILE.AVC.HiP"), AMF_TEXT("PROFILE.AVC.Hi10P"), AMF_TEXT("PROFILE.AVC.Hi422P"), AMF_TEXT("PROFILE.AVC.Hi444P"),
	AMF_TEXT("PROFILE.SVC.BP"), AMF_TEXT("PROFILE.SVC.HiP")
};
const unsigned char AMF_Encoder::h264::PROFILE_VALUES[AMF_Encoder::h264::PROFILES::PROFILE_COUNT_MAX] = {
	66, 88, 77,
	100, 110, 122, 244,
	83, 86
};

// h264 Levels
const char* AMF_Encoder::h264::LEVEL_NAMES[AMF_Encoder::h264::LEVELS::LEVEL_COUNT_MAX] = {
	AMF_TEXT("LEVEL.10"), AMF_TEXT("LEVEL.11"), AMF_TEXT("LEVEL.12"), AMF_TEXT("LEVEL.13"),
	AMF_TEXT("LEVEL.20"), AMF_TEXT("LEVEL.21"), AMF_TEXT("LEVEL.22"),
	AMF_TEXT("LEVEL.30"), AMF_TEXT("LEVEL.31"), AMF_TEXT("LEVEL.32"),
	AMF_TEXT("LEVEL.40"), AMF_TEXT("LEVEL.41"), AMF_TEXT("LEVEL.42"),
	AMF_TEXT("LEVEL.50"), AMF_TEXT("LEVEL.51"), AMF_TEXT("LEVEL.52")
};
const unsigned char AMF_Encoder::h264::LEVEL_VALUES[LEVELS::LEVEL_COUNT_MAX] = {
	10, 11, 12, 13,
	20, 21, 22,
	30, 31, 32,
	40, 41, 42,
	50, 51, 52,
};

obs_encoder_info* AMF_Encoder::h264::encoder_info;
void AMF_Encoder::h264::encoder_register() {
	AMF_LOG_INFO("h264::encoder_register");
	//AMF_Encoder::h264::encoder_info = { 0 };

	if (!AMF_Encoder::h264::encoder_info) {
		AMF_Encoder::h264::encoder_info = new obs_encoder_info();
		AMF_Encoder::h264::encoder_info->id = "amf_h264_encoder";
		AMF_Encoder::h264::encoder_info->type = obs_encoder_type::OBS_ENCODER_VIDEO;
		AMF_Encoder::h264::encoder_info->codec = "h264";

		// Functions
		AMF_Encoder::h264::encoder_info->get_name = &AMF_Encoder::h264::get_name;
		AMF_Encoder::h264::encoder_info->create = &AMF_Encoder::h264::create;
		AMF_Encoder::h264::encoder_info->destroy = &AMF_Encoder::h264::destroy;
		AMF_Encoder::h264::encoder_info->encode = &AMF_Encoder::h264::encode;
		AMF_Encoder::h264::encoder_info->get_defaults = &AMF_Encoder::h264::get_defaults;
		AMF_Encoder::h264::encoder_info->get_properties = &AMF_Encoder::h264::get_properties;
		//AMF_Encoder::h264::encoder_info->update = &AMF_Encoder::h264::update;

		obs_register_encoder(AMF_Encoder::h264::encoder_info);
	}
}

const char* AMF_Encoder::h264::get_name(void* type_data) {
	AMF_LOG_INFO("h264::get_name");
	return AMF_TEXT_T("Name");
}

void* AMF_Encoder::h264::create(obs_data_t* settings, obs_encoder_t* encoder) {
	AMF_LOG_INFO("h264::create");
	AMF_Encoder::h264* enc = new AMF_Encoder::h264(settings, encoder);
	return enc;
}

AMF_Encoder::h264::h264(obs_data_t* settings, obs_encoder_t* encoder) {
	AMF_LOG_INFO("Create: Initialization Request...");

	// OBS Settings
	video_t *video = obs_encoder_video(encoder);
	const struct video_output_info *voi = video_output_get_info(video);

	s_Width = obs_encoder_get_width(encoder);
	s_Height = obs_encoder_get_height(encoder);
	s_FPS_num = voi->fps_num; s_FPS_den = voi->fps_den;
	switch (voi->format) {
		case VIDEO_FORMAT_RGBA:
			s_surfaceFormat = amf::AMF_SURFACE_RGBA;
			break;
		case VIDEO_FORMAT_NV12:
		default:
			s_surfaceFormat = amf::AMF_SURFACE_NV12;
			break;
	}

	// Select Memory Type
	s_memoryType = amf::AMF_MEMORY_HOST; // Host for now.

	AMF_RESULT res = AMFCreateContext(&amf_context);
	if (res != AMF_OK) {
		AMF_LOG_ERROR("Create: Failed to create AMF context, error code %d: %s.", res, amf::AMFGetResultText(res));
	}

	// Encoder Component
	switch (obs_data_get_int(settings, "AMF_VIDEO_ENCODER_PROFILE_ENUM")) {
		case h264::PROFILES::PROFILE_SVC_BP:
		case h264::PROFILES::PROFILE_SVC_HiP:
			res = AMFCreateComponent(amf_context, AMFVideoEncoderVCE_SVC, &this->amf_encoder);
		default:
			res = AMFCreateComponent(amf_context, AMFVideoEncoderVCE_AVC, &this->amf_encoder);
	}
	if (res != AMF_OK) {
		AMF_LOG_ERROR("Create: Failed to create AMF context, error code %d: %s.", res, amf::AMFGetResultText(res));
	}

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
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, obs_data_get_int(settings, "AMF_VIDEO_ENCODER_USAGE_ENUM"));

	///Profile & Profile Level
	int64_t t_profile = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_PROFILE_ENUM");
	if (t_profile != -1)
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_Encoder::h264::PROFILE_VALUES[t_profile]);
	int64_t t_profileLevel = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_PROFILE_LEVEL");
	if (t_profileLevel != -1)
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, AMF_Encoder::h264::LEVEL_VALUES[t_profileLevel]);

	/// Rate Control
	int64_t t_rateControl = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD");
	if (t_rateControl != -1)
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, t_rateControl);
	int64_t t_skipFrameEnable = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE");
	if (t_skipFrameEnable != -1)
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, t_skipFrameEnable == 1);

	/// CBR, VBR
	int64_t t_bitrateTarget = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_TARGET_BITRATE");
	if (t_bitrateTarget != -1)
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, t_bitrateTarget);
	int64_t t_bitratePeak = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_PEAK_BITRATE");
	if (t_bitratePeak != -1)
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, t_bitratePeak);
	int64_t t_vbvBufferSize = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE");
	if (t_vbvBufferSize != -1)
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, t_vbvBufferSize);

	/// Constrained QP
	int64_t t_qpMin = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_MIN_QP");
	if (t_qpMin != -1)
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_MIN_QP, t_qpMin);
	int64_t t_qpMax = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_MAX_QP");
	if (t_qpMax != -1)
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_MAX_QP, t_qpMax);
	int64_t t_qpI = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_QP_I");
	if (t_qpI != -1)
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_QP_I, t_qpI);
	int64_t t_qpP = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_QP_P");
	if (t_qpP != -1)
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_QP_P, t_qpP);
	int64_t t_qpB = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_QP_B");
	if (t_qpB != -1)
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_QP_B, t_qpB);

	// Other
	res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE,
		obs_data_get_bool(settings, "AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE"));
	int64_t t_gopSize = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_GOP_SIZE");
	if (t_gopSize != -1)
		res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_GOP_SIZE, t_gopSize);

	// During Runtime (only set it here anyway)
	//res = amf_encoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, 2);

	res = amf_encoder->Init(s_surfaceFormat, s_Width, s_Height);
	if (res != AMF_OK) {
		AMF_LOG_ERROR("Failed to create AMF context, error code %d: %s.", res, amf::AMFGetResultText(res));
	}

	AMF_LOG_INFO("Create: Request completed.");
}

void AMF_Encoder::h264::destroy(void* data) {
	AMF_Encoder::h264* enc = static_cast<AMF_Encoder::h264*>(data);
	delete enc;
	data = nullptr;
}

AMF_Encoder::h264::~h264() {
	if (amf_encoder)
		amf_encoder->Terminate();
	if (amf_context)
		amf_context->Terminate();
}

bool AMF_Encoder::h264::encode(void *data, struct encoder_frame *frame, struct encoder_packet *packet, bool *received_packet) {
	if (!frame)
		AMF_LOG_INFO("h264::encode: Frame is Null");
	if (!packet)
		AMF_LOG_INFO("h264::encode: Packet is Null");
	if (!received_packet)
		AMF_LOG_INFO("h264::encode: Received_Packet is Null");

	return static_cast<AMF_Encoder::h264*>(data)->encode(frame, packet, received_packet);
}

void AMF_Encoder::h264::queue_frame(encoder_frame* frame) {
	AMF_RESULT res;
	amf::AMFSurfacePtr surfaceIn;

	// Early-Exit if either the frame or the contained data is invalid.
	if (!frame || !frame->data[0]) {
		//// Drain Queues.
		res = amf_encoder->Drain();
		return;
	}

	// Create Surface depending on Memory Type.
	if (s_memoryType == amf::AMF_MEMORY_HOST) {
		// Host: RAM.
		switch (s_surfaceFormat) {
			case amf::AMF_SURFACE_NV12:
				// Y:U+V, Two Plane
				res = amf_context->AllocSurface(s_memoryType, s_surfaceFormat, s_Width, s_Height, &surfaceIn);
				size_t iMax = surfaceIn->GetPlanesCount();
				for (uint8_t i = 0; i < iMax; i++) {
					amf::AMFPlane* plane = surfaceIn->GetPlaneAt(i);
					void* plane_nat = plane->GetNative();
					
					// Copy to target buffer. Strangely distorted, perhaps not the right way?
					std::memcpy(plane_nat, frame->data[i], plane->GetVPitch() * plane->GetHeight());
//					else
//						std::memcpy(plane_nat, frame->data[i], plane->GetVPitch() * plane->GetHeight());
				}
				break;
			//case amf::AMF_SURFACE_RGBA:
			//	// RGBA, Single Plane
			//	res = amf_context->CreateSurfaceFromHostNative(s_surfaceFormat, s_Width, s_Height, s_Width, s_Height, frame->data[0], &surfaceIn, NULL);
			//	break;
		}
	}
	if (res != AMF_OK) { // Failed to create Surface.
		const wchar_t* errormsg = amf::AMFGetResultText(res);
		char* outbuf = new char[1024];
		wcstombs(outbuf, errormsg, 1024);
		AMF_LOG_ERROR("Encode: Failed to copy to AMF Surface, error code %d: %s.", res, outbuf);
		delete outbuf;
		return;
	}

	// Set per-Surface Data.
	//surfaceIn->AddObserver(&amf_surfaceObserver);
	surfaceIn->SetPts(frame->pts);

	// Queue into Input Queue.
	h264_input_frame* myFrame = new h264_input_frame;
	myFrame->surface = surfaceIn;
	this->inputQueue.push(myFrame);

	//// Attempt to send it off.
	//res = amf_encoder->SubmitInput(surfaceIn);
	//if (res == AMF_OK) {
	//	return;
	//} else if (res == AMF_INPUT_FULL) {
	//} else {

	//	if (res == AMF_INPUT_FULL) {
	//		// Drain Input Queue
	//		do {
	//			res = amf_encoder->Drain();
	//		} while (res == AMF_INPUT_FULL);
	//	} else if (res != AMF_OK) {
	//	}
	//}
}

void AMF_Encoder::h264::update_queues() {
	AMF_RESULT res;
	amf::AMFDataPtr pData;

	// Input.
	if (!inputQueue.empty()) {
		do {
			h264_input_frame* myFrame = inputQueue.front();
			res = amf_encoder->SubmitInput(myFrame->surface);
			if (res == AMF_OK) {
				inputQueue.pop();
				//myFrame->surface->Release(); // Does SubmitInput do this for me?
				delete myFrame;
			}
		} while ((!inputQueue.empty()) && (res == AMF_OK));
		if (res != AMF_OK && res != AMF_INPUT_FULL) {
			const wchar_t* errormsg = amf::AMFGetResultText(res);
			char* outbuf = new char[1024];
			wcstombs(outbuf, errormsg, 1024);
			AMF_LOG_ERROR("Update Queues: Failed to send to Encoder, error code %d: %s.", res, outbuf);
			delete outbuf;
		}
	}

	// Output.
	do {
		res = amf_encoder->QueryOutput(&pData);
		if (res == AMF_OK) {
			h264_output_frame* myFrame = new h264_output_frame();
			myFrame->data = pData;
			outputQueue.push(myFrame);
		}
	} while (res == AMF_OK);
	if (res != AMF_OK && res != AMF_REPEAT) {
		const wchar_t* errormsg = amf::AMFGetResultText(res);
		char* outbuf = new char[1024];
		wcstombs(outbuf, errormsg, 1024);
		AMF_LOG_ERROR("Encode: Output failed, error code %d: %s.", res, outbuf);
		delete outbuf;
	}
}

void AMF_Encoder::h264::dequeue_frame(encoder_packet* packet, bool* received_packet) {
	if (outputQueue.empty())
		return;

	h264_output_frame* myFrame = outputQueue.front();
	if (myFrame) {
		outputQueue.pop();

		amf::AMFBufferPtr pBuffer(myFrame->data);
		packet->size = pBuffer->GetSize();

		if ((largeBuffer && (largeBufferSize < packet->size)) || (!largeBuffer)) {
			if (largeBuffer) {
				AMF_LOG_INFO("Dequeue_Frame: Buffer too small (%d < %d), deleting...", largeBufferSize, packet->size);
				delete[] largeBuffer;
			}
			AMF_LOG_INFO("Dequeue_Frame: Creating Buffer with size %d...", packet->size);
			largeBuffer = new uint8_t[packet->size + 1];
			largeBufferSize = packet->size;
		}

		packet->data = largeBuffer;
		std::memcpy(packet->data, pBuffer->GetNative(), packet->size);
		packet->type = OBS_ENCODER_VIDEO;

		packet->pts = myFrame->data->GetPts();
		packet->dts = myFrame->data->GetPts();
		packet->keyframe = true;

		// Free AMF Memory
		//myFrame->data->Release();
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

void AMF_Encoder::h264::get_defaults(obs_data_t *settings) {
	AMF_LOG_INFO("h264::get_defaults");

	// Quality Preset & Usage
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_QUALITY_PRESET", AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY);
	// amf_int64(AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM); default = depends on USAGE; Quality Preset 
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_USAGE_ENUM", AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY);
	// amf_int64(AMF_VIDEO_ENCODER_USAGE_ENUM); default = N/A; Encoder usage type. fully configures parameter set. 

	// Profile & Level
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_PROFILE_ENUM", -1);
	// amf_int64(AMF_VIDEO_ENCODER_PROFILE_ENUM) ; default = AMF_VIDEO_ENCODER_PROFILE_MAIN;  H264 profile
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_PROFILE_LEVEL", -1);
	// amf_int64; default = 42; H264 profile level

	// Rate Control
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD", -1);
	// amf_int64(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM); default = depends on USAGE; Rate Control Method 
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME", -1);
	// bool; default =  depends on USAGE; Rate Control Based Frame Skip 

	// CBR, VBR
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_TARGET_BITRATE", -1);
	// amf_int64; default = depends on USAGE; Target bit rate in bits
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_PEAK_BITRATE", -1);
	// amf_int64; default = depends on USAGE; Peak bit rate in bits
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE", -1);
	// amf_int64; default = depends on USAGE; VBV Buffer Size in bits

	// Constrained QP
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_MIN_QP", -1);
	// amf_int64; default = depends on USAGE; Min QP; range = 0-51
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_MAX_QP", -1);
	// amf_int64; default = depends on USAGE; Max QP; range = 0-51
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_QP_I", -1);
	// amf_int64; default = 22; I-frame QP; range = 0-51
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_QP_P", -1);
	// amf_int64; default = 22; P-frame QP; range = 0-51
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_QP_B", -1);
	// amf_int64; default = 22; B-frame QP; range = 0-51

	// Other
	obs_data_set_default_bool(settings, "AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE", false);
	// bool; default = false; Filler Data Enable
	obs_data_set_default_int(settings, "AMF_VIDEO_ENCODER_GOP_SIZE", -1);
	// amf_int64; default = 60; GOP Size, in frames

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

	// Quality Preset & Usage
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_QUALITY_PRESET", AMF_TEXT_T("PRESET"),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_T("PRESET.SPEED"), AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
	obs_property_list_add_int(list, AMF_TEXT_T("PRESET.BALANCED"), AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED);
	obs_property_list_add_int(list, AMF_TEXT_T("PRESET.QUALITY"), AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY);

	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_USAGE_ENUM", AMF_TEXT_T("USAGE"),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_T("USAGE.TRANSCODING"), AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
	obs_property_list_add_int(list, AMF_TEXT_T("USAGE.ULTRALOWLATENCY"), AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
	obs_property_list_add_int(list, AMF_TEXT_T("USAGE.LOWLATENCY"), AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY);
	obs_property_list_add_int(list, AMF_TEXT_T("USAGE.WEBCAM"), AMF_VIDEO_ENCODER_USAGE_WEBCAM);

	// Profile & Level
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_PROFILE_ENUM", AMF_TEXT_T("PROFILE"),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_T("PROFILE.DEFAULT"), -1);
	for (unsigned int i = 0; i < AMF_Encoder::h264::PROFILES::PROFILE_COUNT_MAX; i++) {
		obs_property_list_add_int(list, obs_module_text(AMF_Encoder::h264::PROFILE_NAMES[i]), i);
	}

	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_PROFILE_LEVEL", AMF_TEXT_T("LEVEL"),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_T("LEVEL.DEFAULT"), -1);
	for (unsigned int i = 0; i < AMF_Encoder::h264::LEVELS::LEVEL_COUNT_MAX; i++) {
		obs_property_list_add_int(list, obs_module_text(AMF_Encoder::h264::LEVEL_NAMES[i]), i);
	}

	// Rate Control
	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD", AMF_TEXT_T("RATE_CONTROL"),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_T("RATE_CONTROL.DEFAULT"), -1);
	obs_property_list_add_int(list, AMF_TEXT_T("RATE_CONTROL.CONSTRAINEDQP"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTRAINED_QP);
	obs_property_list_add_int(list, AMF_TEXT_T("RATE_CONTROL.CBR"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR);
	obs_property_list_add_int(list, AMF_TEXT_T("RATE_CONTROL.PEAK_CONSTRAINED_VBR"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR);
	obs_property_list_add_int(list, AMF_TEXT_T("RATE_CONTROL.LATENCY_CONSTRAINED_VBR"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR);

	list = obs_properties_add_list(props, "AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME", AMF_TEXT_T("SKIP_FRAME"),
		OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(list, AMF_TEXT_T("SKIP_FRAME.DEFAULT"), -1);
	obs_property_list_add_int(list, AMF_TEXT_T("SKIP_FRAME.DISABLE"), 0);
	obs_property_list_add_int(list, AMF_TEXT_T("SKIP_FRAME.ENABLE"), 1);

	// CBR, VBR
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_TARGET_BITRATE", AMF_TEXT_T("BITRATE.TARGET"), -1, 65535, 1);
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_PEAK_BITRATE", AMF_TEXT_T("BITRATE.PEAK"), -1, 65535, 1);
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE", AMF_TEXT_T("VBV_BUFFER_SIZE"), -1, 65535, 1);

	// Constrained QP
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_MIN_QP", AMF_TEXT_T("QP.MIN"), -1, 51, 1);
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_MAX_QP", AMF_TEXT_T("QP.MAX"), -1, 51, 1);
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_QP_I", AMF_TEXT_T("QP.I"), -1, 51, 1);
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_QP_P", AMF_TEXT_T("QP.P"), -1, 51, 1);
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_QP_B", AMF_TEXT_T("QP.B"), -1, 51, 1);

	// Other
	obs_properties_add_bool(props, "AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE", AMF_TEXT_T("FILLER_DATA"));
	obs_properties_add_int_slider(props, "AMF_VIDEO_ENCODER_GOP_SIZE", AMF_TEXT_T("GOP_SIZE"), -1, 8192, 1);

	return props;
}

bool AMF_Encoder::h264::update(void *data, obs_data_t *settings) {
	return static_cast<AMF_Encoder::h264*>(data)->update(settings);
}

bool AMF_Encoder::h264::update(obs_data_t* settings) {
	AMF_LOG_INFO("h264::update");


	//int t_rateControlMethod = obs_data_get_int(settings, "AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD");
	//switch (t_rateControlMethod) {
	//	case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTRAINED_QP:
	//		break;
	//	case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR:
	//	case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR:

	//	case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR:
	//		break;
	//}

	//obs_properties_get(settings, )

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
			break;
		/*case amf::AMF_SURFACE_YV12:
			info->format = VIDEO_FORMAT_;
			break;*/
		case amf::AMF_SURFACE_BGRA:
			info->format = VIDEO_FORMAT_BGRA;
			break;
		/*case amf::AMF_SURFACE_ARGB:
			info->format = VIDEO_FORMAT_;
			return;*/
		case amf::AMF_SURFACE_RGBA:
			info->format = VIDEO_FORMAT_RGBA;
			break;
		case amf::AMF_SURFACE_GRAY8:
			info->format = VIDEO_FORMAT_Y800;
			break;
		case amf::AMF_SURFACE_YUV420P:
			info->format = VIDEO_FORMAT_I420;
			break;
		/*case amf::AMF_SURFACE_U8V8:
			info->format = VIDEO_FORMAT_Y800;
			break;*/
		case amf::AMF_SURFACE_YUY2:
			info->format = VIDEO_FORMAT_YUY2;
			break;
	}
	//info->range = VIDEO_RANGE_FULL;
	//info->colorspace = VIDEO_CS_709;
}

void AMF_STD_CALL AMF_Encoder::h264_SurfaceObserver::OnSurfaceDataRelease(amf::AMFSurface* pSurface) {
	//pSurface->
}
