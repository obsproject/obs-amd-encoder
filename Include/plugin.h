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
#include <inttypes.h>

#define NOMINMAX
#define NOINOUT

#pragma warning (push)
#pragma warning (disable: 4201)
#include "libobs/obs-module.h"
#include "libobs/util/platform.h"
#pragma warning (pop)

// Plugin
#define PLUGIN_NAME				"AMD Advanced Media Framework"
#include "Version.h"

#define PLOG(level, ...)		blog(level, "[AMF] " __VA_ARGS__);
#define PLOG_ERROR(...)			PLOG(LOG_ERROR,   __VA_ARGS__)
#define PLOG_WARNING(...)		PLOG(LOG_WARNING, __VA_ARGS__)
#define PLOG_INFO(...)			PLOG(LOG_INFO,    __VA_ARGS__)
#define PLOG_DEBUG(...)			PLOG(LOG_DEBUG,   __VA_ARGS__)

// Utility
#define vstr(s) dstr(s)
#define dstr(s) #s

#define clamp(val,low,high) (val > high ? high : (val < low ? low : val))
#ifdef max
#undef max
#endif
#define max(val,high) (val > high ? val : high)
#ifdef min
#undef min
#endif
#define min(val,low ) (val < low  ? val  : low)

#ifdef IN
#undef IN
#endif
#define IN
#ifdef OUT
#undef OUT
#endif
#define OUT

#define QUICK_FORMAT_MESSAGE(var, ...) std::string var = ""; { \
		std::vector<char> QUICK_FORMAT_MESSAGE_buf(1024); \
		snprintf(QUICK_FORMAT_MESSAGE_buf.data(), QUICK_FORMAT_MESSAGE_buf.size(), __VA_ARGS__); \
		var = std::string(QUICK_FORMAT_MESSAGE_buf.data()); \
	}

#ifndef __FUNCTION_NAME__
#if defined(_WIN32) || defined(_WIN64)   //WINDOWS
#define __FUNCTION_NAME__   __FUNCTION__  
#else          //*NIX
#define __FUNCTION_NAME__   __func__ 
#endif
#endif

enum class Presets : int8_t {
	None = -1,
	ResetToDefaults = 0,
	Recording,
	HighQuality,
	Indistinguishable,
	Lossless,
	Twitch,
	YouTube,
};
