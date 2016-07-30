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
#include <exception>
#include <stdexcept>
#include <memory>
#include <chrono>
#include <string> // std::string
#include <sstream> // std::stringstream

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

#define AMF_TEXT(x) ("AMF.h264." ## x)
#define AMF_TEXT_T(x) obs_module_text(AMF_TEXT(x))
#define AMF_PROPERTY(x) ("amf_h264_" ## x)

#define AMF_LOG(level, format, ...) \
    blog(level, "[AMF_Encoder::h264] " format, ##__VA_ARGS__)
#define AMF_LOG_ERROR(format, ...)   AMF_LOG(LOG_ERROR,   format, ##__VA_ARGS__)
#define AMF_LOG_WARNING(format, ...) AMF_LOG(LOG_WARNING, format, ##__VA_ARGS__)
#define AMF_LOG_INFO(format, ...)    AMF_LOG(LOG_INFO,    format, ##__VA_ARGS__)
#define AMF_LOG_DEBUG(format, ...)   AMF_LOG(LOG_DEBUG,   format, ##__VA_ARGS__)


namespace AMF_Encoder {
	class h264_SurfaceObserver : public amf::AMFSurfaceObserver {

		public:
		virtual void AMF_STD_CALL OnSurfaceDataRelease(amf::AMFSurface* pSurface) override;

	};

	class h264 {
		public:
			// h264 Profiles
		enum PROFILES {
			PROFILE_AVC_BP,
			PROFILE_AVC_XP,
			PROFILE_AVC_MP,
			PROFILE_AVC_HiP,
			PROFILE_AVC_Hi10P,
			PROFILE_AVC_Hi422P,
			PROFILE_AVC_Hi444P,
			PROFILE_SVC_BP,
			PROFILE_SVC_HiP,
			PROFILE_COUNT_MAX
		};
		static const char* PROFILE_NAMES[PROFILES::PROFILE_COUNT_MAX];
		static const unsigned char PROFILE_VALUES[PROFILES::PROFILE_COUNT_MAX];

		// h264 Levels
		enum LEVELS {
			LEVEL_1,
			LEVEL_1_1,
			LEVEL_1_2,
			LEVEL_1_3,
			LEVEL_2,
			LEVEL_2_1,
			LEVEL_2_2,
			LEVEL_3,
			LEVEL_3_1,
			LEVEL_3_2,
			LEVEL_4,
			LEVEL_4_1,
			LEVEL_4_2,
			LEVEL_5,
			LEVEL_5_1,
			LEVEL_5_2,
			LEVEL_COUNT_MAX
		};
		static const char* LEVEL_NAMES[LEVELS::LEVEL_COUNT_MAX];
		static const unsigned char LEVEL_VALUES[LEVELS::LEVEL_COUNT_MAX];

		public:
			// Module Code
		static void encoder_register();

		/**
		* Gets the full translated name of this encoder
		*
		* @param  type_data  The type_data variable of this structure
		* @return            Translated name of the encoder
		*/
		static const char* get_name(void* type_data);

		/**
		* Creates the encoder with the specified settings
		*
		* @param  settings  Settings for the encoder
		* @param  encoder   OBS encoder context
		* @return           Data associated with this encoder context, or
		*                   NULL if initialization failed.
		*/
		static void* create(obs_data_t* settings, obs_encoder_t* encoder);
		h264(obs_data_t* settings, obs_encoder_t* encoder);

		/**
		* Destroys the encoder data
		*
		* @param  data  Data associated with this encoder context
		*/
		static void destroy(void* data);
		~h264();

		/**
		* Encodes frame(s), and outputs encoded packets as they become
		* available.
		*
		* @param       data             Data associated with this encoder
		*                               context
		* @param[in]   frame            Raw audio/video data to encode
		* @param[out]  packet           Encoder packet output, if any
		* @param[out]  received_packet  Set to true if a packet was received,
		*                               false otherwise
		* @return                       true if successful, false otherwise.
		*/
		static bool encode(void *data, struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet);
		bool encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet);

		/* ----------------------------------------------------------------- */
		/* Optional implementation */

		/**
		* Gets the default settings for this encoder
		*
		* @param[out]  settings  Data to assign default settings to
		*/
		static void get_defaults(obs_data_t *settings);

		/**
		* Gets the property information of this encoder
		*
		* @return         The properties data
		*/
		static obs_properties_t* get_properties(void* data);
		obs_properties_t* get_properties();

		/**
		* Updates the settings for this encoder (usually used for things like
		* changeing birate while active)
		*
		* @param  data      Data associated with this encoder context
		* @param  settings  New settings for this encoder
		* @return           true if successful, false otherwise
		*/
		static bool update(void *data, obs_data_t *settings);
		bool update(obs_data_t* settings);

		///**
		//* Returns extra data associated with this encoder (usually header)
		//*
		//* @param  data             Data associated with this encoder context
		//* @param[out]  extra_data  Pointer to receive the extra data
		//* @param[out]  size        Pointer to receive the size of the extra
		//*                          data
		//* @return                  true if extra data available, false
		//*                          otherwise
		//*/
		//bool(*get_extra_data)(void *data, uint8_t **extra_data, size_t *size);

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

		/**
		* Returns desired video format information
		*
		* @param          data  Data associated with this encoder context
		* @param[in/out]  info  Video format information
		*/
		static void get_video_info(void *data, struct video_scale_info *info);
		void get_video_info(struct video_scale_info* info);

		//void *type_data;
		//void(*free_type_data)(void *type_data);
		
		private:
		// OBS Data
		obs_encoder_t* encoder;
		obs_data_t* settings;

		// Settings
		amf::AMF_MEMORY_TYPE s_memoryType;
		amf::AMF_SURFACE_FORMAT s_surfaceFormat;
		int s_Width, s_Height;
		int s_FPS_num, s_FPS_den;


		// Internal
		byte* frame_buf;
		amf::AMFContextPtr amf_context;
		amf::AMFComponentPtr amf_encoder;
		h264_SurfaceObserver amf_surfaceObserver;
	};

}