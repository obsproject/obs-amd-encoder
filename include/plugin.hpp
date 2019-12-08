/*
 * A Plugin that integrates the AMD AMF encoder into OBS Studio
 * Copyright (C) 2016 - 2018 Michael Fabian Dirks
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
#include <inttypes.h>
#include "version.hpp"

#ifndef LITE_OBS
#pragma warning(push)
#pragma warning(disable : 4201)
extern "C" {
#include <obs-module.h>
#include <util/base.h>
#include <util/platform.h>
}
#pragma warning(pop)
#endif

// Plugin
#define PLUGIN_NAME "AMD Advanced Media Framework"

#ifndef LITE_OBS
#define PLOG(level, ...) blog(level, "[AMF] " __VA_ARGS__)
#define PLOG_ERROR(format, ...) PLOG(LOG_ERROR, format, __VA_ARGS__)
#define PLOG_VAR(var) var
#else
#define PLOG(...) (void)0
#define PLOG_ERROR(format, ...) printf("[AMF] " format "\n", __VA_ARGS__)
#define PLOG_VAR(var)
#endif
#define PLOG_WARNING(...) PLOG(LOG_WARNING, __VA_ARGS__)
#define PLOG_INFO(...) PLOG(LOG_INFO, __VA_ARGS__)
#define PLOG_DEBUG(...) PLOG(LOG_DEBUG, __VA_ARGS__)

// Utility
#define vstr(s) dstr(s)
#define dstr(s) #s

#define amf_clamp(val, low, high) (val > high ? high : (val < low ? low : val))
#ifdef max
#undef max
#endif
#define max(val, high) (val > high ? val : high)
#ifdef min
#undef min
#endif
#define min(val, low) (val < low ? val : low)

#ifdef IN
#undef IN
#endif
#define IN
#ifdef OUT
#undef OUT
#endif
#define OUT

#ifdef _WIN64
#define BIT_STR "64"
#else
#define BIT_STR "32"
#endif

#define QUICK_FORMAT_MESSAGE(var, ...)                                                           \
	std::string var = "";                                                                        \
	{                                                                                            \
		std::vector<char> QUICK_FORMAT_MESSAGE_buf(1024);                                        \
		snprintf(QUICK_FORMAT_MESSAGE_buf.data(), QUICK_FORMAT_MESSAGE_buf.size(), __VA_ARGS__); \
		var = std::string(QUICK_FORMAT_MESSAGE_buf.data());                                      \
	}

#ifndef __FUNCTION_NAME__
#if defined(_WIN32) || defined(_WIN64) //WINDOWS
#define __FUNCTION_NAME__ __FUNCTION__
#else //*NIX
#define __FUNCTION_NAME__ __func__
#endif
#endif

enum class Presets : int8_t {
	None            = -1,
	ResetToDefaults = 0,
	Recording,
	HighQuality,
	Indistinguishable,
	Lossless,
	Twitch,
	YouTube,
};

enum class ViewMode : uint8_t { Basic, Advanced, Expert, Master };
