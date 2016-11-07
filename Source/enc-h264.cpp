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

#include "api-d3d9.h"
#include "api-d3d11.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin;
using namespace Plugin::AMD;
using namespace Plugin::Interface;

enum Presets {
	None = -1,
	ResetToDefaults = 0,
	Recording,
	HighQuality,
	Indistinguishable,
	Lossless,
	Twitch,
	YouTube,
};
enum ViewMode {
	Basic,
	Advanced,
	Expert,
	Master
};

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

	//////////////////////////////////////////////////////////////////////////
	// Deprecated old Encoder
	static obs_encoder_info* encoder_info_simple = new obs_encoder_info();
	static const char* encoder_name_simple = "amd_amf_h264_simple";

	std::memset(encoder_info_simple, 0, sizeof(obs_encoder_info));

	// Initialize Structure
	encoder_info_simple->id = encoder_name_simple;
	encoder_info_simple->type = obs_encoder_type::OBS_ENCODER_VIDEO;
	encoder_info_simple->codec = encoder_codec;

	// Functions
	encoder_info_simple->get_name = &get_name_simple;
	encoder_info_simple->get_defaults = &get_defaults;
	encoder_info_simple->get_properties = &get_properties;
	encoder_info_simple->create = &create;
	encoder_info_simple->destroy = &destroy;
	encoder_info_simple->encode = &encode;
	encoder_info_simple->update = &update;
	encoder_info_simple->get_video_info = &get_video_info;
	encoder_info_simple->get_extra_data = &get_extra_data;

	obs_register_encoder(encoder_info_simple);
	//////////////////////////////////////////////////////////////////////////
}

const char* Plugin::Interface::H264Interface::get_name(void*) {
	static const char* name = "H264 Encoder (AMD Advanced Media Framework)";
	return name;
}

//////////////////////////////////////////////////////////////////////////
// Deprecated old Encoder
const char* Plugin::Interface::H264Interface::get_name_simple(void*) {
	static const char* name = "[DEPRECATED] H264 Encoder (AMD Advanced Media Framework)";
	return name;
}
//////////////////////////////////////////////////////////////////////////
void* Plugin::Interface::H264Interface::create(obs_data_t* settings, obs_encoder_t* encoder) {
	Plugin::Interface::H264Interface* enc = nullptr;
	try {
		AMF_LOG_INFO("Starting up...");
		enc = new Plugin::Interface::H264Interface(settings, encoder);
		return enc;
	} catch (std::exception e) {
		AMF_LOG_ERROR("Exception: %s", e.what());
		AMF_LOG_ERROR("Unable to create Encoder, see log for more information.");
		if (enc)
			delete enc;
		return NULL;
	} catch (...) {
		AMF_LOG_ERROR("Unhandled Exception during start up.");
		if (enc)
			delete enc;
		return NULL;
	}
}

#pragma warning( push )
#pragma warning( disable: 4702 )
void Plugin::Interface::H264Interface::destroy(void* data) {
	try {
		AMF_LOG_INFO("Shutting down...");
		Plugin::Interface::H264Interface* enc = static_cast<Plugin::Interface::H264Interface*>(data);
		delete enc;
	} catch (std::exception e) {
		AMF_LOG_ERROR("Exception: %s", e.what());
		AMF_LOG_ERROR("Unable to destroy Encoder, see log for more information.");
	} catch (...) {
		AMF_LOG_ERROR("Unhandled Exception during shut down.");
	}
	data = nullptr;
}
#pragma warning( pop )

bool Plugin::Interface::H264Interface::encode(void *data, struct encoder_frame *frame, struct encoder_packet *packet, bool *received_packet) {
	try {
		return static_cast<Plugin::Interface::H264Interface*>(data)->encode(frame, packet, received_packet);
	} catch (std::exception e) {
		AMF_LOG_ERROR("Exception: %s", e.what());
		AMF_LOG_ERROR("Unable to encode, see log for more information.");
		return false;
	} catch (...) {
		throw;
	}
}

void Plugin::Interface::H264Interface::get_defaults(obs_data_t *data) {
	#pragma region OBS - Enforce Streaming Service Restrictions
	// OBS - Enforce Streaming Service Restrictions
	obs_data_set_default_int(data, "bitrate", -1);
	obs_data_set_default_int(data, "keyint_sec", -1);
	obs_data_set_default_string(data, "rate_control", "");
	obs_data_set_default_string(data, "profile", "");
	obs_data_set_default_string(data, "preset", "");
	obs_data_set_int(data, "bitrate", -1);
	obs_data_set_int(data, "keyint_sec", -1);
	obs_data_set_string(data, "rate_control", "");
	obs_data_set_string(data, "profile", "");
	obs_data_set_string(data, "preset", "");
	#pragma endregion OBS - Enforce Streaming Service Restrictions

	obs_data_set_default_int(data, AMF_H264_PRESET, -1);

	// Static Properties
	obs_data_set_default_int(data, AMF_H264_USAGE, VCEUsage_Transcoding);
	obs_data_set_default_int(data, AMF_H264_PROFILE, VCEProfile_Main);
	obs_data_set_default_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_Automatic);
	obs_data_set_default_int(data, AMF_H264_MAXIMUMLTRFRAMES, 0);

	// Rate Control Properties
	obs_data_set_default_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantBitrate);
	obs_data_set_default_int(data, AMF_H264_BITRATE_TARGET, 3500);
	obs_data_set_default_int(data, AMF_H264_BITRATE_PEAK, 9000);
	obs_data_set_default_int(data, AMF_H264_QP_MINIMUM, 0);
	obs_data_set_default_int(data, AMF_H264_QP_MAXIMUM, 51);
	obs_data_set_default_int(data, AMF_H264_QP_IFRAME, 22);
	obs_data_set_default_int(data, AMF_H264_QP_PFRAME, 22);
	obs_data_set_default_int(data, AMF_H264_QP_BFRAME, 22);
	obs_data_set_default_int(data, AMF_H264_QP_BPICTURE_DELTA, 4);
	obs_data_set_default_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, 2);
	obs_data_set_default_int(data, AMF_H264_VBVBUFFER, 0);
	obs_data_set_default_int(data, AMF_H264_VBVBUFFER_SIZE, 3500);
	obs_data_set_default_double(data, AMF_H264_VBVBUFFER_STRICTNESS, 100);
	obs_data_set_default_double(data, AMF_H264_VBVBUFFER_FULLNESS, 100);
	obs_data_set_default_int(data, AMF_H264_MAXIMUMACCESSUNITSIZE, 0);
	obs_data_set_default_int(data, AMF_H264_FILLERDATA, 1);
	obs_data_set_default_int(data, AMF_H264_FRAMESKIPPING, 0);
	obs_data_set_default_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, 0);

	// Picture Control Properties
	obs_data_set_default_double(data, AMF_H264_KEYFRAME_INTERVAL, 2);
	obs_data_set_default_int(data, AMF_H264_IDR_PERIOD, 60);
	obs_data_set_default_int(data, AMF_H264_HEADER_INSERTION_SPACING, 0);
	obs_data_set_default_int(data, AMF_H264_BPICTURE_PATTERN, (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames ? VCEBPicturePattern_Three : VCEBPicturePattern_None));
	obs_data_set_default_int(data, AMF_H264_BPICTURE_REFERENCE, (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames ? 1 : 0));
	obs_data_set_default_int(data, AMF_H264_DEBLOCKINGFILTER, 1);
	obs_data_set_default_int(data, AMF_H264_SLICESPERFRAME, 0);
	obs_data_set_default_int(data, AMF_H264_INTRAREFRESHNUMMBSPERSLOT, 0);

	// Miscellaneous Control Properties
	obs_data_set_default_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Balanced);
	obs_data_set_default_int(data, AMF_H264_SCANTYPE, VCEScanType_Progressive);
	obs_data_set_default_int(data, AMF_H264_MOTIONESTIMATION, 3);
	obs_data_set_default_int(data, AMF_H264_CABAC, 0);

	// System Properties
	obs_data_set_default_int(data, AMF_H264_MEMORYTYPE, VCEMemoryType_Host);
	obs_data_set_default_int(data, AMF_H264_USE_OPENCL, 0);
	obs_data_set_default_int(data, AMF_H264_SURFACEFORMAT, -1);
	obs_data_set_default_int(data, AMF_H264_VIEW, ViewMode::Basic);
	obs_data_set_default_int(data, AMF_H264_UNLOCK_PROPERTIES, 0);
	obs_data_set_default_bool(data, AMF_H264_DEBUG, 0);
}

void fill_device_list(obs_property_t* p, obs_data_t* data) {
	std::vector<Plugin::API::Device> devices;

	switch ((VCEMemoryType)obs_data_get_int(data, AMF_H264_MEMORYTYPE)) {
		case VCEMemoryType_Auto:
			#ifdef _WIN32
			if (IsWindows8OrGreater()) {
				// DirectX 11
				devices = Plugin::API::Direct3D11::EnumerateDevices();
			} else if (IsWindowsXPOrGreater()) {
				// DirectX 9
				//devices = Plugin::API::Direct3D9::EnumerateDevices();
			} else
				#endif
			{
				// OpenGL
			}
			break;

			#ifdef _WIN32
		case VCEMemoryType_DirectX9:
			if (IsWindowsXPOrGreater()) // DirectX 9
				//devices = Plugin::API::Direct3D9::EnumerateDevices();
				break;
		case VCEMemoryType_DirectX11:
			if (IsWindows8OrGreater()) // DirectX 11
				devices = Plugin::API::Direct3D11::EnumerateDevices();
			break;
			#endif
		case VCEMemoryType_OpenGL:
			// OpenGL
			break;
		/*case VCEMemoryType_Host:
			break;*/
	}

	obs_property_list_clear(p);
	obs_property_list_add_string(p, TEXT_T(AMF_UTIL_DEFAULT), "");
	for (Plugin::API::Device device : devices) {
		obs_property_list_add_string(p, device.Name.c_str(), device.UniqueId.c_str());
	}
}

