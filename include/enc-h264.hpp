/*
 * A Plugin that integrates the AMD AMF encoder into OBS Studio
 * Copyright (C) 2016 - 2018 Michael Fabian Dirks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#pragma once
#include "amf-encoder-h264.hpp"
#include "plugin.hpp"

namespace Plugin {
	namespace Interface {
		class H264Interface {
			public:
			static void              encoder_register();
			static const char*       get_name(void* type_data) noexcept;
			static void              get_defaults(obs_data_t* settings) noexcept;
			static obs_properties_t* get_properties(void* data) noexcept;

			static bool properties_modified(obs_properties_t* props, obs_property_t*, obs_data_t* data) noexcept;

			static void* create(obs_data_t* settings, obs_encoder_t* encoder) noexcept;
			static void  destroy(void* data) noexcept;
			static bool  update(void* data, obs_data_t* settings) noexcept;
			static bool  encode(void* data, struct encoder_frame* frame, struct encoder_packet* packet,
								bool* received_packet) noexcept;
			static void  get_video_info(void* data, struct video_scale_info* info) noexcept;
			static bool  get_extra_data(void* data, uint8_t** extra_data, size_t* size) noexcept;

			//////////////////////////////////////////////////////////////////////////
			// Module Code
			//////////////////////////////////////////////////////////////////////////
			public:
			H264Interface(obs_data_t* settings, obs_encoder_t* encoder);
			~H264Interface();

			bool update(obs_data_t* settings);
			bool encode(struct encoder_frame* frame, struct encoder_packet* packet, bool* received_packet);
			void get_video_info(struct video_scale_info* info);
			bool get_extra_data(uint8_t** extra_data, size_t* size);

			//////////////////////////////////////////////////////////////////////////
			// Storage
			//////////////////////////////////////////////////////////////////////////
			private:
			std::unique_ptr<Plugin::AMD::EncoderH264> m_VideoEncoder;
			obs_encoder_t*                            m_Encoder;
		};
	} // namespace Interface
} // namespace Plugin
