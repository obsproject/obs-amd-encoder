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
#include "plugin.h"
#include "api-base.h"
#include "amf.h"
#include "amf-capabilities.h"
#include <sstream>

#include "enc-h264.h"
#include "enc-h265.h"
#ifdef _WIN32
#include <windows.h>
#endif

using namespace Plugin;
using namespace Plugin::AMD;

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) {
	return TRUE;
}

OBS_DECLARE_MODULE();
OBS_MODULE_AUTHOR("Michael Fabian Dirks");
OBS_MODULE_USE_DEFAULT_LOCALE("enc-amf", "en-US");

#ifdef _DEBUG
#include "components/VideoEncoderVCE.h"
#include "components/VideoEncoderHEVC.h"
#include "components/VideoConverter.h"

static std::string fastPrintVariant(const char* text, amf::AMFVariantStruct variant) {
	std::vector<char> buf(1024);
	switch (variant.type) {
		case amf::AMF_VARIANT_EMPTY:
			snprintf(buf.data(), buf.size(), "%s%s", text, "Empty");
			break;
		case amf::AMF_VARIANT_BOOL:
			snprintf(buf.data(), buf.size(), "%s%s", text, variant.boolValue ? "true" : "false");
			break;
		case amf::AMF_VARIANT_INT64:
			snprintf(buf.data(), buf.size(), "%s%lld", text, variant.int64Value);
			break;
		case amf::AMF_VARIANT_DOUBLE:
			snprintf(buf.data(), buf.size(), "%s%f", text, variant.doubleValue);
			break;
		case amf::AMF_VARIANT_RECT:
			snprintf(buf.data(), buf.size(), "%s[%ld,%ld,%ld,%ld]", text,
				variant.rectValue.top, variant.rectValue.left,
				variant.rectValue.bottom, variant.rectValue.right);
			break;
		case amf::AMF_VARIANT_SIZE:
			snprintf(buf.data(), buf.size(), "%s%ldx%ld", text,
				variant.sizeValue.width, variant.sizeValue.height);
			break;
		case amf::AMF_VARIANT_POINT:
			snprintf(buf.data(), buf.size(), "%s[%ld,%ld]", text,
				variant.pointValue.x, variant.pointValue.y);
			break;
		case amf::AMF_VARIANT_RATE:
			snprintf(buf.data(), buf.size(), "%s%ld/%ld", text,
				variant.rateValue.num, variant.rateValue.den);
			break;
		case amf::AMF_VARIANT_RATIO:
			snprintf(buf.data(), buf.size(), "%s%ld:%ld", text,
				variant.ratioValue.num, variant.ratioValue.den);
			break;
		case amf::AMF_VARIANT_COLOR:
			snprintf(buf.data(), buf.size(), "%s(%d,%d,%d,%d)", text,
				variant.colorValue.r,
				variant.colorValue.g,
				variant.colorValue.b,
				variant.colorValue.a);
			break;
		case amf::AMF_VARIANT_STRING:
			snprintf(buf.data(), buf.size(), "%s'%s'", text,
				variant.stringValue);
			break;
		case amf::AMF_VARIANT_WSTRING:
			snprintf(buf.data(), buf.size(), "%s'%ls'", text,
				variant.wstringValue);
			break;
	}
	return std::string(buf.data());
};

static void printDebugInfo(amf::AMFComponentPtr m_AMFEncoder) {
	amf::AMFPropertyInfo* pInfo;
	size_t propCount = m_AMFEncoder->GetPropertyCount();
	//PLOG_INFO("-- Internal AMF Encoder Properties --");
	for (size_t propIndex = 0; propIndex < propCount; propIndex++) {
		static const char* typeToString[] = {
			"Empty",
			"Boolean",
			"Int64",
			"Double",
			"Rect",
			"Size",
			"Point",
			"Rate",
			"Ratio",
			"Color",
			"String",
			"WString",
			"Interface"
		};

		AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(propIndex, (const amf::AMFPropertyInfo**) &pInfo);
		if (res != AMF_OK)
			continue;

		amf::AMFVariantStruct curStruct = amf::AMFVariantStruct();
		m_AMFEncoder->GetProperty(pInfo->name, &curStruct);

		auto vcur = fastPrintVariant("Current: ", curStruct);
		auto vdef = fastPrintVariant("Default: ", pInfo->defaultValue);
		auto vmin = fastPrintVariant("Minimum: ", pInfo->minValue);
		auto vmax = fastPrintVariant("Maximum: ", pInfo->maxValue);
		std::stringstream venum;
		if (pInfo->pEnumDescription) {
			const amf::AMFEnumDescriptionEntry* pEnumEntry = pInfo->pEnumDescription;
			while (pEnumEntry->name != nullptr) {
				QUICK_FORMAT_MESSAGE(tmp, "%ls[%ld]", pEnumEntry->name, pEnumEntry->value);
				venum << tmp.c_str() << "; ";
				pEnumEntry++;
			}
		}

		PLOG_INFO("%ls(Description: %ls, Type: %s, Index %d, Content Type: %d, Access: %s%s%s, Values: {%s, %s, %s, %s%s%s})",
			pInfo->name,
			pInfo->desc,
			typeToString[pInfo->type],
			propIndex,
			pInfo->contentType,
			(pInfo->accessType & amf::AMF_PROPERTY_ACCESS_READ) ? "R" : "",
			(pInfo->accessType & amf::AMF_PROPERTY_ACCESS_WRITE) ? "W" : "",
			(pInfo->accessType & amf::AMF_PROPERTY_ACCESS_WRITE_RUNTIME) ? "X" : "",
			vcur.c_str(), vdef.c_str(), vmin.c_str(), vmax.c_str(),
			(venum.str().length() > 0) ? ", Enum: " : "", venum.str().c_str()
		);
	}
}
#endif

