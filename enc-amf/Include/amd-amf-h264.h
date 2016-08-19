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
#include <algorithm>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// Plugin
#include "plugin.h"
#include "amd-amf.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////

namespace Plugin {
	namespace AMD {
		// Internal Properties
		enum H264EncoderType {
			H264EncoderType_AVC,	// Advanced Video Coding
			H264EncoderType_SVC,	// Scalable Video Coding
		};
		enum H264MemoryType {
			H264MemoryType_Host,			// Host-Managed Memory
			H264MemoryType_DirectX9,		// DirectX9
			H264MemoryType_DirectX11,	// DirectX11
			H264MemoryType_OpenGL,		// OpenGL
			H264MemoryType_OpenCL,		// OpenCL
		};
		enum H264SurfaceFormat {
			H264SurfaceFormat_NV12,	// NV12
			H264SurfaceFormat_I420,	// YUV 4:2:0
			H264SurfaceFormat_RGBA,	// RGBA
		};

		// Static Properties
		enum H264Usage {
			H264Usage_Transcoding,
			H264Usage_UltraLowLatency,
			H264Usage_LowLatency,
			H264Usage_Webcam,			// For SVC
		};
		enum H264Profile {
			H264Profile_Baseline,
			H264Profile_Main,
			H264Profile_High,

			H264Profile_Unknown = -1,
		};
		enum H264ProfileLevel {
			H264ProfileLevel_10,
			H264ProfileLevel_11,
			H264ProfileLevel_12,
			H264ProfileLevel_13,
			H264ProfileLevel_20,
			H264ProfileLevel_21,
			H264ProfileLevel_22,
			H264ProfileLevel_30,
			H264ProfileLevel_31,
			H264ProfileLevel_32,
			H264ProfileLevel_40,
			H264ProfileLevel_41,
			H264ProfileLevel_42,
			H264ProfileLevel_50,
			H264ProfileLevel_51,
			H264ProfileLevel_52,

			H264ProfileLevel_Unknown = -1,
		};
		enum H264RateControlMethod {
			H264RateControlMethod_CQP,
			H264RateControlMethod_CBR,
			H264RateControlMethod_VBR,
			H264RateControlMethod_VBR_LAT,

			H264RateControlMethod_ConstantQP = H264RateControlMethod_CQP,
			H264RateControlMethod_ConstantQuantizationParameter = H264RateControlMethod_CQP,
			H264RateControlMethod_ConstantBitrate = H264RateControlMethod_CBR,
			H264RateControlMethod_VariableBitrate = H264RateControlMethod_VBR,
			H264RateControlMethod_LatencyConstrainedVariableBitrate = H264RateControlMethod_VBR_LAT,
		};
		enum H264BPicturesPattern {
			H264BPicturesPattern_None,
			H264BPicturesPattern_One,
			H264BPicturesPattern_Two,
			H264BPicturesPattern_Three,
		};
		enum H264ScanType {
			H264ScanType_Progressive,
			H264ScanType_Interlaced,
		};
		enum H264QualityPreset {
			H264QualityPreset_Balanced,
			H264QualityPreset_Quality,
			H264QualityPreset_Speed,
		};

		class H264VideoEncoder {
			//////////////////////////////////////////////////////////////////////////
			#pragma region Functions
			//////////////////////////////////////////////////////////////////////////
			private:
			static void InputThreadMain(Plugin::AMD::H264VideoEncoder* p_this);
			static void OutputThreadMain(Plugin::AMD::H264VideoEncoder* p_this);

			#pragma endregion Functions
			//////////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////////////
			#pragma region Initializer & Finalizer
			//////////////////////////////////////////////////////////////////////////
			public:
			H264VideoEncoder(H264EncoderType p_Type, H264MemoryType p_MemoryType);
			~H264VideoEncoder();
			#pragma endregion Initializer & Finalizer

			//////////////////////////////////////////////////////////////////////////
			#pragma region Methods
			//////////////////////////////////////////////////////////////////////////
			public:
			void Start();
			void Stop();
			bool SendInput(struct encoder_frame*& frame);
			void GetOutput(struct encoder_packet*& packet, bool*& received_packet);
			bool GetExtraData(uint8_t**& data, size_t*& size);
			void GetVideoInfo(struct video_scale_info*& vsi);

