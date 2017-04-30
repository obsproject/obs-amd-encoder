/*
MIT License

Copyright (c) 2016-2017

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
#include "amf-encoder-h264.h"
#include "plugin.h"

namespace Plugin {
	namespace Interface {
		class H264Interface {
			public:
			static void encoder_register();
			static const char* get_name(void* type_data);
			static void get_defaults(obs_data_t *settings);
			static obs_properties_t* get_properties(void* data);
			
			static bool properties_modified(obs_properties_t *props, obs_property_t *, obs_data_t *data);

			static void* create(obs_data_t* settings, obs_encoder_t* encoder);
			static void destroy(void* data);
			static bool update(void *data, obs_data_t *settings);
			static bool encode(void *data, struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet);
			static void get_video_info(void *data, struct video_scale_info *info);
			static bool get_extra_data(void *data, uint8_t** extra_data, size_t* size);

			//////////////////////////////////////////////////////////////////////////
			// Module Code
			//////////////////////////////////////////////////////////////////////////
			public:

			H264Interface(obs_data_t* settings, obs_encoder_t* encoder);
			~H264Interface();

			bool update(obs_data_t* settings);
			bool encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet);
			void get_video_info(struct video_scale_info* info);
			bool get_extra_data(uint8_t** extra_data, size_t* size);

			//////////////////////////////////////////////////////////////////////////
			// Storage
			//////////////////////////////////////////////////////////////////////////
			private:
			std::unique_ptr<Plugin::AMD::EncoderH264> m_VideoEncoder;
			obs_encoder_t* m_Encoder;
		};
	}
}