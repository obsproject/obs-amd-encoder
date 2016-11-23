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
#include <condition_variable>
#include <algorithm>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <chrono>

// Plugin
#include "plugin.h"
#include "amd-amf.h"
#include "api-base.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////

namespace Plugin {
	namespace AMD {
		// Internal Properties
		enum VCEEncoderType {
			VCEEncoderType_AVC,		// Advanced Video Coding
			VCEEncoderType_SVC,		// Scalable Video Coding
			VCEEncoderType_HEVC,	// High-Efficiency Video Coding (Discovered in amfrt64.dll)
		};
		enum VCEMemoryType {
			VCEMemoryType_Host,			// Host-Managed Memory
			VCEMemoryType_DirectX9,		// DirectX9
			VCEMemoryType_DirectX11,	// DirectX11
			VCEMemoryType_OpenGL,		// OpenGL
		};
		enum VCEColorFormat {
			// 4:2:0 Formats
			VCEColorFormat_NV12,	// NV12
			VCEColorFormat_I420,	// YUV 4:2:0
			// 4:2:2 Formats
			VCEColorFormat_YUY2,
			// Uncompressed
			VCEColorFormat_BGRA,	// ARGB
			VCEColorFormat_RGBA,	// RGBA
			// Other
			VCEColorFormat_GRAY,
		};
		enum VCEColorProfile {
			VCEColorProfile_601,
			VCEColorProfile_709,
			VCEColorProfile_2020, // HDR
		};

		// Static Properties
		enum VCEUsage {
			VCEUsage_Transcoding,
			VCEUsage_UltraLowLatency,
			VCEUsage_LowLatency,
			VCEUsage_Webcam,			// For SVC
		};
		enum VCEQualityPreset {
			VCEQualityPreset_Speed,
			VCEQualityPreset_Balanced,
			VCEQualityPreset_Quality,
		};
		enum VCEProfile {
			VCEProfile_Baseline = 66,
			VCEProfile_Main = 77,
			VCEProfile_High = 100,
			VCEProfile_ConstrainedBaseline = 256,
			VCEProfile_ConstrainedHigh = 257
		};
		enum VCEProfileLevel {
			VCEProfileLevel_Automatic = 0,
			VCEProfileLevel_10 = 10,
			VCEProfileLevel_11,
			VCEProfileLevel_12,
			VCEProfileLevel_13,
			VCEProfileLevel_20 = 20,
			VCEProfileLevel_21,
			VCEProfileLevel_22,
			VCEProfileLevel_30 = 30,
			VCEProfileLevel_31,
			VCEProfileLevel_32,
			VCEProfileLevel_40 = 40,
			VCEProfileLevel_41,
			VCEProfileLevel_42,
			VCEProfileLevel_50 = 50,
			VCEProfileLevel_51,
			VCEProfileLevel_52,
			VCEProfileLevel_60 = 60,
			VCEProfileLevel_61,
			VCEProfileLevel_62,
		};
		enum VCEScanType {
			VCEScanType_Progressive,
			VCEScanType_Interlaced,
		};
		enum VCECodingType {
			VCECodingType_Default = 0,
			VCECodingType_CABAC = 1,
			VCECodingType_CALVC = 2,
		};

		// Dynamic Properties
		enum VCERateControlMethod {
			VCERateControlMethod_ConstantQP,
			VCERateControlMethod_ConstantBitrate,
			VCERateControlMethod_VariableBitrate_PeakConstrained,
			VCERateControlMethod_VariableBitrate_LatencyConstrained,
		};
		enum VCEBFramePattern {
			VCEBFramePattern_None,
			VCEBFramePattern_One,
			VCEBFramePattern_Two,
			VCEBFramePattern_Three,
		};

		class VCEEncoder {
			//////////////////////////////////////////////////////////////////////////
			#pragma region Functions
			//////////////////////////////////////////////////////////////////////////
			private:
			static void InputThreadMain(Plugin::AMD::VCEEncoder* p_this);
			static void OutputThreadMain(Plugin::AMD::VCEEncoder* p_this);

			#pragma endregion Functions
			//////////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////////////
			#pragma region Initializer & Finalizer
			//////////////////////////////////////////////////////////////////////////
			public:
			VCEEncoder(
				VCEEncoderType p_Type,
				std::string p_VideoAPI,
				uint64_t p_VideoAdapterId,
				bool p_OpenCL,
				VCEColorFormat p_SurfaceFormat = VCEColorFormat_NV12
			);
			~VCEEncoder();
			#pragma endregion Initializer & Finalizer

