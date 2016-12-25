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
#include "amf-capabilities.h"
#include "amf-h264.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin::AMD;

namespace Plugin {
	namespace Utility {
		H264ProfileLevel inline GetMinimumProfileLevel(std::pair<uint32_t, uint32_t> frameSize, std::pair<uint32_t, uint32_t> frameRate) {
			typedef std::pair<uint32_t, uint32_t> levelRestriction;
			typedef std::pair<H264ProfileLevel, levelRestriction> level;

			static const level profileLevelLimit[] = { // [Level, [Samples, Samples_Per_Sec]]
				level(H264ProfileLevel::L10, levelRestriction(25344, 380160)),
				level(H264ProfileLevel::L11, levelRestriction(101376, 768000)),
				level(H264ProfileLevel::L12, levelRestriction(101376, 1536000)),
				level(H264ProfileLevel::L13, levelRestriction(101376, 3041280)),
				level(H264ProfileLevel::L20, levelRestriction(101376, 3041280)),
				level(H264ProfileLevel::L21, levelRestriction(202752, 5068800)),
				level(H264ProfileLevel::L22, levelRestriction(414720, 5184000)),
				level(H264ProfileLevel::L30, levelRestriction(414720, 10368000)),
				level(H264ProfileLevel::L31, levelRestriction(921600, 27648000)),
				level(H264ProfileLevel::L32, levelRestriction(1310720, 55296000)),
				//level(H264ProfileLevel::40, levelRestriction(2097152, 62914560)), // Technically identical to 4.1, but backwards compatible.
				level(H264ProfileLevel::L41, levelRestriction(2097152, 62914560)),
				level(H264ProfileLevel::L42, levelRestriction(2228224, 133693440)),
				level(H264ProfileLevel::L50, levelRestriction(5652480, 150994944)),
				level(H264ProfileLevel::L51, levelRestriction(9437184, 251658240)),
				level(H264ProfileLevel::L52, levelRestriction(9437184, 530841600)),
				level((H264ProfileLevel)-1, levelRestriction(0, 0))
			};

			uint32_t samples = frameSize.first * frameSize.second;
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
			return H264ProfileLevel::L52;
		}

