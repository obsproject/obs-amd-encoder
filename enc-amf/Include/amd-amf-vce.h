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

			VCEMemoryType_Auto = -1,	// Auto-Detect
		};
		enum VCEComputeType {
			VCEComputeType_None, // Default
			VCEComputeType_OpenCL, // OpenCL
		};
		enum VCESurfaceFormat {
			// 4:2:0 Formats
			VCESurfaceFormat_NV12,	// NV12
			VCESurfaceFormat_I420,	// YUV 4:2:0
			// 4:2:2 Formats
			VCESurfaceFormat_YUY2,
			// Uncompressed
			VCESurfaceFormat_BGRA,	// ARGB
			VCESurfaceFormat_RGBA,	// RGBA
			// Other
			VCESurfaceFormat_GRAY,
		};

		// Static Properties
		enum VCEUsage {
			VCEUsage_Transcoding,
			VCEUsage_UltraLowLatency,
			VCEUsage_LowLatency,
			VCEUsage_Webcam,			// For SVC
		};
		enum VCEProfile {
			VCEProfile_Baseline = 66,
			VCEProfile_Main = 77,
			VCEProfile_High = 100,

			VCEProfile_Unknown = -1,
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

			VCEProfileLevel_Unknown = -1,
		};
		enum VCERateControlMethod {
			VCERateControlMethod_ConstantQP,
			VCERateControlMethod_ConstantBitrate,
			VCERateControlMethod_VariableBitrate_PeakConstrained,
			VCERateControlMethod_VariableBitrate_LatencyConstrained,
		};
		enum VCEBPicturePattern {
			VCEBPicturePattern_None,
			VCEBPicturePattern_One,
			VCEBPicturePattern_Two,
			VCEBPicturePattern_Three,
		};
		enum VCEScanType {
			VCEScanType_Progressive,
			VCEScanType_Interlaced,
		};
		enum VCEQualityPreset {
			VCEQualityPreset_Speed,
			VCEQualityPreset_Balanced,
			VCEQualityPreset_Quality,
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
			VCEEncoder(VCEEncoderType p_Type, VCESurfaceFormat p_SurfaceFormat = VCESurfaceFormat_NV12, VCEMemoryType p_MemoryType = VCEMemoryType_Auto, VCEComputeType p_ComputeType = VCEComputeType_None);
			~VCEEncoder();
			#pragma endregion Initializer & Finalizer

			//////////////////////////////////////////////////////////////////////////
			#pragma region Methods
			//////////////////////////////////////////////////////////////////////////
			public:
			void Start();
			void Restart();
			void Stop();
			bool SendInput(struct encoder_frame* frame);
			void GetOutput(struct encoder_packet* packet, bool* received_packet);
			bool GetExtraData(uint8_t**& data, size_t*& size);
			void GetVideoInfo(struct video_scale_info*& vsi);

			// Threading
			private:
			void InputThreadLogic();
			void OutputThreadLogic();

			// Utility
			inline amf::AMFSurfacePtr CreateSurfaceFromFrame(struct encoder_frame*& frame);

			#pragma region AMF Properties
			public:
			void LogProperties();

			/*	Usage Type
			 *	Sets up the entire encoder to a specific set of properties.
			 *	Must be called first if you want to override properties. */
			void SetUsage(VCEUsage usage);
			VCEUsage GetUsage();

			/*	Quality Preset */
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
			 *	- With user control of LTR, B-pictures and Intra-refresh features are not supported.
			 *	- The actual maximum number of LTRs allowed depends on H.264 Annex A Table A-1 Level limits, which defines dependencies between the H.264 Level number, encoding resolution, and DPB size. The DPB size limit impacts the maximum number of LTR allowed.
			 **/
			void SetMaximumLongTermReferenceFrames(uint32_t maximumLTRFrames);	// Long-Term Reference Frames. If 0, Encoder decides, if non-0 B-Pictures and Intra-Refresh are not supported.
			uint32_t GetMaximumLongTermReferenceFrames();

			/*	Output Resolution */
			void SetFrameSize(uint32_t width, uint32_t height);
			std::pair<uint32_t, uint32_t> GetFrameSize();

			/// Encoder Rate Control
			/*	Selects the rate control method:
			 *	- CQP - Constrained QP,
			 *	- CBR - Constant Bitrate,
			 *	- VBR - Peak Constrained VBR,
			 *	- VBR_LAT - Latency Constrained VBR
			 *
			 *	Remarks:
			 *	- When SVC encoding is enabled, all Rate-control parameters (with some restrictions) can be configured differently for a particular SVC-layer. An SVC-layer is denoted by an index pair [SVC-Temporal Layer index][SVC-Quality Layer index]. E.g. The bitrate may be configured differently for SVC-layers [0][0] and [1][0].
			 *	- We restrict all SVC layers to have the same Rate Control method. Some RC parameters are not enabled with SVC encoding (e.g. all parameters related to B-pictures).
			 **/
			void SetRateControlMethod(VCERateControlMethod method);
			VCERateControlMethod GetRateControlMethod();
			/*	Enables skip frame for rate control */
			void SetRateControlSkipFrameEnabled(bool enabled);
			bool IsRateControlSkipFrameEnabled();
			/*	Sets the minimum QP */
			void SetMinimumQP(uint8_t qp);
			uint8_t GetMinimumQP();
			/*	Sets the maximum QP */
			void SetMaximumQP(uint8_t qp);
			uint8_t GetMaximumQP();
			/*	Sets the target bitrate */
			void SetTargetBitrate(uint32_t bitrate);
			uint32_t GetTargetBitrate();
			/*	Sets the peak bitrate */
			void SetPeakBitrate(uint32_t bitrate);
			uint32_t GetPeakBitrate();
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
			void SetVBVBufferAutomatic(float_t strictness);
			uint32_t GetVBVBufferSize();
			/*	Sets the initial VBV Buffer Fullness */
			void SetInitialVBVBufferFullness(double_t fullness);
			double_t GetInitialVBVBufferFullness();
			/*	Enables/Disables constraints on QP variation within a picture to meet HRD requirement(s) */
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
			void SetBPicturePattern(VCEBPicturePattern pattern);
			VCEBPicturePattern GetBPicturePattern();
			/*	Enables or disables using B-pictures as references */
			void SetBPictureReferenceEnabled(bool enabled);
			bool IsBPictureReferenceEnabled();
			/// Encoder Miscellaneous Parameters
			/*	Selects progressive or interlaced scan */
			void SetScanType(VCEScanType scanType);
			VCEScanType GetScanType();
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

			/// Hidden Parameters
			void SetNominalRange(bool enabled);
			bool GetNominalRange();
			void SetWaitForTask(bool enabled);
			bool GetWaitForTask();
			void SetGOPSize(uint32_t size);
			uint32_t GetGOPSize();
			void SetAspectRatio(uint32_t x, uint32_t y);
			std::pair<uint32_t, uint32_t> GetAspectRatio();
			void SetCABACEnabled(bool enabled);
			bool IsCABACEnabled();
			//void SetQualityEnhancementMode(uint32_t qualityEnhancementMode);
			//uint32_t GetQualityEnhancementMode();

			// VCE Parameters
			// - SliceControlMode: AMF_VIDEO_ENCODER_SLICE_CTRL_MODE_MB_ROW, AMF_VIDEO_ENCODER_SLICE_CTRL_MODE_MB

			// HEVC Parameters
			// - MinQP_I, MaxQP_I
			// - MinQP_P, MaxQP_P
			// - QPCBOFFSET, QPCROFFSET
			// - GOPPerIDR
			// - GOPSizeMin, GOPSizeMax
			// - EnableGOPAlignment

			#pragma endregion AMF Properties

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
			amf::AMFComputePtr m_AMFCompute;

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
			VCEComputeType m_ComputeType;
			VCESurfaceFormat m_SurfaceFormat;
			bool m_Flag_IsStarted,
				m_Flag_Threading;
			std::pair<uint32_t, uint32_t> m_FrameSize,
				m_FrameRate;
			double_t m_FrameRateDivisor,
				m_FrameRateReverseDivisor;
			size_t m_InputQueueLimit,
				m_InputQueueLastSize;
			uint32_t m_TimerPeriod;
			
			#pragma endregion Members
			//////////////////////////////////////////////////////////////////////////
		};
	}
}