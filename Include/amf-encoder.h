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

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "plugin.h"
#include "amf.h"
#include "api-base.h"

#include "components/VideoConverter.h"
#include "components/VideoEncoderVCE.h"
#include "components/VideoEncoderHEVC.h"

#define AMF_PRESENT_TIMESTAMP L"PTS"
#define AMF_SUBMIT_TIMESTAMP L"STS"

namespace Plugin {
	namespace AMD {
		// Initialization Parameters
		enum class Codec : uint8_t {
			H264AVC,
			H264SVC,
			HEVC,
		};
		enum class ColorFormat : uint8_t {
			/*	Support Table
			 *	Open Broadcaster	AMD AMF
			 *	-- 4:2:0 Formats --
			 *	VIDEO_FORMAT_I420	AMF_SURFACE_YUV420P
			 *	VIDEO_FORMAT_NV12	AMF_SURFACE_NV12
			 *						AMF_SURFACE_YV12
			 *	-- 4:2:2 Formats --
			 *	VIDEO_FORMAT_YVYV
			 *	VIDEO_FORMAT_YUY2	AMF_SURFACE_YUY2
			 *	VIDEO_FORMAT_UYVY
			 *
			 *	-- 4:4:4 Formats --
			 *	VIDEO_FORMAT_I444
			 *
			 *	-- Packed Uncompressed Formats --
			 *	VIDEO_FORMAT_RGBA	AMF_SURFACE_RGBA
			 *	VIDEO_FORMAT_BGRA	AMF_SURFACE_BGRA
			 *						AMF_SURFACE_ARGB
			 *	VIDEO_FORMAT_BGRX
			 *
			 *	-- Single/Dual Channel Formats --
			 *	VIDEO_FORMAT_Y800	AMF_SURFACE_GRAY8
			 *						AMF_SURFACE_U8V8
			 *
			 *	-- HDR Color Formats --
			 *						AMF_SURFACE_P010
			 *						AMF_SURFACE_RGBA_F16
			 */

			I420,
			NV12,
			YUY2,
			BGRA,
			RGBA,
			GRAY,
		};
		enum class ColorSpace : uint8_t {
			BT609,
			BT701,
			BT2020,
		};

		// Properties
		enum class Usage : uint8_t {
			Transcoding,
			UltraLowLatency,
			LowLatency,
			Webcam
		};
		enum class QualityPreset : uint8_t {
			Speed,
			Balanced,
			Quality,
		};
		enum class Profile : uint16_t {
			ConstrainedBaseline,
			Baseline,
			Main,
			ConstrainedHigh,
			High,
		};
		enum class ProfileLevel : uint8_t {
			Automatic,
			L10 = 10,
			L11,
			L12,
			L13,
			L20 = 20,
			L21,
			L22,
			L30 = 30,
			L31,
			L32,
			L40 = 40,
			L41,
			L42,
			L50 = 50,
			L51,
			L52,
			L60 = 60,
			L61,
			L62,
		};
		enum class CodingType : uint8_t {
			Automatic,
			CALVC,
			CABAC,
		};
		enum class RateControlMethod : uint8_t {
			ConstantQP,
			LatencyConstrainedVariableBitrate,
			PeakConstrainedVariableBitrate,
			ConstantBitrate,
		};
		enum class PrePassMode : uint8_t {
			Disabled,
			Enabled,
			EnabledAtHalfScale,
			EnabledAtQuarterScale,
		};

		class Encoder {
			protected:
			Encoder(Codec codec,
				std::shared_ptr<API::Base> videoAPI, API::Adapter videoAdapter, bool useOpenCL,
				ColorFormat colorFormat, ColorSpace colorSpace, bool fullRangeColor);
			public:
			virtual ~Encoder();

			public:

			// Properties - Initialization
			virtual std::vector<Usage> CapsUsage() = 0;
			virtual void SetUsage(Usage v) = 0;
			virtual Usage GetUsage() = 0;

			// Properties - Static
			virtual std::vector<QualityPreset> CapsQualityPreset() = 0;
			virtual void SetQualityPreset(QualityPreset v) = 0;
			virtual QualityPreset GetQualityPreset() = 0;

			virtual std::vector<Profile> CapsProfile() = 0;
			virtual void SetProfile(Profile v) = 0;
			virtual Profile GetProfile() = 0;

			virtual std::vector<ProfileLevel> CapsProfileLevel() = 0;
			virtual void SetProfileLevel(ProfileLevel v) = 0;
			virtual ProfileLevel GetProfileLevel() = 0;
			
			virtual std::pair<uint64_t, uint64_t> CapsMaximumReferenceFrames() = 0;
			virtual void SetMaximumReferenceFrames(uint64_t v) = 0;
			virtual uint64_t GetMaximumReferenceFrames() = 0;

			virtual std::pair<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t>> CapsResolution() = 0;
			virtual void SetResolution(std::pair<uint32_t, uint32_t> v) = 0;
			virtual std::pair<uint32_t, uint32_t> GetResolution() = 0;

