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

#include "amf-capabilities.h"

#ifdef _WIN32
#include <windows.h>
#include <VersionHelpers.h>
#endif

using namespace Plugin;
using namespace Plugin::AMD;

#pragma region Singleton
static CapabilityManager* __instance;
static std::mutex __instance_mutex;
void Plugin::AMD::CapabilityManager::Initialize() {
	const std::lock_guard<std::mutex> lock(__instance_mutex);
	__instance = new CapabilityManager();
}

CapabilityManager* Plugin::AMD::CapabilityManager::Instance() {
	const std::lock_guard<std::mutex> lock(__instance_mutex);
	return __instance;
}

void Plugin::AMD::CapabilityManager::Finalize() {
	const std::lock_guard<std::mutex> lock(__instance_mutex);
	delete __instance;
	__instance = nullptr;
}
#pragma endregion Singleton

Plugin::AMD::CapabilityManager::CapabilityManager() {
	RefreshCapabilities();
}

Plugin::AMD::CapabilityManager::~CapabilityManager() {}

void Plugin::AMD::CapabilityManager::RefreshCapabilities() {
	// Key order: API, Adapter, Codec
	for (auto api : API::Base::EnumerateAPIs()) {
		for (auto adapter : api->EnumerateAdapters()) {
			for (auto codec : { Codec::H264AVC/*, Codec::H264SVC*/, Codec::HEVC }) {
				bool isSupported = false;
				try {
					AMD::Encoder* enc = nullptr;
					if (codec == Codec::H264AVC || codec == Codec::H264SVC) {
						enc = (AMD::Encoder*)new AMD::EncoderH264(api->GetName(), ((int64_t)adapter.idHigh << 32ul) + adapter.idLow, false, ColorFormat::NV12, ColorSpace::BT701, false);
					} else {
						//enc = (AMD::Encoder*)new AMD::EncoderH265(api->GetName(), adapter.idHigh << 32ul + adapter.idLow, false, ColorFormat::NV12, ColorSpace::BT701, false);
					}
					delete enc;
					isSupported = true;
				} catch (...) {}

				m_CapabilityMap[std::make_tuple(api->GetType(), adapter, codec)] = isSupported;
			}
		}
	}
}