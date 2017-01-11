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
#include "amf.h"
#include "api-base.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////

namespace Plugin {
	namespace AMD {
		// Internal Properties
		enum class H264EncoderType : uint8_t {
			AVC = 0,	// Advanced Video Coding
			SVC,		// Scalable Video Coding
			HEVC,		// High-Efficiency Video Coding (Discovered in amfrt64.dll)
		};
		enum class H264MemoryType : uint8_t {
			Host = 0,	// Host-Managed Memory
			DirectX9,	// DirectX9
			DirectX11,	// DirectX11
			OpenGL,		// OpenGL
		};
		enum class H264ColorFormat : uint8_t {
			// 4:2:0 Formats
			NV12 = 0,	// NV12
			I420,		// YUV 4:2:0
			// 4:2:2 Formats
			YUY2,
			// Uncompressed
			BGRA,		// ARGB
			RGBA,		// RGBA
			// Other
			GRAY,		// Y 4:0:0
		};
		enum class H264ColorProfile : uint8_t {
			Rec601 = 0,
			Rec709,
			Rec2020, // Truer to world color, used for HDR
		};

		// Static Properties
		enum class H264Usage : uint8_t {
			Transcoding = 0,	// Only one capable of streaming to services.
			UltraLowLatency,	// Low Latency Recording or Network Streaming
			LowLatency,			// Low Latency Recording or Network Streaming
			Webcam,				// For SVC Recording and Streaming
		};
		enum class H264QualityPreset : uint8_t {
			Speed = 0,
			Balanced,
			Quality,
		};
		enum class H264Profile : uint16_t {
			Baseline = 66,
			Main = 77,
			High = 100,
			ConstrainedBaseline = 256,
			ConstrainedHigh = 257
		};
		enum class H264ProfileLevel : uint8_t {
			Automatic = 0,
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
		};
		enum class H264ScanType : uint8_t {
			Progressive = 0,
			Interlaced,
		};
		enum class H264CodingType : uint8_t {
			Default = 0,
			CABAC = 1,
			CALVC = 2,
		};

		// Dynamic Properties
		enum class H264RateControlMethod : uint8_t {
			ConstantQP = 0,
			ConstantBitrate,
			VariableBitrate_PeakConstrained,
			VariableBitrate_LatencyConstrained = 3,
		};
		enum class H264BFramePattern : uint8_t {
			None = 0,
			One,
			Two,
			Three,
		};

		// Experimental
		enum class H264SliceMode : uint8_t {
			Horizontal = 1,
			Vertical = 2
		};
		enum class H264SliceControlMode : uint8_t {
			Off = 0,
			Macroblock = 1, // AMF_VIDEO_ENCODER_SLICE_CTRL_MODE_MB
			Invalid,
			Macroblock_Row = 3 // AMF_VIDEO_ENCODER_SLICE_CTRL_MODE_MB_ROW
		};
		
		class H264Encoder {
			#pragma region Initializer & Finalizer
			public:
			H264Encoder(
				H264EncoderType p_Type,
				std::string p_VideoAPI,
				uint64_t p_VideoAdapterId,
				bool p_OpenCL,
				H264ColorFormat p_SurfaceFormat = H264ColorFormat::NV12
			);
			~H264Encoder();
			#pragma endregion Initializer & Finalizer

			public:
			void Start();
			void Restart();
			void Stop();
			bool IsStarted();

			bool SendInput(struct encoder_frame* frame);
			bool GetOutput(struct encoder_packet* packet, bool* received_packet);
			bool GetExtraData(uint8_t**& data, size_t*& size);
			void GetVideoInfo(struct video_scale_info*& vsi);

			#pragma region Properties
			public:
			void LogProperties();

			// Static

			#pragma region Startup Properties
			// Set which Usage preset to use.
			// Changing this will also change a lot of other properties.
			void SetUsage(H264Usage usage);
			H264Usage GetUsage();

			// Set which Quality Preset AMF should use.
			// Affects the quality of the output.
			void SetQualityPreset(H264QualityPreset preset);
			H264QualityPreset GetQualityPreset();

			// Set the Profile the output should have.
			void SetProfile(H264Profile profile);
			H264Profile GetProfile();

			// Set the Profile Level the output should have.
			void SetProfileLevel(H264ProfileLevel level);
			H264ProfileLevel GetProfileLevel();
			#pragma endregion Startup Properties