			virtual void SetAspectRatio(std::pair<uint32_t, uint32_t> v) = 0;
			virtual std::pair<uint32_t, uint32_t> GetAspectRatio() = 0;

			virtual void SetFrameRate(std::pair<uint32_t, uint32_t> v) = 0;
			virtual std::pair<uint32_t, uint32_t> GetFrameRate() = 0;
			protected:
			void UpdateFrameRateValues();
			public:

			virtual std::vector<CodingType> CapsCodingType() = 0;
			virtual void SetCodingType(CodingType v) = 0;
			virtual CodingType GetCodingType() = 0;
			
			// Properties - Dynamic
			virtual std::vector<RateControlMethod> CapsRateControlMethod() = 0;
			virtual void SetRateControlMethod(RateControlMethod v) = 0;
			virtual RateControlMethod GetRateControlMethod() = 0;

			virtual std::vector<PrePassMode> CapsPrePassMode() = 0;
			virtual void SetPrePassMode(PrePassMode v) = 0;
			virtual PrePassMode GetPrePassMode() = 0;

			virtual void SetVariableBitrateAverageQualityEnabled(bool v) = 0;
			virtual bool IsVariableBitrateAverageQualityEnabled() = 0;

			virtual void SetFrameSkippingEnabled(bool v) = 0;
			virtual bool IsFrameSkippingEnabled() = 0;

			/// Enforce Hypothethical Reference Decoder Restrictions
			virtual void SetEnforceHRDEnabled(bool v) = 0;
			virtual bool IsEnforceHRDEnabled() = 0;

			virtual std::pair<uint64_t, uint64_t> CapsTargetBitrate() = 0;
			virtual void SetTargetBitrate(uint64_t v) = 0;
			virtual uint64_t GetTargetBitrate() = 0;

			virtual std::pair<uint64_t, uint64_t> CapsPeakBitrate() = 0;
			virtual void SetPeakBitrate(uint64_t v) = 0;
			virtual uint64_t GetPeakBitrate() = 0;

			virtual void SetIFrameQP(uint8_t v) = 0;
			virtual uint8_t GetIFrameQP() = 0;

			virtual void SetPFrameQP(uint8_t v) = 0;
			virtual uint8_t GetPFrameQP() = 0;

			virtual std::pair<uint64_t, uint64_t> CapsVBVBufferSize() = 0;
			virtual void SetVBVBufferSize(uint64_t v) = 0;
			virtual uint64_t GetVBVBufferSize() = 0;

			virtual void SetVBVBufferInitialFullness(float v) = 0;
			virtual float GetInitialVBVBufferFullness() = 0;

			// Properties - Picture Control
			virtual void SetIDRPeriod(uint64_t v) = 0;
			virtual uint64_t GetIDRPeriod() = 0;
			// Perhaps add utility function for Keyframe Interval? Simplifies stuff.
			// Header Insertion Spacing?

			virtual void SetDeblockingFilterEnabled(bool v) = 0;
			virtual bool IsDeblockingFilterEnabled() = 0;

			// Properties - Motion Estimation
			virtual void SetMotionEstimationQuarterPixelEnabled(bool v) = 0;
			virtual bool GetMotionEstimationQuarterPixelEnabled() = 0;
			virtual void SetMotionEstimationHalfPixelEnabled(bool v) = 0;
			virtual bool GetMotionEstimationHalfPixelEnabled() = 0;

			// Unknown:
			// Intra-Refresh stuff
			// Slicing stuff
			// ConstraintSetFlags (H264 only?)

			// Control
			void Start();
			void Restart();
			void Stop();

			bool Encode(struct encoder_frame* f, struct encoder_packet* p, bool* b);

			protected:
			uint64_t m_UniqueId;

			// AMF Internals
			Plugin::AMD::AMF* m_AMF;
			amf::AMFFactory* m_AMFFactory;
			amf::AMFContextPtr m_AMFContext;
			amf::AMFComputePtr m_AMFCompute;
			amf::AMFComponentPtr m_AMFEncoder;
			amf::AMFComponentPtr m_AMFConverter;
			amf::AMF_MEMORY_TYPE m_AMFMemoryType;
			amf::AMF_SURFACE_FORMAT m_AMFSurfaceFormat;
			std::vector<uint8_t> m_PacketDataBuffer;

			// API Related
			std::shared_ptr<API::Base> m_API;
			API::Adapter m_APIAdapter;
			void* m_APIDevice;

			// Properties
			Codec m_Codec;
			ColorFormat m_ColorFormat;
			ColorSpace m_ColorSpace;
			bool m_FullColorRange;

			std::pair<uint32_t, uint32_t> m_Resolution;
			std::pair<uint32_t, uint32_t> m_FrameRate;
			uint64_t m_FrameRateTimeStepAMF;
			double_t m_FrameRateTimeStep;

			// Flags
			bool m_Started;
			bool m_OpenCLSubmission;
			bool m_OpenCLConversion;

		};
	}
}