obs_properties_t* Plugin::Interface::H264Interface::get_properties(void*) {
	obs_properties* props = obs_properties_create();
	obs_property_t* p;

	#pragma region Preset
	p = obs_properties_add_list(props, AMF_H264_PRESET, TEXT_T(AMF_H264_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_modified_callback(p, preset_modified);
	obs_property_list_add_int(p, "", -1);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PRESET_RESETTODEFAULTS), Presets::ResetToDefaults);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PRESET_RECORDING), Presets::Recording);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PRESET_HIGHQUALITY), Presets::HighQuality);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PRESET_INDISTINGUISHABLE), Presets::Indistinguishable);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PRESET_LOSSLESS), Presets::Lossless);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PRESET_TWITCH), Presets::Twitch);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_PRESET_YOUTUBE), Presets::YouTube);
	#pragma endregion Preset

	#pragma region Static Properties
	#pragma region Usage
	p = obs_properties_add_list(props, AMF_H264_USAGE, TEXT_T(AMF_H264_USAGE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_USAGE_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_USAGE_TRANSCODING), VCEUsage_Transcoding);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_USAGE_ULTRALOWLATENCY), VCEUsage_UltraLowLatency);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_USAGE_LOWLATENCY), VCEUsage_LowLatency);
	//obs_property_list_add_int(list, TEXT_T(AMF_H264_USAGE_WEBCAM), VCEUsage_Webcam); // Requires SVC? SVC is not implemented by default.
	#pragma endregion Usage
	#pragma region Quality Preset
	p = obs_properties_add_list(props, AMF_H264_QUALITY_PRESET, TEXT_T(AMF_H264_QUALITY_PRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QUALITY_PRESET_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_QUALITY_PRESET_SPEED), VCEQualityPreset_Speed);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_QUALITY_PRESET_BALANCED), VCEQualityPreset_Balanced);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_QUALITY_PRESET_QUALITY), VCEQualityPreset_Quality);
	#pragma endregion Quality Preset
	#pragma region Profile
	p = obs_properties_add_list(props, AMF_H264_PROFILE, TEXT_T(AMF_H264_PROFILE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_PROFILE_DESCRIPTION));
	switch (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxProfile) {
		case 100:
			obs_property_list_add_int(p, "High", VCEProfile_High);
		case 77:
			obs_property_list_add_int(p, "Main", VCEProfile_Main);
		case 66:
			obs_property_list_add_int(p, "Baseline", VCEProfile_Baseline);
			break;
	}
	//obs_property_list_add_int(p, "Constrained Baseline", VCEProfile_ConstrainedBaseline);
	//obs_property_list_add_int(p, "Constrained High", VCEProfile_ConstrainedHigh);
	#pragma endregion Profile
	#pragma region Profile Level
	p = obs_properties_add_list(props, AMF_H264_PROFILELEVEL, TEXT_T(AMF_H264_PROFILELEVEL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_PROFILELEVEL_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_AUTOMATIC), VCEProfileLevel_Automatic);
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
	p = obs_properties_add_int_slider(props, AMF_H264_MAXIMUMLTRFRAMES, TEXT_T(AMF_H264_MAXIMUMLTRFRAMES), 0, 2, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_MAXIMUMLTRFRAMES_DESCRIPTION));
	obs_property_set_modified_callback(p, maximum_ltr_frames_modified);
	#pragma endregion Long Term Reference Frames
	#pragma endregion Static Properties

	#pragma region Rate Control Properties
	//p = obs_properties_add_bool(props, "rcp_delimiter", "------ Rate Control Properties ------");
	#pragma region Method
	p = obs_properties_add_list(props, AMF_H264_RATECONTROLMETHOD, TEXT_T(AMF_H264_RATECONTROLMETHOD), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_CQP), VCERateControlMethod_ConstantQP);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_CBR), VCERateControlMethod_ConstantBitrate);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_VBR), VCERateControlMethod_VariableBitrate_PeakConstrained);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_RATECONTROLMETHOD_VBR_LAT), VCERateControlMethod_VariableBitrate_LatencyConstrained);
	obs_property_set_modified_callback(p, rate_control_method_modified);
	#pragma endregion Method
	#pragma region Method Parameters
	/// Bitrate Constraints
	p = obs_properties_add_int(props, AMF_H264_BITRATE_TARGET, TEXT_T(AMF_H264_BITRATE_TARGET), 0, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BITRATE_TARGET_DESCRIPTION));
	p = obs_properties_add_int(props, AMF_H264_BITRATE_PEAK, TEXT_T(AMF_H264_BITRATE_PEAK), 0, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BITRATE_PEAK_DESCRIPTION));
	/// Minimum QP, Maximum QP
	p = obs_properties_add_int_slider(props, AMF_H264_QP_MINIMUM, TEXT_T(AMF_H264_QP_MINIMUM), 0, 51, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QP_MINIMUM_DESCRIPTION));
	p = obs_properties_add_int_slider(props, AMF_H264_QP_MAXIMUM, TEXT_T(AMF_H264_QP_MAXIMUM), 0, 51, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QP_MAXIMUM_DESCRIPTION));
	/// Method: Constant QP
	p = obs_properties_add_int_slider(props, AMF_H264_QP_IFRAME, TEXT_T(AMF_H264_QP_IFRAME), 0, 51, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QP_IFRAME_DESCRIPTION));
	p = obs_properties_add_int_slider(props, AMF_H264_QP_PFRAME, TEXT_T(AMF_H264_QP_PFRAME), 0, 51, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QP_PFRAME_DESCRIPTION));
	p = obs_properties_add_int_slider(props, AMF_H264_QP_BFRAME, TEXT_T(AMF_H264_QP_BFRAME), 0, 51, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QP_BFRAME_DESCRIPTION));
	/// B-Picture Related
	p = obs_properties_add_int_slider(props, AMF_H264_QP_BPICTURE_DELTA, TEXT_T(AMF_H264_QP_BPICTURE_DELTA), -10, 10, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QP_BPICTURE_DELTA_DESCRIPTION));
	p = obs_properties_add_int_slider(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, TEXT_T(AMF_H264_QP_REFERENCE_BPICTURE_DELTA), -10, 10, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_QP_REFERENCE_BPICTURE_DELTA_DESCRIPTION));
	#pragma endregion Method Parameters
	#pragma region VBV Buffer
	p = obs_properties_add_list(props, AMF_H264_VBVBUFFER, TEXT_T(AMF_H264_VBVBUFFER), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VBVBUFFER_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_AUTOMATIC), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_MANUAL), 1);
	obs_property_set_modified_callback(p, view_modified);
	p = obs_properties_add_float_slider(props, AMF_H264_VBVBUFFER_STRICTNESS, TEXT_T(AMF_H264_VBVBUFFER_STRICTNESS), 0.0, 100.0, 0.1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VBVBUFFER_STRICTNESS_DESCRIPTION));
	p = obs_properties_add_int_slider(props, AMF_H264_VBVBUFFER_SIZE, TEXT_T(AMF_H264_VBVBUFFER_SIZE), 1, 1000000, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VBVBUFFER_SIZE_DESCRIPTION));
	p = obs_properties_add_float_slider(props, AMF_H264_VBVBUFFER_FULLNESS, TEXT_T(AMF_H264_VBVBUFFER_FULLNESS), 0.0, 100.0, 100.0 / 64.0);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VBVBUFFER_FULLNESS_DESCRIPTION));
	#pragma endregion VBV Buffer
	/// Max Access Unit Size
	p = obs_properties_add_int_slider(props, AMF_H264_MAXIMUMACCESSUNITSIZE, TEXT_T(AMF_H264_MAXIMUMACCESSUNITSIZE), 0, 100000000, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_MAXIMUMACCESSUNITSIZE_DESCRIPTION));
	#pragma region Flags
	/// Filler Data (Only supported by CBR so far)
	p = obs_properties_add_list(props, AMF_H264_FILLERDATA, TEXT_T(AMF_H264_FILLERDATA), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_FILLERDATA_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	/// Frame Skipping
	p = obs_properties_add_list(props, AMF_H264_FRAMESKIPPING, TEXT_T(AMF_H264_FRAMESKIPPING), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_FRAMESKIPPING_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	/// Enforce Hypothetical Reference Decoder Compatibility
	p = obs_properties_add_list(props, AMF_H264_ENFORCEHRDCOMPATIBILITY, TEXT_T(AMF_H264_ENFORCEHRDCOMPATIBILITY), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_ENFORCEHRDCOMPATIBILITY_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	#pragma endregion Flags
	#pragma endregion Rate Control Properties

	#pragma region Picture Control Properties
	//p = obs_properties_add_bool(props, "pcp_delimiter", "------ Picture Control Properties ------");
	#pragma region IDR Period / Keyframe Interval / Header Insertion Spacing
	p = obs_properties_add_float(props, AMF_H264_KEYFRAME_INTERVAL, TEXT_T(AMF_H264_KEYFRAME_INTERVAL), 0, 100, 0.001);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_KEYFRAME_INTERVAL_DESCRIPTION));
	p = obs_properties_add_int(props, AMF_H264_IDR_PERIOD, TEXT_T(AMF_H264_IDR_PERIOD), 1, 1000, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_IDR_PERIOD_DESCRIPTION));
	p = obs_properties_add_int(props, AMF_H264_HEADER_INSERTION_SPACING, TEXT_T(AMF_H264_HEADER_INSERTION_SPACING), 0, 1000, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_HEADER_INSERTION_SPACING_DESCRIPTION));
	#pragma endregion IDR Period / Keyframe Interval / Header Insertion Spacing
	#pragma region B-Pictures
	/// B-Pictures Pattern
	p = obs_properties_add_int_slider(props, AMF_H264_BPICTURE_PATTERN, TEXT_T(AMF_H264_BPICTURE_PATTERN), 0, (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames ? 3 : 0), 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BPICTURE_PATTERN_DESCRIPTION));
	obs_property_set_modified_callback(p, bpictures_modified);
	/// Enable Reference to B-Frames (2nd Generation GCN and newer)
	p = obs_properties_add_list(props, AMF_H264_BPICTURE_REFERENCE, TEXT_T(AMF_H264_BPICTURE_REFERENCE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_BPICTURE_REFERENCE_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	#pragma endregion B-Pictures
	/// De-Blocking Filter
	p = obs_properties_add_list(props, AMF_H264_DEBLOCKINGFILTER, TEXT_T(AMF_H264_DEBLOCKINGFILTER), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_DEBLOCKINGFILTER_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	/// Number of Slices Per Frame 
	p = obs_properties_add_int_slider(props, AMF_H264_SLICESPERFRAME, TEXT_T(AMF_H264_SLICESPERFRAME), 0, 65535, 1);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_SLICESPERFRAME_DESCRIPTION));
	/// Intra Refresh Number of Macro Blocks per Slot
	obs_properties_add_int_slider(props, AMF_H264_INTRAREFRESHNUMMBSPERSLOT, TEXT_T(AMF_H264_INTRAREFRESHNUMMBSPERSLOT), 0, 65535, 1);
	#pragma endregion Picture Control Properties

	#pragma region Miscellaneous Control Properties
	//p = obs_properties_add_bool(props, "msc_delimiter", "------ Miscellaneous Properties ------");
	/// Scan Type
	p = obs_properties_add_list(props, AMF_H264_SCANTYPE, TEXT_T(AMF_H264_SCANTYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_SCANTYPE_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_SCANTYPE_PROGRESSIVE), VCEScanType_Progressive);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_SCANTYPE_INTERLACED), VCEScanType_Interlaced);
	/// Motion Estimation
	p = obs_properties_add_list(props, AMF_H264_MOTIONESTIMATION, TEXT_T(AMF_H264_MOTIONESTIMATION), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_MOTIONESTIMATION_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_MOTIONESTIMATION_NONE), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_MOTIONESTIMATION_HALF), 1);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_MOTIONESTIMATION_QUARTER), 2);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_MOTIONESTIMATION_BOTH), 3);
	/// CABAC
	p = obs_properties_add_list(props, AMF_H264_CABAC, TEXT_T(AMF_H264_CABAC), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_CABAC_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	#pragma endregion Miscellaneous Control Properties

	#pragma region System Properties
	//p = obs_properties_add_bool(props, "sys_delimiter", "------ System Properties ------");
	/// Memory Type
	p = obs_properties_add_list(props, AMF_H264_MEMORYTYPE, TEXT_T(AMF_H264_MEMORYTYPE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_MEMORYTYPE_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_AUTOMATIC), VCEMemoryType_Auto);
	obs_property_list_add_int(p, "Host", VCEMemoryType_Host);
	#if defined(_WIN32) || defined(_WIN64)
	if (IsWindowsXPOrGreater()) {
		obs_property_list_add_int(p, "DirectX 9", VCEMemoryType_DirectX9);
	}
	if (IsWindows8OrGreater()) {
		obs_property_list_add_int(p, "DirectX 11", VCEMemoryType_DirectX11);
	}
	#endif
	obs_property_list_add_int(p, "OpenGL", VCEMemoryType_OpenGL);
	obs_property_set_modified_callback(p, view_modified);
	/// Device
	p = obs_properties_add_list(props, AMF_H264_DEVICE, TEXT_T(AMF_H264_DEVICE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_DEVICE_DESCRIPTION));
	/// Compute Type
	p = obs_properties_add_list(props, AMF_H264_USE_OPENCL, TEXT_T(AMF_H264_USE_OPENCL), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_USE_OPENCL_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_DISABLED), 0);
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_TOGGLE_ENABLED), 1);
	/// Surface Format
	p = obs_properties_add_list(props, AMF_H264_SURFACEFORMAT, TEXT_T(AMF_H264_SURFACEFORMAT), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_SURFACEFORMAT_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_UTIL_AUTOMATIC), -1);
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
	p = obs_properties_add_list(props, AMF_H264_VIEW, TEXT_T(AMF_H264_VIEW), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, TEXT_T(AMF_H264_VIEW_DESCRIPTION));
	obs_property_list_add_int(p, TEXT_T(AMF_H264_VIEW_BASIC), ViewMode::Basic);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_VIEW_ADVANCED), ViewMode::Advanced);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_VIEW_EXPERT), ViewMode::Expert);
	obs_property_list_add_int(p, TEXT_T(AMF_H264_VIEW_MASTER), ViewMode::Master);
	obs_property_set_modified_callback(p, view_modified);
	/// Unlock Properties to full range.
	p = obs_properties_add_bool(props, AMF_H264_UNLOCK_PROPERTIES, TEXT_T(AMF_H264_UNLOCK_PROPERTIES));
	obs_property_set_long_description(p, TEXT_T(AMF_H264_UNLOCK_PROPERTIES_DESCRIPTION));
	obs_property_set_modified_callback(p, unlock_properties_modified);

	/// Debug
	p = obs_properties_add_bool(props, AMF_H264_DEBUG, TEXT_T(AMF_H264_DEBUG));
	obs_property_set_long_description(p, TEXT_T(AMF_H264_DEBUG_DESCRIPTION));
	#pragma endregion System Properties

	return props;
}

