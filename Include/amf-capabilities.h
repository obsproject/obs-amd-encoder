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