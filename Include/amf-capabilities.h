/*
 * A Plugin that integrates the AMD AMF encoder into OBS Studio
 * Copyright (C) 2016 - 2017 Michael Fabian Dirks
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
//////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <inttypes.h>
#include <vector>
#include <list>
#include <map>
#include <tuple>

// Plugin
#include "plugin.h"
#include "amf.h"
#include "amf-encoder.h"
#include "amf-encoder-h264.h"
#include "amf-encoder-h265.h"
#include "api-base.h"

// AMF
#include "components\ComponentCaps.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////

namespace Plugin {
	namespace AMD {
		class CapabilityManager {
			#pragma region Singleton
			public:
			static void Initialize();
			static CapabilityManager* Instance();
			static void Finalize();

			private: // Private Initializer & Finalizer
			CapabilityManager();
			~CapabilityManager();

			public: // Remove all Copy operators
			CapabilityManager(CapabilityManager const&) = delete;
			void operator=(CapabilityManager const&) = delete;
			#pragma endregion Singleton

			bool IsCodecSupported(AMD::Codec codec);
			bool IsCodecSupportedByAPI(AMD::Codec codec, API::Type api);
			bool IsCodecSupportedByAPIAdapter(AMD::Codec codec, API::Type api, API::Adapter adapter);

			private:
			std::map<
				std::tuple<API::Type, API::Adapter, AMD::Codec>,
				bool> m_CapabilityMap;

		};
	}
}