bool Plugin::Interface::H264Interface::preset_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	auto caps = VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC);
	Presets preset = (Presets)obs_data_get_int(data, AMF_H264_PRESET);

	// Reset State
	{
		obs_property_t* pn = obs_properties_first(props);
		do {
			obs_property_set_enabled(pn, true);
		} while (obs_property_next(&pn));

		if (preset != Presets::None) {
			// System Properties
			obs_data_set_int(data, AMF_H264_UNLOCK_PROPERTIES, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_UNLOCK_PROPERTIES), false);
			unlock_properties_modified(props, nullptr, data);
		}

		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_TARGET), 1000, caps->maxBitrate / 1000, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_PEAK), 1000, caps->maxBitrate / 1000, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_VBVBUFFER_SIZE), 1000, 100000, 1);
	}

	switch (preset) {
		case ResetToDefaults:
			#pragma region Default
		{
			obs_property_t* pn = obs_properties_first(props);
			while (pn) {
				const char* name = obs_property_name(pn);
				switch (obs_property_get_type(pn)) {
					case obs_property_type::OBS_PROPERTY_BOOL:
						obs_data_set_bool(data, name, obs_data_get_default_bool(data, name));
						break;
					case obs_property_type::OBS_PROPERTY_FLOAT:
						obs_data_set_double(data, name, obs_data_get_default_double(data, name));
						break;
					case obs_property_type::OBS_PROPERTY_INT:
						obs_data_set_int(data, name, obs_data_get_default_int(data, name));
						break;
					case obs_property_type::OBS_PROPERTY_TEXT:
						obs_data_set_string(data, name, obs_data_get_default_string(data, name));
						break;
					case obs_property_type::OBS_PROPERTY_LIST:
						switch (obs_property_list_format(pn)) {
							case obs_combo_format::OBS_COMBO_FORMAT_INT:
								obs_data_set_int(data, name, obs_data_get_default_int(data, name));
								break;
							case obs_combo_format::OBS_COMBO_FORMAT_FLOAT:
								obs_data_set_double(data, name, obs_data_get_default_double(data, name));
								break;
							case obs_combo_format::OBS_COMBO_FORMAT_STRING:
								obs_data_set_string(data, name, obs_data_get_default_string(data, name));
								break;
						}
						break;
				}
				obs_property_next(&pn);
			}
		}
		break;
		#pragma endregion Default
		case Recording:
			#pragma region Recording
			// Static Properties
			//obs_data_set_int(data, AMF_H264_USAGE, VCEUsage_Transcoding);
			obs_data_set_int(data, AMF_H264_PROFILE, VCEProfile_High);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			switch (caps->maxProfileLevel) {
				case 52:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_52);
					break;
				case 51:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_51);
					break;
				case 50:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_50);
					break;
				default:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_Automatic);
					break;
			}
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);
			//obs_data_set_int(data, AMF_H264_MAXIMUMLTRFRAMES, obs_data_get_default_int(data, AMF_H264_MAXIMUMLTRFRAMES));

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_VariableBitrate_LatencyConstrained);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			if (obs_data_get_int(data, AMF_H264_BITRATE_TARGET) < 10000 * (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1000 : 1))
				obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 10000 * (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1000 : 1));
			obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_TARGET), 10000, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / 1000, 1);
			//obs_data_set_int(data, AMF_H264_BITRATE_PEAK, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1 : 1000));
			obs_data_set_int(data, AMF_H264_QP_MINIMUM, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_MINIMUM), false);
			obs_data_set_int(data, AMF_H264_QP_MAXIMUM, 51);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_MAXIMUM), false);
			/*obs_data_set_int(data, AMF_H264_QP_IFRAME, 0);
			obs_data_set_int(data, AMF_H264_QP_PFRAME, 0);
			obs_data_set_int(data, AMF_H264_QP_BFRAME, 0);*/
			obs_data_set_int(data, AMF_H264_QP_BPICTURE_DELTA, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_BPICTURE_DELTA), false);
			obs_data_set_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA), false);
			obs_data_set_int(data, AMF_H264_VBVBUFFER, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER), false);
			obs_data_set_double(data, AMF_H264_VBVBUFFER_STRICTNESS, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER_STRICTNESS), false);
			obs_data_set_double(data, AMF_H264_VBVBUFFER_FULLNESS, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER_FULLNESS), false);
			//obs_data_set_int(data, AMF_H264_MAXIMUMACCESSUNITSIZE, 0);
			obs_data_set_int(data, AMF_H264_FILLERDATA, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_FILLERDATA), false);
			//obs_data_set_int(data, AMF_H264_FRAMESKIPPING, 0);
			//obs_data_set_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, 0);

			// Picture Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 1);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);
			//obs_data_set_int(data, AMF_H264_IDR_PERIOD, 60);
			//obs_data_set_int(data, AMF_H264_HEADER_INSERTION_SPACING, 0);
			//obs_data_set_int(data, AMF_H264_BPICTURE_PATTERN, obs_data_get_default_int(data, AMF_H264_BPICTURE_PATTERN));
			//obs_data_set_int(data, AMF_H264_BPICTURE_REFERENCE, obs_data_get_default_int(data, AMF_H264_BPICTURE_REFERENCE));
			//obs_data_set_int(data, AMF_H264_SLICESPERFRAME, 0);
			//obs_data_set_int(data, AMF_H264_INTRAREFRESHNUMMBSPERSLOT, 0);

			// Miscellaneous Properties
			//obs_data_set_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Quality);
			obs_data_set_int(data, AMF_H264_SCANTYPE, VCEScanType_Progressive);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_SCANTYPE), false);
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			//obs_data_set_int(data, AMF_H264_CABAC, 0);
			break;
			#pragma endregion Recording
		case HighQuality:
			#pragma region High Quality
			// Static Properties
			//obs_data_set_int(data, AMF_H264_USAGE, VCEUsage_Transcoding);
			obs_data_set_int(data, AMF_H264_PROFILE, VCEProfile_High);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			switch (caps->maxProfileLevel) {
				case 52:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_52);
					break;
				case 51:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_51);
					break;
				case 50:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_50);
					break;
				default:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_Automatic);
					break;
			}
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);
			//obs_data_set_int(data, AMF_H264_MAXIMUMLTRFRAMES, obs_data_get_default_int(data, AMF_H264_MAXIMUMLTRFRAMES));

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantQP);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			//obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 35000 * (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1000 : 1));
			//obs_data_set_int(data, AMF_H264_BITRATE_PEAK, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1 : 1000));
			/*obs_data_set_int(data, AMF_H264_QP_MINIMUM, 0);
			obs_data_set_int(data, AMF_H264_QP_MAXIMUM, 51);*/
			obs_data_set_int(data, AMF_H264_QP_IFRAME, 26);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_IFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_PFRAME, 24);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_PFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_BFRAME, 22);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_BFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_BPICTURE_DELTA, -2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_BPICTURE_DELTA), false);
			obs_data_set_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, -2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA), false);
			obs_data_set_int(data, AMF_H264_VBVBUFFER, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER), false);
			obs_data_set_double(data, AMF_H264_VBVBUFFER_STRICTNESS, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER_STRICTNESS), false);
			obs_data_set_double(data, AMF_H264_VBVBUFFER_FULLNESS, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER_FULLNESS), false);
			//obs_data_set_int(data, AMF_H264_MAXIMUMACCESSUNITSIZE, 0);
			//obs_data_set_int(data, AMF_H264_FILLERDATA, 0);
			//obs_data_set_int(data, AMF_H264_FRAMESKIPPING, 0);
			//obs_data_set_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, 0);

			// Picture Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 1);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);
			//obs_data_set_int(data, AMF_H264_IDR_PERIOD, 60);
			//obs_data_set_int(data, AMF_H264_HEADER_INSERTION_SPACING, 0);
			//obs_data_set_int(data, AMF_H264_BPICTURE_PATTERN, obs_data_get_default_int(data, AMF_H264_BPICTURE_PATTERN));
			//obs_data_set_int(data, AMF_H264_BPICTURE_REFERENCE, obs_data_get_default_int(data, AMF_H264_BPICTURE_REFERENCE));
			//obs_data_set_int(data, AMF_H264_SLICESPERFRAME, 0);
			//obs_data_set_int(data, AMF_H264_INTRAREFRESHNUMMBSPERSLOT, 0);

			// Miscellaneous Properties
			//obs_data_set_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Quality);
			obs_data_set_int(data, AMF_H264_SCANTYPE, VCEScanType_Progressive);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_SCANTYPE), false);
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			//obs_data_set_int(data, AMF_H264_CABAC, 0);
			break;
			#pragma endregion High Quality
		case Indistinguishable:
			#pragma region Indistinguishable
			// Static Properties
			//obs_data_set_int(data, AMF_H264_USAGE, VCEUsage_Transcoding);
			obs_data_set_int(data, AMF_H264_PROFILE, VCEProfile_High);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			switch (caps->maxProfileLevel) {
				case 52:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_52);
					break;
				case 51:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_51);
					break;
				case 50:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_50);
					break;
				default:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_Automatic);
					break;
			}
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);
			//obs_data_set_int(data, AMF_H264_MAXIMUMLTRFRAMES, obs_data_get_default_int(data, AMF_H264_MAXIMUMLTRFRAMES));

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantQP);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			//obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 35000 * (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1000 : 1));
			//obs_data_set_int(data, AMF_H264_BITRATE_PEAK, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1 : 1000));
			/*obs_data_set_int(data, AMF_H264_QP_MINIMUM, 0);
			obs_data_set_int(data, AMF_H264_QP_MAXIMUM, 51);*/
			obs_data_set_int(data, AMF_H264_QP_IFRAME, 21);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_IFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_PFRAME, 19);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_PFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_BFRAME, 17);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_BFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_BPICTURE_DELTA, -2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_BPICTURE_DELTA), false);
			obs_data_set_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, -2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA), false);
			obs_data_set_int(data, AMF_H264_VBVBUFFER, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER), false);
			obs_data_set_double(data, AMF_H264_VBVBUFFER_STRICTNESS, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER_STRICTNESS), false);
			obs_data_set_double(data, AMF_H264_VBVBUFFER_FULLNESS, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER_FULLNESS), false);
			//obs_data_set_int(data, AMF_H264_MAXIMUMACCESSUNITSIZE, 0);
			//obs_data_set_int(data, AMF_H264_FILLERDATA, 0);
			//obs_data_set_int(data, AMF_H264_FRAMESKIPPING, 0);
			//obs_data_set_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, 0);

			// Picture Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 1);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);
			//obs_data_set_int(data, AMF_H264_IDR_PERIOD, 60);
			//obs_data_set_int(data, AMF_H264_HEADER_INSERTION_SPACING, 0);
			//obs_data_set_int(data, AMF_H264_BPICTURE_PATTERN, obs_data_get_default_int(data, AMF_H264_BPICTURE_PATTERN));
			//obs_data_set_int(data, AMF_H264_BPICTURE_REFERENCE, obs_data_get_default_int(data, AMF_H264_BPICTURE_REFERENCE));
			//obs_data_set_int(data, AMF_H264_SLICESPERFRAME, 0);
			//obs_data_set_int(data, AMF_H264_INTRAREFRESHNUMMBSPERSLOT, 0);

			// Miscellaneous Properties
			//obs_data_set_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Quality);
			obs_data_set_int(data, AMF_H264_SCANTYPE, VCEScanType_Progressive);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_SCANTYPE), false);
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			//obs_data_set_int(data, AMF_H264_CABAC, 0);
			break;
			#pragma endregion Indistinguishable
		case Lossless:
			#pragma region Lossless
			// Static Properties
			//obs_data_set_int(data, AMF_H264_USAGE, VCEUsage_Transcoding);
			obs_data_set_int(data, AMF_H264_PROFILE, VCEProfile_High);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			switch (caps->maxProfileLevel) {
				case 52:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_52);
					break;
				case 51:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_51);
					break;
				case 50:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_50);
					break;
				default:
					obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_Automatic);
					break;
			}
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);
			//obs_data_set_int(data, AMF_H264_MAXIMUMLTRFRAMES, 0);

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantQP);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			//obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 35000 * (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1000 : 1));
			//obs_data_set_int(data, AMF_H264_BITRATE_PEAK, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1 : 1000));
			/*obs_data_set_int(data, AMF_H264_QP_MINIMUM, 0);
			obs_data_set_int(data, AMF_H264_QP_MAXIMUM, 51);*/
			obs_data_set_int(data, AMF_H264_QP_IFRAME, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_IFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_PFRAME, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_PFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_BFRAME, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_BFRAME), false);
			obs_data_set_int(data, AMF_H264_QP_BPICTURE_DELTA, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_BPICTURE_DELTA), false);
			obs_data_set_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA), false);
			obs_data_set_int(data, AMF_H264_VBVBUFFER, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER), false);
			obs_data_set_double(data, AMF_H264_VBVBUFFER_STRICTNESS, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER_STRICTNESS), false);
			obs_data_set_double(data, AMF_H264_VBVBUFFER_FULLNESS, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER_FULLNESS), false);
			//obs_data_set_int(data, AMF_H264_MAXIMUMACCESSUNITSIZE, 0);
			//obs_data_set_int(data, AMF_H264_FILLERDATA, 0);
			//obs_data_set_int(data, AMF_H264_FRAMESKIPPING, 0);
			//obs_data_set_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, 0);

			// Picture Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 1);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);
			obs_data_set_int(data, AMF_H264_IDR_PERIOD, 30);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_IDR_PERIOD), false);
			//obs_data_set_int(data, AMF_H264_HEADER_INSERTION_SPACING, 0);
			obs_data_set_int(data, AMF_H264_BPICTURE_PATTERN, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_BPICTURE_PATTERN), false);
			obs_data_set_int(data, AMF_H264_BPICTURE_REFERENCE, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_BPICTURE_REFERENCE), false);
			//obs_data_set_int(data, AMF_H264_SLICESPERFRAME, 0);
			//obs_data_set_int(data, AMF_H264_INTRAREFRESHNUMMBSPERSLOT, 0);

			// Miscellaneous Properties
			//obs_data_set_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Quality);
			obs_data_set_int(data, AMF_H264_SCANTYPE, VCEScanType_Progressive);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_SCANTYPE), false);
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			//obs_data_set_int(data, AMF_H264_CABAC, 0);
			break;
			#pragma endregion Lossless
		case Twitch:
			#pragma region Twitch
			// Static Properties
			obs_data_set_int(data, AMF_H264_USAGE, VCEUsage_Transcoding);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_USAGE), false);
			obs_data_set_int(data, AMF_H264_PROFILE, VCEProfile_Main);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_Automatic);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);
			//obs_data_set_int(data, AMF_H264_MAXIMUMLTRFRAMES, obs_data_get);

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantBitrate);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			if (obs_data_get_int(data, AMF_H264_BITRATE_TARGET) < 1000 * (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1000 : 1))
				obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 1000 * (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1000 : 1));
			if (obs_data_get_int(data, AMF_H264_BITRATE_TARGET) > 4000 * (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1000 : 1))
				obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 4000 * (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1000 : 1));
			obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_TARGET), 1000, 4000, 1);
			//obs_data_set_int(data, AMF_H264_BITRATE_PEAK, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1 : 1000));
			obs_data_set_int(data, AMF_H264_QP_MINIMUM, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_MINIMUM), false);
			obs_data_set_int(data, AMF_H264_QP_MAXIMUM, 51);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_MAXIMUM), false);
			/*obs_data_set_int(data, AMF_H264_QP_IFRAME, 0);
			obs_data_set_int(data, AMF_H264_QP_PFRAME, 0);
			obs_data_set_int(data, AMF_H264_QP_BFRAME, 0);*/
			obs_data_set_int(data, AMF_H264_QP_BPICTURE_DELTA, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_BPICTURE_DELTA), false);
			obs_data_set_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA), false);
			obs_data_set_int(data, AMF_H264_VBVBUFFER, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER), false);
			obs_data_set_double(data, AMF_H264_VBVBUFFER_STRICTNESS, 90);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER_STRICTNESS), false);
			obs_data_set_double(data, AMF_H264_VBVBUFFER_FULLNESS, 100);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER_FULLNESS), false);
			//obs_data_set_int(data, AMF_H264_MAXIMUMACCESSUNITSIZE, 0);
			obs_data_set_int(data, AMF_H264_FILLERDATA, 1);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_FILLERDATA), false);
			//obs_data_set_int(data, AMF_H264_FRAMESKIPPING, 0);
			//obs_data_set_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, 0);

			// Picture Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);
			//obs_data_set_int(data, AMF_H264_IDR_PERIOD, 120);
			//obs_data_set_int(data, AMF_H264_HEADER_INSERTION_SPACING, 0);
			//obs_data_set_int(data, AMF_H264_BPICTURE_PATTERN, 0);
			//obs_data_set_int(data, AMF_H264_BPICTURE_REFERENCE, 0);
			//obs_data_set_int(data, AMF_H264_SLICESPERFRAME, 0);
			//obs_data_set_int(data, AMF_H264_INTRAREFRESHNUMMBSPERSLOT, 0);

			// Miscellaneous Properties
			//obs_data_set_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Quality);
			obs_data_set_int(data, AMF_H264_SCANTYPE, VCEScanType_Progressive);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_SCANTYPE), false);
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			//obs_data_set_int(data, AMF_H264_CABAC, 0);
			break;
			#pragma endregion Twitch
		case YouTube:
			#pragma region YouTube
			// Static Properties
			obs_data_set_int(data, AMF_H264_USAGE, VCEUsage_Transcoding);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_USAGE), false);
			obs_data_set_int(data, AMF_H264_PROFILE, VCEProfile_Main);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILE), false);
			obs_data_set_int(data, AMF_H264_PROFILELEVEL, VCEProfileLevel_Automatic);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_PROFILELEVEL), false);
			//obs_data_set_int(data, AMF_H264_MAXIMUMLTRFRAMES, obs_data_get);

			// Rate Control Properties
			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, VCERateControlMethod_ConstantBitrate);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), false);
			if (obs_data_get_int(data, AMF_H264_BITRATE_TARGET) < 1000 * (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1000 : 1))
				obs_data_set_int(data, AMF_H264_BITRATE_TARGET, 1000 * (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1000 : 1));
			if (obs_data_get_int(data, AMF_H264_BITRATE_PEAK) > 25000 * (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1000 : 1))
				obs_data_set_int(data, AMF_H264_BITRATE_PEAK, 25000 * (obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1000 : 1));
			obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_TARGET), 1000, 25000, 1);
			obs_data_set_int(data, AMF_H264_QP_MINIMUM, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_MINIMUM), false);
			obs_data_set_int(data, AMF_H264_QP_MAXIMUM, 51);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_MAXIMUM), false);
			/*obs_data_set_int(data, AMF_H264_QP_IFRAME, 0);
			obs_data_set_int(data, AMF_H264_QP_PFRAME, 0);
			obs_data_set_int(data, AMF_H264_QP_BFRAME, 0);*/
			obs_data_set_int(data, AMF_H264_QP_BPICTURE_DELTA, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_BPICTURE_DELTA), false);
			obs_data_set_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA), false);
			obs_data_set_int(data, AMF_H264_VBVBUFFER, 0);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER), false);
			obs_data_set_double(data, AMF_H264_VBVBUFFER_STRICTNESS, 75);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER_STRICTNESS), false);
			obs_data_set_double(data, AMF_H264_VBVBUFFER_FULLNESS, 100);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_VBVBUFFER_FULLNESS), false);
			//obs_data_set_int(data, AMF_H264_MAXIMUMACCESSUNITSIZE, 0);
			obs_data_set_int(data, AMF_H264_FILLERDATA, 1);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_FILLERDATA), false);
			//obs_data_set_int(data, AMF_H264_FRAMESKIPPING, 0);
			//obs_data_set_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, 0);

			// Picture Control Properties
			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, 2);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), false);
			//obs_data_set_int(data, AMF_H264_IDR_PERIOD, 120);
			//obs_data_set_int(data, AMF_H264_HEADER_INSERTION_SPACING, 0);
			//obs_data_set_int(data, AMF_H264_BPICTURE_PATTERN, 0);
			//obs_data_set_int(data, AMF_H264_BPICTURE_REFERENCE, 0);
			//obs_data_set_int(data, AMF_H264_SLICESPERFRAME, 0);
			//obs_data_set_int(data, AMF_H264_INTRAREFRESHNUMMBSPERSLOT, 0);

			// Miscellaneous Properties
			//obs_data_set_int(data, AMF_H264_QUALITY_PRESET, VCEQualityPreset_Quality);
			obs_data_set_int(data, AMF_H264_SCANTYPE, VCEScanType_Progressive);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_SCANTYPE), false);
			obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, 3);
			obs_property_set_enabled(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), false);
			//obs_data_set_int(data, AMF_H264_CABAC, 0);
			break;
			#pragma endregion YouTube
	}
	unlock_properties_modified(props, nullptr, data);
	return view_modified(props, nullptr, data);
}

