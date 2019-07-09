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

#include <components/VideoEncoderHEVC.h>

namespace Plugin {
	namespace AMD {
		namespace H265 {
			enum class Tier : uint8_t {
				Main,
				High,
			};
			enum class GOPType : uint8_t {
				Fixed,    // Fixed Interval GOP
				Variable, // Variable Interval GOP
			};
			enum class HeaderInsertionMode : uint8_t {
				None         = 0,
				AlignedToGOP = 1,
				AlignedToIDR = 2,
			};
		} // namespace H265

		class EncoderH265 : public Encoder {
			public:
			EncoderH265(std::shared_ptr<API::IAPI> videoAPI, const API::Adapter& videoAdapter,
						bool useOpenCLSubmission = false, bool useOpenCLConversion = false,
						ColorFormat colorFormat = ColorFormat::NV12, ColorSpace colorSpace = ColorSpace::BT709,
						bool fullRangeColor = false, bool useAsyncQueue = false, size_t asyncQueueSize = 0);
			virtual ~EncoderH265();

			// Initialization
			virtual std::vector<Usage> CapsUsage() override;
			virtual void               SetUsage(Usage v) override;
			virtual Usage              GetUsage() override;

			// Static
			virtual std::vector<QualityPreset> CapsQualityPreset() override;
			virtual void                       SetQualityPreset(QualityPreset v) override;
			virtual QualityPreset              GetQualityPreset() override;

#ifndef LITE_OBS
			virtual std::pair<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t>> CapsResolution() override;
			virtual void                          SetResolution(std::pair<uint32_t, uint32_t> v) override;
			virtual std::pair<uint32_t, uint32_t> GetResolution() override;

			virtual void                          SetAspectRatio(std::pair<uint32_t, uint32_t> v) override;
			virtual std::pair<uint32_t, uint32_t> GetAspectRatio() override;

			virtual void                          SetFrameRate(std::pair<uint32_t, uint32_t> v) override;
			virtual std::pair<uint32_t, uint32_t> GetFrameRate() override;

			virtual std::vector<Profile> CapsProfile() override;
			virtual void                 SetProfile(Profile v) override;
			virtual Profile              GetProfile() override;

			virtual std::vector<ProfileLevel> CapsProfileLevel() override;
			virtual void                      SetProfileLevel(ProfileLevel v) override;
			virtual void                      SetProfileLevel(ProfileLevel v, std::pair<uint32_t, uint32_t> r,
															  std::pair<uint32_t, uint32_t> h) override;
			virtual ProfileLevel              GetProfileLevel() override;

			std::vector<H265::Tier> CapsTier();
			void                    SetTier(H265::Tier v);
			H265::Tier              GetTier();

			virtual std::pair<uint64_t, uint64_t> CapsMaximumReferenceFrames() override;
			virtual void                          SetMaximumReferenceFrames(uint64_t v) override;
			virtual uint64_t                      GetMaximumReferenceFrames() override;

			virtual std::vector<CodingType> CapsCodingType() override;
			virtual void                    SetCodingType(CodingType v) override;
			virtual CodingType              GetCodingType() override;

			virtual std::pair<uint32_t, uint32_t> CapsMaximumLongTermReferenceFrames() override;
			virtual void                          SetMaximumLongTermReferenceFrames(uint32_t v) override;
			virtual uint32_t                      GetMaximumLongTermReferenceFrames() override;

			/// Rate Control
			virtual std::vector<RateControlMethod> CapsRateControlMethod() override;
			virtual void                           SetRateControlMethod(RateControlMethod v) override;
			virtual RateControlMethod              GetRateControlMethod() override;

			virtual std::vector<PrePassMode> CapsPrePassMode() override;
			virtual void                     SetPrePassMode(PrePassMode v) override;
			virtual PrePassMode              GetPrePassMode() override;

			virtual void SetVarianceBasedAdaptiveQuantizationEnabled(bool v) override;
			virtual bool IsVarianceBasedAdaptiveQuantizationEnabled() override;

			virtual void SetHighMotionQualityBoost(bool v);
			virtual bool GetHighMotionQualityBoost();

			/// VBV Buffer
			virtual std::pair<uint64_t, uint64_t> CapsVBVBufferSize() override;
			virtual void                          SetVBVBufferSize(uint64_t v) override;
			virtual uint64_t                      GetVBVBufferSize() override;

			virtual void  SetVBVBufferInitialFullness(double v) override;
			virtual float GetInitialVBVBufferFullness() override;

