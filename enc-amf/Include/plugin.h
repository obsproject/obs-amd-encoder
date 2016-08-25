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
#include <exception>
#include <stdexcept>

// Open Broadcaster Software
#include "OBS-Studio/libobs/obs-module.h"
#include "OBS-Studio/libobs/obs-encoder.h"

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////

#define vstr(s) dstr(s)
#define dstr(s) #s

#define PLUGIN_VERSION_MAJOR		1
#define PLUGIN_VERSION_MINOR		3
#define PLUGIN_VERSION_RELEASE		0pre4
#define PLUGIN_VERSION_FULL			(((uint64_t)PLUGIN_VERSION_MAJOR << 48ull) | ((uint64_t)PLUGIN_VERSION_MINOR << 16ull) | ((uint64_t)PLUGIN_VERSION_RELEASE))
#define PLUGIN_VERSION_TEXT			vstr(PLUGIN_VERSION_MAJOR) "." vstr(PLUGIN_VERSION_MINOR) "." vstr(PLUGIN_VERSION_RELEASE) "-" vstr(AMF_VERSION_MAJOR) "." vstr(AMF_VERSION_MINOR) "." vstr(AMF_VERSION_RELEASE) "." vstr(AMF_VERSION_BUILD_NUM)

#define AMF_LOG(level, format, ...) \
    blog(level, "[AMF Encoder] " format, ##__VA_ARGS__)
#define AMF_LOG_ERROR(format, ...)   AMF_LOG(LOG_ERROR,   format, ##__VA_ARGS__)
#define AMF_LOG_WARNING(format, ...) AMF_LOG(LOG_WARNING, format, ##__VA_ARGS__)
#define AMF_LOG_INFO(format, ...)    AMF_LOG(LOG_INFO,    format, ##__VA_ARGS__)
#define AMF_LOG_CONFIG(format, ...)  AMF_LOG(350,         format, ##__VA_ARGS__)
#define AMF_LOG_DEBUG(format, ...)   AMF_LOG(LOG_DEBUG,   format, ##__VA_ARGS__)

#define AMF_TEXT(x) ("AMF." ## x)
#define AMF_TEXT_T(x) obs_module_text(AMF_TEXT(x))

#define ThrowExceptionWithAMFError(format, res, ...) {\
	std::vector<char> _throwexceptionwithamferror_buf(8192);\
	sprintf(_throwexceptionwithamferror_buf.data(), format, ##__VA_ARGS__, Plugin::AMD::AMF::GetInstance()->GetTrace()->GetResultText(res), res);\
	AMF_LOG_ERROR("%s", _throwexceptionwithamferror_buf.data()); \
	throw new std::exception(_throwexceptionwithamferror_buf.data(), res); \
}