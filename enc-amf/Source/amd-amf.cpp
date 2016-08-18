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
#include "amd-amf.h"

// AMD AMF SDK
#include "components\Component.h"
#include "components\ComponentCaps.h"
#include "components\VideoEncoderVCE.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////

using namespace Plugin::AMD;

std::shared_ptr<Plugin::AMD::AMF> Plugin::AMD::AMF::__instance;

std::shared_ptr<Plugin::AMD::AMF> Plugin::AMD::AMF::GetInstance() {
	if (!__instance) {
		__instance = std::shared_ptr<Plugin::AMD::AMF>(new AMF());
	}

	return std::shared_ptr<Plugin::AMD::AMF>(__instance);
}

Plugin::AMD::AMF::AMF() {
	AMF_RESULT res;
	amf::AMFFactory* pFactory;

	AMF_LOG_INFO("<Plugin::AMD::AMF::AMF> Loading Libraries...");

	// Increase Timer precision.
	m_TimerPeriod = 1;
	while (timeBeginPeriod(m_TimerPeriod) == TIMERR_NOCANDO) {
		++m_TimerPeriod;
	}

	// Load Dynamic Library
	AMF_LOG_INFO("<Plugin::AMD::AMF::AMF> Loading Library...");
	m_AMFModule = LoadLibraryW(AMF_DLL_NAME);
	if (!m_AMFModule) {
		DWORD error = GetLastError();
		AMF_LOG_ERROR("<Plugin::AMD::AMF::AMF> LoadLibrary(AMF_DLL_NAME) failed with error code %d.", error);
		throw;
	}

	// Find Function: Query Version
	AMFQueryVersion = (AMFQueryVersion_Fn)GetProcAddress(m_AMFModule, AMF_QUERY_VERSION_FUNCTION_NAME);
	if (!AMFQueryVersion) {
		DWORD error = GetLastError();
		AMF_LOG_ERROR("<Plugin::AMD::AMF::AMF> GetProcAddress(..., AMF_QUERY_VERSION_FUNCTION_NAME) failed with error code %d.", error);
		throw;
	}

	// Query Runtime Version
	m_AMFVersion_Compiler = AMF_FULL_VERSION;
	res = AMFQueryVersion(&m_AMFVersion_Runtime);
	if (res != AMF_OK) {
		AMF_LOG_ERROR("<Plugin::AMD::AMF::AMF> Unable to retrieve version with error code %d.", res);
		throw;
	}
	AMF_LOG_INFO("<Plugin::AMD::AMF::AMF> Version Information:");
	AMF_LOG_INFO("<Plugin::AMD::AMF::AMF>	Compile: %d.%d.%d.%d",
		(m_AMFVersion_Compiler >> 48ull) & 0xFFFF,
		(m_AMFVersion_Compiler >> 32ull) & 0xFFFF,
		(m_AMFVersion_Compiler >> 16ull) & 0xFFFF,
		(m_AMFVersion_Compiler & 0xFFFF));
	AMF_LOG_INFO("<Plugin::AMD::AMF::AMF>	Runtime: %d.%d.%d.%d", 
		(m_AMFVersion_Runtime >> 48ull) & 0xFFFF,
		(m_AMFVersion_Runtime >> 32ull) & 0xFFFF,
		(m_AMFVersion_Runtime >> 16ull) & 0xFFFF,
		(m_AMFVersion_Runtime & 0xFFFF));

	// Find Function: Init
	AMFInit = (AMFInit_Fn)GetProcAddress(m_AMFModule, AMF_INIT_FUNCTION_NAME);
	if (!AMFInit) {
		DWORD error = GetLastError();
		AMF_LOG_ERROR("<Plugin::AMD::AMF::AMF> GetProcAddress(..., AMF_INIT_FUNCTION_NAME) failed with error code %d.", error);
		throw;
	}

	// Initialize AMF Libraries
	AMF_LOG_INFO("<Plugin::AMD::AMF::AMF> Initializing AMF Library...");
	res = AMFInit(m_AMFVersion_Runtime, &pFactory);
	if (res != AMF_OK) {
		AMF_LOG_ERROR("<Plugin::AMD::AMF::AMF> Unable to retrieve version with error code %d.", res);
		throw;
	}
	m_AMFFactory = std::shared_ptr<amf::AMFFactory>(pFactory);

	// Retrieve other Pointers
	m_AMFTrace = m_AMFFactory->GetTrace();
	m_AMFDebug = m_AMFFactory->GetDebug();

	AMF_LOG_INFO("<Plugin::AMD::AMF::AMF> Loading successful!");
}

Plugin::AMD::AMF::~AMF() {
	// Free Library again
	if (m_AMFModule)
		FreeLibrary(m_AMFModule);

	// Restore Timer precision.
	timeEndPeriod(m_TimerPeriod);
}

