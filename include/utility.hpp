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
#include "amf-encoder-h264.hpp"
#include "amf-encoder-h265.hpp"
#include "amf-encoder.hpp"
#include "amf.hpp"

#include "components/VideoConverter.h"
#include "components/VideoEncoderHEVC.h"
#include "components/VideoEncoderVCE.h"

#ifndef LITE_OBS
extern "C" {
#include "obs-properties.h"
}
#endif

namespace Utility {
	uint64_t    GetUniqueIdentifier();
	const char* obs_module_text_multi(const char* val, uint8_t depth = (uint8_t)1);

#ifndef LITE_OBS
	void fill_api_list(obs_property_t* property, Plugin::AMD::Codec codec);
	void fill_device_list(obs_property_t* property, std::string api_name, Plugin::AMD::Codec codec);
#endif

	// Codec
	const char*    CodecToString(Plugin::AMD::Codec v);
	const wchar_t* CodecToAMF(Plugin::AMD::Codec v);

	// Color Format
	const char*             ColorFormatToString(Plugin::AMD::ColorFormat v);
	amf::AMF_SURFACE_FORMAT ColorFormatToAMF(Plugin::AMD::ColorFormat v);

	// Color Space
	const char*                            ColorSpaceToString(Plugin::AMD::ColorSpace v);
	AMF_VIDEO_CONVERTER_COLOR_PROFILE_ENUM ColorSpaceToAMFConverter(Plugin::AMD::ColorSpace v);
	AMF_COLOR_TRANSFER_CHARACTERISTIC_ENUM ColorSpaceToTransferCharacteristic(Plugin::AMD::ColorSpace v);

	// Usage
	const char*                       UsageToString(Plugin::AMD::Usage v);
	AMF_VIDEO_ENCODER_USAGE_ENUM      UsageToAMFH264(Plugin::AMD::Usage v);
	Plugin::AMD::Usage                UsageFromAMFH264(AMF_VIDEO_ENCODER_USAGE_ENUM v);
	AMF_VIDEO_ENCODER_HEVC_USAGE_ENUM UsageToAMFH265(Plugin::AMD::Usage v);
	Plugin::AMD::Usage                UsageFromAMFH265(AMF_VIDEO_ENCODER_HEVC_USAGE_ENUM v);

	// Quality Preset
	const char*                                QualityPresetToString(Plugin::AMD::QualityPreset v);
	AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM      QualityPresetToAMFH264(Plugin::AMD::QualityPreset v);
	Plugin::AMD::QualityPreset                 QualityPresetFromAMFH264(AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM v);
	AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_ENUM QualityPresetToAMFH265(Plugin::AMD::QualityPreset v);
	Plugin::AMD::QualityPreset                 QualityPresetFromAMFH265(AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_ENUM v);

	// Profile
	const char*                         ProfileToString(Plugin::AMD::Profile v);
	AMF_VIDEO_ENCODER_PROFILE_ENUM      ProfileToAMFH264(Plugin::AMD::Profile v);
	Plugin::AMD::Profile                ProfileFromAMFH264(AMF_VIDEO_ENCODER_PROFILE_ENUM v);
	AMF_VIDEO_ENCODER_HEVC_PROFILE_ENUM ProfileToAMFH265(Plugin::AMD::Profile v);
	Plugin::AMD::Profile                ProfileFromAMFH265(AMF_VIDEO_ENCODER_HEVC_PROFILE_ENUM v);

	// Tier
	const char*                      TierToString(Plugin::AMD::H265::Tier v);
	AMF_VIDEO_ENCODER_HEVC_TIER_ENUM TierToAMFH265(Plugin::AMD::H265::Tier v);
	Plugin::AMD::H265::Tier          TierFromAMFH265(AMF_VIDEO_ENCODER_HEVC_TIER_ENUM v);

	// Coding Type
	const char*                   CodingTypeToString(Plugin::AMD::CodingType v);
	AMF_VIDEO_ENCODER_CODING_ENUM CodingTypeToAMFH264(Plugin::AMD::CodingType v);
	Plugin::AMD::CodingType       CodingTypeFromAMFH264(AMF_VIDEO_ENCODER_CODING_ENUM v);
	int64_t                       CodingTypeToAMFH265(Plugin::AMD::CodingType v);
	Plugin::AMD::CodingType       CodingTypeFromAMFH265(int64_t v);

	// Rate Control Method
	const char*                                RateControlMethodToString(Plugin::AMD::RateControlMethod v);
	AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM RateControlMethodToAMFH264(Plugin::AMD::RateControlMethod v);
	Plugin::AMD::RateControlMethod RateControlMethodFromAMFH264(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM v);
	AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_ENUM RateControlMethodToAMFH265(Plugin::AMD::RateControlMethod v);
	Plugin::AMD::RateControlMethod RateControlMethodFromAMFH265(AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_ENUM v);

	// Pre-Pass Method
	const char*                           PrePassModeToString(Plugin::AMD::PrePassMode v);
	AMF_VIDEO_ENCODER_PREENCODE_MODE_ENUM PrePassModeToAMFH264(Plugin::AMD::PrePassMode v);
	Plugin::AMD::PrePassMode              PrePassModeFromAMFH264(AMF_VIDEO_ENCODER_PREENCODE_MODE_ENUM v);

	// GOP Type
	const char*                GOPTypeToString(Plugin::AMD::H265::GOPType v);
	Plugin::AMD::H265::GOPType GOPTypeFromAMFH265(int64_t v);
	int64_t                    GOPTypeToAMFH265(Plugin::AMD::H265::GOPType v);

	// Slicing
	const char* SliceModeToString(Plugin::AMD::H264::SliceMode v);
	const char* SliceControlModeToString(Plugin::AMD::SliceControlMode v);

	Plugin::AMD::ProfileLevel H264ProfileLevel(std::pair<uint32_t, uint32_t> resolution,
											   std::pair<uint32_t, uint32_t> frameRate);
	Plugin::AMD::ProfileLevel H265ProfileLevel(std::pair<uint32_t, uint32_t> resolution,
											   std::pair<uint32_t, uint32_t> frameRate);

	//////////////////////////////////////////////////////////////////////////
	// Threading Specific
	//////////////////////////////////////////////////////////////////////////
#if (defined _WIN32) || (defined _WIN64)
	void SetThreadName(uint32_t dwThreadID, const char* threadName);
#endif
	void SetThreadName(std::thread* pthread, const char* threadName);
	void SetThreadName(const char* threadName);
} // namespace Utility