			/// Picture Control
			std::vector<H265::GOPType> CapsGOPType();
			void                       SetGOPType(H265::GOPType v);
			H265::GOPType              GetGOPType();

			void     SetGOPSize(uint32_t v);
			uint32_t GetGOPSize();

			void     SetGOPSizeMin(uint32_t v);
			uint32_t GetGOPSizeMin();

			void     SetGOPSizeMax(uint32_t v);
			uint32_t GetGOPSizeMax();

			virtual void SetGOPAlignmentEnabled(bool v) override;
			virtual bool IsGOPAlignmentEnabled() override;

			virtual void     SetIDRPeriod(uint32_t v) override; // Distance in GOPs
			virtual uint32_t GetIDRPeriod() override;

			void                      SetHeaderInsertionMode(H265::HeaderInsertionMode v);
			H265::HeaderInsertionMode GetHeaderInsertionMode();

			virtual void SetDeblockingFilterEnabled(bool v) override;
			virtual bool IsDeblockingFilterEnabled() override;

			/// Motion Estimation
			virtual void SetMotionEstimationQuarterPixelEnabled(bool v) override;
			virtual bool IsMotionEstimationQuarterPixelEnabled() override;

			virtual void SetMotionEstimationHalfPixelEnabled(bool v) override;
			virtual bool IsMotionEstimationHalfPixelEnabled() override;

			// Dynamic
			virtual void SetFrameSkippingEnabled(bool v) override;
			virtual bool IsFrameSkippingEnabled() override;

			virtual void SetEnforceHRDEnabled(bool v) override;
			virtual bool IsEnforceHRDEnabled() override;

			virtual void SetFillerDataEnabled(bool v) override;
			virtual bool IsFillerDataEnabled() override;

			virtual std::pair<uint8_t, uint8_t> CapsIFrameQPMinimum();
			void                                SetIFrameQPMinimum(uint8_t v);
			uint8_t                             GetIFrameQPMinimum();

			virtual std::pair<uint8_t, uint8_t> CapsIFrameQPMaximum();
			void                                SetIFrameQPMaximum(uint8_t v);
			uint8_t                             GetIFrameQPMaximum();

			virtual std::pair<uint8_t, uint8_t> CapsPFrameQPMinimum();
			void                                SetPFrameQPMinimum(uint8_t v);
			uint8_t                             GetPFrameQPMinimum();

			virtual std::pair<uint8_t, uint8_t> CapsPFrameQPMaximum();
			void                                SetPFrameQPMaximum(uint8_t v);
			uint8_t                             GetPFrameQPMaximum();

			virtual std::pair<uint64_t, uint64_t> CapsTargetBitrate() override;
			virtual void                          SetTargetBitrate(uint64_t v) override;
			virtual uint64_t                      GetTargetBitrate() override;

			virtual std::pair<uint64_t, uint64_t> CapsPeakBitrate() override;
			virtual void                          SetPeakBitrate(uint64_t v) override;
			virtual uint64_t                      GetPeakBitrate() override;

			virtual std::pair<uint8_t, uint8_t> CapsIFrameQP() override;
			virtual void                        SetIFrameQP(uint8_t v) override;
			virtual uint8_t                     GetIFrameQP() override;

			virtual std::pair<uint8_t, uint8_t> CapsPFrameQP() override;
			virtual void                        SetPFrameQP(uint8_t v) override;
			virtual uint8_t                     GetPFrameQP() override;

			virtual void     SetMaximumAccessUnitSize(uint32_t v) override;
			virtual uint32_t GetMaximumAccessUnitSize() override;

			std::pair<uint32_t, uint32_t> CapsInputQueueSize();
			void                          SetInputQueueSize(uint32_t v);
			uint32_t                      GetInputQueueSize();

			// Internal
			virtual void LogProperties() override;

			protected:
			virtual void        PacketPriorityAndKeyframe(amf::AMFDataPtr& d, struct encoder_packet* p) override;
			virtual AMF_RESULT  GetExtraDataInternal(amf::AMFVariant* p) override;
			virtual std::string HandleTypeOverride(amf::AMFSurfacePtr& d, uint64_t index) override;

			AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_ENUM m_FrameSkipType = AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_NONE;

			//Remaining Properties
			// PerformanceCounter (Interface, but which one?)
			// HevcMaxNumOfTemporalLayers/HevcNumOfTemporalLayers/HevcTemporalLayerSelect - Only supports QP_I/P?
			// BPicturesPattern (replaced by merge mode?)
			// HevcMaxMBPerSec (PCI-E bandwidth, min/max)
#endif
		};
	} // namespace AMD
} // namespace Plugin
