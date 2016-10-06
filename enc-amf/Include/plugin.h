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
#include <thread>

// Open Broadcaster Software
#pragma warning( disable: 4201 )
#include "libobs/obs-module.h"
#include "libobs/obs-encoder.h"

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////

#define vstr(s) dstr(s)
#define dstr(s) #s

#define PLUGIN_VERSION_MAJOR			1
#define PLUGIN_VERSION_MINOR			3
#define PLUGIN_VERSION_PATCH			3
#define PLUGIN_VERSION_BUILD			1
#define PLUGIN_VERSION_FULL				(((uint64_t)PLUGIN_VERSION_MAJOR << 48ull) | ((uint64_t)PLUGIN_VERSION_MINOR << 32ull) | ((uint64_t)PLUGIN_VERSION_PATCH) | ((uint64_t)PLUGIN_VERSION_BUILD))
#define PLUGIN_VERSION_TEXT				vstr(PLUGIN_VERSION_MAJOR) "." vstr(PLUGIN_VERSION_MINOR) "." vstr(PLUGIN_VERSION_PATCH) "." vstr(PLUGIN_VERSION_BUILD) "-" vstr(AMF_VERSION_MAJOR) "." vstr(AMF_VERSION_MINOR) "." vstr(AMF_VERSION_RELEASE) "." vstr(AMF_VERSION_BUILD_NUM)

#define AMF_LOG(level, format, ...)		blog(level, "[AMF Encoder] " format, ##__VA_ARGS__);
#define AMF_LOG_ERROR(format, ...)		AMF_LOG(LOG_ERROR,   format, ##__VA_ARGS__)
#define AMF_LOG_WARNING(format, ...)	AMF_LOG(LOG_WARNING, format, ##__VA_ARGS__)
#define AMF_LOG_INFO(format, ...)		AMF_LOG(LOG_INFO,    format, ##__VA_ARGS__)
#define AMF_LOG_CONFIG(format, ...)		AMF_LOG(350,         format, ##__VA_ARGS__)
#define AMF_LOG_DEBUG(format, ...)		AMF_LOG(LOG_DEBUG,   format, ##__VA_ARGS__)

#define TEXT_AMF(x)						("AMF." ## x)
#define TEXT_AMF_T(x)					obs_module_text(TEXT_AMF(x))
#define TEXT_AMF_H264(x)				(TEXT_AMF("H264." ## x))
#define TEXT_AMF_H264_T(x)				obs_module_text(TEXT_AMF_H264(x))
#define TEXT_AMF_H264ADVANCED(x)		(TEXT_AMF("H264Advanced." ## x))
#define TEXT_AMF_H264ADVANCED_T(x)		obs_module_text(TEXT_AMF_H264ADVANCED(x))
#define TEXT_AMF_H264SIMPLE(x)			(TEXT_AMF("H264Simple." ## x))
#define TEXT_AMF_H264SIMPLE_T(x)		obs_module_text(TEXT_AMF_H264SIMPLE(x))
#define TEXT_AMF_UTIL(x)				(TEXT_AMF("Util." ## x))
#define TEXT_AMF_UTIL_T(x)				obs_module_text(TEXT_AMF_UTIL(x))

#define ThrowExceptionWithAMFError(format, res, ...) {\
	std::vector<char> _throwexceptionwithamferror_buf(8192);\
	sprintf_s(_throwexceptionwithamferror_buf.data(), _throwexceptionwithamferror_buf.size(), format, ##__VA_ARGS__, Plugin::AMD::AMF::GetInstance()->GetTrace()->GetResultText(res), res);\
	AMF_LOG_ERROR("%s", _throwexceptionwithamferror_buf.data()); \
	throw new std::exception(_throwexceptionwithamferror_buf.data(), res); \
}

//////////////////////////////////////////////////////////////////////////
// Defines - Translation Strings
//////////////////////////////////////////////////////////////////////////

