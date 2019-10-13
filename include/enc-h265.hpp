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
#include "amf-encoder-h265.hpp"
#include "plugin.hpp"

namespace Plugin {
	namespace Interface {
		class H265Interface {
			public:
			static void              encoder_register();
			static const char*       get_name(void* type_data) noexcept;
			static void              get_defaults(obs_data_t* data) noexcept;
			static obs_properties_t* get_properties(void* ptr) noexcept;

			static bool properties_modified(obs_properties_t* props, obs_property_t*, obs_data_t* data) noexcept;

			static void* create(obs_data_t* settings, obs_encoder_t* encoder) noexcept;
			static void  destroy(void* ptr) noexcept;
			static bool  update(void* ptr, obs_data_t* data) noexcept;
			static bool  encode(void* ptr, struct encoder_frame* frame, struct encoder_packet* packet,
								bool* received_packet) noexcept;
			static void  get_video_info(void* ptr, struct video_scale_info* info) noexcept;
			static bool  get_extra_data(void* ptr, uint8_t** extra_data, size_t* size) noexcept;

			//////////////////////////////////////////////////////////////////////////
			// Module Code
			//////////////////////////////////////////////////////////////////////////
			public:
			H265Interface(obs_data_t* data, obs_encoder_t* encoder);
			~H265Interface();

			bool update(obs_data_t* data);
			bool encode(struct encoder_frame* frame, struct encoder_packet* packet, bool* received_packet);
			void get_video_info(struct video_scale_info* info);
			bool get_extra_data(uint8_t** extra_data, size_t* size);

			//////////////////////////////////////////////////////////////////////////
			// Storage
			//////////////////////////////////////////////////////////////////////////
			private:
			std::unique_ptr<Plugin::AMD::EncoderH265> m_VideoEncoder;
			obs_encoder_t*                            m_Encoder;
		};
	} // namespace Interface
} // namespace Plugin
