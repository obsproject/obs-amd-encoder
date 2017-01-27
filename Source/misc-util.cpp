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

// Plugin
#include "plugin.h"
#include "amf.h"
#include "amf-encoder.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin::AMD;

namespace Utility {
	static uint64_t GetUniqueIdentifier() {
		static std::mutex __mutex;
		static uint64_t __curId;

		const std::lock_guard<std::mutex> lock(__mutex);
		return ++__curId;
	}

	// Codec
	inline const char* CodecToString(Plugin::AMD::Codec v) {
		switch (v) {
			case Codec::H264AVC:
				return "H264/AVC";
			case Codec::H264SVC:
				return "H264/SVC";
			case Codec::HEVC:
				return "H265/HEVC";
		}
		throw std::runtime_error("Invalid Parameter");
	}
	inline const wchar_t* CodecToAMF(Plugin::AMD::Codec v) {
		switch (v) {
			case Codec::H264AVC:
				return AMFVideoEncoderVCE_AVC;
			case Codec::H264SVC:
				return AMFVideoEncoderVCE_SVC;
			case Codec::HEVC:
				return AMFVideoEncoder_HEVC;
		}
		throw std::runtime_error("Invalid Parameter");
	}

	// Color Format
	inline const char* ColorFormatToString(Plugin::AMD::ColorFormat v) {
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
	inline amf::AMF_SURFACE_FORMAT ColorFormatToAMF(Plugin::AMD::ColorFormat v) {
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
	inline const char* ColorSpaceToString(Plugin::AMD::ColorSpace v) {
		switch (v) {
			case ColorSpace::BT609:
				return "609";
			case ColorSpace::BT701:
				return "701";
			case ColorSpace::BT2020:
				return "2020";
		}
		throw std::runtime_error("Invalid Parameter");
	}
	inline AMF_VIDEO_CONVERTER_COLOR_PROFILE_ENUM ColorSpaceToAMFConverter(Plugin::AMD::ColorSpace v) {
		switch (v) {
			case ColorSpace::BT609:
				return AMF_VIDEO_CONVERTER_COLOR_PROFILE_601;
			case ColorSpace::BT701:
				return AMF_VIDEO_CONVERTER_COLOR_PROFILE_709;
			case ColorSpace::BT2020:
				return AMF_VIDEO_CONVERTER_COLOR_PROFILE_2020;
		}
		throw std::runtime_error("Invalid Parameter");
	}
	
	// Usage
	inline const char* UsageToString(Plugin::AMD::Usage v) {
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
	inline AMF_VIDEO_ENCODER_USAGE_ENUM UsageToAMFH264(Plugin::AMD::Usage v) {
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
	inline Plugin::AMD::Usage UsageFromAMFH264(AMF_VIDEO_ENCODER_USAGE_ENUM v) {
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
	inline AMF_VIDEO_ENCODER_HEVC_USAGE_ENUM UsageToAMFH265(Plugin::AMD::Usage v) {
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

	// Quality Preset
	inline const char* QualityPresetToString(Plugin::AMD::QualityPreset v) {
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
	inline AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM QualityPresetToAMFH264(Plugin::AMD::QualityPreset v) {
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
	inline Plugin::AMD::QualityPreset QualityPresetFromAMFH264(AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM v) {
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
	inline AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_ENUM QualityPresetToAMFH265(Plugin::AMD::QualityPreset v) {
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
	inline Plugin::AMD::QualityPreset QualityPresetFromAMFH265(AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_ENUM v) {
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
	inline const char* ProfileToString(Plugin::AMD::Profile v) {
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
	inline AMF_VIDEO_ENCODER_PROFILE_ENUM ProfileToAMFH264(Plugin::AMD::Profile v) {
		switch (v) {
			case Profile::ConstrainedBaseline:
				return (AMF_VIDEO_ENCODER_PROFILE_ENUM)0;
			case Profile::Baseline:
				return AMF_VIDEO_ENCODER_PROFILE_BASELINE;
			case Profile::Main:
				return AMF_VIDEO_ENCODER_PROFILE_MAIN;
			case Profile::ConstrainedHigh:
				return (AMF_VIDEO_ENCODER_PROFILE_ENUM)1;
			case Profile::High:
				return AMF_VIDEO_ENCODER_PROFILE_HIGH;
		}
		throw std::runtime_error("Invalid Parameter");
	}
	inline Plugin::AMD::Profile ProfileFromAMFH264(AMF_VIDEO_ENCODER_PROFILE_ENUM v) {
		switch (v) {
			case 0:
				return Profile::ConstrainedBaseline;
			case AMF_VIDEO_ENCODER_PROFILE_BASELINE:
				return Profile::Baseline;
			case AMF_VIDEO_ENCODER_PROFILE_MAIN:
				return Profile::Main;
			case 1:
				return Profile::ConstrainedHigh;
			case AMF_VIDEO_ENCODER_PROFILE_HIGH:
				return Profile::High;
		}
		throw std::runtime_error("Invalid Parameter");
	}
	inline AMF_VIDEO_ENCODER_HEVC_PROFILE_ENUM ProfileToAMFH265(Plugin::AMD::Profile v) {
		throw std::logic_error("The method or operation is not implemented.");
	}
	inline Plugin::AMD::Profile ProfileFromAMFH265(AMF_VIDEO_ENCODER_HEVC_PROFILE_ENUM v) {
		throw std::logic_error("The method or operation is not implemented.");
	}

	// Coding Type
	inline const char* CodingTypeToString(Plugin::AMD::CodingType v) {
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
	inline AMF_VIDEO_ENCODER_CODING_ENUM CodingTypeToAMFH264(Plugin::AMD::CodingType v) {
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
	inline Plugin::AMD::CodingType CodingTypeFromAMFH264(AMF_VIDEO_ENCODER_CODING_ENUM v) {
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
	inline int64_t CodingTypeToAMFH265(Plugin::AMD::CodingType v) {
		switch (v) {
			case CodingType::Automatic:
				return 0;
			case CodingType::CABAC:
				return 1;
		}
		throw std::runtime_error("Invalid Parameter");
	}
	inline Plugin::AMD::CodingType CodingTypeFromAMFH265(int64_t v) {
		switch (v) {
			case 0:
				return CodingType::Automatic;
			case 1:
				return CodingType::CABAC;
		}
		throw std::runtime_error("Invalid Parameter");
	}

	// Rate Control Method
	inline const char* RateControlMethodToString(Plugin::AMD::RateControlMethod v) {
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
	inline AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM RateControlMethodToAMFH264(Plugin::AMD::RateControlMethod v) {
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
	inline Plugin::AMD::RateControlMethod RateControlMethodFromAMFH264(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM v) {
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

	// Pre-Pass Method
	inline const char* PrePassModeToString(Plugin::AMD::PrePassMode v) {
		switch (v) {
			case PrePassMode::Disabled:
				return "Disabled";
			case PrePassMode::Enabled:
				return "Enabled (Full Scale)";
			case PrePassMode::EnabledAtHalfScale:
				return "Enabled (Half Scale)";
			case PrePassMode::EnabledAtQuarterScale:
				return "Enabled (Quarter Scale)";
		}
		throw std::runtime_error("Invalid Parameter");
	}
	inline AMF_VIDEO_ENCODER_PREENCODE_MODE_ENUM PrePassModeToAMFH264(Plugin::AMD::PrePassMode v) {
		switch (v) {
			case PrePassMode::Disabled:
				return AMF_VIDEO_ENCODER_PREENCODE_DISABLED;
			case PrePassMode::Enabled:
				return AMF_VIDEO_ENCODER_PREENCODE_ENABLED;
			case PrePassMode::EnabledAtHalfScale:
				return AMF_VIDEO_ENCODER_PREENCODE_ENABLED_DOWNSCALEFACTOR_2;
			case PrePassMode::EnabledAtQuarterScale:
				return AMF_VIDEO_ENCODER_PREENCODE_ENABLED_DOWNSCALEFACTOR_4;
		}
		throw std::runtime_error("Invalid Parameter");
	}
	inline Plugin::AMD::PrePassMode PrePassModeFromAMFH264(AMF_VIDEO_ENCODER_PREENCODE_MODE_ENUM v) {
		switch (v) {
			case AMF_VIDEO_ENCODER_PREENCODE_DISABLED:
				return PrePassMode::Disabled;
			case AMF_VIDEO_ENCODER_PREENCODE_ENABLED:
				return PrePassMode::Enabled;
			case AMF_VIDEO_ENCODER_PREENCODE_ENABLED_DOWNSCALEFACTOR_2:
				return PrePassMode::EnabledAtHalfScale;
			case AMF_VIDEO_ENCODER_PREENCODE_ENABLED_DOWNSCALEFACTOR_4:
				return PrePassMode::EnabledAtQuarterScale;
		}
		throw std::runtime_error("Invalid Parameter");
	}



	//H264ProfileLevel inline GetMinimumProfileLevel(std::pair<uint32_t, uint32_t> frameSize, std::pair<uint32_t, uint32_t> frameRate) {
	//	typedef std::pair<uint32_t, uint32_t> levelRestriction;
	//	typedef std::pair<H264ProfileLevel, levelRestriction> level;

	//	static const level profileLevelLimit[] = { // [Level, [Samples, Samples_Per_Sec]]
	//		level(H264ProfileLevel::L10, levelRestriction(25344, 380160)),
	//		level(H264ProfileLevel::L11, levelRestriction(101376, 768000)),
	//		level(H264ProfileLevel::L12, levelRestriction(101376, 1536000)),
	//		level(H264ProfileLevel::L13, levelRestriction(101376, 3041280)),
	//		level(H264ProfileLevel::L20, levelRestriction(101376, 3041280)),
	//		level(H264ProfileLevel::L21, levelRestriction(202752, 5068800)),
	//		level(H264ProfileLevel::L22, levelRestriction(414720, 5184000)),
	//		level(H264ProfileLevel::L30, levelRestriction(414720, 10368000)),
	//		level(H264ProfileLevel::L31, levelRestriction(921600, 27648000)),
	//		level(H264ProfileLevel::L32, levelRestriction(1310720, 55296000)),
	//		//level(H264ProfileLevel::40, levelRestriction(2097152, 62914560)), // Technically identical to 4.1, but backwards compatible.
	//		level(H264ProfileLevel::L41, levelRestriction(2097152, 62914560)),
	//		level(H264ProfileLevel::L42, levelRestriction(2228224, 133693440)),
	//		level(H264ProfileLevel::L50, levelRestriction(5652480, 150994944)),
	//		level(H264ProfileLevel::L51, levelRestriction(9437184, 251658240)),
	//		level(H264ProfileLevel::L52, levelRestriction(9437184, 530841600)),
	//		level((H264ProfileLevel)-1, levelRestriction(0, 0))
	//	};

	//	uint32_t samples = frameSize.first * frameSize.second;
	//	uint32_t samples_sec = (uint32_t)ceil((double_t)samples * ((double_t)frameRate.first / (double_t)frameRate.second));

	//	level curLevel = profileLevelLimit[0];
	//	for (uint32_t index = 0; (int32_t)curLevel.first != -1; index++) {
	//		curLevel = profileLevelLimit[index];

	//		if (samples > curLevel.second.first)
	//			continue;

	//		if (samples_sec > curLevel.second.second)
	//			continue;

	//		return curLevel.first;
	//	}
	//	return H264ProfileLevel::L52;
	//}
}