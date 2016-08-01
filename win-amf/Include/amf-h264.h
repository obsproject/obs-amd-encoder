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
// System
#include <exception>
#include <stdexcept>
#include <vector>

// Plugin
#include "win-amf.h"

// AMF
#include "AMD-Media-SDK/1.1/inc/ErrorCodes.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/Component.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderVCE.h"

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////

#define AMF_H264(x) ("AMF." ## x)
#define AMF_H264_T(x) obs_module_text(AMF_TEXT(x))

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
namespace AMFEncoder {
	enum H264_Encoder_Type {
		H264_ENCODER_TYPE_AVC, // Advanced Video Coding
		H264_ENCODER_TYPE_SVC, // Scalable Video Coding
		H264_ENCODER_TYPE_HEVC, // High-Efficiency Video Coding (Experimental)
	};

	enum H264_Memory_Type {
		H264_MEMORY_TYPE_HOST,      // Use Host-Managed Memory
		H264_MEMORY_TYPE_DIRECTX11, // Copy straight from DirectX11
		H264_MEMORY_TYPE_OPENGL     // Copy straight from OpenGL
	};

	enum H264_Surface_Format {
		H264_SURFACE_FORMAT_NV12, // NV 12
		H264_SURFACE_FORMAT_I420, // YUV 420 Progressive
		H264_SURFACE_FORMAT_I444, // Not supported by SDK
		H264_SURFACE_FORMAT_RGB   // RGBA
	};

	enum H264_Usage {
		H264_USAGE_TRANSCODING,
		H264_USAGE_ULTRA_LOW_LATENCY,
		H264_USAGE_LOW_LATENCY,
		H264_USAGE_WEBCAM,
	};

	enum H264_Quality_Preset {
		H264_QUALITY_PRESET_SPEED,
		H264_QUALITY_PRESET_BALANCED,
		H264_QUALITY_PRESET_QUALITY
	};

	enum H264_Profile {
		H264_PROFILE_BASELINE,
		H264_PROFILE_MAIN,
		H264_PROFILE_HIGH,
	};

	enum H264_Profile_Level {
		H264_PROFILE_LEVEL_1,
		H264_PROFILE_LEVEL_11,
		H264_PROFILE_LEVEL_12,
		H264_PROFILE_LEVEL_13,
		H264_PROFILE_LEVEL_2,
		H264_PROFILE_LEVEL_21,
		H264_PROFILE_LEVEL_22,
		H264_PROFILE_LEVEL_3,
		H264_PROFILE_LEVEL_31,
		H264_PROFILE_LEVEL_32,
		H264_PROFILE_LEVEL_4,
		H264_PROFILE_LEVEL_41,
		H264_PROFILE_LEVEL_42,
		H264_PROFILE_LEVEL_5,
		H264_PROFILE_LEVEL_51,
		H264_PROFILE_LEVEL_52,
	};

	enum H264_ScanType {
		H264_SCANTYPE_PROGRESSIVE,
		H264_SCANTYPE_INTERLACED
	};

	/**
	* Class for the actual AMF Encoder.
	*
	*/
	class VCE {
		public:

		//////////////////////////////////////////////////////////////////////////
		// Initializer & Finalizer
		//////////////////////////////////////////////////////////////////////////
		VCE(H264_Encoder_Type);
		~VCE();

		//////////////////////////////////////////////////////////////////////////
		// Properties
		//////////////////////////////////////////////////////////////////////////
		// Pre-Start Properties
		void SetMemoryType(H264_Memory_Type);
		H264_Memory_Type GetMemoryType();
		void SetSurfaceFormat(H264_Surface_Format);
		H264_Surface_Format GetSurfaceFormat();
		void SetUsage(H264_Usage);
		H264_Usage GetUsage();
		void SetQualityPreset(H264_Quality_Preset);
		H264_Quality_Preset GetQualityPreset();
		void SetProfile(H264_Profile);
		H264_Profile GetProfile();
		void SetProfileLevel(H264_Profile_Level);
		H264_Profile_Level GetProfileLevel();
		void SetMaxOfLTRFrames(uint32_t);
		uint32_t GetMaxOfLTRFrames();
		void SetScanType(H264_ScanType);
		H264_ScanType GetScanType();
		void SetFrameSize(std::pair<uint32_t, uint32_t>&);
		std::pair<uint32_t, uint32_t> GetFrameSize();
		void SetFrameRate(std::pair<uint32_t, uint32_t>&);
		std::pair<uint32_t, uint32_t> GetFrameRate();

		//////////////////////////////////////////////////////////////////////////
		// Core Functions
		//////////////////////////////////////////////////////////////////////////
		bool Start();
		void Stop();
		void SendInput(struct encoder_frame*&);
		void GetOutput(struct encoder_packet*&, bool*&);
		
		//////////////////////////////////////////////////////////////////////////
		// Internal-only, do not expose.
		//////////////////////////////////////////////////////////////////////////
		private:
		bool m_isStarted;

		// Pre-Startup
		H264_Encoder_Type m_encoderType;
		H264_Memory_Type m_memoryType;
		H264_Surface_Format m_surfaceFormat;
		H264_Usage m_usage;
		H264_Quality_Preset m_qualityPreset;
		H264_Profile m_profile;
		H264_Profile_Level m_profileLevel;

		// AMF
		amf::AMFContextPtr m_AMFContext;
		amf::AMFComponentPtr m_AMFEncoder;

		static void throwAMFError(const char* errorMsg, AMF_RESULT res);
		static void tempFormatAMFError(std::vector<char>* buffer, const char* format, AMF_RESULT res);
	};

}