			// AMF Properties
			public:
			/// Internal
			void SetInputSurfaceFormat(H264SurfaceFormat p_Format);
			H264SurfaceFormat GetInputSurfaceFormat();
			void SetOutputSurfaceFormat(H264SurfaceFormat p_Format);
			H264SurfaceFormat GetOutputSurfaceFormat();
			/// Encoder Static Parameters
			/*	Selects the AMF Usage */
			void SetUsage(H264Usage usage);
			H264Usage GetUsage();
			/*	Selects the H.264 Profile */
			void SetProfile(H264Profile profile);
			H264Profile GetProfile();
			/*	Selects the H.264 Profile Level */
			void SetProfileLevel(H264ProfileLevel level);
			H264ProfileLevel GetProfileLevel();
			/*	The number of long-term references controlled by the user.
			 *
			 *	Remarks:
			 *  - When == 0, the encoder may or may not use LTRs during encoding.
			 *	- When >0, the user has control over all LTR.
			 *	- With user control of LTR, B-pictures and Intra-refresh features are not supported.
			 *	- The actual maximum number of LTRs allowed depends on H.264 Annex A Table A-1 Level limits, which defines dependencies between the H.264 Level number, encoding resolution, and DPB size. The DPB size limit impacts the maximum number of LTR allowed.
			 **/
			void SetMaxLTRFrames(uint32_t maximumLTRFrames);	// Long-Term Reference Frames. If 0, Encoder decides, if non-0 B-Pictures and Intra-Refresh are not supported.
			uint32_t GetMaxLTRFrames();
			/// Encoder Resolution Parameters
			void SetFrameSize(uint32_t width, uint32_t height);
			std::pair<uint32_t, uint32_t> GetFrameSize();
			/// Encoder Rate Control
			/*	Sets the target bitrate */
			void SetTargetBitrate(uint32_t bitrate);
			uint32_t GetTargetBitrate();
			/*	Sets the peak bitrate */
			void SetPeakBitrate(uint32_t bitrate);
			uint32_t GetPeakBitrate();
			/*	Selects the rate control method:
			 *	- CQP – Constrained QP,
			 *	- CBR - Constant Bitrate,
			 *	- VBR - Peak Constrained VBR,
			 *	- VBR_LAT - Latency Constrained VBR
			 *
			 *	Remarks:
			 *	- When SVC encoding is enabled, all Rate-control parameters (with some restrictions) can be configured differently for a particular SVC-layer. An SVC-layer is denoted by an index pair [SVC-Temporal Layer index][SVC-Quality Layer index]. E.g. The bitrate may be configured differently for SVC-layers [0][0] and [1][0].
			 *	- We restrict all SVC layers to have the same Rate Control method. Some RC parameters are not enabled with SVC encoding (e.g. all parameters related to B-pictures).
			 **/
			void SetRateControlMethod(H264RateControlMethod method);
			H264RateControlMethod GetRateControlMethod();
			/*	Enables skip frame for rate control */
			void SetRateControlSkipFrameEnabled(bool enabled);
			bool IsRateControlSkipFrameEnabled();
			/*	Sets the minimum QP */
			void SetMinimumQP(uint8_t qp);
			uint8_t GetMinimumQP();
			/*	Sets the maximum QP */
			void SetMaximumQP(uint8_t qp);
			uint8_t GetMaximumQP();
			/*	Sets the Constant QP for I-Pictures.
			 *
			 *	Remarks:
			 *	- Only available for CQP rate control method.
			 **/
			void SetIFrameQP(uint8_t qp);
			uint8_t GetIFrameQP();
			/*	Sets the Constant QP for P-Pictures.
			*
			*	Remarks:
			*	- Only available for CQP rate control method.
			**/
			void SetPFrameQP(uint8_t qp);
			uint8_t GetPFrameQP();
			/*	Sets the Constant QP for B-Pictures.
			*
			*	Remarks:
			*	- Only available for CQP rate control method.
			**/
			void SetBFrameQP(uint8_t qp);
			uint8_t GetBFrameQP();
			/*	Sets the Frame Rate numerator and denumerator */
			void SetFrameRate(uint32_t num, uint32_t den);
			std::pair<uint32_t, uint32_t> GetFrameRate();
			/*	Sets the VBV Buffer Size in bits */
			void SetVBVBufferSize(uint32_t size);
			uint32_t GetVBVBufferSize();
			/*	Sets the initial VBV Buffer Fullness */
			void SetInitialVBVBufferFullness(double_t fullness);
			double_t GetInitialVBVBufferFullness();
			/*	Enables/Disables contraints on QP variation within a picture to meet HRD requirement(s) */
			void SetEnforceHRDRestrictionsEnabled(bool enforce);
			bool IsEnforceHRDRestrictionsEnabled();
			/*	Enables/Disables filler data */
			void SetFillerDataEnabled(bool enabled);
			bool IsFillerDataEnabled();
			/*	Sets Maximum AU Size in bits */
			void SetMaximumAccessUnitSize(uint32_t size);
			uint32_t GetMaximumAccessUnitSize();
			/*	Selects the delta QP of non-reference B pictures with respect to I pictures */
			void SetBPictureDeltaQP(int8_t qp);
			int8_t GetBPictureDeltaQP();
			/*	Selects delta QP of reference B pictures with respect to I pictures */
			void SetReferenceBPictureDeltaQP(int8_t qp);
			int8_t GetReferenceBPictureDeltaQP();
			/// Encoder Picture Control Parameters
			/*	Sets the headers insertion spacing */
			void SetHeaderInsertionSpacing(uint32_t spacing); // Similar to IDR Period, spacing (in frames) between headers.
			uint32_t GetHeaderInsertionSpacing();
			/*	Sets IDR period. IDRPeriod= 0 turns IDR off */
			void SetIDRPeriod(uint32_t period);
			uint32_t GetIDRPeriod();
			/*	Turns on/off the de-blocking filter */
			void SetDeBlockingFilterEnabled(bool enabled);
			bool IsDeBlockingFilterEnabled();
			/*	Sets the number of intra-refresh macro-blocks per slot */
			void SetIntraRefreshMBsNumberPerSlot(uint32_t mbs);
			uint32_t GetIntraRefreshMBsNumberPerSlot();
			/*	Sets the number of slices per frame */
			void SetSlicesPerFrame(uint32_t slices);
			uint32_t GetSlicesPerFrame();
			/*	Sets the number of consecutive B-pictures in a GOP. BPicturesPattern = 0 indicates that B-pictures are not used */
			void SetBPicturesPattern(H264BPicturesPattern pattern);
			H264BPicturesPattern GetBPicturesPattern();
			/*	Enables or disables using B-pictures as references */
			void SetBReferenceEnabled(bool enabled);
			bool IsBReferenceEnabled();
			/// Encoder Miscellaneos Parameters
			/*	Selects progressive or interlaced scan */
			void SetScanType(H264ScanType scanType);
			H264ScanType GetScanType();
			/*	Selects the quality preset */
			void SetQualityPreset(H264QualityPreset preset);
			H264QualityPreset GetQualityPreset();
			/// Encoder Motion Estimation Parameters
			/*	Turns on/off half-pixel motion estimation */
			void SetHalfPixelMotionEstimationEnabled(bool enabled);
			bool IsHalfPixelMotionEstimationEnabled();
			/*	Turns on/off quarter-pixel motion estimation */
			void SetQuarterPixelMotionEstimationEnabled(bool enabled);
			bool IsQuarterPixelMotionEstimationEnabled();
			/// Encoder SVC Parameters (Only Webcam Usage)
			/*	Change the number of temporal enhancement layers. The maximum number allowed is set by the corresponding create parameter.
			 *	
			 *	Remarks:
			 *	- Actual modification of the number of temporal enhancement layers will be delayed until the start of the next temporal GOP.
			 *	- B-pictures and Intra-refresh features are not supported with SVC.
			 **/
			void SetNumberOfTemporalEnhancementLayers(uint32_t layers);
			uint32_t GetNumberOfTemporalEnhancementLayers();
			/*	Set the currently modified SVC Temporal Layer */
			void SetCurrentTemporalLayer(uint32_t layer);
			uint32_t GetCurrentTemporalLayer();
			/*	Set the currently modified SVC Temporal Quality Layer
			 *	
			 *	Remarks:
			 *	- Quality layers are not supported on VCE 1.0. “QL0” must be used for quality layers.
			 **/
			void SetCurrentTemporalQualityLayer(uint32_t layer);
			uint32_t GetCurrentTemporalQualityLayer();

