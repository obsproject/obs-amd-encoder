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
#include <stdint.h>
#include <inttypes.h>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

// AMF
#include "AMD-Media-SDK/1.1/inc/ErrorCodes.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/Component.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderVCE.h"

// OBS
#include "OBS-Studio/libobs/obs-module.h"
#include "OBS-Studio/libobs/obs-encoder.h"

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////

#define AMF_H264(x) ("AMF." ## x)
#define AMF_VCE_T(x) obs_module_text(AMF_TEXT(x))

#define AMF_VCE_PROPERTY_FRAMEINDEX	L"OBSFrameIndex"
#define AMF_VCE_MAX_QUEUED_FRAMES	180

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
namespace AMFEncoder {
	enum VCE_Encoder_Type {
		VCE_ENCODER_TYPE_AVC, // Advanced Video Coding
		VCE_ENCODER_TYPE_SVC, // Scalable Video Coding (Broken: ->SetFrameRate() won't work)
		VCE_ENCODER_TYPE_HEVC, // High-Efficiency Video Coding (Experimental)
	};

	enum VCE_Memory_Type {
		VCE_MEMORY_TYPE_HOST,      // Use Host-Managed Memory
		VCE_MEMORY_TYPE_DIRECTX11, // Copy straight from DirectX11
		VCE_MEMORY_TYPE_OPENGL     // Copy straight from OpenGL
	};

	enum VCE_Surface_Format {
		VCE_SURFACE_FORMAT_NV12, // NV 12
		VCE_SURFACE_FORMAT_I420, // YUV 420 Progressive
		VCE_SURFACE_FORMAT_I444, // Not supported by SDK?
		VCE_SURFACE_FORMAT_RGB   // RGBA
	};

	enum VCE_Usage {
		VCE_USAGE_TRANSCODING,
		VCE_USAGE_ULTRA_LOW_LATENCY,
		VCE_USAGE_LOW_LATENCY,
		VCE_USAGE_WEBCAM,
	};

	enum VCE_Quality_Preset {
		VCE_QUALITY_PRESET_BALANCED,
		VCE_QUALITY_PRESET_SPEED,
		VCE_QUALITY_PRESET_QUALITY
	};

	enum VCE_Profile {
		VCE_PROFILE_BASELINE,
		VCE_PROFILE_MAIN,
		VCE_PROFILE_HIGH,
	};

	enum VCE_Profile_Level {
		VCE_PROFILE_LEVEL_10,
		VCE_PROFILE_LEVEL_11,
		VCE_PROFILE_LEVEL_12,
		VCE_PROFILE_LEVEL_13,
		VCE_PROFILE_LEVEL_20,
		VCE_PROFILE_LEVEL_21,
		VCE_PROFILE_LEVEL_22,
		VCE_PROFILE_LEVEL_30,
		VCE_PROFILE_LEVEL_31,
		VCE_PROFILE_LEVEL_32,
		VCE_PROFILE_LEVEL_40,
		VCE_PROFILE_LEVEL_41,
		VCE_PROFILE_LEVEL_42,
		VCE_PROFILE_LEVEL_50,
		VCE_PROFILE_LEVEL_51,
		VCE_PROFILE_LEVEL_52, // Experimental
	};

	enum VCE_ScanType {
		VCE_SCANTYPE_PROGRESSIVE,
		VCE_SCANTYPE_INTERLACED
	};

	enum VCE_Rate_Control_Method {
		VCE_RATE_CONTROL_CONSTRAINED_QUANTIZATION_PARAMETER,	// Constrained Quantization Parameter
		VCE_RATE_CONTROL_CONSTANT_BITRATE,						// Constant Bitrate
		VCE_RATE_CONTROL_VARIABLE_BITRATE_PEAK_CONSTRAINED,		// Variable Bitrate, Peak Constrained
		VCE_RATE_CONTROL_VARIABLE_BITRATE_LATENCY_CONSTRAINED	// Variable Bitrate, Latency Constrained
	};

	#ifdef THREADED
	struct BaseThreadedData {
		std::vector<uint8_t> data;
		uint64_t frameIndex;
	};
	struct InputThreadFrame : public BaseThreadedData {};
	struct OutputThreadPacket : public BaseThreadedData {
		AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM dataType;
	};
	#endif

	/**
	 * Class for the actual AMF Encoder.
	 */
	class VCE {
		public:

		//////////////////////////////////////////////////////////////////////////
		// Initializer & Finalizer
		//////////////////////////////////////////////////////////////////////////
		VCE(VCE_Encoder_Type);
		~VCE();

