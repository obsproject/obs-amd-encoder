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
#include "amf-encoder.hpp"
#include "plugin.hpp"

#include <components/VideoEncoderVCE.h>

namespace Plugin {
	namespace AMD {
		namespace H264 {
			enum class SliceMode : uint8_t {
				Row    = 1, // Horizontal?
				Column = 2, // Vertical?
			};
		}

		class EncoderH264 : public Encoder {
			public:
			EncoderH264(std::shared_ptr<API::IAPI> videoAPI, const API::Adapter& videoAdapter,
						bool useOpenCLSubmission = false, bool useOpenCLConversion = false,
						ColorFormat colorFormat = ColorFormat::NV12, ColorSpace colorSpace = ColorSpace::BT709,
						bool fullRangeColor = false, bool useAsyncQueue = false, size_t asyncQueueSize = 0);
			virtual ~EncoderH264();

			// Properties - Initialization
			virtual std::vector<Usage> CapsUsage() override;
			virtual void               SetUsage(Usage v) override;
			virtual Usage              GetUsage() override;

			// Properties - Static
			virtual std::vector<QualityPreset> CapsQualityPreset() override;
			virtual void                       SetQualityPreset(QualityPreset v) override;
			virtual QualityPreset              GetQualityPreset() override;

#ifndef LITE_OBS
			virtual std::vector<Profile> CapsProfile() override;
			virtual void                 SetProfile(Profile v) override;
			virtual Profile              GetProfile() override;

			virtual std::vector<ProfileLevel> CapsProfileLevel() override;
			virtual void                      SetProfileLevel(ProfileLevel v) override;
			virtual void                      SetProfileLevel(ProfileLevel v, std::pair<uint32_t, uint32_t> r,
															  std::pair<uint32_t, uint32_t> h) override;
			virtual ProfileLevel              GetProfileLevel() override;

			virtual std::pair<uint64_t, uint64_t> CapsMaximumReferenceFrames() override;
			virtual void                          SetMaximumReferenceFrames(uint64_t v) override;
			virtual uint64_t                      GetMaximumReferenceFrames() override;

			virtual std::pair<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t>> CapsResolution() override;
			virtual void                          SetResolution(std::pair<uint32_t, uint32_t> v) override;
			virtual std::pair<uint32_t, uint32_t> GetResolution() override;

			virtual void                          SetAspectRatio(std::pair<uint32_t, uint32_t> v) override;
			virtual std::pair<uint32_t, uint32_t> GetAspectRatio() override;

			virtual void                          SetFrameRate(std::pair<uint32_t, uint32_t> v) override;
			virtual std::pair<uint32_t, uint32_t> GetFrameRate() override;

			virtual std::vector<CodingType> CapsCodingType() override;
			virtual void                    SetCodingType(CodingType v) override;
			virtual CodingType              GetCodingType() override;

			virtual std::pair<uint32_t, uint32_t> CapsMaximumLongTermReferenceFrames() override;
			virtual void                          SetMaximumLongTermReferenceFrames(uint32_t v) override;
			virtual uint32_t                      GetMaximumLongTermReferenceFrames() override;

			virtual void SetHighMotionQualityBoost(bool v);
			virtual bool GetHighMotionQualityBoost();

			// Properties - Dynamic
			virtual std::vector<RateControlMethod> CapsRateControlMethod() override;
			virtual void                           SetRateControlMethod(RateControlMethod v) override;
			virtual RateControlMethod              GetRateControlMethod() override;

			virtual std::vector<PrePassMode> CapsPrePassMode() override;
			virtual void                     SetPrePassMode(PrePassMode v) override;
			virtual PrePassMode              GetPrePassMode() override;

			virtual void SetVarianceBasedAdaptiveQuantizationEnabled(bool v) override;
			virtual bool IsVarianceBasedAdaptiveQuantizationEnabled() override;

			virtual void SetFrameSkippingEnabled(bool v) override;
			virtual bool IsFrameSkippingEnabled() override;

			virtual void SetEnforceHRDEnabled(bool v) override;
			virtual bool IsEnforceHRDEnabled() override;

			virtual void SetFillerDataEnabled(bool v) override;
			virtual bool IsFillerDataEnabled() override;

