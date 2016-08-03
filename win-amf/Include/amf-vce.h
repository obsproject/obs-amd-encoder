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
		VCE_QUALITY_PRESET_SPEED,
		VCE_QUALITY_PRESET_BALANCED,
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
		VCE_RATE_CONTROL_CONSTRAINED_QP,			// Constrained QP, not to be mistaken with Constant QP (CQP)
		VCE_RATE_CONTROL_CBR,						// Constant Bitrate
		VCE_RATE_CONTROL_VBR_PEAK_CONSTRAINED,		// Variable Bitrate, Peak Constrained
		VCE_RATE_CONTROL_VBR_LATENCY_CONSTRAINED	// Variable Bitrate, Latency Constrained
	};

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
		// Properties
		//////////////////////////////////////////////////////////////////////////
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
		void EnableFrameSkipping(bool);
		bool IsFrameSkippingEnabled();
		void EnableEnforceHRD(bool);
		bool IsEnforceHRDEnabled();
		void EnableFillerData(bool);
		bool IsFillerDataEnabled();

		//////////////////////////////////////////////////////////////////////////
		// Core Functions
		//////////////////////////////////////////////////////////////////////////
		void Start();
		void Stop();
		bool SendInput(struct encoder_frame*&);
		void GetOutput(struct encoder_packet*&, bool*&);
		bool GetExtraData(uint8_t**&, size_t*&);
		void GetVideoInfo(struct video_scale_info*&);

		//////////////////////////////////////////////////////////////////////////
		// Internal-only, do not expose.
		//////////////////////////////////////////////////////////////////////////
		private:
		bool m_isStarted;

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
		
		// Dynamic Properties
		/// Rate Control Method
		VCE_Rate_Control_Method m_rateControlMethod;
		bool m_skipFrameEnabled;
		bool m_enforceHRDEnabled;

		// AMF
		amf::AMFContextPtr m_AMFContext;
		amf::AMFComponentPtr m_AMFEncoder;

		// Buffers
		std::vector<uint8_t> m_FrameDataBuffer;
		std::vector<uint8_t> m_PacketDataBuffer;
		std::vector<uint8_t> m_ExtraDataBuffer;

		//////////////////////////////////////////////////////////////////////////
		// Logging & Exception Helpers
		//////////////////////////////////////////////////////////////////////////
		static void throwAMFError(const char* errorMsg, AMF_RESULT res);
		static void formatAMFError(std::vector<char>* buffer, const char* format, AMF_RESULT res);

		template<typename _T>
		static void formatAMFErrorAdvanced(std::vector<char>* buffer, const char* format, _T other, AMF_RESULT res);
		template<typename _T>
		static void throwAMFErrorAdvanced(const char* errorMsg, _T other, AMF_RESULT res);
	};
}