		//////////////////////////////////////////////////////////////////////////
		#pragma region Methods
		//////////////////////////////////////////////////////////////////////////

		// Core Methods
		void Start();
		void Stop();
		bool SendInput(struct encoder_frame*&);
		void GetOutput(struct encoder_packet*&, bool*&);
		bool GetExtraData(uint8_t**&, size_t*&);
		void GetVideoInfo(struct video_scale_info*&);

		// Utility Methods
		private:
		amf::AMFSurfacePtr inline CreateSurfaceFromFrame(struct encoder_frame*& frame);

		// Threading
		#ifdef THREADED
		static void InputThreadMain(VCE* cls);
		static void OutputThreadMain(VCE* cls);
		void InputThreadMethod();
		void OutputThreadMethod();
		#endif

		// Logging & Exception Helpers
		static void formatAMFError(std::vector<char>* buffer, const char* format, AMF_RESULT res);
		template<typename _T> static void formatAMFErrorAdvanced(std::vector<char>* buffer, const char* format, _T other, AMF_RESULT res);
		static void throwAMFError(const char* errorMsg, AMF_RESULT res);
		template<typename _T> static void throwAMFErrorAdvanced(const char* errorMsg, _T other, AMF_RESULT res);

		// Static Properties
		/// Memory Type & Surface Format
		void SetMemoryType(VCE_Memory_Type);
		VCE_Memory_Type GetMemoryType();
		void SetSurfaceFormat(VCE_Surface_Format);
		VCE_Surface_Format GetSurfaceFormat();
		/// Encoder Parameters
		void SetUsage(VCE_Usage);
		VCE_Usage GetUsage();
		void SetQualityPreset(VCE_Quality_Preset);
		VCE_Quality_Preset GetQualityPreset();
		void SetProfile(VCE_Profile);
		VCE_Profile GetProfile();
		void SetProfileLevel(VCE_Profile_Level);
		VCE_Profile_Level GetProfileLevel();
		void SetMaxLTRFrames(uint32_t); // Long Term Reference Frames (for Hierarchical B-Picture-Based Video Coding)
		uint32_t GetMaxLTRFrames();
		void SetScanType(VCE_ScanType);
		VCE_ScanType GetScanType();
		/// Frame Size & Rate
		void SetFrameSize(std::pair<uint32_t, uint32_t>&);
		std::pair<uint32_t, uint32_t> GetFrameSize();
		void SetFrameRate(std::pair<uint32_t, uint32_t>&);
		std::pair<uint32_t, uint32_t> GetFrameRate();

		// Dynamic Properties
		/// Rate Control
		void SetRateControlMethod(VCE_Rate_Control_Method);
		VCE_Rate_Control_Method GetRateControlMethod();
		void SetFrameSkippingEnabled(bool);
		bool IsFrameSkippingEnabled();
		/// Other
		void SetFillerDataEnabled(bool);
		bool IsFillerDataEnabled();
		void SetEnforceHRDEnabled(bool); // Hierarchical Reference Decoder
		bool IsEnforceHRDEnabled(); // Hierarchical Reference Decoder
		/// Video Coding Settings
		void SetGOPSize(uint32_t); // Group of Pictures
		uint32_t GetGOPSize(); // Group of Pictures
		void SetVBVBufferSize(uint32_t); // Video Buffering Verifier Buffer
		uint32_t GetVBVBufferSize(); // Video Buffering Verifier Buffer
		void SetInitialVBVBufferFullness(double_t); // Video Buffering Verifier Buffer Fullness
		double_t GetInitialVBVBufferFullness(); // Video Buffering Verifier Buffer Fullness
		void SetMaximumAccessUnitSize(uint32_t);
		uint32_t GetMaximumAccessUnitSize();
		/// B-Picture Stuff
		void SetBPictureDeltaQP(int8_t);
		int8_t GetBPictureDeltaQP();
		void SetReferenceBPictureDeltaQP(int8_t);
		int8_t GetReferenceBPictureDeltaQP();
		/// Rate Control: CQP
		void SetMinimumQP(uint8_t);
		uint8_t GetMinimumQP();
		void SetMaximumQP(uint8_t);
		uint8_t GetMaximumQP();
		void SetIFrameQP(uint8_t);
		uint8_t GetIFrameQP();
		void SetPFrameQP(uint8_t);
		uint8_t GetPFrameQP();
		void SetBFrameQP(uint8_t);
		uint8_t GetBFrameQP();
		/// Rate Control: CBR, VBR
		void SetTargetBitrate(uint32_t);
		uint32_t GetTargetBitrate();
		void SetPeakBitrate(uint32_t);
		uint32_t GetPeakBitrate();
		/// Picture Control Properties
		void SetHeaderInsertionSpacing(uint16_t);
		uint16_t GetHeaderInsertionSpacing();
		void SetNumberOfBPictures(uint8_t);
		uint8_t GetNumberOfBPictures();
		void SetDeblockingFilterEnabled(bool);
		bool IsDeblockingFilterEnabled();
		void SetReferenceToBFrameEnabled(bool);
		bool IsReferenceToBFrameEnabled();
		void SetIDRPeriod(uint32_t);
		uint32_t GetIDRPeriod();
		void SetInfraRefreshMBsPerSlotInMacroblocks(uint32_t);
		uint32_t GetInfraRefreshMBsPerSlotInMacroblocks();
		void SetNumberOfSlicesPerFrame(uint32_t);
		uint32_t GetNumberOfSlicesPerFrame();
		/// Motion Estimation
		void SetHalfPixelMotionEstimationEnabled(bool);
		bool GetHalfPixelMotionEstimationEnabled();
		void SetQuarterPixelMotionEstimationEnabled(bool);
		bool GetQuarterPixelMotionEstimationEnabled();
		/// Other
		void SetNumberOfTemporalEnhancementLayers(uint32_t);
		uint32_t GetNumberOfTemporalEnhancementLayers();

