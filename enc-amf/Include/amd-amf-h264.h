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
		};
		enum H264RateControlMethod {
			H264RateControlMethod_CQP,
			H264RateControlMethod_CBR,
			H264RateControlMethod_VBR,
			H264RateControlMethod_VBR_LAT,

			H264RateControlMethod_ConstrainedQP = H264RateControlMethod_CQP,
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
			// Initializer & Finalizer
			//////////////////////////////////////////////////////////////////////////
			public:
			H264VideoEncoder(H264EncoderType p_Type, H264MemoryType p_MemoryType);
			~H264VideoEncoder();

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
			void SetUsage(H264Usage usage);
			H264Usage GetUsage();
			void SetProfile(H264Profile profile);
			H264Profile GetProfile();
			void SetProfileLevel(H264ProfileLevel level);
			H264ProfileLevel GetProfileLevel();
			void SetMaxLTRFrames(uint32_t maximumLTRFrames);	// Long-Term Reference Frames. If 0, Encoder decides, if non-0 B-Pictures and Intra-Refresh are not supported.
			uint32_t GetMaxLTRFrames();
			/// Encoder Resolution Parameters
			void SetFrameSize(uint32_t width, uint32_t height);
			std::pair<uint32_t, uint32_t> GetFrameSize();
			/// Encoder Rate Control
			void SetTargetBitrate(uint32_t bitrate);
			uint32_t GetTargetBitrate();
			void SetPeakBitrate(uint32_t bitrate);
			uint32_t GetPeakBitrate();
			void SetRateControlMethod(H264RateControlMethod method);
			H264RateControlMethod GetRateControlMethod();
			void SetRateControlSkipFrameEnabled(bool enabled);
			bool IsRateControlSkipFrameEnabled();
			void SetMinimumQP(uint8_t qp);
			uint8_t GetMinimumQP();
			void SetMaximumQP(uint8_t qp);
			uint8_t GetMaximumQP();
			void SetIFrameQP(uint8_t qp);
			uint8_t GetIFrameQP();
			void SetPFrameQP(uint8_t qp);
			uint8_t GetPFrameQP();
			void SetBFrameQP(uint8_t qp);
			uint8_t GetBFrameQP();
			void SetFrameRate(uint32_t num, uint32_t den);
			std::pair<uint32_t, uint32_t> GetFrameRate();
			void SetVBVBufferSize(uint32_t size);
			uint32_t GetVBVBufferSize();
			void SetVBVBufferFullness(double_t fullness);
			double_t GetVBVBufferFullness();
			void SetEnforceHRD(bool enforce);
			bool GetEnforceHRD();
			void SetMaxAUSize(uint32_t size);
			uint32_t GetMaxAUSize();
			void SetBPictureDeltaQP(int8_t qp);
			int8_t GetBPictureDeltaQP();
			void SetReferenceBPictureDeltaQP(int8_t qp);
			int8_t GetReferenceBPictureDeltaQP();
			/// Encoder Picture Control Parameters
			void SetHeaderInsertionSpacing(uint32_t spacing); // Similar to IDR Period, spacing (in frames) between headers.
			uint16_t GetHeaderInsertionSpacing();
			void SetIDRPeriod(uint32_t period);
			uint32_t GetIDRPeriod();
			void SetDeBlockingFilterEnabled(bool enabled);
			bool IsDeBlockingFilterEnabled();
			void SetIntraRefreshMBsNumberPerSlot(uint32_t mbs);
			uint32_t GetIntraRefreshMBsNumberPerSlot();
			void SetSlicesPerFrame(uint32_t slices);
			uint32_t GetSlicesPerFrame();
			void SetBPicturesPattern(H264BPicturesPattern pattern);
			H264BPicturesPattern GetBPicturesPattern();
			void SetBReferenceEnabled(bool enabled);
			bool IsBReferenceEnabled();
			/// Encoder Miscellaneos Parameters
			void SetScanType(H264ScanType scanType);
			H264ScanType GetScanType();
			void SetQualityPreset(H264QualityPreset preset);
			H264QualityPreset GetQualityPreset();
			/// Encoder Motion Estimation Parameters
			void SetHalfPixelMotionEstimation(bool enabled);
			void SetQuarterPixelMotionEstimation(bool enabled);

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