// Shared
#define AMF_H264_USAGE							TEXT_AMF_H264("Usage")
#define AMF_H264_USAGE_TRANSCODING				TEXT_AMF_H264("Usage.Transcoding")
#define AMF_H264_USAGE_ULTRALOWLATENCY			TEXT_AMF_H264("Usage.UltraLowLatency")
#define AMF_H264_USAGE_LOWLATENCY				TEXT_AMF_H264("Usage.LowLatency")
#define AMF_H264_USAGE_WEBCAM					TEXT_AMF_H264("Usage.Webcam")
#define AMF_H264_QUALITY_PRESET					TEXT_AMF_H264("QualityPreset")
#define AMF_H264_QUALITY_PRESET_SPEED			TEXT_AMF_H264("QualityPreset.Speed")
#define AMF_H264_QUALITY_PRESET_BALANCED		TEXT_AMF_H264("QualityPreset.Balanced")
#define AMF_H264_QUALITY_PRESET_QUALITY			TEXT_AMF_H264("QualityPreset.Quality")
#define AMF_H264_PROFILE						TEXT_AMF_H264("Profile")
#define AMF_H264_PROFILELEVEL					TEXT_AMF_H264("ProfileLevel")
#define AMF_H264_RATECONTROLMETHOD				TEXT_AMF_H264("RateControlMethod")
#define AMF_H264_RATECONTROLMETHOD_CQP			TEXT_AMF_H264("RateControlMethod.CQP")
#define AMF_H264_RATECONTROLMETHOD_CBR			TEXT_AMF_H264("RateControlMethod.CBR")
#define AMF_H264_RATECONTROLMETHOD_VBR			TEXT_AMF_H264("RateControlMethod.VBR.Peak")
#define AMF_H264_RATECONTROLMETHOD_VBR_LAT		TEXT_AMF_H264("RateControlMethod.VBR.Latency")
#define AMF_H264_BITRATE_TARGET					TEXT_AMF_H264("Bitrate.Target")
#define AMF_H264_BITRATE_PEAK					TEXT_AMF_H264("Bitrate.Peak")
#define AMF_H264_QP_MINIMUM						TEXT_AMF_H264("QP.Minimum")
#define AMF_H264_QP_MAXIMUM						TEXT_AMF_H264("QP.Maximum")
#define AMF_H264_QP_IFRAME						TEXT_AMF_H264("QP.IFrame")
#define AMF_H264_QP_PFRAME						TEXT_AMF_H264("QP.PFrame")
#define AMF_H264_QP_BFRAME						TEXT_AMF_H264("QP.BFrame")
#define AMF_H264_QP_BPICTURE_DELTA				TEXT_AMF_H264("QP.BPictureDelta")
#define AMF_H264_QP_REFERENCE_BPICTURE_DELTA	TEXT_AMF_H264("QP.ReferenceBPictureDelta")
#define AMF_H264_FILLERDATA						TEXT_AMF_H264("FillerData")
#define AMF_H264_FRAMESKIPPING					TEXT_AMF_H264("FrameSkipping")
#define AMF_H264_ENFORCEHRDCOMPATIBILITY		TEXT_AMF_H264("EnforceHRDCompatibility")
#define AMF_H264_DEBLOCKINGFILTER				TEXT_AMF_H264("DeblockingFilter")
#define AMF_H264_SCANTYPE						TEXT_AMF_H264("ScanType")
#define AMF_H264_SCANTYPE_PROGRESSIVE			TEXT_AMF_H264("ScanType.Progressive")
#define AMF_H264_SCANTYPE_INTERLACED			TEXT_AMF_H264("ScanType.Interlaced")
#define AMF_H264_BPICTURE_PATTERN				TEXT_AMF_H264("BPicture.Pattern")
#define AMF_H264_BPICTURE_REFERENCE				TEXT_AMF_H264("BPicture.Reference")
#define AMF_H264_GOP_SIZE						TEXT_AMF_H264("GOPSize")
#define AMF_H264_CABAC							TEXT_AMF_H264("CABAC")
#define AMF_H264_DEBUGTRACING					TEXT_AMF_H264("DebugTracing")
#define AMF_H264_MEMORYTYPE						TEXT_AMF_H264("MemoryType")
#define AMF_H264_COMPUTETYPE					TEXT_AMF_H264("ComputeType")
#define AMF_H264_SURFACEFORMAT					TEXT_AMF_H264("SurfaceFormat")

// Utility
#define AMF_UTIL_DEFAULT						TEXT_AMF_UTIL("Default")
#define AMF_UTIL_AUTOMATIC						TEXT_AMF_UTIL("Automatic")
#define AMF_UTIL_TOGGLE_DISABLED				TEXT_AMF_UTIL("Toggle.Disabled")
#define AMF_UTIL_TOGGLE_ENABLED					TEXT_AMF_UTIL("Toggle.Enabled")

//////////////////////////////////////////////////////////////////////////
// Threading Specific
//////////////////////////////////////////////////////////////////////////

#if (defined _WIN32) | (defined _WIN64)
void SetThreadName(uint32_t dwThreadID, const char* threadName);
void SetThreadName(const char* threadName);
void SetThreadName(std::thread* thread, const char* threadName);

#else
void SetThreadName(std::thread* thread, const char* threadName);
void SetThreadName(const char* threadName);

#endif