		#pragma endregion Methods
		//////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////
		#pragma region Members
		//////////////////////////////////////////////////////////////////////////
		
		// Flags
		bool m_isStarted;

		// Buffers
		std::vector<uint8_t> m_FrameDataBuffer;
		std::vector<uint8_t> m_PacketDataBuffer;
		std::vector<uint8_t> m_ExtraDataBuffer;

		// Binding: AMF
		amf::AMFContextPtr m_AMFContext;
		amf::AMFComponentPtr m_AMFEncoder;

		// Threading
		#ifdef THREADED
		std::thread m_InputThread, m_OutputThread;
		struct InputThreadData {
			std::mutex m_mutex;
			std::condition_variable m_condVar;
			std::queue<amf::AMFSurfacePtr> m_queue;
		} m_InputThreadData;

		struct OutputThreadData {
			std::mutex m_mutex;
			std::condition_variable m_condVar;
			std::queue<OutputThreadPacket> m_queue;
		} m_OutputThreadData;
		#endif

		// Static Properties
		/// Encoder Type, Memory Type & Surface Format
		VCE_Encoder_Type m_encoderType;
		VCE_Memory_Type m_memoryType;
		VCE_Surface_Format m_surfaceFormat;
		/// Encoder Parameters
		VCE_Usage m_usage;
		VCE_Quality_Preset m_qualityPreset;
		VCE_Profile m_profile;
		VCE_Profile_Level m_profileLevel;
		uint32_t m_maxLTRFrames;
		VCE_ScanType m_scanType;
		/// Frame Size & Rate
		std::pair<uint32_t, uint32_t> m_frameSize;
		std::pair<uint32_t, uint32_t> m_frameRate;
		double_t m_frameRateDiv;

		// Dynamic Properties
		/// Rate Control Method
		VCE_Rate_Control_Method m_rateControlMethod;
		bool m_skipFrameEnabled;
		/// Other
		bool m_fillerDataEnabled;
		bool m_enforceHRDEnabled;
		/// Video Coding Settings
		uint32_t m_GOPSize;
		uint32_t m_VBVBufferSize;
		double_t m_initalVBVBufferFullness;
		uint32_t m_maximumAccessUnitSize;
		/// B-Picture Stuff
		uint8_t m_BPictureDeltaQP;
		uint8_t m_refBPictureDeltaQP;
		/// Rate Control: CQP
		uint8_t m_minimumQP, m_maximumQP;
		uint8_t m_IFrameQP, m_PFrameQP, m_BFrameQP;
		/// Rate Control: CBR, VBR
		uint32_t m_targetBitrate, m_peakBitrate;
		/// Picture Control Properties
		uint16_t m_headerInsertionSpacing;
		uint8_t m_numberOfBPictures;
		bool m_deblockingFilterEnabled;
		bool m_referenceToBFrameEnabled;
		uint32_t m_IDRPeriod;
		uint32_t m_intraRefreshMBPerSlotInMacrobocks;
		uint32_t m_numberOfSlicesPerFrame;
		/// Motion Estimation
		bool m_halfPixelMotionEstimationEnabled;
		bool m_quarterPixelMotionEstimationEnabled;
		/// Other
		uint32_t m_numberOfTemporalEnhancementLayers;

		#pragma endregion Members
		//////////////////////////////////////////////////////////////////////////
	};
}