			//////////////////////////////////////////////////////////////////////////
			#pragma region Methods
			//////////////////////////////////////////////////////////////////////////
			public:
			void Start();
			void Restart();
			void Stop();
			bool IsStarted();
			bool SendInput(struct encoder_frame* frame);
			bool GetOutput(struct encoder_packet* packet, bool* received_packet);
			bool GetExtraData(uint8_t**& data, size_t*& size);
			void GetVideoInfo(struct video_scale_info*& vsi);

			// Threading
			private:
			void InputThreadLogic();
			void OutputThreadLogic();

			// Utility
			inline amf::AMFSurfacePtr CreateSurfaceFromFrame(struct encoder_frame*& frame);

			public:
			void LogProperties();
			
			/************************************************************************/
			/* Static Properties                                                    */
			/************************************************************************/

			/*	Usage Type
			*	Sets up the entire encoder to a specific set of properties.
			*	Must be called first if you want to override properties. */
			void SetUsage(VCEUsage usage);
			VCEUsage GetUsage();

			/** Quality Preset
			* Static Property, changes cause a restart. */
			void SetQualityPreset(VCEQualityPreset preset);
			VCEQualityPreset GetQualityPreset();
			
			/*	H.264 Profile */
			void SetProfile(VCEProfile profile);
			VCEProfile GetProfile();

			/*	H.264 Profile Level */
			void SetProfileLevel(VCEProfileLevel level);
			VCEProfileLevel GetProfileLevel();

			/*	The number of long-term references controlled by the user.
			 *
			 *	Remarks:
			 *  - When == 0, the encoder may or may not use LTRs during encoding.
			 *	- When >0, the user has control over all LTR.
			 *	- With user control of LTR, B-Frames and Intra-refresh features are not supported.
			 *	- The actual maximum number of LTRs allowed depends on H.264 Annex A Table A-1 Level limits, which defines dependencies between the H.264 Level number, encoding resolution, and DPB size. The DPB size limit impacts the maximum number of LTR allowed.
			 **/
			void SetMaximumLongTermReferenceFrames(uint32_t maximumLTRFrames);	// Long-Term Reference Frames. If 0, Encoder decides, if non-0 B-Frames and Intra-Refresh are not supported.
			uint32_t GetMaximumLongTermReferenceFrames();

			/* Coding Type */
			void SetCodingType(VCECodingType type);
			VCECodingType GetCodingType();

			/************************************************************************/
			/* Frame Properties                                                     */
			/************************************************************************/

			// Set which Color Profile the input frame is.
			void SetColorProfile(VCEColorProfile profile);
			VCEColorProfile GetColorProfile();

			// Set if the input frame is in full color range.
			void SetFullColorRangeEnabled(bool enabled);
			bool IsFullColorRangeEnabled();

			/* Selects progressive or interlaced scan
			* Static Property, changes cause a restart. */
			void SetScanType(VCEScanType scanType);
			VCEScanType GetScanType();

			/*	Output Resolution */
			void SetFrameSize(uint32_t width, uint32_t height);
			std::pair<uint32_t, uint32_t> GetFrameSize();

			/*	Sets the Frame Rate numerator and denumerator */
			void SetFrameRate(uint32_t num, uint32_t den);
			std::pair<uint32_t, uint32_t> GetFrameRate();

			/************************************************************************/
			/* Rate Control Properties                                              */
			/************************************************************************/
			
			/*	Selects the rate control method:
			*	- CQP - Constrained QP,
			*	- CBR - Constant Bitrate,
			*	- VBR - Peak Constrained VBR,
			*	- VBR_LAT - Latency Constrained VBR
			*
			*	Remarks:
			*	- When SVC encoding is enabled, all Rate-control parameters (with some restrictions) can be configured differently for a particular SVC-layer. An SVC-layer is denoted by an index pair [SVC-Temporal Layer index][SVC-Quality Layer index]. E.g. The bitrate may be configured differently for SVC-layers [0][0] and [1][0].
			*	- We restrict all SVC layers to have the same Rate Control method. Some RC parameters are not enabled with SVC encoding (e.g. all parameters related to B-Frames).
			**/
			void SetRateControlMethod(VCERateControlMethod method);
			VCERateControlMethod GetRateControlMethod();

			/*	Sets the target bitrate */
			void SetTargetBitrate(uint32_t bitrate);
			uint32_t GetTargetBitrate();

			/*	Sets the peak bitrate */
			void SetPeakBitrate(uint32_t bitrate);
			uint32_t GetPeakBitrate();

