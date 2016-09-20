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
#include <windows.h>

// Plugin
#include "plugin.h"
#include "amd-amf.h"
#include "amd-amf-vce.h"
#include "amd-amf-vce-capabilities.h"
#include "enc-h264-simple.h"
#include "enc-h264.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain(HINSTANCE , DWORD , LPVOID ) {
	return TRUE;
}

OBS_DECLARE_MODULE();
OBS_MODULE_AUTHOR("Michael Fabian Dirks");
OBS_MODULE_USE_DEFAULT_LOCALE("enc-amf", "en-US");

/**
* Required: Called when the module is loaded.  Use this function to load all
* the sources/encoders/outputs/services for your module, or anything else that
* may need loading.
*
* @return           Return true to continue loading the module, otherwise
*                   false to indicate failure and unload the module
*/
MODULE_EXPORT bool obs_module_load(void) {
	try {
		AMF_LOG_INFO("Version " PLUGIN_VERSION_TEXT);
		
		// Attempt to load libraries
		auto instance = Plugin::AMD::AMF::GetInstance();

		// Report Capabilities
		Plugin::AMD::VCECapabilities::ReportCapabilities();

		// Register Encoders
		Plugin::Interface::H264SimpleInterface::encoder_register();
		Plugin::Interface::H264Interface::encoder_register();
		return true;
	} catch(...) {
	}
	return true;
}

/** Optional: Called when the module is unloaded.  */
MODULE_EXPORT void obs_module_unload(void) {}

/** Optional: Returns the full name of the module */
MODULE_EXPORT const char* obs_module_name() {
	return "AMD Media Framework Plugin";
}

/** Optional: Returns a description of the module */
MODULE_EXPORT const char* obs_module_description() {
	return "AMD Media Framework Plugin";
}

//////////////////////////////////////////////////////////////////////////
// Threading Specific
//////////////////////////////////////////////////////////////////////////

#if (defined _WIN32) | (defined _WIN64) // Windows
#include <windows.h>

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO {
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(uint32_t dwThreadID, const char* threadName) {

	// DWORD dwThreadID = ::GetThreadId( static_cast<HANDLE>( t.native_handle() ) );

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	} __except (EXCEPTION_EXECUTE_HANDLER) {}
}
void SetThreadName(const char* threadName) {
	SetThreadName(GetCurrentThreadId(), threadName);
}
void SetThreadName(std::thread* thread, const char* threadName) {
	DWORD threadId = ::GetThreadId(static_cast<HANDLE>(thread->native_handle()));
	SetThreadName(threadId, threadName);
}

#else // Linux, Mac
#include <sys/prctl.h>

void SetThreadName(std::thread* thread, const char* threadName) {
	auto handle = thread->native_handle();
	pthread_setname_np(handle, threadName);
}
void SetThreadName(const char* threadName) {
	prctl(PR_SET_NAME, threadName, 0, 0, 0);
}

#endif