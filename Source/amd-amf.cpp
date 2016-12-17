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

//////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////
#include <vector>
#include <mutex>

#include "amd-amf.h"
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
	virtual void Write(const wchar_t* scope, const wchar_t* message) override {
		const wchar_t* realmsg = &(message[(33 + wcslen(scope) + 2)]); // Skip Time & Scope
		size_t msgLen = wcslen(realmsg) - (sizeof(wchar_t));

		blog(LOG_INFO, "[AMF Encoder] [%.*ls][%ls] %.*ls", 
			12, &(message[11]),
			scope, 
			msgLen, realmsg);
	}

	virtual void Flush() override {}

	static std::shared_ptr<CustomWriter> GetInstance() {
		static std::shared_ptr<CustomWriter> __instance = std::make_shared<CustomWriter>();
		static std::mutex __mutex;

		const std::lock_guard<std::mutex> lock(__mutex);
		return __instance;
	}
};

std::shared_ptr<Plugin::AMD::AMF> Plugin::AMD::AMF::GetInstance() {
	static std::shared_ptr<AMF> __instance = std::make_shared<AMF>();
	static std::mutex __mutex;

	const std::lock_guard<std::mutex> lock(__mutex);
	return __instance;
}

Plugin::AMD::AMF::AMF() {
	AMF_RESULT res = AMF_OK;

	// Initialize AMF Library
	AMF_LOG_DEBUG("<Plugin::AMD::AMF::AMF> Initializing...");

	#pragma region Null Class Members
	m_TimerPeriod = 0;
	m_AMFVersion_Compiler = 0;
	m_AMFVersion_Runtime = 0;
	m_AMFModule = 0;

	m_AMFFactory = nullptr;
	m_AMFTrace = nullptr;
	m_AMFDebug = nullptr;
	AMFQueryVersion = nullptr;
	AMFInit = nullptr;
	#pragma endregion Null Class Members

	/// Load AMF Runtime into Memory.
	m_AMFModule = LoadLibraryW(AMF_DLL_NAME);
	if (!m_AMFModule) {
		DWORD error = GetLastError();
		std::vector<char> buf(1024);
		sprintf(buf.data(), "Unable to load '%ls', error code %ld.", AMF_DLL_NAME, error);
		AMF_LOG_ERROR("%s", buf.data());
		throw std::exception(buf.data(), error);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::AMF::AMF> Loaded '%ls'.", AMF_DLL_NAME);
	#ifdef _WIN32 // Windows: Get Product Version
	std::vector<char> verbuf(GetFileVersionInfoSizeW(AMF_DLL_NAME, nullptr));
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
	sprintf(buf.data(), "%s%04x%04x%s",
		"\\StringFileInfo\\",
		lpTranslate[0].wLanguage,
		lpTranslate[0].wCodePage,
		"\\ProductVersion");

	// Retrieve file description for language and code page "i". 
	void* pProductVersion;
	uint32_t lProductVersionSize;
	VerQueryValueA(pBlock, buf.data(), &pProductVersion, &lProductVersionSize);
	#endif _WIN32 // Windows: Get Product Version

	/// Find Function for Querying AMF Version.
	#pragma region Query AMF Runtime Version
	AMFQueryVersion = (AMFQueryVersion_Fn)GetProcAddress(m_AMFModule, AMF_QUERY_VERSION_FUNCTION_NAME);
	if (!AMFQueryVersion) {
		DWORD error = GetLastError();
		std::vector<char> buf(1024);
		sprintf(buf.data(), "<Plugin::AMD::AMF::AMF> Finding Address of Function '%s' failed with error code %ld.", AMF_QUERY_VERSION_FUNCTION_NAME, error);
		AMF_LOG_ERROR("%s", buf.data());
		throw std::exception(buf.data(), error);
	}
	/// Query Runtime Version
	m_AMFVersion_Compiler = AMF_FULL_VERSION;
	res = AMFQueryVersion(&m_AMFVersion_Runtime);
	if (res != AMF_OK)
		ThrowExceptionWithAMFError("<Plugin::AMD::AMF::AMF> Querying Version failed with error %ls (code %ld).", res);
	#pragma endregion Query AMF Runtime Version

		/// Find Function for Initializing AMF.
	AMFInit = (AMFInit_Fn)GetProcAddress(m_AMFModule, AMF_INIT_FUNCTION_NAME);
	if (!AMFInit) {
		DWORD error = GetLastError();
		std::vector<char> buf(1024);
		sprintf(buf.data(), "<Plugin::AMD::AMF::AMF> Finding Address of Function '%s' failed with error code %ld.", AMF_INIT_FUNCTION_NAME, error);
		AMF_LOG_ERROR("%s", buf.data());
		throw std::exception(buf.data(), error);
	} else {
		res = AMFInit(m_AMFVersion_Runtime, &m_AMFFactory);
		if (res != AMF_OK)
			ThrowExceptionWithAMFError("<Plugin::AMD::AMF::AMF> Initializing AMF Library failed with error %ls (code %ld).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::AMF::AMF> AMF Library initialized.");

	/// Retrieve Trace Object.
	res = m_AMFFactory->GetTrace(&m_AMFTrace);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::AMF::AMF> Retrieving Trace object failed with error %ls (code %ld).", res);
	}

	/// Retrieve Debug Object.
	res = m_AMFFactory->GetDebug(&m_AMFDebug);
	if (res != AMF_OK) {
		AMF_LOG_ERROR("<Plugin::AMD::AMF::AMF> Retrieving Debug object failed with error code %ls (code %ld).", res);
		throw std::exception("", res);
	}

	/// Register Custom Trace Writer and disable Debug Tracing.
	m_AMFTrace->RegisterWriter(L"OBSWriter", CustomWriter::GetInstance().get(), true);
	this->EnableDebugTrace(false);

	// Log success
	AMF_LOG_INFO("Version " PLUGIN_VERSION_TEXT " loaded (Compiled: %d.%d.%d.%d, Runtime: %d.%d.%d.%d, Library: %.*s).",
		(uint16_t)((m_AMFVersion_Compiler >> 48ull) & 0xFFFF),
		(uint16_t)((m_AMFVersion_Compiler >> 32ull) & 0xFFFF),
		(uint16_t)((m_AMFVersion_Compiler >> 16ull) & 0xFFFF),
		(uint16_t)((m_AMFVersion_Compiler & 0xFFFF)),
		(uint16_t)((m_AMFVersion_Runtime >> 48ull) & 0xFFFF),
		(uint16_t)((m_AMFVersion_Runtime >> 32ull) & 0xFFFF),
		(uint16_t)((m_AMFVersion_Runtime >> 16ull) & 0xFFFF),
		(uint16_t)((m_AMFVersion_Runtime & 0xFFFF)),
		lProductVersionSize, pProductVersion
	);

	AMF_LOG_DEBUG("<Plugin::AMD::AMF::AMF> Initialized.");
}

Plugin::AMD::AMF::~AMF() {
	AMF_LOG_DEBUG("<Plugin::AMD::AMF::AMF> Finalizing.");

	/// Unregister Writer
	if (m_AMFTrace)
		m_AMFTrace->UnregisterWriter(L"OBSWriter");

	// Free Library again
	if (m_AMFModule)
		FreeLibrary(m_AMFModule);

	#pragma region Null Class Members
	m_TimerPeriod = 0;
	m_AMFVersion_Compiler = 0;
	m_AMFVersion_Runtime = 0;
	m_AMFModule = 0;

	m_AMFFactory = nullptr;
	m_AMFTrace = nullptr;
	m_AMFDebug = nullptr;
	AMFQueryVersion = nullptr;
	AMFInit = nullptr;
	#pragma endregion Null Class Members

	AMF_LOG_DEBUG("<Plugin::AMD::AMF::AMF> Finalized.");
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
		throw std::exception(__FUNCTION_NAME__ " called without a AMFTrace object!");
	if (!m_AMFDebug)
		throw std::exception(__FUNCTION_NAME__ " called without a AMFDebug object!");

	m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_CONSOLE, false);
	m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_CONSOLE, AMF_TRACE_ERROR);
	#ifdef _DEBUG
	m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_DEBUG_OUTPUT, true);
	m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, AMF_TRACE_TEST);
	m_AMFTrace->SetPath(L"C:/AMFTrace.log");
	#else
	m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_DEBUG_OUTPUT, false);
	m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, AMF_TRACE_ERROR);
	#endif
	m_AMFTrace->EnableWriter(AMF_TRACE_WRITER_FILE, false);
	m_AMFTrace->SetWriterLevel(AMF_TRACE_WRITER_FILE, AMF_TRACE_ERROR);

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