		#pragma region VCEEncoderType
		inline const char* VCEEncoderTypeAsString(H264EncoderType type) {
			const char* types[] = {
				"AVC",
				"SVC",
				"HEVC"
			};
			return types[(uint8_t)type];
		}
		inline const wchar_t* VCEEncoderTypeAsAMF(H264EncoderType type) {
			const wchar_t* types[] = {
				AMFVideoEncoderVCE_AVC,
				AMFVideoEncoderVCE_SVC,
				L"AMFVideoEncoderHW_HEVC"
			};
			return types[(uint8_t)type];
		}
		#pragma endregion VCEEncoderType
		#pragma region VCEMemoryType
		inline const char* MemoryTypeAsString(H264MemoryType memoryType) {
			static const char* memoryTypeToString[] = {
				"Host",
				"DirectX9",
				"DirectX11",
				"OpenGL"
			};
			return memoryTypeToString[(uint8_t)memoryType];
		}
		inline amf::AMF_MEMORY_TYPE MemoryTypeAsAMF(H264MemoryType memoryType) {
			static amf::AMF_MEMORY_TYPE memoryTypeToAMF[] = {
				amf::AMF_MEMORY_HOST,
				amf::AMF_MEMORY_DX9,
				amf::AMF_MEMORY_DX11,
				amf::AMF_MEMORY_OPENGL,
			};
			return memoryTypeToAMF[(uint8_t)memoryType];
		}
		#pragma endregion VCEMemoryType
		#pragma region VCESurfaceFormat
		inline const char* SurfaceFormatAsString(H264ColorFormat surfaceFormat) {
			static const char* surfaceFormatToString[] = {
				"NV12",
				"I420",
				"YUY2",
				"BGRA",
				"RGBA",
				"GRAY",
			};
			return surfaceFormatToString[(uint8_t)surfaceFormat];
		}
		inline amf::AMF_SURFACE_FORMAT SurfaceFormatAsAMF(H264ColorFormat surfaceFormat) {
			static amf::AMF_SURFACE_FORMAT surfaceFormatToAMF[] = {
				// 4:2:0 Formats
				amf::AMF_SURFACE_NV12,
				amf::AMF_SURFACE_YUV420P,
				// 4:2:2 Formats
				amf::AMF_SURFACE_YUY2,
				// Uncompressed
				amf::AMF_SURFACE_BGRA,
				amf::AMF_SURFACE_RGBA,
				// Other
				amf::AMF_SURFACE_GRAY8,
			};
			return surfaceFormatToAMF[(uint8_t)surfaceFormat];
		}
		#pragma endregion VCESurfaceFormat
		#pragma region VCEUsage
		inline const char* UsageAsString(H264Usage usage) {
			static const char* usageToString[] = {
				"Transcoding",
				"Ultra Low Latency",
				"Low Latency",
				"Webcam"
			};
			return usageToString[(uint8_t)usage];
		}
		inline AMF_VIDEO_ENCODER_USAGE_ENUM UsageAsAMF(H264Usage usage) {
			static AMF_VIDEO_ENCODER_USAGE_ENUM usageToAMF[] = {
				AMF_VIDEO_ENCODER_USAGE_TRANSCONDING,
				AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY,
				AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY,
				AMF_VIDEO_ENCODER_USAGE_WEBCAM,
			};
			return usageToAMF[(uint8_t)usage];
		}
		inline H264Usage UsageFromAMF(uint32_t usage) {
			static H264Usage usageFromAMF[] = {
				H264Usage::Transcoding,
				H264Usage::UltraLowLatency,
				H264Usage::LowLatency,
				H264Usage::Webcam,
			};
			return usageFromAMF[(uint8_t)usage];
		}
		#pragma endregion VCEUsage
		#pragma region VCEQualityPreset
		inline const char* QualityPresetAsString(H264QualityPreset preset) {
			static const char* qualityPresetToString[] = {
				"Speed",
				"Balanced",
				"Quality"
			};
			return qualityPresetToString[(uint8_t)preset];
		}
		#pragma endregion VCEQualityPreset
		#pragma region VCEProfile
		inline const char* ProfileAsString(H264Profile profile) {
			switch (profile) {
				case H264Profile::Baseline:
					return "Baseline";
				case H264Profile::Main:
					return "Main";
				case H264Profile::High:
					return "High";
				case H264Profile::ConstrainedBaseline:
					return "Constrained Baseline";
				case H264Profile::ConstrainedHigh:
					return "Constrained High";
			}

			return "Invalid";
		}
		#pragma endregion VCEProfile
		#pragma region VCERateControlMethod
		inline const char* RateControlMethodAsString(H264RateControlMethod method) {
			static const char* rateControlMethodToString[] = {
				"Constant Quantization Parameter (CQP)",
				"Constant Bitrate (CBR)",
				"Peak Constrained Variable Bitrate (VBR)",
				"Latency Constrained Variable Bitrate (VBR_LAT)"
			};
			return rateControlMethodToString[(uint8_t)method];
		}
		inline AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM RateControlMethodAsAMF(H264RateControlMethod method) {
			static AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM CustomToAMF[] = {
				AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP,
				AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR,
				AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR,
				AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR,
			};
			return CustomToAMF[(uint8_t)method];
		}
		inline H264RateControlMethod RateControlMethodFromAMF(uint32_t method) {
			static H264RateControlMethod AMFToCustom[] = {
				H264RateControlMethod::ConstantQP,
				H264RateControlMethod::ConstantBitrate,
				H264RateControlMethod::VariableBitrate_PeakConstrained,
				H264RateControlMethod::VariableBitrate_LatencyConstrained,
			};
			return AMFToCustom[(uint8_t)method];
		}
		#pragma endregion VCERateControlMethod

		inline const char* CodingTypeAsString(H264CodingType type) {
			switch (type) {
				case H264CodingType::CABAC:
					return "CABAC";
				case H264CodingType::CALVC:
					return "CALVC";
				case H264CodingType::Default:
					return "Default";
			}
			return "MEMORY CORRUPTION";
		}
		inline const char* SliceModeAsString(H264SliceMode mode) {
			switch (mode) {
				case H264SliceMode::Horizontal:
					return "Horizontal";
				case H264SliceMode::Vertical:
					return "Vertical";
			}
			return "MEMORY CORRUPTION";
		}
		inline const char* SliceControlModeAsString(H264SliceControlMode mode) {
			switch (mode) {
				case H264SliceControlMode::Off:
					return "Off";
				case H264SliceControlMode::Macroblock:
					return "Macroblock";
				case H264SliceControlMode::Macroblock_Row:
					return "Macroblock Row";
			}
			return "MEMORY CORRUPTION";
		}
	}
}