			// Threading
			private:
			void InputThreadLogic();
			void OutputThreadLogic();

			// Utility
			inline amf::AMFSurfacePtr CreateSurfaceFromFrame(struct encoder_frame*& frame);

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
			amf::AMFComponentPtr m_AMFEncoder;

			// Internal Properties
			H264EncoderType m_EncoderType;
			H264MemoryType m_MemoryType;
			H264SurfaceFormat m_InputSurfaceFormat, m_OutputSurfaceFormat;
			std::pair<uint32_t, uint32_t> m_FrameSize, m_FrameRate;
			double_t m_FrameRateDivisor;

			// Threading
			bool m_IsStarted;
			struct ThreadData {
				std::vector<uint8_t> data;
				uint64_t frame;
				AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM type;
			};
			struct {
				std::thread thread;
				std::mutex mutex;
				std::condition_variable condvar;
				std::queue<amf::AMFSurfacePtr> queue;
			} m_ThreadedInput;
			struct {
				std::thread thread;
				std::mutex mutex;
				std::condition_variable condvar;
				std::queue<ThreadData> queue;
			} m_ThreadedOutput;

			// Static Buffers
			std::vector<uint8_t> m_PacketDataBuffer;
			std::vector<uint8_t> m_ExtraDataBuffer;

			#pragma endregion Members
			//////////////////////////////////////////////////////////////////////////

		};
	}
}