			/*	Sets the minimum QP */
			void SetMinimumQP(uint8_t qp);
			uint8_t GetMinimumQP();

			/*	Sets the maximum QP */
			void SetMaximumQP(uint8_t qp);
			uint8_t GetMaximumQP();

			/*	Sets the Constant QP for I-Frames.
			*
			*	Remarks:
			*	- Only available for CQP rate control method.
			**/
			void SetIFrameQP(uint8_t qp);
			uint8_t GetIFrameQP();

			/*	Sets the Constant QP for P-Frames.
			*
			*	Remarks:
			*	- Only available for CQP rate control method.
			**/
			void SetPFrameQP(uint8_t qp);
			uint8_t GetPFrameQP();

			/*	Sets the Constant QP for B-Frames.
			*
			*	Remarks:
			*	- Only available for CQP rate control method.
			**/
			void SetBFrameQP(uint8_t qp);
			uint8_t GetBFrameQP();

			/*	Sets the VBV Buffer Size in bits */
			void SetVBVBufferSize(uint32_t size);
			void SetVBVBufferAutomatic(double_t strictness);
			uint32_t GetVBVBufferSize();

			/*	Sets the initial VBV Buffer Fullness */
			void SetInitialVBVBufferFullness(double_t fullness);
			double_t GetInitialVBVBufferFullness();

			/*	Sets Maximum AU Size in bits */
			void SetMaximumAccessUnitSize(uint32_t size);
			uint32_t GetMaximumAccessUnitSize();

			/*	Enables/Disables filler data */
			void SetFillerDataEnabled(bool enabled);
			bool IsFillerDataEnabled();

			/*	Enables skip frame for rate control */
			void SetFrameSkippingEnabled(bool enabled);
			bool IsFrameSkippingEnabled();

			/*	Enables/Disables constraints on QP variation within a picture to meet HRD requirement(s) */
			void SetEnforceHRDRestrictionsEnabled(bool enforce);
			bool IsEnforceHRDRestrictionsEnabled();

			/************************************************************************/
			/* Frame Control Properties                                           */
			/************************************************************************/

			/*	Sets IDR period. IDRPeriod= 0 turns IDR off */
			void SetIDRPeriod(uint32_t period);
			uint32_t GetIDRPeriod();

			/*	Sets the headers insertion spacing */
			void SetHeaderInsertionSpacing(uint32_t spacing); // Similar to IDR Period, spacing (in frames) between headers.
			uint32_t GetHeaderInsertionSpacing();

			/*	Sets the number of consecutive B-Frames in a GOP. BFramesPattern = 0 indicates that B-Frames are not used */
			void SetBFramePattern(VCEBFramePattern pattern);
			VCEBFramePattern GetBFramePattern();

			/* Selects the delta QP of non-reference B-Frames with respect to the last non-B-Frame */
			void SetBFrameDeltaQP(int8_t qp);
			int8_t GetBFrameDeltaQP();

			/*	Enables or disables using B-Frames as references */
			void SetBFrameReferenceEnabled(bool enabled);
			bool IsBFrameReferenceEnabled();

			/* Selects delta QP of reference B-Frames with respect to the last non-B-Frame */
			void SetBFrameReferenceDeltaQP(int8_t qp);
			int8_t GetBFrameReferenceDeltaQP();

			/*	Turns on/off the Deblocking Filter */
			void SetDeblockingFilterEnabled(bool enabled);
			bool IsDeblockingFilterEnabled();

			/*	Sets the number of slices per frame */
			void SetSlicesPerFrame(uint32_t slices);
			uint32_t GetSlicesPerFrame();

			// Macroblocks per Intra-Refresh Slot
			// Intra-Refresh Coding
			void SetIntraRefreshMacroblocksPerSlot(uint32_t macroblocks);
			uint32_t GetIntraRefreshMacroblocksPerSlot();

			/************************************************************************/
			/* Miscellaneous Control Properties                                     */
			/************************************************************************/
			
			/* Turns on/off half-pixel motion estimation */
			void SetHalfPixelMotionEstimationEnabled(bool enabled);
			bool IsHalfPixelMotionEstimationEnabled();

			/* Turns on/off quarter-pixel motion estimation */
			void SetQuarterPixelMotionEstimationEnabled(bool enabled);
			bool IsQuarterPixelMotionEstimationEnabled();

			/************************************************************************/
			/* Experimental Properties                                              */
			/************************************************************************/
			uint32_t GetMaxMBPerSec();
			
			void SetWaitForTaskEnabled(bool enabled);
			bool IsWaitForTaskEnabled();

