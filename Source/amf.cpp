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

//////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////
#include <vector>
#include <mutex>

#include "amf.h"
#include "windows.h"

// AMD AMF SDK
#include "components\Component.h"
#include "components\ComponentCaps.h"
#include "components\VideoEncoderVCE.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin::AMD;

class CustomWriter : public amf::AMFTraceWriter {
	public:

	virtual void __cdecl Write(const wchar_t* scope, const wchar_t* message) override {
		const wchar_t* realmsg = &(message[(33 + wcslen(scope) + 2)]); // Skip Time & Scope
		size_t msgLen = wcslen(realmsg) - (sizeof(wchar_t));

		blog(LOG_DEBUG, "[AMF Runtime] [%.*ls][%ls] %.*ls",
			12, &(message[11]),
			scope,
			msgLen, realmsg);
	}

	virtual void __cdecl Flush() override {}
};

#pragma region Singleton
static AMF* __instance;
static std::mutex __instance_mutex;
void Plugin::AMD::AMF::Initialize() {
	const std::lock_guard<std::mutex> lock(__instance_mutex);
	if (!__instance)
		__instance = new AMF();
}

AMF* Plugin::AMD::AMF::Instance() {
	const std::lock_guard<std::mutex> lock(__instance_mutex);
	return __instance;
}

void Plugin::AMD::AMF::Finalize() {
	const std::lock_guard<std::mutex> lock(__instance_mutex);
	if (__instance)
		delete __instance;
	__instance = nullptr;
}
#pragma endregion Singleton

Plugin::AMD::AMF::AMF() {
	AMF_RESULT res = AMF_OK;

	#pragma region Null Class Members
	m_TimerPeriod = 0;
	m_AMFVersion_Plugin = AMF_FULL_VERSION;
	m_AMFVersion_Runtime = 0;
	m_AMFModule = 0;

	m_AMFFactory = nullptr;
	m_AMFTrace = nullptr;
	m_AMFDebug = nullptr;
	AMFQueryVersion = nullptr;
	AMFInit = nullptr;
	#pragma endregion Null Class Members

	#ifdef _WIN32
	std::vector<char> verbuf;
	void* pProductVersion = nullptr;
	uint32_t lProductVersionSize = 0;
	#endif

	// Initialize AMF Library
	PLOG_DEBUG("<" __FUNCTION_NAME__ "> Initializing...");

	// Load AMF Runtime Library
	m_AMFModule = LoadLibraryW(AMF_DLL_NAME);
	if (!m_AMFModule) {
		QUICK_FORMAT_MESSAGE(msg, "Unable to load '%ls', error code %ld.",
			AMF_DLL_NAME,
			GetLastError());
		throw std::exception(msg.data());
	} else {
		PLOG_DEBUG("<" __FUNCTION_NAME__ "> Loaded '%ls'.", AMF_DLL_NAME);
	}

	// Windows: Get Product Version for Driver Matching
	#ifdef _WIN32 
	{
		verbuf.resize(GetFileVersionInfoSizeW(AMF_DLL_NAME, nullptr) * 2);
		GetFileVersionInfoW(AMF_DLL_NAME, 0, (DWORD)verbuf.size(), verbuf.data());

		void* pBlock = verbuf.data();

		// Read the list of languages and code pages.
		struct LANGANDCODEPAGE {
			WORD wLanguage;
			WORD wCodePage;
		} *lpTranslate;
		UINT cbTranslate = sizeof(LANGANDCODEPAGE);

		VerQueryValueA(pBlock, "\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate);

		std::vector<char> buf(1024);
		snprintf(buf.data(), buf.size(), "%s%04x%04x%s",
			"\\StringFileInfo\\",
			lpTranslate[0].wLanguage,
			lpTranslate[0].wCodePage,
			"\\ProductVersion");

		// Retrieve file description for language and code page "i". 
		VerQueryValueA(pBlock, buf.data(), &pProductVersion, &lProductVersionSize);
	}
	#endif _WIN32

	// Query Runtime Version
	AMFQueryVersion = (AMFQueryVersion_Fn)GetProcAddress(m_AMFModule, AMF_QUERY_VERSION_FUNCTION_NAME);
	if (!AMFQueryVersion) {
		QUICK_FORMAT_MESSAGE(msg, "Incompatible AMF Runtime (could not find '%s'), error code %ld.",
			AMF_QUERY_VERSION_FUNCTION_NAME,
			GetLastError());
		throw std::exception(msg.data());
	} else {
		res = AMFQueryVersion(&m_AMFVersion_Runtime);
		if (res != AMF_OK) {
			QUICK_FORMAT_MESSAGE(msg, "Querying Version failed, error code %ld.",
				res);
			throw std::exception(msg.data());
		}
	}

	/// Initialize AMF
	AMFInit = (AMFInit_Fn)GetProcAddress(m_AMFModule, AMF_INIT_FUNCTION_NAME);
	if (!AMFInit) {
		QUICK_FORMAT_MESSAGE(msg, "Incompatible AMF Runtime (could not find '%s'), error code %ld.",
			AMF_QUERY_VERSION_FUNCTION_NAME,
			GetLastError());
		throw std::exception(msg.data());
	} else {
		res = AMFInit(m_AMFVersion_Runtime, &m_AMFFactory);
		if (res != AMF_OK) {
			QUICK_FORMAT_MESSAGE(msg, "Initializing AMF Library failed, error code %ld.",
				res);
			throw std::exception(msg.data());
		}
	}
	PLOG_DEBUG("<" __FUNCTION_NAME__ "> AMF Library initialized.");

	/// Retrieve Trace Object.
	res = m_AMFFactory->GetTrace(&m_AMFTrace);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(msg, "Retrieving AMF Trace class failed, error code %ld.",
			res);
		throw std::exception(msg.data());
	}

	/// Retrieve Debug Object.
	res = m_AMFFactory->GetDebug(&m_AMFDebug);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(msg, "Retrieving AMF Debug class failed, error code %ld.",
			res);
		throw std::exception(msg.data());
	}

	/// Register Trace Writer and disable Debug Tracing.
	#ifdef _WIN64
	m_TraceWriter = new CustomWriter();
	m_AMFTrace->RegisterWriter(L"OBSWriter", m_TraceWriter, true);
	#endif
	this->EnableDebugTrace(false);

	// Log success
	PLOG_INFO("Version %d.%d.%d loaded (Compiled: %d.%d.%d.%d, Runtime: %d.%d.%d.%d, Library: %.*s).",
		PLUGIN_VERSION_MAJOR,
		PLUGIN_VERSION_MINOR,
		PLUGIN_VERSION_PATCH,
		(uint16_t)((m_AMFVersion_Plugin >> 48ull) & 0xFFFF),
		(uint16_t)((m_AMFVersion_Plugin >> 32ull) & 0xFFFF),
		(uint16_t)((m_AMFVersion_Plugin >> 16ull) & 0xFFFF),
		(uint16_t)((m_AMFVersion_Plugin & 0xFFFF)),
		(uint16_t)((m_AMFVersion_Runtime >> 48ull) & 0xFFFF),
		(uint16_t)((m_AMFVersion_Runtime >> 32ull) & 0xFFFF),
		(uint16_t)((m_AMFVersion_Runtime >> 16ull) & 0xFFFF),
		(uint16_t)((m_AMFVersion_Runtime & 0xFFFF)),
		lProductVersionSize, pProductVersion
	);

	PLOG_DEBUG("<" __FUNCTION_NAME__ "> Initialized.");
}

