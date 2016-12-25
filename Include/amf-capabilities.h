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
#include <list>
#include <map>
#include <tuple>

// Plugin
#include "plugin.h"
#include "amf.h"
#include "amf-h264.h"
#include "api-base.h"

// AMF
#include "components\ComponentCaps.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////

namespace Plugin {
	namespace AMD {
		volatile struct VCEDeviceCapabilities {
			amf::AMF_ACCELERATION_TYPE acceleration_type;
			uint32_t maxProfile;
			uint32_t maxProfileLevel;
			uint32_t maxBitrate;
			uint32_t minReferenceFrames;
			uint32_t maxReferenceFrames;
			bool supportsBFrames;
			bool supportsFixedSliceMode;
			uint32_t maxTemporalLayers;
			uint32_t maxNumOfStreams;
			uint32_t maxNumOfHwInstances;

			struct IOCaps {
				int32_t minWidth, maxWidth;
				int32_t minHeight, maxHeight;
				bool supportsInterlaced;
				uint32_t verticalAlignment;

				std::vector<std::pair<amf::AMF_SURFACE_FORMAT, bool>> formats;
				std::vector<std::pair<amf::AMF_MEMORY_TYPE, bool>> memoryTypes;
			} input, output;

			Plugin::AMD::VCEDeviceCapabilities::VCEDeviceCapabilities();
		};

		class VCECapabilities {
			//////////////////////////////////////////////////////////////////////////
			// Singleton
			//////////////////////////////////////////////////////////////////////////
			public:
			static std::shared_ptr<Plugin::AMD::VCECapabilities> GetInstance();
			static void ReportCapabilities(std::shared_ptr<Plugin::API::Base> api);
			static void ReportAdapterCapabilities(std::shared_ptr<Plugin::API::Base> api,
				Plugin::API::Adapter adapter);
			static void ReportAdapterTypeCapabilities(std::shared_ptr<Plugin::API::Base> api,
				Plugin::API::Adapter adapter,
				H264EncoderType type);
			static void ReportAdapterTypeIOCapabilities(std::shared_ptr<Plugin::API::Base> api,
				Plugin::API::Adapter adapter,
				H264EncoderType type,
				bool output);

			//////////////////////////////////////////////////////////////////////////
			// Class
			//////////////////////////////////////////////////////////////////////////
			public:
			VCECapabilities();
			~VCECapabilities();

			bool Refresh();
			std::vector<std::pair<H264EncoderType, VCEDeviceCapabilities>>
				GetAllAdapterCapabilities(std::shared_ptr<Plugin::API::Base> api, Plugin::API::Adapter adapter);
			VCEDeviceCapabilities
				GetAdapterCapabilities(std::shared_ptr<Plugin::API::Base> api, Plugin::API::Adapter adapter, H264EncoderType type);

			private:
			std::map<std::tuple<std::string, Plugin::API::Adapter, Plugin::AMD::H264EncoderType>, VCEDeviceCapabilities> capabilityMap;
		};
	}
}