/**
* Required: Called when the module is loaded.  Use this function to load all
* the sources/encoders/outputs/services for your module, or anything else that
* may need loading.
*
* @return           Return true to continue loading the module, otherwise
*                   false to indicate failure and unload the module
*/
MODULE_EXPORT bool obs_module_load(void) {
	PLOG_DEBUG("<" __FUNCTION_NAME__ "> Loading...");

	// AMF
	try {
		Plugin::AMD::AMF::Initialize();
	} catch (const std::exception& e) {
		PLOG_ERROR("Encountered Exception during AMF initialization: %s", e.what());
		return false;
	} catch (...) {
		PLOG_ERROR("Unexpected Exception during AMF initialization.");
		return false;
	}

	// Initialize Graphics APIs
	Plugin::API::InitializeAPIs();

	// AMF Capabilities
	try {
		Plugin::AMD::CapabilityManager::Initialize();
	} catch (const std::exception& e) {
		PLOG_ERROR("Encountered Exception during Capability Manager initialization: %s", e.what());
		return false;
	} catch (...) {
		PLOG_ERROR("Unexpected Exception during Capability Manager initialization.");
		return false;
	}

	// Register Encoders
	Plugin::Interface::H264Interface::encoder_register();
	Plugin::Interface::H265Interface::encoder_register();

	#ifdef _DEBUG
	{
		PLOG_INFO("Dumping Parameter Information...");
		const wchar_t* encoders[] = {
			AMFVideoEncoderVCE_AVC,
			AMFVideoEncoder_HEVC
		};
		auto m_AMF = AMF::Instance();
		auto m_AMFFactory = m_AMF->GetFactory();
		amf::AMFTrace* m_AMFTrace;
		m_AMFFactory->GetTrace(&m_AMFTrace);
		amf::AMFDebug* m_AMFDebug;
		m_AMFFactory->GetDebug(&m_AMFDebug);
		m_AMFDebug->AssertsEnable(true);
		m_AMFDebug->EnablePerformanceMonitor(true);
		m_AMFTrace->SetGlobalLevel(AMF_TRACE_TEST);
		m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_CONSOLE, AMF_TRACE_TEST);
		m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_FILE, AMF_TRACE_TEST);
		m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, AMF_TRACE_TEST);
		m_AMFTrace->SetPath(L"AMFTrace.log");
		m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_FILE, true);
		m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_DEBUG_OUTPUT, true);
		m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_CONSOLE, true);
		m_AMFTrace->TraceEnableAsync(true);
		amf::AMFContextPtr m_AMFContext;
		if (m_AMFFactory->CreateContext(&m_AMFContext) == AMF_OK) {
			for (auto enc : encoders) {
				m_AMFContext->InitDX11(nullptr);
				amf::AMFComponentPtr m_AMFComponent;
				if (m_AMFFactory->CreateComponent(m_AMFContext, enc, &m_AMFComponent) == AMF_OK) {
					PLOG_INFO("-- %ls --", enc);
					printDebugInfo(m_AMFComponent);
					m_AMFComponent->Terminate();
				}
			}
			amf::AMFComponentPtr m_AMFComponent, m_AMFComponent2;
			if (m_AMFFactory->CreateComponent(m_AMFContext, AMFVideoConverter, &m_AMFComponent) == AMF_OK) {
				m_AMFComponent->Init(amf::AMF_SURFACE_NV12, 1280, 720);
				if (m_AMFFactory->CreateComponent(m_AMFContext, AMFVideoEncoderVCE_AVC, &m_AMFComponent2) == AMF_OK) {
					PLOG_INFO("-- %s --", "Converter");
					m_AMFComponent->SetProperty(AMF_VIDEO_CONVERTER_COLOR_PROFILE, AMF_VIDEO_CONVERTER_COLOR_PROFILE_JPEG);
					printDebugInfo(m_AMFComponent);
					m_AMFComponent2->Terminate();
				}
				m_AMFComponent->Terminate();
			}
			m_AMFContext->Terminate();
		}

	}
	#endif

	PLOG_DEBUG("<" __FUNCTION_NAME__ "> Loaded.");
	return true;
}

/** Optional: Called when the module is unloaded.  */
MODULE_EXPORT void obs_module_unload(void) {
	Plugin::AMD::CapabilityManager::Finalize();
	Plugin::API::FinalizeAPIs();
	Plugin::AMD::AMF::Finalize();
}

/** Optional: Returns the full name of the module */
MODULE_EXPORT const char* obs_module_name() {
	return "AMD Media Framework Plugin";
}

/** Optional: Returns a description of the module */
MODULE_EXPORT const char* obs_module_description() {
	return "AMD Media Framework Plugin";
}