bool Plugin::Interface::H264Interface::maximum_ltr_frames_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	return view_modified(props, nullptr, data);
}

bool Plugin::Interface::H264Interface::rate_control_method_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	return view_modified(props, nullptr, data);
}

bool Plugin::Interface::H264Interface::bpictures_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	view_modified(props, nullptr, data);
	return false;
}

bool Plugin::Interface::H264Interface::view_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	uint32_t view = (uint32_t)obs_data_get_int(data, AMF_H264_VIEW);
	bool vis_basic = view >= ViewMode::Basic,
		vis_advanced = view >= ViewMode::Advanced,
		vis_expert = view >= ViewMode::Expert,
		vis_master = view >= ViewMode::Master,
		vis_rcm_bitrate_target = false,
		vis_rcm_bitrate_peak = false,
		vis_rcm_qp = false,
		vis_rcm_qp_b = false,
		vis_rcm_fillerdata = false;

	// Static Properties
	obs_property_set_visible(obs_properties_get(props, AMF_H264_USAGE), vis_master);
	if (!vis_master)
		obs_data_set_int(data, AMF_H264_USAGE, obs_data_get_default_int(data, AMF_H264_USAGE));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_PROFILE), vis_advanced);
	if (!vis_basic)
		obs_data_set_int(data, AMF_H264_PROFILE, obs_data_get_default_int(data, AMF_H264_PROFILE));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_PROFILELEVEL), vis_expert);
	if (!vis_expert)
		obs_data_set_int(data, AMF_H264_PROFILELEVEL, obs_data_get_default_int(data, AMF_H264_PROFILELEVEL));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_MAXIMUMLTRFRAMES), vis_master);
	if (!vis_master)
		obs_data_set_int(data, AMF_H264_MAXIMUMLTRFRAMES, obs_data_get_default_int(data, AMF_H264_MAXIMUMLTRFRAMES));
	bool using_ltr_frames = obs_data_get_int(data, AMF_H264_MAXIMUMLTRFRAMES) > 0;

	// Rate Control Properties
	obs_property_set_visible(obs_properties_get(props, AMF_H264_RATECONTROLMETHOD), vis_basic);
	if (!vis_basic)
		obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, obs_data_get_default_int(data, AMF_H264_RATECONTROLMETHOD));
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
			break;
		case VCERateControlMethod_ConstantQP:
			vis_rcm_qp = true;
			vis_rcm_qp_b = VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames
				&& (using_ltr_frames == false);
			break;
	}

	obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_TARGET), (vis_basic && vis_rcm_bitrate_target));
	if (!((vis_basic && vis_rcm_bitrate_target)))
		obs_data_set_int(data, AMF_H264_BITRATE_TARGET, obs_data_get_default_int(data, AMF_H264_BITRATE_TARGET));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_BITRATE_PEAK), (vis_basic && vis_rcm_bitrate_peak));
	if (!((vis_basic && vis_rcm_bitrate_peak)))
		obs_data_set_int(data, AMF_H264_BITRATE_PEAK, obs_data_get_default_int(data, AMF_H264_BITRATE_PEAK));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_MINIMUM), vis_advanced && !vis_rcm_qp);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_MAXIMUM), vis_advanced && !vis_rcm_qp);
	if (!(vis_advanced && !vis_rcm_qp)) {
		obs_data_set_int(data, AMF_H264_QP_MINIMUM, obs_data_get_default_int(data, AMF_H264_QP_MINIMUM));
		obs_data_set_int(data, AMF_H264_QP_MAXIMUM, obs_data_get_default_int(data, AMF_H264_QP_MAXIMUM));
	}

	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_IFRAME), (vis_basic && vis_rcm_qp));
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_PFRAME), (vis_basic && vis_rcm_qp));
	if (!(vis_basic && vis_rcm_qp)) {
		obs_data_set_int(data, AMF_H264_QP_IFRAME, obs_data_get_default_int(data, AMF_H264_QP_IFRAME));
		obs_data_set_int(data, AMF_H264_QP_PFRAME, obs_data_get_default_int(data, AMF_H264_QP_PFRAME));
	}
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BFRAME), (vis_basic && vis_rcm_qp_b));
	if (!(vis_basic && vis_rcm_qp_b))
		obs_data_set_int(data, AMF_H264_QP_BFRAME, obs_data_get_default_int(data, AMF_H264_QP_BFRAME));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_BPICTURE_DELTA), (vis_advanced && vis_rcm_qp_b));
	obs_property_set_visible(obs_properties_get(props, AMF_H264_QP_REFERENCE_BPICTURE_DELTA), (vis_advanced && vis_rcm_qp_b));
	if (!(vis_advanced && vis_rcm_qp_b)) {
		obs_data_set_int(data, AMF_H264_QP_BPICTURE_DELTA, obs_data_get_default_int(data, AMF_H264_QP_BPICTURE_DELTA));
		obs_data_set_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA, obs_data_get_default_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA));
	}

	obs_property_set_visible(obs_properties_get(props, AMF_H264_VBVBUFFER), vis_advanced);
	if (!vis_advanced)
		obs_data_set_int(data, AMF_H264_VBVBUFFER, obs_data_get_default_int(data, AMF_H264_VBVBUFFER));
	uint32_t vbvBuffer = (uint32_t)obs_data_get_int(data, AMF_H264_VBVBUFFER);

	obs_property_set_visible(obs_properties_get(props, AMF_H264_VBVBUFFER_STRICTNESS), (vis_advanced && (vbvBuffer == 0)));
	if (!(vis_advanced && (vbvBuffer == 0)))
		obs_data_set_double(data, AMF_H264_VBVBUFFER_STRICTNESS, obs_data_get_default_double(data, AMF_H264_VBVBUFFER_STRICTNESS));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_VBVBUFFER_SIZE), vis_advanced && (vbvBuffer == 1));
	if (!(vis_advanced && (vbvBuffer == 1)))
		obs_data_set_int(data, AMF_H264_VBVBUFFER_SIZE, obs_data_get_default_int(data, AMF_H264_VBVBUFFER_SIZE));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_VBVBUFFER_FULLNESS), vis_expert);
	if (!vis_expert)
		obs_data_set_double(data, AMF_H264_VBVBUFFER_FULLNESS, obs_data_get_default_double(data, AMF_H264_VBVBUFFER_FULLNESS));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_FILLERDATA), vis_rcm_fillerdata);
	if (!vis_rcm_fillerdata)
		obs_data_set_int(data, AMF_H264_FILLERDATA, obs_data_get_default_int(data, AMF_H264_FILLERDATA));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_FRAMESKIPPING), vis_advanced);
	if (!vis_advanced)
		obs_data_set_int(data, AMF_H264_FRAMESKIPPING, obs_data_get_default_int(data, AMF_H264_FRAMESKIPPING));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_ENFORCEHRDCOMPATIBILITY), vis_expert);
	if (!vis_expert)
		obs_data_set_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY, obs_data_get_default_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_MAXIMUMACCESSUNITSIZE), vis_master);
	if (!vis_master)
		obs_data_set_int(data, AMF_H264_MAXIMUMACCESSUNITSIZE, obs_data_get_default_int(data, AMF_H264_MAXIMUMACCESSUNITSIZE));

	// Picture Control Properties
	obs_property_set_visible(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), !vis_master);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_IDR_PERIOD), vis_master);
	if (vis_master)
		obs_data_set_int(data, AMF_H264_KEYFRAME_INTERVAL, obs_data_get_default_int(data, AMF_H264_KEYFRAME_INTERVAL));
	else
		obs_data_set_int(data, AMF_H264_IDR_PERIOD, obs_data_get_default_int(data, AMF_H264_IDR_PERIOD));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_HEADER_INSERTION_SPACING), vis_master);
	if (!vis_master)
		obs_data_set_int(data, AMF_H264_HEADER_INSERTION_SPACING, obs_data_get_default_int(data, AMF_H264_HEADER_INSERTION_SPACING));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_DEBLOCKINGFILTER), vis_advanced);
	if (!vis_advanced)
		obs_data_set_int(data, AMF_H264_DEBLOCKINGFILTER, obs_data_get_default_int(data, AMF_H264_DEBLOCKINGFILTER));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_INTRAREFRESHNUMMBSPERSLOT), vis_master);
	if (!vis_master)
		obs_data_set_int(data, AMF_H264_INTRAREFRESHNUMMBSPERSLOT, obs_data_get_default_int(data, AMF_H264_INTRAREFRESHNUMMBSPERSLOT));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_SLICESPERFRAME), vis_master);
	if (!vis_master)
		obs_data_set_int(data, AMF_H264_SLICESPERFRAME, obs_data_get_default_int(data, AMF_H264_SLICESPERFRAME));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_PATTERN), vis_advanced && !using_ltr_frames);
	obs_property_set_visible(obs_properties_get(props, AMF_H264_BPICTURE_REFERENCE), vis_advanced && !using_ltr_frames);
	if (!vis_advanced) {
		obs_data_set_int(data, AMF_H264_BPICTURE_PATTERN, obs_data_get_default_int(data, AMF_H264_BPICTURE_PATTERN));
		obs_data_set_int(data, AMF_H264_BPICTURE_REFERENCE, obs_data_get_default_int(data, AMF_H264_BPICTURE_REFERENCE));
	}
	if (using_ltr_frames) {
		obs_data_set_int(data, AMF_H264_BPICTURE_PATTERN, VCEBPicturePattern_None);
		obs_data_set_int(data, AMF_H264_BPICTURE_REFERENCE, 0);
	}

	// Miscellaneous Properties
	obs_property_set_visible(obs_properties_get(props, AMF_H264_SCANTYPE), vis_master);
	if (!vis_master)
		obs_data_set_int(data, AMF_H264_SCANTYPE, obs_data_get_default_int(data, AMF_H264_SCANTYPE));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_QUALITY_PRESET), vis_basic);
	if (!vis_basic)
		obs_data_set_int(data, AMF_H264_QUALITY_PRESET, obs_data_get_default_int(data, AMF_H264_QUALITY_PRESET));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_MOTIONESTIMATION), vis_expert);
	if (!vis_expert)
		obs_data_set_int(data, AMF_H264_MOTIONESTIMATION, obs_data_get_default_int(data, AMF_H264_MOTIONESTIMATION));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_CABAC), vis_expert);
	if (!vis_expert)
		obs_data_set_int(data, AMF_H264_CABAC, obs_data_get_default_int(data, AMF_H264_CABAC));

	// System Properties
	obs_property_set_visible(obs_properties_get(props, AMF_H264_MEMORYTYPE), vis_expert);
	if (!vis_expert)
		obs_data_set_int(data, AMF_H264_MEMORYTYPE, obs_data_get_default_int(data, AMF_H264_MEMORYTYPE));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_DEVICE), vis_expert && (obs_data_get_int(data, AMF_H264_MEMORYTYPE) != 0));
	if (vis_expert && (obs_data_get_int(data, AMF_H264_MEMORYTYPE) != 0)) {
		fill_device_list(obs_properties_get(props, AMF_H264_DEVICE), data);
	} else {
		obs_data_set_string(data, AMF_H264_DEVICE, "");
	}

	obs_property_set_visible(obs_properties_get(props, AMF_H264_USE_OPENCL), vis_expert);
	if (!vis_expert)
		obs_data_set_int(data, AMF_H264_USE_OPENCL, obs_data_get_default_int(data, AMF_H264_USE_OPENCL));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_SURFACEFORMAT), vis_master);
	if (!vis_master)
		obs_data_set_int(data, AMF_H264_SURFACEFORMAT, obs_data_get_default_int(data, AMF_H264_SURFACEFORMAT));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_VIEW), vis_basic);
	if (!vis_basic)
		obs_data_set_int(data, AMF_H264_VIEW, obs_data_get_default_int(data, AMF_H264_VIEW));

	obs_property_set_visible(obs_properties_get(props, AMF_H264_UNLOCK_PROPERTIES), vis_master);
	if (!vis_master)
		obs_data_set_int(data, AMF_H264_UNLOCK_PROPERTIES, obs_data_get_default_int(data, AMF_H264_UNLOCK_PROPERTIES));
	unlock_properties_modified(props, nullptr, data);

	obs_property_set_visible(obs_properties_get(props, AMF_H264_DEBUG), vis_basic);
	if (!vis_basic)
		obs_data_set_int(data, AMF_H264_DEBUG, obs_data_get_default_int(data, AMF_H264_DEBUG));

	return true;
}

