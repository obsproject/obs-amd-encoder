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

// Microsoft as always does not follow the standard and declares safe functions unsafe.
// Or even straight up marks them as deprecated, what the fuck Microsoft?
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#endif

#include <cstdint>
#include <inttypes.h>
#include <exception>
#include <stdexcept>
#include <thread>
#include <memory>

// Open Broadcaster Software
#pragma warning( disable: 4201 )
#include "libobs/obs-module.h"
#include "libobs/obs-encoder.h"

MODULE_EXTERN const char *obs_module_text_multi(const char *val, uint8_t depth = (uint8_t)1);

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////

#define vstr(s) dstr(s)
#define dstr(s) #s
#define clamp(val,low,high) (val > high ? high : (val < low ? low : val))

#include "Version.h"
#define PLUGIN_VERSION_FULL				(((uint64_t)PLUGIN_VERSION_MAJOR << 48ull) | ((uint64_t)PLUGIN_VERSION_MINOR << 32ull) | ((uint64_t)PLUGIN_VERSION_PATCH) << 16ul | ((uint64_t)PLUGIN_VERSION_BUILD))
#define PLUGIN_VERSION_TEXT				vstr(PLUGIN_VERSION_MAJOR) "." vstr(PLUGIN_VERSION_MINOR) "." vstr(PLUGIN_VERSION_PATCH) "." vstr(PLUGIN_VERSION_BUILD)

#define AMF_LOG(level, format, ...)		blog(level, "[AMF Encoder] " format, ##__VA_ARGS__);
#define AMF_LOG_ERROR(format, ...)		AMF_LOG(LOG_ERROR,   format, ##__VA_ARGS__)
#define AMF_LOG_WARNING(format, ...)	AMF_LOG(LOG_WARNING, format, ##__VA_ARGS__)
#define AMF_LOG_INFO(format, ...)		AMF_LOG(LOG_INFO,    format, ##__VA_ARGS__)
#define AMF_LOG_CONFIG(format, ...)		AMF_LOG(350,         format, ##__VA_ARGS__)
#define AMF_LOG_DEBUG(format, ...)		AMF_LOG(LOG_DEBUG,   format, ##__VA_ARGS__)

#define ThrowException(format, ...) {\
	std::vector<char> _throwexceptionwithamferror_buf(8192);\
	sprintf_s(_throwexceptionwithamferror_buf.data(), _throwexceptionwithamferror_buf.size(), format, ##__VA_ARGS__);\
	AMF_LOG_WARNING("%s", _throwexceptionwithamferror_buf.data()); \
	throw std::exception(_throwexceptionwithamferror_buf.data()); \
}
#define ThrowExceptionWithAMFError(format, res, ...) {\
	std::vector<char> _throwexceptionwithamferror_buf(8192);\
	sprintf_s(_throwexceptionwithamferror_buf.data(), _throwexceptionwithamferror_buf.size(), format, ##__VA_ARGS__, Plugin::AMD::AMF::GetInstance()->GetTrace()->GetResultText(res), res);\
	AMF_LOG_WARNING("%s", _throwexceptionwithamferror_buf.data()); \
	throw std::exception(_throwexceptionwithamferror_buf.data()); \
}

#ifndef __FUNCTION_NAME__
#if defined(_WIN32) || defined(_WIN64)   //WINDOWS
#define __FUNCTION_NAME__   __FUNCTION__  
#else          //*NIX
#define __FUNCTION_NAME__   __func__ 
#endif
#endif

//////////////////////////////////////////////////////////////////////////
// Defines - Translation Strings
//////////////////////////////////////////////////////////////////////////

#define TEXT_T(x)						obs_module_text_multi(x)
#define TEXT_AMF(x)						("AMF." ## x)
#define TEXT_AMF_H264(x)				(TEXT_AMF("H264." ## x))
#define TEXT_AMF_UTIL(x)				(TEXT_AMF("Util." ## x))

// Utility
#define AMF_UTIL_DEFAULT									TEXT_AMF_UTIL("Default")
#define AMF_UTIL_AUTOMATIC									TEXT_AMF_UTIL("Automatic")
#define AMF_UTIL_MANUAL										TEXT_AMF_UTIL("Manual")
#define AMF_UTIL_TOGGLE_DISABLED							TEXT_AMF_UTIL("Toggle.Disabled")
#define AMF_UTIL_TOGGLE_ENABLED								TEXT_AMF_UTIL("Toggle.Enabled")

//////////////////////////////////////////////////////////////////////////
// Threading Specific
//////////////////////////////////////////////////////////////////////////

#if (defined _WIN32) || (defined _WIN64)
void SetThreadName(uint32_t dwThreadID, const char* threadName);
void SetThreadName(const char* threadName);
void SetThreadName(std::thread* thread, const char* threadName);

#else
void SetThreadName(std::thread* thread, const char* threadName);
void SetThreadName(const char* threadName);

#endif
