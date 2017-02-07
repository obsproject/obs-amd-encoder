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

#include "plugin.h"
#include "amf-encoder.h"
#include "components/VideoEncoderVCE.h"

namespace Plugin {
	namespace AMD {
		class EncoderH264 : public Encoder {
			public:
			EncoderH264(std::shared_ptr<API::IAPI> videoAPI, API::Adapter videoAdapter, bool useOpenCL,
				ColorFormat colorFormat, ColorSpace colorSpace, bool fullRangeColor);
			virtual ~EncoderH264();

			// Properties - Initialization
			virtual std::vector<Usage> CapsUsage() override;
			virtual void SetUsage(Usage v) override;
			virtual Usage GetUsage() override;

			// Properties - Static
			virtual std::vector<QualityPreset> CapsQualityPreset() override;
			virtual void SetQualityPreset(QualityPreset v) override;
			virtual QualityPreset GetQualityPreset() override;

			virtual std::vector<Profile> CapsProfile() override;
			virtual void SetProfile(Profile v) override;
			virtual Profile GetProfile() override;

			virtual std::vector<ProfileLevel> CapsProfileLevel() override;
			virtual void SetProfileLevel(ProfileLevel v) override;
			virtual ProfileLevel GetProfileLevel() override;

			virtual std::pair<uint64_t, uint64_t> CapsMaximumReferenceFrames() override;
			virtual void SetMaximumReferenceFrames(uint64_t v) override;
			virtual uint64_t GetMaximumReferenceFrames() override;

			virtual std::pair<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t>> CapsResolution() override;
			virtual void SetResolution(std::pair<uint32_t, uint32_t> v) override;
			virtual std::pair<uint32_t, uint32_t> GetResolution() override;

			virtual void SetAspectRatio(std::pair<uint32_t, uint32_t> v) override;
			virtual std::pair<uint32_t, uint32_t> GetAspectRatio() override;
			
			virtual void SetFrameRate(std::pair<uint32_t, uint32_t> v) override;
			virtual std::pair<uint32_t, uint32_t> GetFrameRate() override;

			virtual std::vector<CodingType> CapsCodingType() override;
			virtual void SetCodingType(CodingType v) override;
			virtual CodingType GetCodingType() override;

			// Properties - Dynamic
			virtual std::vector<RateControlMethod> CapsRateControlMethod() override;
			virtual void SetRateControlMethod(RateControlMethod v) override;
			virtual RateControlMethod GetRateControlMethod() override;

			virtual std::vector<PrePassMode> CapsPrePassMode() override;
			virtual void SetPrePassMode(PrePassMode v) override;
			virtual PrePassMode GetPrePassMode() override;

			virtual void SetVariableBitrateAverageQualityEnabled(bool v) override;
			virtual bool IsVariableBitrateAverageQualityEnabled() override;

			virtual void SetFrameSkippingEnabled(bool v) override;
			virtual bool IsFrameSkippingEnabled() override;

			virtual void SetEnforceHRDEnabled(bool v) override;
			virtual bool IsEnforceHRDEnabled() override;

			virtual void SetFillerDataEnabled(bool v) override;
			virtual bool IsFillerDataEnabled() override;

			void SetQPMinimum(uint8_t v);
			uint8_t GetQPMinimum();

			void SetQPMaximum(uint8_t v);
			uint8_t GetQPMaximum();

			virtual std::pair<uint64_t, uint64_t> CapsTargetBitrate() override;
			virtual void SetTargetBitrate(uint64_t v) override;
			virtual uint64_t GetTargetBitrate() override;

			virtual std::pair<uint64_t, uint64_t> CapsPeakBitrate() override;
			virtual void SetPeakBitrate(uint64_t v) override;
			virtual uint64_t GetPeakBitrate() override;

			virtual void SetIFrameQP(uint8_t v) override;
			virtual uint8_t GetIFrameQP() override;

			virtual void SetPFrameQP(uint8_t v) override;
			virtual uint8_t GetPFrameQP() override;

			virtual void SetBFrameQP(uint8_t v);
			virtual uint8_t GetBFrameQP();

			virtual std::pair<uint64_t, uint64_t> CapsVBVBufferSize() override;
			virtual void SetVBVBufferSize(uint64_t v) override;
			virtual uint64_t GetVBVBufferSize() override;

			virtual void SetVBVBufferInitialFullness(float v) override;
			virtual float GetInitialVBVBufferFullness() override;

			// Properties - Picture Control
			void SetIDRPeriod(uint64_t v);
			uint64_t GetIDRPeriod();

			virtual void SetDeblockingFilterEnabled(bool v) override;
			virtual bool IsDeblockingFilterEnabled() override;

			virtual uint8_t CapsBFramePattern();
			virtual void SetBFramePattern(uint8_t v);
			virtual uint8_t GetBFramePattern();

			virtual void SetBFrameDeltaQP(int8_t v);
			virtual int8_t GetBFrameDeltaQP();
			
			virtual void SetBFrameReferenceEnabled(bool v);
			virtual bool IsBFrameReferenceEnabled();

			virtual void SetBFrameReferenceDeltaQP(int8_t v);
			virtual int8_t GetBFrameReferenceDeltaQP();

			// Properties - Motion Estimation
			virtual void SetMotionEstimationQuarterPixelEnabled(bool v) override;
			virtual bool IsMotionEstimationQuarterPixelEnabled() override;

			virtual void SetMotionEstimationHalfPixelEnabled(bool v) override;
			virtual bool IsMotionEstimationHalfPixelEnabled() override;

			virtual void PacketPriorityAndKeyframe(amf::AMFDataPtr d, struct encoder_packet* p) override;

			virtual AMF_RESULT GetExtraDataInternal(amf::AMFVariant* p) override;


			// Properties - Exclusive
			// TODO: Wait For Task (H264 Exclusive, Unknown effect)
			// TODO: Scan Type (Do we really want to expose this? OBS has no interlaced modes...)
			// TODO: Intra-Refresh
		};
	}
}