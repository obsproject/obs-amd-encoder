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
#include "AMD-Media-SDK/1.1/inc/amf/components/ComponentCaps.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderCaps.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderVCECaps.h"

// Plugin
#include "amf-vce.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
namespace AMFEncoder {
	class VCE_Capabilities {
		//////////////////////////////////////////////////////////////////////////
		// Singleton
		//////////////////////////////////////////////////////////////////////////
		static VCE_Capabilities* instance;

		public:
		static VCE_Capabilities* getInstance();

		//////////////////////////////////////////////////////////////////////////
		// Class
		//////////////////////////////////////////////////////////////////////////
		public:
		// Structure
		struct EncoderCaps {
			amf::AMF_ACCELERATION_TYPE acceleration_type;
			uint32_t maxStreamCount;
			uint32_t maxBitrate;

			struct H264 {
				uint32_t maxNumOfTemporalLayers;
				amf::H264EncoderJobPriority maxSupportedJobPriority;
				bool isBPictureSupported;
				bool isFixedByteSliceModeSupported;
				bool canOutput3D;
				uint32_t minReferenceFrames, maxReferenceFrames;

				std::vector<AMF_VIDEO_ENCODER_PROFILE_ENUM> profiles;
				std::vector<uint32_t> levels;
				std::vector<AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM> rateControlMethods;
			} h264;

			struct IOCaps {
				int32_t minWidth, maxWidth;
				int32_t minHeight, maxHeight;
				bool isInterlacedSupported;
				uint32_t verticalAlignment;

				std::vector<std::pair<amf::AMF_SURFACE_FORMAT,bool>> formats;
				std::vector<std::pair<amf::AMF_MEMORY_TYPE, bool>> memoryTypes;
			} input, output;
		} m_AVCCaps, m_SVCCaps;


		VCE_Capabilities();
		~VCE_Capabilities();

		bool refreshCapabilities();
		EncoderCaps* getEncoderCaps(VCE_Encoder_Type);
		EncoderCaps::IOCaps* getIOCaps(VCE_Encoder_Type, bool output);
	};
}