bool Plugin::Interface::H264Interface::unlock_properties_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	bool unlocked = obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES);
	bool last_unlocked = obs_data_get_bool(data, "last_" vstr(AMF_H264_UNLOCK_PROPERTIES));

	if (last_unlocked != unlocked) {
		uint32_t m = (unlocked == true ? 1000 : 1),
			d = (unlocked == false ? 1000 : 1);
		obs_data_set_int(data, AMF_H264_BITRATE_TARGET, (obs_data_get_int(data, AMF_H264_BITRATE_TARGET) * m) / d);
		obs_data_set_int(data, AMF_H264_BITRATE_PEAK, (obs_data_get_int(data, AMF_H264_BITRATE_PEAK) * m) / d);
		obs_data_set_int(data, AMF_H264_VBVBUFFER_SIZE, (obs_data_get_int(data, AMF_H264_VBVBUFFER_SIZE) * m) / d);

		obs_data_set_bool(data, "last_" vstr(AMF_H264_UNLOCK_PROPERTIES), unlocked);

		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_TARGET),
			obs_property_int_min(obs_properties_get(props, AMF_H264_BITRATE_TARGET)) / d * m,
			obs_property_int_max(obs_properties_get(props, AMF_H264_BITRATE_TARGET)) / d * m, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_PEAK),
			obs_property_int_min(obs_properties_get(props, AMF_H264_BITRATE_PEAK)) / d * m,
			obs_property_int_max(obs_properties_get(props, AMF_H264_BITRATE_PEAK)) / d * m, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_VBVBUFFER_SIZE),
			obs_property_int_min(obs_properties_get(props, AMF_H264_VBVBUFFER_SIZE)) / d * m,
			obs_property_int_max(obs_properties_get(props, AMF_H264_VBVBUFFER_SIZE)) / d * m, 1);
	}

	if (unlocked) {
		//obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_TARGET), 1000, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate, 1);
		//obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_PEAK), 1000, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate, 1);
		//obs_property_int_set_limits(obs_properties_get(props, AMF_H264_VBVBUFFER_SIZE), 1000, 100000000, 1);
		obs_property_float_set_limits(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), 0, 100, 1);
		obs_property_int_set_limits(obs_properties_get(props, AMF_H264_IDR_PERIOD), 0, 1000, 1);
	} else {
		//obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_TARGET), 1, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / 1000, 1);
		//obs_property_int_set_limits(obs_properties_get(props, AMF_H264_BITRATE_PEAK), 1, VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxBitrate / 1000, 1);
		//obs_property_int_set_limits(obs_properties_get(props, AMF_H264_VBVBUFFER_SIZE), 1, 100000, 1);
		obs_property_float_set_limits(obs_properties_get(props, AMF_H264_KEYFRAME_INTERVAL), 1, 100, 1);
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
Plugin::Interface::H264Interface::H264Interface(obs_data_t* data, obs_encoder_t* encoder) {
	AMF_LOG_DEBUG("<H264Interface::H264Interface> Initializing...");

	// OBS Settings
	uint32_t m_cfgWidth = obs_encoder_get_width(encoder);
	uint32_t m_cfgHeight = obs_encoder_get_height(encoder);
	video_t *video = obs_encoder_video(encoder);
	const struct video_output_info *voi = video_output_get_info(video);
	uint32_t m_cfgFPSnum = voi->fps_num;
	uint32_t m_cfgFPSden = voi->fps_den;

	//////////////////////////////////////////////////////////////////////////
	/// Initialize Encoder
	bool debug = obs_data_get_bool(data, AMF_H264_DEBUG);
	Plugin::AMD::AMF::GetInstance()->EnableDebugTrace(debug);
	VCESurfaceFormat surfFormat = (VCESurfaceFormat)obs_data_get_int(data, AMF_H264_SURFACEFORMAT);
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
	m_VideoEncoder = new VCEEncoder(VCEEncoderType_AVC, surfFormat,
		(VCEMemoryType)obs_data_get_int(data, AMF_H264_MEMORYTYPE),
		!!obs_data_get_int(data, AMF_H264_USE_OPENCL),
		std::string(obs_data_get_string(data, AMF_H264_DEVICE)));

	/// Static Properties
	m_VideoEncoder->SetUsage((VCEUsage)obs_data_get_int(data, AMF_H264_USAGE));
	/// Framesize & Framerate
	m_VideoEncoder->SetFrameSize(m_cfgWidth, m_cfgHeight);
	m_VideoEncoder->SetFrameRate(m_cfgFPSnum, m_cfgFPSden);
	/// 
	m_VideoEncoder->SetProfile((VCEProfile)obs_data_get_int(data, AMF_H264_PROFILE));
	m_VideoEncoder->SetProfileLevel((VCEProfileLevel)obs_data_get_int(data, AMF_H264_PROFILELEVEL));
	if ((uint32_t)obs_data_get_int(data, AMF_H264_MAXIMUMLTRFRAMES) > 0)
		m_VideoEncoder->SetBPicturePattern(VCEBPicturePattern_None);
	m_VideoEncoder->SetMaximumLongTermReferenceFrames((uint32_t)obs_data_get_int(data, AMF_H264_MAXIMUMLTRFRAMES));
	/// Miscellaneous Properties
	m_VideoEncoder->SetScanType((VCEScanType)obs_data_get_int(data, AMF_H264_SCANTYPE));
	m_VideoEncoder->SetQualityPreset((VCEQualityPreset)obs_data_get_int(data, AMF_H264_QUALITY_PRESET));
	m_VideoEncoder->SetCABACEnabled(!!obs_data_get_int(data, AMF_H264_CABAC));

	// OBS - Enforce Streaming Service Restrictions
	#pragma region OBS - Enforce Streaming Service Restrictions
	{
		// Profile
		const char* p_str = obs_data_get_string(data, "profile");
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
					obs_data_set_string(data, "profile", "baseline");
					break;
				case VCEProfile_Main:
					obs_data_set_string(data, "profile", "main");
					break;
				case VCEProfile_High:
					obs_data_set_string(data, "profile", "high");
					break;
			}
		}

		// Preset
		const char* preset = obs_data_get_string(data, "preset");
		if (strcmp(preset, "") != 0) {
			if (strcmp(preset, "speed") == 0) {
				m_VideoEncoder->SetQualityPreset(VCEQualityPreset_Speed);
			} else if (strcmp(preset, "balanced") == 0) {
				m_VideoEncoder->SetQualityPreset(VCEQualityPreset_Balanced);
			} else if (strcmp(preset, "quality") == 0) {
				m_VideoEncoder->SetQualityPreset(VCEQualityPreset_Quality);
			}
			obs_data_set_int(data, AMF_H264_QUALITY_PRESET, m_VideoEncoder->GetQualityPreset());
		} else {
			switch (m_VideoEncoder->GetQualityPreset()) {
				case VCEQualityPreset_Speed:
					obs_data_set_string(data, "preset", "speed");
					break;
				case VCEQualityPreset_Balanced:
					obs_data_set_string(data, "preset", "balanced");
					break;
				case VCEQualityPreset_Quality:
					obs_data_set_string(data, "preset", "quality");
					break;
			}
		}
	}
	#pragma endregion OBS - Enforce Streaming Service Restrictions

	/*m_VideoEncoder->SetRateControlPreanalysisEnabled(true);
	m_VideoEncoder->SetNominalRange(true);
	m_VideoEncoder->SetWaitForTask(true);
	m_VideoEncoder->SetMaximumNumberOfReferenceFrames(VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->maxReferenceFrames);*/

	// Dynamic Properties (Can be changed during Encoding)
	this->update(data);

	// Initialize (locks static properties)
	try {
		m_VideoEncoder->Start();
	} catch (...) {
		throw;
	}

	// Dynamic Properties (Can be changed during Encoding)
	//this->update(data);

	AMF_LOG_DEBUG("<H264Interface::H264Interface> Complete.");
}