			#pragma region Frame Properties
			// Set which Color Profile the input frame is.
			void SetColorProfile(H264ColorProfile profile);
			H264ColorProfile GetColorProfile();

			// Set if the input frame is in full color range.
			void SetFullRangeColorEnabled(bool enabled);
			bool IsFullRangeColorEnabled();

			// Resolution for the input and output.
			void SetResolution(uint32_t width, uint32_t height);
			std::pair<uint32_t, uint32_t> GetResolution();

			// Framerate of the input and output.
			void SetFrameRate(uint32_t num, uint32_t den);
			std::pair<uint32_t, uint32_t> GetFrameRate();

			// Scanning method for input (and output?).
			void SetScanType(H264ScanType scanType);
			H264ScanType GetScanType();
			#pragma endregion Frame Properties

			// Dynamic

			#pragma region Rate Control
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
			void SetRateControlMethod(H264RateControlMethod method);
			H264RateControlMethod GetRateControlMethod();

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

			// Set the fixed QP value for I-Frames.
 			void SetIFrameQP(uint8_t qp);
			uint8_t GetIFrameQP();

			// Set the fixed QP value for P-Frames.
			void SetPFrameQP(uint8_t qp);
			uint8_t GetPFrameQP();

			// Set the fixed QP value for B-Frames.
			void SetBFrameQP(uint8_t qp);
			uint8_t GetBFrameQP();

			// Set the Video Buffer Verifier (VBV) size in bits per second (bps).
			void SetVBVBufferSize(uint32_t size);
			// Set the Video Buffer Verifier (VBV) size using a strictness constraint.
			void SetVBVBufferAutomatic(double_t strictness);
			uint32_t GetVBVBufferSize();

			/*	Sets the initial VBV Buffer Fullness */
			void SetInitialVBVBufferFullness(double_t fullness);
			double_t GetInitialVBVBufferFullness();

			/*	Enables/Disables filler data */
			void SetFillerDataEnabled(bool enabled);
			bool IsFillerDataEnabled();

			/*	Enables skip frame for rate control */
			void SetFrameSkippingEnabled(bool enabled);
			bool IsFrameSkippingEnabled();

			/*	Enables/Disables constraints on QP variation within a picture to meet HRD requirement(s) */
			void SetEnforceHRDRestrictionsEnabled(bool enforce);
			bool IsEnforceHRDRestrictionsEnabled();
			#pragma endregion Rate Control

			#pragma region Picture Control
			// Set the Instantaneous-Decoder-Refresh (IDR) Period in frames.
			void SetIDRPeriod(uint32_t period);
			uint32_t GetIDRPeriod();

			#pragma region B-Frames
			/*	Sets the number of consecutive B-Frames. BFramesPattern = 0 indicates that B-Frames are not used */
			void SetBFramePattern(H264BFramePattern pattern);
			H264BFramePattern GetBFramePattern();

			/* Selects the delta QP of non-reference B-Frames with respect to the last non-B-Frame */
			void SetBFrameDeltaQP(int8_t qp);
			int8_t GetBFrameDeltaQP();

			/*	Enables or disables using B-Frames as references */
			void SetBFrameReferenceEnabled(bool enabled);
			bool IsBFrameReferenceEnabled();

			/* Selects delta QP of reference B-Frames with respect to the last non-B-Frame */
			void SetBFrameReferenceDeltaQP(int8_t qp);
			int8_t GetBFrameReferenceDeltaQP();
			#pragma endregion B-Frames
			#pragma endregion Picture Control

			#pragma region Miscellaneous
			/*	Turns on/off the Deblocking Filter */
			void SetDeblockingFilterEnabled(bool enabled);
			bool IsDeblockingFilterEnabled();

			#pragma region Motion Estimation
			/* Turns on/off half-pixel motion estimation */
			void SetHalfPixelMotionEstimationEnabled(bool enabled);
			bool IsHalfPixelMotionEstimationEnabled();

			/* Turns on/off quarter-pixel motion estimation */
			void SetQuarterPixelMotionEstimationEnabled(bool enabled);
			bool IsQuarterPixelMotionEstimationEnabled();
			#pragma endregion Motion Estimation
			#pragma endregion Miscellaneous

			#pragma region Experimental Properties
			// Get the maximum amount of MBps the encoder can output.
			uint32_t GetMaxMBPerSec();

