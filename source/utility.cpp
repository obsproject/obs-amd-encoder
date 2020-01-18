/*
 * A Plugin that integrates the AMD AMF encoder into OBS Studio
 * Copyright (C) 2016 - 2018 Michael Fabian Dirks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#pragma once

#include "utility.hpp"
#include <map>
#include <sstream>
#include "amf-capabilities.hpp"
#include "amf-encoder-h264.hpp"
#include "amf-encoder-h265.hpp"
#include "amf-encoder.hpp"
#include "amf.hpp"

#include <components/VideoConverter.h>
#include <components/VideoEncoderHEVC.h>
#include <components/VideoEncoderVCE.h>

using namespace Plugin::AMD;

uint64_t Utility::GetUniqueIdentifier()
{
	static std::mutex __mutex;
	static uint64_t   __curId;

	const std::lock_guard<std::mutex> lock(__mutex);
	return ++__curId;
}

const char* Utility::obs_module_text_multi(const char* key, uint8_t depth)
{
	static std::map<std::string, std::string> translatedMap;
#ifndef LITE_OBS
	// Check if it already was translated.
	if (!translatedMap.count(std::string(key))) { // If not, translate it now.
		const char* out = obs_module_text(key);

		// Allow for nested translations using \@...\@ sequences.
		if (depth > 0) {
			// I'm pretty sure this can be optimized a ton if necessary.

			size_t seqStart = 0, seqEnd = 0;
			bool   haveSequence = false;

			std::stringstream fout;

			// Walk the given string.
			std::string walkable = std::string(out);

			for (size_t pos = 0; pos <= walkable.length(); pos++) {
				std::string walked = walkable.substr(pos, 2);

				if (walked == "\\@") { // Sequence Start/End
					if (haveSequence) {
						seqEnd = pos;

						std::string sequence = walkable.substr(seqStart, seqEnd - seqStart);
						fout << obs_module_text_multi(sequence.c_str(), depth--);
					} else {
						seqStart = pos + 2;
					}
					haveSequence = !haveSequence;
					pos          = pos + 1;
				} else if (!haveSequence) {
					fout << walked.substr(0, 1); // Append the left character.
				}
			}

			std::pair<std::string, std::string> kv = std::pair<std::string, std::string>(std::string(key), fout.str());
			translatedMap.insert(kv);
		} else {
			return out;
		}
	}

	auto value = translatedMap.find(std::string(key));
	return value->second.c_str();
#else
	depth;
	return key;
#endif
}

#ifndef LITE_OBS
void Utility::fill_api_list(obs_property_t* property, Plugin::AMD::Codec codec)
{
	obs_property_list_clear(property);

	auto cm = Plugin::AMD::CapabilityManager::Instance();

	for (auto api : Plugin::API::EnumerateAPIs()) {
		if (cm->IsCodecSupportedByAPI(codec, api->GetType()))
			obs_property_list_add_string(property, api->GetName().c_str(), api->GetName().c_str());
	}
}

void Utility::fill_device_list(obs_property_t* property, std::string api_name, Plugin::AMD::Codec codec)
{
	obs_property_list_clear(property);

	auto api = Plugin::API::GetAPI(api_name);
	auto cm  = Plugin::AMD::CapabilityManager::Instance();

	for (auto adapter : api->EnumerateAdapters()) {
		union {
			int32_t id[2];
			int64_t v;
		} adapterid = {adapter.idLow, adapter.idHigh};
		if (cm->IsCodecSupportedByAPIAdapter(codec, api->GetType(), adapter))
			obs_property_list_add_int(property, adapter.Name.c_str(), adapterid.v);
	}
}
#endif

// Codec
const char* Utility::CodecToString(Plugin::AMD::Codec v)
{
	switch (v) {
	case Codec::AVC:
		return "H264/AVC";
	case Codec::SVC:
		return "H264/SVC";
	case Codec::HEVC:
		return "H265/HEVC";
	}
	throw std::runtime_error("Invalid Parameter");
}

const wchar_t* Utility::CodecToAMF(Plugin::AMD::Codec v)
{
	switch (v) {
	case Codec::AVC:
		return AMFVideoEncoderVCE_AVC;
	case Codec::SVC:
		return AMFVideoEncoderVCE_SVC;
	case Codec::HEVC:
		return AMFVideoEncoder_HEVC;
	}
	throw std::runtime_error("Invalid Parameter");
}

// Color Format
const char* Utility::ColorFormatToString(Plugin::AMD::ColorFormat v)
{
	switch (v) {
	case ColorFormat::I420:
		return "YUV 4:2:0";
	case ColorFormat::NV12:
		return "NV12";
	case ColorFormat::YUY2:
		return "YUY2";
	case ColorFormat::BGRA:
		return "BGRA";
	case ColorFormat::RGBA:
		return "RGBA";
	case ColorFormat::GRAY:
		return "GRAY";
	}
	throw std::runtime_error("Invalid Parameter");
}

amf::AMF_SURFACE_FORMAT Utility::ColorFormatToAMF(Plugin::AMD::ColorFormat v)
{
	switch (v) {
	case ColorFormat::I420:
		return amf::AMF_SURFACE_YUV420P;
	case ColorFormat::NV12:
		return amf::AMF_SURFACE_NV12;
	case ColorFormat::YUY2:
		return amf::AMF_SURFACE_YUY2;
	case ColorFormat::BGRA:
		return amf::AMF_SURFACE_BGRA;
	case ColorFormat::RGBA:
		return amf::AMF_SURFACE_RGBA;
	case ColorFormat::GRAY:
		return amf::AMF_SURFACE_GRAY8;
	}
	throw std::runtime_error("Invalid Parameter");
}

// Color Space
const char* Utility::ColorSpaceToString(Plugin::AMD::ColorSpace v)
{
	switch (v) {
	case ColorSpace::BT601:
		return "601";
	case ColorSpace::BT709:
		return "709";
	case ColorSpace::BT2020:
		return "2020";
	case ColorSpace::SRGB:
		return "sRGB";
	}
	throw std::runtime_error("Invalid Parameter");
}

AMF_VIDEO_CONVERTER_COLOR_PROFILE_ENUM
Utility::ColorSpaceToAMFConverter(Plugin::AMD::ColorSpace v)
{
	switch (v) {
	case ColorSpace::BT601:
		return AMF_VIDEO_CONVERTER_COLOR_PROFILE_601;
	case ColorSpace::BT709:
	case ColorSpace::SRGB:
		return AMF_VIDEO_CONVERTER_COLOR_PROFILE_709;
	case ColorSpace::BT2020:
		return AMF_VIDEO_CONVERTER_COLOR_PROFILE_2020;
	}
	throw std::runtime_error("Invalid Parameter");
}

AMF_COLOR_TRANSFER_CHARACTERISTIC_ENUM Utility::ColorSpaceToTransferCharacteristic(Plugin::AMD::ColorSpace v)
{
	switch (v) {
	case ColorSpace::BT601:
		return AMF_COLOR_TRANSFER_CHARACTERISTIC_SMPTE170M;
	case ColorSpace::BT709:
		return AMF_COLOR_TRANSFER_CHARACTERISTIC_BT709;
	case ColorSpace::BT2020:
		return AMF_COLOR_TRANSFER_CHARACTERISTIC_BT2020_10;
	case ColorSpace::SRGB:
		return AMF_COLOR_TRANSFER_CHARACTERISTIC_IEC61966_2_1;
	}
	throw std::runtime_error("Invalid Parameter");
}

// Usage
const char* Utility::UsageToString(Plugin::AMD::Usage v)
{
	switch (v) {
	case Usage::Transcoding:
		return "Transcoding";
	case Usage::UltraLowLatency:
		return "Ultra Low Latency";
	case Usage::LowLatency:
		return "Low Latency";
	case Usage::Webcam:
		return "Webcam";
	}
	throw std::runtime_error("Invalid Parameter");
}

AMF_VIDEO_ENCODER_USAGE_ENUM Utility::UsageToAMFH264(Plugin::AMD::Usage v)
{
	switch (v) {
	case Usage::Transcoding:
		return AMF_VIDEO_ENCODER_USAGE_TRANSCONDING;
	case Usage::UltraLowLatency:
		return AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY;
	case Usage::LowLatency:
		return AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY;
	case Usage::Webcam:
		return AMF_VIDEO_ENCODER_USAGE_WEBCAM;
	}
	throw std::runtime_error("Invalid Parameter");
}

Plugin::AMD::Usage Utility::UsageFromAMFH264(AMF_VIDEO_ENCODER_USAGE_ENUM v)
{
	switch (v) {
	case AMF_VIDEO_ENCODER_USAGE_TRANSCONDING:
		return Plugin::AMD::Usage::Transcoding;
	case AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY:
		return Plugin::AMD::Usage::UltraLowLatency;
	case AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY:
		return Plugin::AMD::Usage::LowLatency;
	case AMF_VIDEO_ENCODER_USAGE_WEBCAM:
		return Plugin::AMD::Usage::Webcam;
	}
	throw std::runtime_error("Invalid Parameter");
}

AMF_VIDEO_ENCODER_HEVC_USAGE_ENUM Utility::UsageToAMFH265(Plugin::AMD::Usage v)
{
	switch (v) {
	case Usage::Transcoding:
		return AMF_VIDEO_ENCODER_HEVC_USAGE_TRANSCONDING;
	case Usage::UltraLowLatency:
		return AMF_VIDEO_ENCODER_HEVC_USAGE_ULTRA_LOW_LATENCY;
	case Usage::LowLatency:
		return AMF_VIDEO_ENCODER_HEVC_USAGE_LOW_LATENCY;
	case Usage::Webcam:
		return AMF_VIDEO_ENCODER_HEVC_USAGE_WEBCAM;
	}
	throw std::runtime_error("Invalid Parameter");
}

Plugin::AMD::Usage Utility::UsageFromAMFH265(AMF_VIDEO_ENCODER_HEVC_USAGE_ENUM v)
{
	switch (v) {
	case AMF_VIDEO_ENCODER_HEVC_USAGE_TRANSCONDING:
		return Usage::Transcoding;
	case AMF_VIDEO_ENCODER_HEVC_USAGE_ULTRA_LOW_LATENCY:
		return Usage::UltraLowLatency;
	case AMF_VIDEO_ENCODER_HEVC_USAGE_LOW_LATENCY:
		return Usage::LowLatency;
	case AMF_VIDEO_ENCODER_HEVC_USAGE_WEBCAM:
		return Usage::Webcam;
	}
	throw std::runtime_error("Invalid Parameter");
}

// Quality Preset
const char* Utility::QualityPresetToString(Plugin::AMD::QualityPreset v)
{
	switch (v) {
	case QualityPreset::Speed:
		return "Speed";
	case QualityPreset::Balanced:
		return "Balanced";
	case QualityPreset::Quality:
		return "Quality";
	}
	throw std::runtime_error("Invalid Parameter");
}

AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM
Utility::QualityPresetToAMFH264(Plugin::AMD::QualityPreset v)
{
	switch (v) {
	case QualityPreset::Speed:
		return AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED;
	case QualityPreset::Balanced:
		return AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED;
	case QualityPreset::Quality:
		return AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY;
	}
	throw std::runtime_error("Invalid Parameter");
}

Plugin::AMD::QualityPreset Utility::QualityPresetFromAMFH264(AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM v)
{
	switch (v) {
	case AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED:
		return QualityPreset::Speed;
	case AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED:
		return QualityPreset::Balanced;
	case AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY:
		return QualityPreset::Quality;
	}
	throw std::runtime_error("Invalid Parameter");
}

AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_ENUM
Utility::QualityPresetToAMFH265(Plugin::AMD::QualityPreset v)
{
	switch (v) {
	case QualityPreset::Speed:
		return AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_SPEED;
	case QualityPreset::Balanced:
		return AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_BALANCED;
	case QualityPreset::Quality:
		return AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_QUALITY;
	}
	throw std::runtime_error("Invalid Parameter");
}

Plugin::AMD::QualityPreset Utility::QualityPresetFromAMFH265(AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_ENUM v)
{
	switch (v) {
	case AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_SPEED:
		return QualityPreset::Speed;
	case AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_BALANCED:
		return QualityPreset::Balanced;
	case AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_QUALITY:
		return QualityPreset::Quality;
	}
	throw std::runtime_error("Invalid Parameter");
}

// Profile
const char* Utility::ProfileToString(Plugin::AMD::Profile v)
{
	switch (v) {
	case Profile::ConstrainedBaseline:
		return "Constrained Baseline";
	case Profile::Baseline:
		return "Baseline";
	case Profile::Main:
		return "Main";
	case Profile::ConstrainedHigh:
		return "Constrained High";
	case Profile::High:
		return "High";
	}
	throw std::runtime_error("Invalid Parameter");
}

AMF_VIDEO_ENCODER_PROFILE_ENUM Utility::ProfileToAMFH264(Plugin::AMD::Profile v)
{
	switch (v) {
	case Profile::ConstrainedBaseline:
		return AMF_VIDEO_ENCODER_PROFILE_CONSTRAINED_BASELINE;
	case Profile::Baseline:
		return AMF_VIDEO_ENCODER_PROFILE_BASELINE;
	case Profile::Main:
		return AMF_VIDEO_ENCODER_PROFILE_MAIN;
	case Profile::ConstrainedHigh:
		return AMF_VIDEO_ENCODER_PROFILE_CONSTRAINED_HIGH;
	case Profile::High:
		return AMF_VIDEO_ENCODER_PROFILE_HIGH;
	}
	throw std::runtime_error("Invalid Parameter");
}

Plugin::AMD::Profile Utility::ProfileFromAMFH264(AMF_VIDEO_ENCODER_PROFILE_ENUM v)
{
#pragma warning(disable : 4063) // Developer Note: I know better, Compiler.
	switch (v) {
	case AMF_VIDEO_ENCODER_PROFILE_CONSTRAINED_BASELINE:
		return Profile::ConstrainedBaseline;
	case AMF_VIDEO_ENCODER_PROFILE_BASELINE:
		return Profile::Baseline;
	case AMF_VIDEO_ENCODER_PROFILE_MAIN:
		return Profile::Main;
	case AMF_VIDEO_ENCODER_PROFILE_CONSTRAINED_HIGH:
		return Profile::ConstrainedHigh;
	case AMF_VIDEO_ENCODER_PROFILE_HIGH:
		return Profile::High;
	}
	throw std::runtime_error("Invalid Parameter");
}

AMF_VIDEO_ENCODER_HEVC_PROFILE_ENUM
Utility::ProfileToAMFH265(Plugin::AMD::Profile v)
{
	switch (v) {
	case Profile::Main:
		return AMF_VIDEO_ENCODER_HEVC_PROFILE_MAIN;
	}
	throw std::runtime_error("Invalid Parameter");
}

Plugin::AMD::Profile Utility::ProfileFromAMFH265(AMF_VIDEO_ENCODER_HEVC_PROFILE_ENUM v)
{
	switch (v) {
	case AMF_VIDEO_ENCODER_HEVC_PROFILE_MAIN:
		return Profile::Main;
	}
	throw std::runtime_error("Invalid Parameter");
}

// Tier
const char* Utility::TierToString(Plugin::AMD::H265::Tier v)
{
	switch (v) {
	case H265::Tier::Main:
		return "Main";
	case H265::Tier::High:
		return "High";
	}
	throw std::runtime_error("Invalid Parameter");
}

AMF_VIDEO_ENCODER_HEVC_TIER_ENUM
Utility::TierToAMFH265(Plugin::AMD::H265::Tier v)
{
	switch (v) {
	case H265::Tier::Main:
		return AMF_VIDEO_ENCODER_HEVC_TIER_MAIN;
	case H265::Tier::High:
		return AMF_VIDEO_ENCODER_HEVC_TIER_HIGH;
	}
	throw std::runtime_error("Invalid Parameter");
}

Plugin::AMD::H265::Tier Utility::TierFromAMFH265(AMF_VIDEO_ENCODER_HEVC_TIER_ENUM v)
{
	switch (v) {
	case AMF_VIDEO_ENCODER_HEVC_TIER_MAIN:
		return H265::Tier::Main;
	case AMF_VIDEO_ENCODER_HEVC_TIER_HIGH:
		return H265::Tier::High;
	}
	throw std::runtime_error("Invalid Parameter");
}

// Coding Type
const char* Utility::CodingTypeToString(Plugin::AMD::CodingType v)
{
	switch (v) {
	case CodingType::Automatic:
		return "Automatic";
	case CodingType::CALVC:
		return "CALVC";
	case CodingType::CABAC:
		return "CABAC";
	}
	throw std::runtime_error("Invalid Parameter");
}

AMF_VIDEO_ENCODER_CODING_ENUM
Utility::CodingTypeToAMFH264(Plugin::AMD::CodingType v)
{
	switch (v) {
	case CodingType::Automatic:
		return AMF_VIDEO_ENCODER_UNDEFINED;
	case CodingType::CALVC:
		return AMF_VIDEO_ENCODER_CALV;
	case CodingType::CABAC:
		return AMF_VIDEO_ENCODER_CABAC;
	}
	throw std::runtime_error("Invalid Parameter");
}

Plugin::AMD::CodingType Utility::CodingTypeFromAMFH264(AMF_VIDEO_ENCODER_CODING_ENUM v)
{
	switch (v) {
	case AMF_VIDEO_ENCODER_UNDEFINED:
		return CodingType::Automatic;
	case AMF_VIDEO_ENCODER_CALV:
		return CodingType::CALVC;
	case AMF_VIDEO_ENCODER_CABAC:
		return CodingType::CABAC;
	}
	throw std::runtime_error("Invalid Parameter");
}

int64_t Utility::CodingTypeToAMFH265(Plugin::AMD::CodingType v)
{
	switch (v) {
	case CodingType::Automatic:
		return 0;
	case CodingType::CABAC:
		return 1;
	}
	throw std::runtime_error("Invalid Parameter");
}

Plugin::AMD::CodingType Utility::CodingTypeFromAMFH265(int64_t v)
{
	switch (v) {
	case 0:
		return CodingType::Automatic;
	case 1:
		return CodingType::CABAC;
	}
	throw std::runtime_error("Invalid Parameter");
}

// Rate Control Method
const char* Utility::RateControlMethodToString(Plugin::AMD::RateControlMethod v)
{
	switch (v) {
	case RateControlMethod::ConstantQP:
		return "Constant Quantization Parameter";
	case RateControlMethod::ConstantBitrate:
		return "Constant Bitrate";
	case RateControlMethod::PeakConstrainedVariableBitrate:
		return "Peak Constrained Variable Bitrate";
	case RateControlMethod::LatencyConstrainedVariableBitrate:
		return "Latency Constrained Variable Bitrate";
	}
	throw std::runtime_error("Invalid Parameter");
}

AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM
Utility::RateControlMethodToAMFH264(Plugin::AMD::RateControlMethod v)
{
	switch (v) {
	case RateControlMethod::ConstantQP:
		return AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP;
	case RateControlMethod::ConstantBitrate:
		return AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR;
	case RateControlMethod::PeakConstrainedVariableBitrate:
		return AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
	case RateControlMethod::LatencyConstrainedVariableBitrate:
		return AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR;
	}
	throw std::runtime_error("Invalid Parameter");
}

Plugin::AMD::RateControlMethod Utility::RateControlMethodFromAMFH264(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM v)
{
	switch (v) {
	case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP:
		return RateControlMethod::ConstantQP;
	case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR:
		return RateControlMethod::ConstantBitrate;
	case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR:
		return RateControlMethod::PeakConstrainedVariableBitrate;
	case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR:
		return RateControlMethod::LatencyConstrainedVariableBitrate;
	}
	throw std::runtime_error("Invalid Parameter");
}

AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_ENUM
Utility::RateControlMethodToAMFH265(Plugin::AMD::RateControlMethod v)
{
	switch (v) {
	case RateControlMethod::ConstantQP:
		return AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CONSTANT_QP;
	case RateControlMethod::ConstantBitrate:
		return AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CBR;
	case RateControlMethod::PeakConstrainedVariableBitrate:
		return AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
	case RateControlMethod::LatencyConstrainedVariableBitrate:
		return AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR;
	}
	throw std::runtime_error("Invalid Parameter");
}

Plugin::AMD::RateControlMethod Utility::RateControlMethodFromAMFH265(AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_ENUM v)
{
	switch (v) {
	case AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CONSTANT_QP:
		return RateControlMethod::ConstantQP;
	case AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CBR:
		return RateControlMethod::ConstantBitrate;
	case AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR:
		return RateControlMethod::PeakConstrainedVariableBitrate;
	case AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR:
		return RateControlMethod::LatencyConstrainedVariableBitrate;
	}
	throw std::runtime_error("Invalid Parameter");
}

// Pre-Pass Method
const char* Utility::PrePassModeToString(Plugin::AMD::PrePassMode v)
{
	switch (v) {
	case PrePassMode::Disabled:
		return "Disabled";
	case PrePassMode::Enabled:
		return "Enabled";
	case PrePassMode::EnabledAtHalfScale:
		return "Enabled (Half Scale)";
	case PrePassMode::EnabledAtQuarterScale:
		return "Enabled (Quarter Scale)";
	}
	throw std::runtime_error("Invalid Parameter");
}

AMF_VIDEO_ENCODER_PREENCODE_MODE_ENUM
Utility::PrePassModeToAMFH264(Plugin::AMD::PrePassMode v)
{
	switch (v) {
	case PrePassMode::Disabled:
		return AMF_VIDEO_ENCODER_PREENCODE_DISABLED;
	case PrePassMode::Enabled:
		return AMF_VIDEO_ENCODER_PREENCODE_ENABLED;
	case PrePassMode::EnabledAtHalfScale:
		return (AMF_VIDEO_ENCODER_PREENCODE_MODE_ENUM)2;
	case PrePassMode::EnabledAtQuarterScale:
		return (AMF_VIDEO_ENCODER_PREENCODE_MODE_ENUM)3;
	}
	throw std::runtime_error("Invalid Parameter");
}
Plugin::AMD::PrePassMode Utility::PrePassModeFromAMFH264(AMF_VIDEO_ENCODER_PREENCODE_MODE_ENUM v)
{
	switch (v) {
	case AMF_VIDEO_ENCODER_PREENCODE_DISABLED:
		return PrePassMode::Disabled;
	case AMF_VIDEO_ENCODER_PREENCODE_ENABLED:
		return PrePassMode::Enabled;
	case 2:
		return PrePassMode::EnabledAtHalfScale;
	case 3:
		return PrePassMode::EnabledAtQuarterScale;
	}
	throw std::runtime_error("Invalid Parameter");
}

// GOP Type
const char* Utility::GOPTypeToString(Plugin::AMD::H265::GOPType v)
{
	switch (v) {
	case H265::GOPType::Fixed:
		return "Fixed";
	case H265::GOPType::Variable:
		return "Variable";
	}
	throw std::runtime_error("Invalid Parameter");
}
Plugin::AMD::H265::GOPType Utility::GOPTypeFromAMFH265(int64_t v)
{
	switch (v) {
	case 0:
		return H265::GOPType::Fixed;
	case 1:
		return H265::GOPType::Variable;
	}
	throw std::runtime_error("Invalid Parameter");
}
int64_t Utility::GOPTypeToAMFH265(Plugin::AMD::H265::GOPType v)
{
	switch (v) {
	case H265::GOPType::Fixed:
		return 0;
	case H265::GOPType::Variable:
		return 1;
	}
	throw std::runtime_error("Invalid Parameter");
}

// Slicing
const char* Utility::SliceModeToString(Plugin::AMD::H264::SliceMode v)
{
	switch (v) {
	case H264::SliceMode::Row:
		return "Row";
	case H264::SliceMode::Column:
		return "Column";
	}
	throw std::runtime_error("Invalid Parameter");
}
const char* Utility::SliceControlModeToString(Plugin::AMD::SliceControlMode v)
{
	switch (v) {
	case SliceControlMode::Unknown0:
		return "Unknown 0";
	case SliceControlMode::Unknown1:
		return "Unknown 1";
	case SliceControlMode::Unknown2:
		return "Unknown 2";
	case SliceControlMode::Unknown3:
		return "Unknown 3";
	}
	throw std::runtime_error("Invalid Parameter");
}

Plugin::AMD::ProfileLevel Utility::H264ProfileLevel(std::pair<uint32_t, uint32_t> resolution,
													std::pair<uint32_t, uint32_t> frameRate)
{
	typedef std::pair<uint32_t, uint32_t>             levelRestriction;
	typedef std::pair<ProfileLevel, levelRestriction> level;

	static const level profileLevelLimit[] = {
		// [Level, [Samples, Samples_Per_Sec]]
		level(ProfileLevel::L10, levelRestriction(25344, 380160)),
		level(ProfileLevel::L11, levelRestriction(101376, 768000)),
		level(ProfileLevel::L12, levelRestriction(101376, 1536000)),
		level(ProfileLevel::L13, levelRestriction(101376, 3041280)),
		level(ProfileLevel::L20, levelRestriction(101376, 3041280)),
		level(ProfileLevel::L21, levelRestriction(202752, 5068800)),
		level(ProfileLevel::L22, levelRestriction(414720, 5184000)),
		level(ProfileLevel::L30, levelRestriction(414720, 10368000)),
		level(ProfileLevel::L31, levelRestriction(921600, 27648000)),
		level(ProfileLevel::L32, levelRestriction(1310720, 55296000)),
		//level(H264ProfileLevel::40, levelRestriction(2097152, 62914560)), // Technically identical to 4.1, but backwards compatible.
		level(ProfileLevel::L41, levelRestriction(2097152, 62914560)),
		level(ProfileLevel::L42, levelRestriction(2228224, 133693440)),
		level(ProfileLevel::L50, levelRestriction(5652480, 150994944)),
		level(ProfileLevel::L51, levelRestriction(9437184, 251658240)),
		level(ProfileLevel::L52, levelRestriction(9437184, 530841600)),
		level((ProfileLevel)-1, levelRestriction(0, 0))};

	uint32_t samples     = resolution.first * resolution.second;
	uint32_t samples_sec = (uint32_t)ceil((double_t)samples * ((double_t)frameRate.first / (double_t)frameRate.second));

	level curLevel = profileLevelLimit[0];
	for (uint32_t index = 0; (int32_t)curLevel.first != -1; index++) {
		curLevel = profileLevelLimit[index];

		if (samples > curLevel.second.first)
			continue;

		if (samples_sec > curLevel.second.second)
			continue;

		return curLevel.first;
	}
	return ProfileLevel::L52;
}
Plugin::AMD::ProfileLevel Utility::H265ProfileLevel(std::pair<uint32_t, uint32_t> resolution,
													std::pair<uint32_t, uint32_t> frameRate)
{
	typedef std::pair<uint32_t, uint32_t>             levelRestriction; // Total, Main/Sec, High/Sec
	typedef std::pair<ProfileLevel, levelRestriction> level;

	static const level profileLevelLimit[] = {// [Level, [Samples, Samples_Per_Sec]]
											  level(ProfileLevel::L10, levelRestriction(36864, 552960)),
											  level(ProfileLevel::L20, levelRestriction(122880, 3686400)),
											  level(ProfileLevel::L21, levelRestriction(245760, 7372800)),
											  level(ProfileLevel::L30, levelRestriction(552960, 16588800)),
											  level(ProfileLevel::L31, levelRestriction(983040, 33177600)),
											  level(ProfileLevel::L40, levelRestriction(2228224, 66846720)),
											  level(ProfileLevel::L41, levelRestriction(2228224, 133693440)),
											  level(ProfileLevel::L50, levelRestriction(8912896, 267386880)),
											  level(ProfileLevel::L51, levelRestriction(8912896, 534773760)),
											  level(ProfileLevel::L52, levelRestriction(8912896, 1069547520)),
											  level(ProfileLevel::L60, levelRestriction(35651584, 1069547520)),
											  level(ProfileLevel::L61, levelRestriction(35651584, 2139095040)),
											  level(ProfileLevel::L62, levelRestriction(35651584, 4278190080)),
											  level((ProfileLevel)-1, levelRestriction(0, 0))};

	uint32_t samples     = resolution.first * resolution.second;
	uint32_t samples_sec = (uint32_t)ceil((double_t)samples * ((double_t)frameRate.first / (double_t)frameRate.second));

	level curLevel = profileLevelLimit[0];
	for (uint32_t index = 0; (int32_t)curLevel.first != -1; index++) {
		curLevel = profileLevelLimit[index];

		if (samples > curLevel.second.first)
			continue;

		if (samples_sec > curLevel.second.second)
			continue;

		return curLevel.first;
	}
	return ProfileLevel::L62;
}

//////////////////////////////////////////////////////////////////////////
// Threading Specific
//////////////////////////////////////////////////////////////////////////

#if (defined _WIN32) || (defined _WIN64) // Windows
#include <windows.h>

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO {
	DWORD  dwType;     // Must be 0x1000.
	LPCSTR szName;     // Pointer to name (in user addr space).
	DWORD  dwThreadID; // Thread ID (-1=caller thread).
	DWORD  dwFlags;    // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void Utility::SetThreadName(uint32_t dwThreadID, const char* threadName)
{
	// DWORD dwThreadID = ::GetThreadId( static_cast<HANDLE>( t.native_handle() ) );

	THREADNAME_INFO info;
	info.dwType     = 0x1000;
	info.szName     = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags    = 0;

	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}
}
void Utility::SetThreadName(const char* threadName)
{
	Utility::SetThreadName(GetCurrentThreadId(), threadName);
}
void Utility::SetThreadName(std::thread* pthread, const char* threadName)
{
	DWORD threadId = ::GetThreadId(static_cast<HANDLE>(pthread->native_handle()));
	Utility::SetThreadName(threadId, threadName);
}

#else // Linux, Mac
#include <sys/prctl.h>

void Utility::SetThreadName(std::thread* pthread, const char* threadName)
{
	auto handle = pthread->native_handle();
	pthread_setname_np(handle, threadName);
}
void Utility::SetThreadName(const char* threadName)
{
	prctl(PR_SET_NAME, threadName, 0, 0, 0);
}

#endif
