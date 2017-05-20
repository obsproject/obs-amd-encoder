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

#pragma once
#include "amf.h"
#include "amf-encoder.h"
#include "components/VideoConverter.h"
#include "amf-encoder-h264.h"
#include "components/VideoEncoderVCE.h"
#include "amf-encoder-h265.h"
#include "components/VideoEncoderHEVC.h"

namespace Utility {
	uint64_t GetUniqueIdentifier();
	const char *obs_module_text_multi(const char *val, uint8_t depth = (uint8_t)1);

	// Codec
	const char* CodecToString(Plugin::AMD::Codec v);
	const wchar_t* CodecToAMF(Plugin::AMD::Codec v);

	// Color Format
	const char* ColorFormatToString(Plugin::AMD::ColorFormat v);
	amf::AMF_SURFACE_FORMAT ColorFormatToAMF(Plugin::AMD::ColorFormat v);

	// Color Space
	const char* ColorSpaceToString(Plugin::AMD::ColorSpace v);
	AMF_VIDEO_CONVERTER_COLOR_PROFILE_ENUM ColorSpaceToAMFConverter(Plugin::AMD::ColorSpace v);

	// Usage
	const char* UsageToString(Plugin::AMD::Usage v);
	AMF_VIDEO_ENCODER_USAGE_ENUM UsageToAMFH264(Plugin::AMD::Usage v);
	Plugin::AMD::Usage UsageFromAMFH264(AMF_VIDEO_ENCODER_USAGE_ENUM v);
	AMF_VIDEO_ENCODER_HEVC_USAGE_ENUM UsageToAMFH265(Plugin::AMD::Usage v);
	Plugin::AMD::Usage UsageFromAMFH265(AMF_VIDEO_ENCODER_HEVC_USAGE_ENUM v);

	// Quality Preset
	const char* QualityPresetToString(Plugin::AMD::QualityPreset v);
	AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM QualityPresetToAMFH264(Plugin::AMD::QualityPreset v);
	Plugin::AMD::QualityPreset QualityPresetFromAMFH264(AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM v);
	AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_ENUM QualityPresetToAMFH265(Plugin::AMD::QualityPreset v);
	Plugin::AMD::QualityPreset QualityPresetFromAMFH265(AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_ENUM v);

	// Profile
	const char* ProfileToString(Plugin::AMD::Profile v);
	AMF_VIDEO_ENCODER_PROFILE_ENUM ProfileToAMFH264(Plugin::AMD::Profile v);
	Plugin::AMD::Profile ProfileFromAMFH264(AMF_VIDEO_ENCODER_PROFILE_ENUM v);
	AMF_VIDEO_ENCODER_HEVC_PROFILE_ENUM ProfileToAMFH265(Plugin::AMD::Profile v);
	Plugin::AMD::Profile ProfileFromAMFH265(AMF_VIDEO_ENCODER_HEVC_PROFILE_ENUM v);

	// Tier
	const char* TierToString(Plugin::AMD::H265::Tier v);
	AMF_VIDEO_ENCODER_HEVC_TIER_ENUM TierToAMFH265(Plugin::AMD::H265::Tier v);
	Plugin::AMD::H265::Tier TierFromAMFH265(AMF_VIDEO_ENCODER_HEVC_TIER_ENUM v);

	// Coding Type
	const char* CodingTypeToString(Plugin::AMD::CodingType v);
	AMF_VIDEO_ENCODER_CODING_ENUM CodingTypeToAMFH264(Plugin::AMD::CodingType v);
	Plugin::AMD::CodingType CodingTypeFromAMFH264(AMF_VIDEO_ENCODER_CODING_ENUM v);
	int64_t CodingTypeToAMFH265(Plugin::AMD::CodingType v);
	Plugin::AMD::CodingType CodingTypeFromAMFH265(int64_t v);

	// Rate Control Method
	const char* RateControlMethodToString(Plugin::AMD::RateControlMethod v);
	AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM RateControlMethodToAMFH264(Plugin::AMD::RateControlMethod v);
	Plugin::AMD::RateControlMethod RateControlMethodFromAMFH264(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM v);
	AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_ENUM RateControlMethodToAMFH265(Plugin::AMD::RateControlMethod v);
	Plugin::AMD::RateControlMethod RateControlMethodFromAMFH265(AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_ENUM v);

	// Pre-Pass Method
	const char* PrePassModeToString(Plugin::AMD::PrePassMode v);
	AMF_VIDEO_ENCODER_PREENCODE_MODE_ENUM PrePassModeToAMFH264(Plugin::AMD::PrePassMode v);
	Plugin::AMD::PrePassMode PrePassModeFromAMFH264(AMF_VIDEO_ENCODER_PREENCODE_MODE_ENUM v);

	// GOP Type
	const char* GOPTypeToString(Plugin::AMD::H265::GOPType v);
	Plugin::AMD::H265::GOPType GOPTypeFromAMFH265(int64_t v);
	int64_t GOPTypeToAMFH265(Plugin::AMD::H265::GOPType v);

	// Slicing
	const char* SliceModeToString(Plugin::AMD::H264::SliceMode v);
	const char* SliceControlModeToString(Plugin::AMD::SliceControlMode v);

	Plugin::AMD::ProfileLevel H264ProfileLevel(std::pair<uint32_t, uint32_t> resolution, std::pair<uint32_t, uint32_t> frameRate);
	Plugin::AMD::ProfileLevel H265ProfileLevel(std::pair<uint32_t, uint32_t> resolution, std::pair<uint32_t, uint32_t> frameRate);

	//////////////////////////////////////////////////////////////////////////
	// Threading Specific
	//////////////////////////////////////////////////////////////////////////

	#if (defined _WIN32) || (defined _WIN64)
	void SetThreadName(uint32_t dwThreadID, const char* threadName);
	void SetThreadName(const char* threadName);
	void SetThreadName(std::thread* pthread, const char* threadName);
	#else
	void SetThreadName(std::thread* pthread, const char* threadName);
	void SetThreadName(const char* threadName);
	#endif
}