			virtual std::pair<uint8_t, uint8_t> CapsQPMinimum();
			void                                SetQPMinimum(uint8_t v);
			uint8_t                             GetQPMinimum();

			virtual std::pair<uint8_t, uint8_t> CapsQPMaximum();
			void                                SetQPMaximum(uint8_t v);
			uint8_t                             GetQPMaximum();

			virtual std::pair<uint64_t, uint64_t> CapsTargetBitrate() override;
			virtual void                          SetTargetBitrate(uint64_t v) override;
			virtual uint64_t                      GetTargetBitrate() override;

			virtual std::pair<uint64_t, uint64_t> CapsPeakBitrate() override;
			virtual void                          SetPeakBitrate(uint64_t v) override;
			virtual uint64_t                      GetPeakBitrate() override;

			virtual std::pair<uint8_t, uint8_t> CapsIFrameQP();
			virtual void                        SetIFrameQP(uint8_t v) override;
			virtual uint8_t                     GetIFrameQP() override;

			virtual std::pair<uint8_t, uint8_t> CapsPFrameQP();
			virtual void                        SetPFrameQP(uint8_t v) override;
			virtual uint8_t                     GetPFrameQP() override;

			virtual std::pair<uint8_t, uint8_t> CapsBFrameQP();
			virtual void                        SetBFrameQP(uint8_t v);
			virtual uint8_t                     GetBFrameQP();

			virtual void     SetMaximumAccessUnitSize(uint32_t v) override;
			virtual uint32_t GetMaximumAccessUnitSize() override;

			virtual std::pair<uint64_t, uint64_t> CapsVBVBufferSize() override;
			virtual void                          SetVBVBufferSize(uint64_t v) override;
			virtual uint64_t                      GetVBVBufferSize() override;

			virtual void  SetVBVBufferInitialFullness(double v) override;
			virtual float GetInitialVBVBufferFullness() override;

			// Properties - Picture Control
			virtual void     SetIDRPeriod(uint32_t v) override;
			virtual uint32_t GetIDRPeriod() override;

			void     SetHeaderInsertionSpacing(uint32_t v);
			uint32_t GetHeaderInsertionSpacing();

			virtual void SetGOPAlignmentEnabled(bool v) override;
			virtual bool IsGOPAlignmentEnabled() override;

			virtual void SetDeblockingFilterEnabled(bool v) override;
			virtual bool IsDeblockingFilterEnabled() override;

			virtual uint8_t CapsBFramePattern();
			virtual void    SetBFramePattern(uint8_t v);
			virtual uint8_t GetBFramePattern();

			virtual void   SetBFrameDeltaQP(int8_t v);
			virtual int8_t GetBFrameDeltaQP();

			virtual void SetBFrameReferenceEnabled(bool v);
			virtual bool IsBFrameReferenceEnabled();

			virtual void   SetBFrameReferenceDeltaQP(int8_t v);
			virtual int8_t GetBFrameReferenceDeltaQP();

			// Properties - Motion Estimation
			virtual void SetMotionEstimationQuarterPixelEnabled(bool v) override;
			virtual bool IsMotionEstimationQuarterPixelEnabled() override;

			virtual void SetMotionEstimationHalfPixelEnabled(bool v) override;
			virtual bool IsMotionEstimationHalfPixelEnabled() override;

			// Properties - Intra-Refresh
			std::pair<uint32_t, uint32_t> CapsIntraRefreshNumMBsPerSlot();
			void                          SetIntraRefreshNumMBsPerSlot(uint32_t v);
			uint32_t                      GetIntraRefreshNumMBsPerSlot();

			void     SetIntraRefreshNumOfStripes(uint32_t v);
			uint32_t GetIntraRefreshNumOfStripes();

			// Internal
			virtual void LogProperties() override;

			protected:
			virtual void        PacketPriorityAndKeyframe(amf::AMFDataPtr& d, struct encoder_packet* p) override;
			virtual AMF_RESULT  GetExtraDataInternal(amf::AMFVariant* p) override;
			virtual std::string HandleTypeOverride(amf::AMFSurfacePtr& d, uint64_t index) override;

			AMF_VIDEO_ENCODER_PICTURE_TYPE_ENUM m_FrameSkipType = AMF_VIDEO_ENCODER_PICTURE_TYPE_NONE;
#endif
		};
	} // namespace AMD
} // namespace Plugin