			void SetPreanalysisPassEnabled(bool enabled);
			bool IsPreanalysisPassEnabled();

			// VBAQ = Variable Bitrate Average Quality?
			void SetVBAQEnabled(bool enabled);
			bool IsVBAQEnabled();

			void SetGOPSize(uint32_t gopSize);
			uint32_t GetGOPSize();

			void SetGOPAlignmentEnabled(bool enabled);
			bool IsGOPAlignementEnabled();

			void SetMaximumReferenceFrames(uint32_t frameCount);
			uint32_t GetMaximumReferenceFrames();
			
			void SetAspectRatio(uint32_t x, uint32_t y);
			std::pair<uint32_t, uint32_t> GetAspectRatio();

			// More:
			// - CodecId (H264 = 5, H264SVC = 8, 2xUNKNOWN)
			// - EngineType (Auto = 0, DX9 = 1, DX11 = 2, XVBA = 3)
			// - ConstraintSetFlags (0 - 255, 1 byte bitset?)
			// - EanbleVBAQ (bool)
			// - LowLatencyInternal (bool)
			// - CommonLowLatencyInternal (bool)
			// - SliceControlMode (0 - 3)
			// - SliceControlSize (0 - INT_MAX)
			// - UniqueInstance (0 - INT_MAX)
			// - EncoderMaxInstances (1 - 2)
			// - MultiInstanceMode (bool)
			// - MultiInstanceCurrentQueue (0 - 1)

			//
			//void SetInstanceID(uint32_t instanceId);
			//uint32_t GetInstanceID();
			//
			//// Stripe = Slice?
			//// Intra-Refresh Coding
			//void SetIntraRefreshNumberOfStripes(uint32_t stripes); // 0 - INT_MAX
			//uint32_t GetIntraRefreshNumberOfStripes();

			//void SetSliceMode(uint32_t mode); // 1 or 2 (Horizontal or Vertical?)
			//uint32_t GetSliceMode();

			//void SetMaximumSliceSize(uint32_t size); // 0 - INT_MAX
			//uint32_t GetMaximumSliceSize();

			//// - SliceControlMode: AMF_VIDEO_ENCODER_SLICE_CTRL_MODE_MB_ROW, AMF_VIDEO_ENCODER_SLICE_CTRL_MODE_MB
			//void SetSliceControlMode(uint32_t mode); // 0, 1, 2, 3
			//uint32_t GetSliceControlMode();

			//void SetSliceControlSize(uint32_t size); // 0 - INT_MAX
			//uint32_t GetSliceControlSize();

			#pragma endregion Methods
			//////////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////////////
			#pragma region Members
			//////////////////////////////////////////////////////////////////////////
			private:

			// AMF Data References
			std::shared_ptr<Plugin::AMD::AMF> m_AMF;
			amf::AMFFactory* m_AMFFactory;
			amf::AMFContextPtr m_AMFContext;
			amf::AMFComponentPtr m_AMFConverter;
			amf::AMFComponentPtr m_AMFEncoder;
			amf::AMFComputePtr m_AMFCompute;

			// API References
			std::shared_ptr<Plugin::API::Base> m_API;
			Plugin::API::Adapter m_APIAdapter;
			void* m_APIInstance;

			// Static Buffers
			std::vector<uint8_t> m_PacketDataBuffer;
			std::vector<uint8_t> m_ExtraDataBuffer;

			// Structured Queue
			struct {
				std::queue<amf::AMFSurfacePtr> queue;

				// Threading
				std::thread thread;
				std::mutex mutex;
				std::condition_variable condvar;
				std::mutex queuemutex;
			} m_Input;
			struct {
				std::queue<amf::AMFDataPtr> queue;

				// Threading
				std::thread thread;
				std::mutex mutex;
				std::condition_variable condvar;
				std::mutex queuemutex;
			} m_Output;

			// Internal Properties
			VCEEncoderType m_EncoderType;
			VCEMemoryType m_MemoryType;
			bool m_OpenCL;
			VCEColorFormat m_SurfaceFormat;
			bool m_Flag_IsStarted,
				m_Flag_FirstFrameSubmitted,
				m_Flag_FirstFrameReceived;
			std::pair<uint32_t, uint32_t> m_FrameSize,
				m_FrameRate;
			double_t m_FrameRateDivisor,
				m_FrameRateReverseDivisor;
			size_t m_InputQueueLimit,
				m_InputQueueLastSize;
			uint32_t m_TimerPeriod;
			VCEColorProfile m_ColorProfile;
			
			#pragma endregion Members
			//////////////////////////////////////////////////////////////////////////
		};
	}
}