Plugin::AMD::AMF::~AMF() {
	PLOG_DEBUG("<" __FUNCTION_NAME__ "> Finalizing.");
	if (m_TraceWriter) {
		//m_AMFTrace->UnregisterWriter(L"OBSWriter");
		delete m_TraceWriter;
		m_TraceWriter = nullptr;
	}

	if (m_AMFModule)
		FreeLibrary(m_AMFModule);
	PLOG_DEBUG("<" __FUNCTION_NAME__ "> Finalized.");

	#pragma region Null Class Members
	m_TimerPeriod = 0;
	m_AMFVersion_Plugin = 0;
	m_AMFVersion_Runtime = 0;
	m_AMFModule = 0;

	m_AMFFactory = nullptr;
	m_AMFTrace = nullptr;
	m_AMFDebug = nullptr;
	AMFQueryVersion = nullptr;
	AMFInit = nullptr;
	#pragma endregion Null Class Members
}

amf::AMFFactory* Plugin::AMD::AMF::GetFactory() {
	return m_AMFFactory;
}

amf::AMFTrace* Plugin::AMD::AMF::GetTrace() {
	return m_AMFTrace;
}

amf::AMFDebug* Plugin::AMD::AMF::GetDebug() {
	return m_AMFDebug;
}

void Plugin::AMD::AMF::EnableDebugTrace(bool enable) {
	if (!m_AMFTrace)
		throw std::exception("<" __FUNCTION_NAME__ "> called without a AMFTrace object!");
	if (!m_AMFDebug)
		throw std::exception("<" __FUNCTION_NAME__ "> called without a AMFDebug object!");

	#ifndef _WIN64
	m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_CONSOLE, false);
	m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_CONSOLE, AMF_TRACE_NOLOG);
	m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_FILE, false);
	m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_FILE, AMF_TRACE_NOLOG);
	m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_DEBUG_OUTPUT, false);
	m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, AMF_TRACE_NOLOG);
	m_AMFDebug->AssertsEnable(false);
	m_AMFDebug->EnablePerformanceMonitor(false);
	m_AMFTrace->TraceEnableAsync(false);
	m_AMFTrace->SetGlobalLevel(AMF_TRACE_NOLOG);
	return;
	#endif

	// Console
	m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_CONSOLE, false);
	m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_CONSOLE, AMF_TRACE_NOLOG);

	// File
	m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_FILE, false);
	m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_FILE, AMF_TRACE_NOLOG);
	m_AMFTrace->SetPath(L"C:\\AMFTrace.log");

	// Debug Output
	#ifdef _DEBUG
	m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_DEBUG_OUTPUT, true);
	m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, AMF_TRACE_TEST);
	#else
	m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_DEBUG_OUTPUT, false);
	m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, AMF_TRACE_NOLOG);
	#endif

	if (enable) {
		m_AMFDebug->AssertsEnable(true);
		m_AMFDebug->EnablePerformanceMonitor(true);
		m_AMFTrace->TraceEnableAsync(true);
		m_AMFTrace->SetGlobalLevel(AMF_TRACE_TEST);
		m_AMFTrace->SetWriterLevel(L"OBSWriter", AMF_TRACE_TEST);
	} else {
		m_AMFDebug->AssertsEnable(false);
		m_AMFDebug->EnablePerformanceMonitor(false);
		m_AMFTrace->TraceEnableAsync(true);
		m_AMFTrace->SetGlobalLevel(AMF_TRACE_WARNING);
		m_AMFTrace->SetWriterLevel(L"OBSWriter", AMF_TRACE_WARNING);
	}
}
