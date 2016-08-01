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
#include <exception>
#include <stdexcept>
#include <memory>
#include <chrono>
#include <string> // std::string
#include <sstream> // std::stringstream
#include <queue>
#include <vector>

#include "OBS-Studio/libobs/obs-module.h"
#include "OBS-Studio/libobs/obs-encoder.h"

// AMF
#include "AMD-Media-SDK/1.1/inc/ErrorCodes.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/CapabilityManager.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/Component.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/ComponentCaps.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderCaps.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderVCE.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderVCECaps.h"

// Plugin
#include "win-amf.h"


//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////
#define AMF_TEXT_H264(x) (AMF_TEXT("h264." ## x))
#define AMF_TEXT_H264_T(x) obs_module_text(AMF_TEXT_H264(x))

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
namespace AMF_Encoder {
	struct h264_input_frame {
		amf::AMFSurfacePtr surface;
		std::vector<uint8_t> surfaceBuffer;
	};
	
	struct h264_output_frame {
		amf::AMFDataPtr data;
	};

	//////////////////////////////////////////////////////////////////////////
	// Encoder Class
	//////////////////////////////////////////////////////////////////////////
	class h264 {
		public:

		// h264 Profiles
		enum PROFILES {
			PROFILE_AVC_BP, PROFILE_AVC_XP, PROFILE_AVC_MP,
			PROFILE_AVC_HiP, PROFILE_AVC_Hi10P, PROFILE_AVC_Hi422P, PROFILE_AVC_Hi444P,
			PROFILE_SVC_BP, PROFILE_SVC_HiP,
			PROFILE_COUNT_MAX
		};
		static const char* PROFILE_NAMES[PROFILES::PROFILE_COUNT_MAX];
		static const unsigned char PROFILE_VALUES[PROFILES::PROFILE_COUNT_MAX];

		// h264 Levels
		enum LEVELS {
			LEVEL_1, LEVEL_1_1, LEVEL_1_2, LEVEL_1_3,
			LEVEL_2, LEVEL_2_1, LEVEL_2_2,
			LEVEL_3, LEVEL_3_1, LEVEL_3_2,
			LEVEL_4, LEVEL_4_1, LEVEL_4_2,
			LEVEL_5, LEVEL_5_1, LEVEL_5_2,
			LEVEL_COUNT_MAX
		};
		static const char* LEVEL_NAMES[LEVELS::LEVEL_COUNT_MAX];
		static const unsigned char LEVEL_VALUES[LEVELS::LEVEL_COUNT_MAX];


		//////////////////////////////////////////////////////////////////////////
		// Static Code
		//////////////////////////////////////////////////////////////////////////
		public:

		static obs_encoder_info* encoder_info;
		static void encoder_register();
		static const char* get_name(void* type_data);
		static void* create(obs_data_t* settings, obs_encoder_t* encoder);
		static void destroy(void* data);
		static bool encode(void *data, struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet);
		static void get_defaults(obs_data_t *settings);
		static obs_properties_t* get_properties(void* data);
		static bool update(void *data, obs_data_t *settings);
		static void get_video_info(void *data, struct video_scale_info *info);
		static bool get_extra_data(void *data, uint8_t** extra_data, size_t* size);

		///**
		//* Gets the SEI data, if any
		//*
		//* @param       data      Data associated with this encoder context
		//* @param[out]  sei_data  Pointer to receive the SEI data
		//* @param[out]  size      Pointer to receive the SEI data size
		//* @return                true if SEI data available, false otherwise
		//*/
		//bool(*get_sei_data)(void *data, uint8_t **sei_data, size_t *size);

		///**
		//* Returns desired audio format and sample information
		//*
		//* @param          data  Data associated with this encoder context
		//* @param[in/out]  info  Audio format information
		//*/
		//void(*get_audio_info)(void *data, struct audio_convert_info *info);


		//void *type_data;
		//void(*free_type_data)(void *type_data);
		
		static void wa_log_amf_error(AMF_RESULT amfResult, char* sMessage);
		static void wa_log_property_int(AMF_RESULT amfResult, char* sProperty, int64_t value);
		static void wa_log_property_bool(AMF_RESULT amfResult, char* sProperty, bool value);

		//////////////////////////////////////////////////////////////////////////
		// Module Code
		//////////////////////////////////////////////////////////////////////////
		public:

		h264(obs_data_t* settings, obs_encoder_t* encoder);
		~h264();

		bool encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet);
		void queue_frame(encoder_frame* frame);
		void update_queues();
		void dequeue_frame(encoder_packet* packet, bool* received_packet);
		bool update(obs_data_t* settings);
		void get_video_info(struct video_scale_info* info);
		bool get_extra_data(uint8_t** extra_data, size_t* size);

		bool update_properties(obs_data_t* settings);

		//////////////////////////////////////////////////////////////////////////
		// Storage
		//////////////////////////////////////////////////////////////////////////
		private:
		// AMF Specific Things
		amf::AMF_MEMORY_TYPE m_AMFMemoryType;
		amf::AMF_SURFACE_FORMAT m_AMFSurfaceFormat;
		amf::AMFContextPtr m_AMFContext;
		amf::AMFComponentPtr m_AMFEncoder;

		// Settings
		int m_cfgWidth, m_cfgHeight;
		int m_cfgFPSnum, m_cfgFPSden;

		// Queues
		std::queue<h264_input_frame*> m_InputQueue;
		std::queue<h264_output_frame*> m_OutputQueue;

		// Internal
		std::vector<uint8_t> m_LargeBuffer;
		std::vector<uint8_t> m_ExtraData;
	};
}