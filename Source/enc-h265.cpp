/*
MIT License

Copyright (c) 2016-2017

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

#include "enc-h265.h"
#include "amf-capabilities.h"
#include "amf-encoder-h265.h"
#include "misc-util.cpp"

#define PREFIX "[HEVC]"

void Plugin::Interface::H265Interface::encoder_register() {
	// Test if we actually have AVC support.
	if (!AMD::CapabilityManager::Instance()->IsCodecSupported(Codec::HEVC)) {
		AMF_LOG_WARNING(PREFIX " Not supported by any GPU, disabling...");
		return;
	}

	// Create structure
	static std::unique_ptr<obs_encoder_info> encoder_info = std::make_unique<obs_encoder_info>();
	std::memset(encoder_info.get(), 0, sizeof(obs_encoder_info));

	// Initialize Structure
	encoder_info->type = obs_encoder_type::OBS_ENCODER_VIDEO;
	static const char* encoder_name = "amd_amf_h265";
	encoder_info->id = encoder_name;
	static const char* encoder_codec = "h265";
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

	obs_register_encoder(encoder_info.get());
}

const char* Plugin::Interface::H265Interface::get_name(void* type_data) {
	static const char* name = "H265/HEVC Encoder (" PLUGIN_NAME_AMF ")";
	return name;
}

void Plugin::Interface::H265Interface::get_defaults(obs_data_t *settings) {

}

obs_properties_t* Plugin::Interface::H265Interface::get_properties(void* ptr) {
	//////////////////////////////////////////////////////////////////////////
	// New UI Design
	//////////////////////////////////////////////////////////////////////////
	// All: Preset
	// ----------- Static Section
	// Mas: Usage
	// All: Quality Preset
	// Adv: Profile
	// Adv: Profile Level
	// Adv: Tier
	// Exp: Aspect Ratio
	// Exp: Coding Type
	// Exp: Maximum Reference Frames
	// ----------- Rate Control Section
	// All: Rate Control Method
	// Adv: Pre-Pass Encoding (if supported)
	// All, CBR&VBR: Target Bitrate
	// All, VBR: Peak Bitrate
	// All, CQP: QP I/P/B
	// Adv, CBR&VBR: Min/Max QP
	// CBR: Filler Data
	// Adv: Frame Skipping
	// Exp: VBAQ
	// Exp: Enforce HRD 
	// ----------- VBV Buffer
	// Adv: VBV Buffer Size
	// Exp: VBV Buffer Initial Fullness
	// ----------- Picture Control
	// All: Keyframe Interval (Float, uses GOP Size Fixed/Min/Max)
	// Mas: IDR Period (in GOPs)
	// Exp: GOP Type
	// Exp: GOP Size
	// Exp: GOP Size Min/Max
	// Exp: Deblocking Filter
	// Exp: Motion Estimation (Dropdown)
	// ----------- Intra-Refresh
	// ToDo: Master Mode only?
	// ----------- System
	// Adv: API
	// Adv: Adapter
	// Exp: OpenCL
	// All: View

	obs_properties* props = obs_properties_create();
	obs_property_t* p;

	// Static Properties
	#pragma region Usage
	p = obs_properties_add_list(props, P_USAGE, P_TEXT(P_USAGE), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TEXT(P_DESC(P_USAGE)));
	obs_property_list_add_int(p, P_TEXT(P_USAGE_TRANSCODING), static_cast<int32_t>(Usage::Transcoding));
	obs_property_list_add_int(p, P_TEXT(P_USAGE_ULTRALOWLATENCY), static_cast<int32_t>(Usage::UltraLowLatency));
	obs_property_list_add_int(p, P_TEXT(P_USAGE_LOWLATENCY), static_cast<int32_t>(Usage::LowLatency));
	obs_property_list_add_int(p, P_TEXT(P_USAGE_WEBCAM), static_cast<int32_t>(Usage::Webcam));
	#pragma endregion Usage

	#pragma region Quality Preset
	p = obs_properties_add_list(props, P_QUALITYPRESET, P_TEXT(P_QUALITYPRESET), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_set_long_description(p, P_TEXT(P_DESC(P_QUALITYPRESET)));
	obs_property_list_add_int(p, P_TEXT(P_QUALITYPRESET_SPEED), static_cast<int32_t>(QualityPreset::Speed));
	obs_property_list_add_int(p, P_TEXT(P_QUALITYPRESET_BALANCED), static_cast<int32_t>(QualityPreset::Balanced));
	obs_property_list_add_int(p, P_TEXT(P_QUALITYPRESET_QUALITY), static_cast<int32_t>(QualityPreset::Quality));
	#pragma endregion Quality Preset

	return props;
}

bool Plugin::Interface::H265Interface::properties_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data) {
	return true;
}

void* Plugin::Interface::H265Interface::create(obs_data_t* data, obs_encoder_t* encoder) {
	return new H265Interface(data, encoder);
}

Plugin::Interface::H265Interface::H265Interface(obs_data_t* data, obs_encoder_t* encoder) {
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Initializing...");

	// OBS Settings
	uint32_t obsWidth = obs_encoder_get_width(encoder);
	uint32_t obsHeight = obs_encoder_get_height(encoder);
	video_t *obsVideoInfo = obs_encoder_video(encoder);
	const struct video_output_info *voi = video_output_get_info(obsVideoInfo);
	uint32_t obsFPSnum = voi->fps_num;
	uint32_t obsFPSden = voi->fps_den;
	//////////////////////////////////////////////////////////////////////////
	/// Initialize Encoder
	bool debug = obs_data_get_bool(data, P_DEBUG);
	Plugin::AMD::AMF::Instance()->EnableDebugTrace(debug);

	ColorFormat colorFormat = ColorFormat::NV12;
	switch (voi->format) {
		case VIDEO_FORMAT_NV12:
			colorFormat = ColorFormat::NV12;
			break;
		case VIDEO_FORMAT_I420:
			colorFormat = ColorFormat::I420;
			break;
		case VIDEO_FORMAT_YUY2:
			colorFormat = ColorFormat::YUY2;
			break;
		case VIDEO_FORMAT_RGBA:
			colorFormat = ColorFormat::RGBA;
			break;
		case VIDEO_FORMAT_BGRA:
			colorFormat = ColorFormat::BGRA;
			break;
		case VIDEO_FORMAT_Y800:
			colorFormat = ColorFormat::GRAY;
			break;
	}
	ColorSpace colorSpace = ColorSpace::BT601;
	switch (voi->colorspace) {
		case VIDEO_CS_601:
			colorSpace = ColorSpace::BT601;
			break;
		case VIDEO_CS_DEFAULT:
		case VIDEO_CS_709:
			colorSpace = ColorSpace::BT709;
			break;
	}

	auto api = API::GetAPI(obs_data_get_string(data, P_VIDEO_API));
	union {
		int64_t v;
		uint32_t id[2];
	} adapterid = { obs_data_get_int(data, P_VIDEO_ADAPTER) };
	auto adapter = api->GetAdapterById(adapterid.id[0], adapterid.id[1]);

	m_VideoEncoder = std::make_unique<Plugin::AMD::EncoderH265>(api, adapter, !!obs_data_get_int(data, P_OPENCL),
		colorFormat, colorSpace, voi->range == VIDEO_RANGE_FULL);

	// Setup stuff

	m_VideoEncoder->Start();
}

void Plugin::Interface::H265Interface::destroy(void* ptr) {
	delete static_cast<H265Interface*>(ptr);
}

Plugin::Interface::H265Interface::~H265Interface() {
	m_VideoEncoder->Stop();
}

bool Plugin::Interface::H265Interface::update(void *ptr, obs_data_t *settings) {
	return static_cast<H265Interface*>(ptr)->update(settings);
}

bool Plugin::Interface::H265Interface::update(obs_data_t* settings) {
	return true;
}

bool Plugin::Interface::H265Interface::encode(void *ptr, struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	return static_cast<H265Interface*>(ptr)->encode(frame, packet, received_packet);
}

bool Plugin::Interface::H265Interface::encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet) {
	return m_VideoEncoder->Encode(frame, packet, received_packet);
}

void Plugin::Interface::H265Interface::get_video_info(void *ptr, struct video_scale_info *info) {
	return static_cast<H265Interface*>(ptr)->get_video_info(info);
}

void Plugin::Interface::H265Interface::get_video_info(struct video_scale_info* info) {
	m_VideoEncoder->GetVideoInfo(info);
}

bool Plugin::Interface::H265Interface::get_extra_data(void *ptr, uint8_t** extra_data, size_t* size) {
	return static_cast<H265Interface*>(ptr)->get_extra_data(extra_data, size);
}

bool Plugin::Interface::H265Interface::get_extra_data(uint8_t** extra_data, size_t* size) {
	return m_VideoEncoder->GetExtraData(extra_data, size);
}