Plugin::Interface::H264Interface::~H264Interface() {
	AMF_LOG_DEBUG("<H264Interface::~H264Interface> Finalizing...");
	if (m_VideoEncoder) {
		m_VideoEncoder->Stop();
		delete m_VideoEncoder;
	}
	AMF_LOG_DEBUG("<H264Interface::~H264Interface> Complete.");
}

bool Plugin::Interface::H264Interface::update(obs_data_t* data) {
	double_t framerate = (double_t)m_VideoEncoder->GetFrameRate().first / (double_t)m_VideoEncoder->GetFrameRate().second;
	int32_t bitrateMultiplier = obs_data_get_bool(data, AMF_H264_UNLOCK_PROPERTIES) ? 1 : 1000;

	// Rate Control Properties
	m_VideoEncoder->SetRateControlMethod((VCERateControlMethod)obs_data_get_int(data, AMF_H264_RATECONTROLMETHOD));
	m_VideoEncoder->SetMinimumQP((uint8_t)obs_data_get_int(data, AMF_H264_QP_MINIMUM));
	m_VideoEncoder->SetMaximumQP((uint8_t)obs_data_get_int(data, AMF_H264_QP_MAXIMUM));
	switch ((VCERateControlMethod)obs_data_get_int(data, AMF_H264_RATECONTROLMETHOD)) {
		case VCERateControlMethod_ConstantBitrate:
			m_VideoEncoder->SetTargetBitrate((uint32_t)obs_data_get_int(data, AMF_H264_BITRATE_TARGET) * bitrateMultiplier);
			m_VideoEncoder->SetPeakBitrate(m_VideoEncoder->GetTargetBitrate());
			break;
		case VCERateControlMethod_VariableBitrate_PeakConstrained:
			m_VideoEncoder->SetTargetBitrate((uint32_t)obs_data_get_int(data, AMF_H264_BITRATE_TARGET) * bitrateMultiplier);
			m_VideoEncoder->SetPeakBitrate((uint32_t)obs_data_get_int(data, AMF_H264_BITRATE_PEAK) * bitrateMultiplier);
			break;
		case VCERateControlMethod_VariableBitrate_LatencyConstrained:
			m_VideoEncoder->SetTargetBitrate((uint32_t)obs_data_get_int(data, AMF_H264_BITRATE_TARGET) * bitrateMultiplier);
			m_VideoEncoder->SetPeakBitrate((uint32_t)obs_data_get_int(data, AMF_H264_BITRATE_PEAK) * bitrateMultiplier);
			break;
		case VCERateControlMethod_ConstantQP:
			m_VideoEncoder->SetIFrameQP((uint8_t)obs_data_get_int(data, AMF_H264_QP_IFRAME));
			m_VideoEncoder->SetPFrameQP((uint8_t)obs_data_get_int(data, AMF_H264_QP_PFRAME));
			try {
				m_VideoEncoder->SetBFrameQP((uint8_t)obs_data_get_int(data, AMF_H264_QP_BFRAME));
			} catch (...) {}
			break;
	}
	if (obs_data_get_int(data, AMF_H264_VBVBUFFER) == 0) {
		m_VideoEncoder->SetVBVBufferAutomatic(obs_data_get_double(data, AMF_H264_VBVBUFFER_STRICTNESS) / 100.0);
	} else {
		m_VideoEncoder->SetVBVBufferSize((uint32_t)obs_data_get_int(data, AMF_H264_VBVBUFFER_SIZE) * bitrateMultiplier);
	}
	m_VideoEncoder->SetInitialVBVBufferFullness(obs_data_get_double(data, AMF_H264_VBVBUFFER_FULLNESS) / 100.0);
	m_VideoEncoder->SetEnforceHRDRestrictionsEnabled(obs_data_get_int(data, AMF_H264_ENFORCEHRDCOMPATIBILITY) == 1);
	m_VideoEncoder->SetFillerDataEnabled(obs_data_get_int(data, AMF_H264_FILLERDATA) == 1);
	m_VideoEncoder->SetFrameSkippingEnabled(obs_data_get_int(data, AMF_H264_FRAMESKIPPING) == 1);
	if (obs_data_get_int(data, AMF_H264_MAXIMUMACCESSUNITSIZE) != 0)
		m_VideoEncoder->SetMaximumAccessUnitSize((uint32_t)obs_data_get_int(data, AMF_H264_MAXIMUMACCESSUNITSIZE));

	// Picture Control Properties
	if (obs_data_get_int(data, AMF_H264_VIEW) == ViewMode::Master)
		m_VideoEncoder->SetIDRPeriod((uint32_t)obs_data_get_int(data, AMF_H264_IDR_PERIOD));
	else
		m_VideoEncoder->SetIDRPeriod(max((uint32_t)(obs_data_get_double(data, AMF_H264_KEYFRAME_INTERVAL) * framerate), 1));
	if (obs_data_get_int(data, AMF_H264_HEADER_INSERTION_SPACING) != 0)
		m_VideoEncoder->SetHeaderInsertionSpacing((uint32_t)obs_data_get_int(data, AMF_H264_HEADER_INSERTION_SPACING));
	m_VideoEncoder->SetDeblockingFilterEnabled(!!obs_data_get_int(data, AMF_H264_DEBLOCKINGFILTER));
	if (VCECapabilities::GetInstance()->GetEncoderCaps(VCEEncoderType_AVC)->supportsBFrames) {
		try {
			m_VideoEncoder->SetBPicturePattern((VCEBPicturePattern)obs_data_get_int(data, AMF_H264_BPICTURE_PATTERN));
			m_VideoEncoder->SetBPictureReferenceEnabled(!!obs_data_get_int(data, AMF_H264_BPICTURE_REFERENCE));
		} catch (...) {}
		try {
			if (m_VideoEncoder->GetBPicturePattern() != VCEBPicturePattern_None) {
				m_VideoEncoder->SetBPictureDeltaQP((int8_t)obs_data_get_int(data, AMF_H264_QP_BPICTURE_DELTA));
				m_VideoEncoder->SetReferenceBPictureDeltaQP((int8_t)obs_data_get_int(data, AMF_H264_QP_REFERENCE_BPICTURE_DELTA));
			}
		} catch (...) {}
	}
	if (obs_data_get_int(data, AMF_H264_SLICESPERFRAME) != 0)
		m_VideoEncoder->SetSlicesPerFrame((uint32_t)obs_data_get_int(data, AMF_H264_SLICESPERFRAME));
	if (obs_data_get_int(data, AMF_H264_INTRAREFRESHNUMMBSPERSLOT) != 0)
		m_VideoEncoder->SetIntraRefreshMBsNumberPerSlot((uint32_t)obs_data_get_int(data, AMF_H264_INTRAREFRESHNUMMBSPERSLOT));

	// Miscellaneous Properties
	m_VideoEncoder->SetHalfPixelMotionEstimationEnabled(!!(obs_data_get_int(data, AMF_H264_MOTIONESTIMATION) & 1));
	m_VideoEncoder->SetQuarterPixelMotionEstimationEnabled(!!(obs_data_get_int(data, AMF_H264_MOTIONESTIMATION) & 2));

	// OBS - Enforce Streaming Service Stuff
	#pragma region OBS Enforce Streaming Service Settings
	{
		// Rate Control Method
		const char* t_str = obs_data_get_string(data, "rate_control");
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

			obs_data_set_int(data, AMF_H264_RATECONTROLMETHOD, m_VideoEncoder->GetRateControlMethod());
		} else {
			switch (m_VideoEncoder->GetRateControlMethod()) {
				case VCERateControlMethod_ConstantBitrate:
					obs_data_set_string(data, "rate_control", "CBR");
					break;
				case VCERateControlMethod_VariableBitrate_PeakConstrained:
					obs_data_set_string(data, "rate_control", "VBR");
					break;
				case VCERateControlMethod_VariableBitrate_LatencyConstrained:
					obs_data_set_string(data, "rate_control", "VBR_LAT");
					break;
				case VCERateControlMethod_ConstantQP:
					obs_data_set_string(data, "rate_control", "CQP");
					break;
			}
		}

		// Bitrate
		uint64_t bitrateOvr = obs_data_get_int(data, "bitrate") * 1000;
		if (bitrateOvr != -1) {
			if (m_VideoEncoder->GetTargetBitrate() > bitrateOvr)
				m_VideoEncoder->SetTargetBitrate((uint32_t)bitrateOvr);

			if (m_VideoEncoder->GetPeakBitrate() > bitrateOvr)
				m_VideoEncoder->SetPeakBitrate((uint32_t)bitrateOvr);

			obs_data_set_int(data, "bitrate", m_VideoEncoder->GetTargetBitrate() / 1000);

			obs_data_set_int(data, AMF_H264_BITRATE_TARGET, m_VideoEncoder->GetTargetBitrate() / bitrateMultiplier);
			obs_data_set_int(data, AMF_H264_BITRATE_PEAK, m_VideoEncoder->GetPeakBitrate() / bitrateMultiplier);
		} else {
			obs_data_set_int(data, "bitrate", m_VideoEncoder->GetTargetBitrate() / 1000);
		}

		// IDR-Period (Keyframes)
		uint32_t fpsNum = m_VideoEncoder->GetFrameRate().first;
		uint32_t fpsDen = m_VideoEncoder->GetFrameRate().second;
		if (obs_data_get_int(data, "keyint_sec") != -1) {
			m_VideoEncoder->SetIDRPeriod((uint32_t)(obs_data_get_int(data, "keyint_sec") * ((double_t)fpsNum / (double_t)fpsDen)));

			obs_data_set_double(data, AMF_H264_KEYFRAME_INTERVAL, (double_t)obs_data_get_int(data, "keyint_sec"));
			obs_data_set_int(data, AMF_H264_IDR_PERIOD, (uint32_t)(obs_data_get_int(data, "keyint_sec") * ((double_t)fpsNum / (double_t)fpsDen)));
		} else {
			obs_data_set_int(data, "keyint_sec", (uint64_t)(m_VideoEncoder->GetIDRPeriod() / ((double_t)fpsNum / (double_t)fpsDen)));
		}
	}
	#pragma endregion OBS Enforce Streaming Service Settings

	if (m_VideoEncoder->IsStarted()) { // Verify
		m_VideoEncoder->LogProperties();
		if (obs_data_get_int(data, AMF_H264_VIEW) >= ViewMode::Master)
			AMF_LOG_ERROR("View Mode 'Master' is active, avoid giving anything but basic support. Error is most likely caused by user settings themselves.");
	}

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