			/* Coding Type */
			void SetCodingType(H264CodingType type);
			H264CodingType GetCodingType();

			void SetWaitForTaskEnabled(bool enabled);
			bool IsWaitForTaskEnabled();

			// Preanalysis Pass is AMDs version of Two-Pass hardware encoding.
			void SetPreAnalysisPassEnabled(bool enabled);
			bool IsPreAnalysisPassEnabled();

			// VBAQ = Variable Bitrate Average Quality?
			// - EanbleVBAQ (bool)
			void SetVBAQEnabled(bool enabled);
			bool IsVBAQEnabled();

			/*	Sets the headers insertion spacing */
			void SetHeaderInsertionSpacing(uint32_t spacing); // Similar to IDR Period, spacing (in frames) between headers.
			uint32_t GetHeaderInsertionSpacing();

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

			/*	Sets Maximum AU Size in bits */
			void SetMaximumAccessUnitSize(uint32_t size);
			uint32_t GetMaximumAccessUnitSize();

			void SetMaximumReferenceFrames(uint32_t frameCount);
			uint32_t GetMaximumReferenceFrames();

			void SetAspectRatio(uint32_t x, uint32_t y);
			std::pair<uint32_t, uint32_t> GetAspectRatio();

			#pragma region Group of Pictures
			void SetGOPSize(uint32_t gopSize);
			uint32_t GetGOPSize();

			void SetGOPAlignmentEnabled(bool enabled);
			bool IsGOPAlignementEnabled();
			#pragma endregion Group of Pictures

			#pragma region Intra Refresh
			// Macroblocks per Intra-Refresh Slot
			// Intra-Refresh Coding
			void SetIntraRefreshMacroblocksPerSlot(uint32_t macroblocks);
			uint32_t GetIntraRefreshMacroblocksPerSlot();

			// - IntraRefreshNumOfStripes (0 - INT_MAX)
			// Intra-Refresh Coding
			void SetIntraRefreshNumberOfStripes(uint32_t stripes);
			uint32_t GetIntraRefreshNumberOfStripes();
			#pragma endregion Intra Refresh

			#pragma region Slicing
			/*	Sets the number of slices per frame */
			void SetSlicesPerFrame(uint32_t slices);
			uint32_t GetSlicesPerFrame();

			// - SliceMode (1 - 2, Default is 1)
			void SetSliceMode(H264SliceMode mode);
			H264SliceMode GetSliceMode();

			// - MaxSliceSize (1 - INT_MAX)
			void SetMaximumSliceSize(uint32_t size);
			uint32_t GetMaximumSliceSize();

			// - SliceControlMode (0 - 3)
			void SetSliceControlMode(H264SliceControlMode mode);
			H264SliceControlMode GetSliceControlMode();

			// - SliceControlSize (0 - INT_MAX)
			void SetSliceControlSize(uint32_t size);
			uint32_t GetSliceControlSize();
			#pragma endregion Slicing

			// More:
			// - CodecId (H264 = 5, H264SVC = 8, 2xUNKNOWN)
			// - EngineType (Auto = 0, DX9 = 1, DX11 = 2, XVBA = 3)
			// - ConstraintSetFlags (0 - 255, 1 byte bitset?)
			// - LowLatencyInternal (bool)
			// - CommonLowLatencyInternal (bool)
			// - UniqueInstance (0 - INT_MAX)
			// - MultiInstanceMode (bool)
			// - MultiInstanceCurrentQueue (0 - 1)
			// - InstanceId (-1 - [# of Streams - 1])
			// - EncoderMaxInstances (1 - [# of Instances])
			#pragma endregion Experimental Properties

			#pragma endregion Properties

			// Threading
			private:
			static void InputThreadMain(Plugin::AMD::H264Encoder* p_this);
			void InputThreadLogic();
			static void OutputThreadMain(Plugin::AMD::H264Encoder* p_this);
			void OutputThreadLogic();
			inline amf::AMFSurfacePtr CreateSurfaceFromFrame(struct encoder_frame*& frame);

			#pragma region Members
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
			H264EncoderType m_EncoderType;
			H264MemoryType m_MemoryType;
			bool m_OpenCL;
			H264ColorFormat m_ColorFormat;
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
			H264ColorProfile m_ColorProfile;
			std::chrono::time_point<std::chrono::high_resolution_clock> m_LastQueueWarnMessageTime;

			#pragma endregion Members
		};
	}
}