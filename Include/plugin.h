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

#include <cstdint>
#include <inttypes.h>
#include <exception>
#include <stdexcept>
#include <thread>
#include <memory>
#include "Version.h"

// Open Broadcaster Software
#include "libobs/obs-module.h"
#include "libobs/obs-encoder.h"

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////

#pragma warning (disable : 4996)

#define vstr(s) dstr(s)
#define dstr(s) #s
#define clamp(val,low,high) (val > high ? high : (val < low ? low : val))

#define PLUGIN_NAME_AMF				"AMD Advanced Media Framework"
#define PLUGIN_VERSION_FULL		(((uint64_t)PLUGIN_VERSION_MAJOR << 48ull) | ((uint64_t)PLUGIN_VERSION_MINOR << 32ull) | ((uint64_t)PLUGIN_VERSION_PATCH) | ((uint64_t)PLUGIN_VERSION_BUILD))
#define PLUGIN_VERSION_TEXT		vstr(PLUGIN_VERSION_MAJOR) "." vstr(PLUGIN_VERSION_MINOR) "." vstr(PLUGIN_VERSION_PATCH) "." vstr(PLUGIN_VERSION_BUILD)

#define AMF_LOG(level, ...)		blog(level, "[AMF] " __VA_ARGS__);
#define AMF_LOG_ERROR(...)		AMF_LOG(LOG_ERROR,   __VA_ARGS__)
#define AMF_LOG_WARNING(...)	AMF_LOG(LOG_WARNING, __VA_ARGS__)
#define AMF_LOG_INFO(...)		AMF_LOG(LOG_INFO,    __VA_ARGS__)
#define AMF_LOG_CONFIG(...)		AMF_LOG(350,         __VA_ARGS__)
#define AMF_LOG_DEBUG(...)		AMF_LOG(LOG_DEBUG,   __VA_ARGS__)

#define QUICK_FORMAT_MESSAGE(var, ...) std::vector<char> var(1024); \
	sprintf_s(var.data(), var.size(), __VA_ARGS__);

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

// Defines - Translation Strings


//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////

MODULE_EXTERN const char *obs_module_text_multi(const char *val, uint8_t depth = (uint8_t)1);

#if (defined _WIN32) || (defined _WIN64)
void SetThreadName(uint32_t dwThreadID, const char* threadName);
void SetThreadName(const char* threadName);
void SetThreadName(std::thread* thread, const char* threadName);

#else
void SetThreadName(std::thread* thread, const char* threadName);
void SetThreadName(const char* threadName);

#endif

//////////////////////////////////////////////////////////////////////////
// Translation Macros
//////////////////////////////////////////////////////////////////////////
#define TEXT_T(x)						obs_module_text_multi(x)
#define TEXT_AMF(x)						("AMF." ## x)
#define TEXT_AMF_H264(x)				(TEXT_AMF("H264." ## x))
#define TEXT_AMF_H265(x)				(TEXT_AMF("H265." ## x))
#define TEXT_AMF_UTIL(x)				(TEXT_AMF("Util." ## x))

// Shared
/// Utility
#define AMF_UTIL_DEFAULT				TEXT_AMF_UTIL("Default")
#define AMF_UTIL_AUTOMATIC				TEXT_AMF_UTIL("Automatic")
#define AMF_UTIL_MANUAL					TEXT_AMF_UTIL("Manual")
#define AMF_UTIL_TOGGLE_DISABLED		TEXT_AMF_UTIL("Toggle.Disabled")
#define AMF_UTIL_TOGGLE_ENABLED			TEXT_AMF_UTIL("Toggle.Enabled")

// H264
/// Presets
#define AMF_H264_PRESET										TEXT_AMF_H264("Preset")
#define AMF_H264_PRESET_RESETTODEFAULTS						TEXT_AMF_H264("Preset.ResetToDefaults")
#define AMF_H264_PRESET_RECORDING							TEXT_AMF_H264("Preset.Recording")
#define AMF_H264_PRESET_HIGHQUALITY							TEXT_AMF_H264("Preset.HighQuality")
#define AMF_H264_PRESET_INDISTINGUISHABLE					TEXT_AMF_H264("Preset.Indistinguishable")
#define AMF_H264_PRESET_LOSSLESS							TEXT_AMF_H264("Preset.Lossless")
#define AMF_H264_PRESET_TWITCH								TEXT_AMF_H264("Preset.Twitch")
#define AMF_H264_PRESET_YOUTUBE								TEXT_AMF_H264("Preset.YouTube")

/// Static
#define AMF_H264_USAGE										TEXT_AMF_H264("Usage")
#define AMF_H264_USAGE_DESCRIPTION							TEXT_AMF_H264("Usage.Description")
#define AMF_H264_USAGE_TRANSCODING							TEXT_AMF_H264("Usage.Transcoding")
#define AMF_H264_USAGE_ULTRALOWLATENCY						TEXT_AMF_H264("Usage.UltraLowLatency")
#define AMF_H264_USAGE_LOWLATENCY							TEXT_AMF_H264("Usage.LowLatency")
#define AMF_H264_QUALITY_PRESET								TEXT_AMF_H264("QualityPreset")
#define AMF_H264_QUALITY_PRESET_DESCRIPTION					TEXT_AMF_H264("QualityPreset.Description")
#define AMF_H264_QUALITY_PRESET_SPEED						TEXT_AMF_H264("QualityPreset.Speed")
#define AMF_H264_QUALITY_PRESET_BALANCED					TEXT_AMF_H264("QualityPreset.Balanced")
#define AMF_H264_QUALITY_PRESET_QUALITY						TEXT_AMF_H264("QualityPreset.Quality")
#define AMF_H264_PROFILE									TEXT_AMF_H264("Profile")
#define AMF_H264_PROFILE_DESCRIPTION						TEXT_AMF_H264("Profile.Description")
#define AMF_H264_PROFILELEVEL								TEXT_AMF_H264("ProfileLevel")
#define AMF_H264_PROFILELEVEL_DESCRIPTION					TEXT_AMF_H264("ProfileLevel.Description")
#define AMF_H264_ASPECTRATIO								TEXT_AMF_H264("AspectRatio")
#define AMF_H264_ASPECTRATIO_DESCRIPTION					TEXT_AMF_H264("AspectRatio.Description")
#define AMF_H264_CODINGTYPE									TEXT_AMF_H264("CodingType")
#define AMF_H264_CODINGTYPE_DESCRIPTION						TEXT_AMF_H264("CodingType.Description")
#define AMF_H264_MAXIMUMREFERENCEFRAMES						TEXT_AMF_H264("MaximumReferenceFrames")
#define AMF_H264_MAXIMUMREFERENCEFRAMES_DESCRIPTION			TEXT_AMF_H264("MaximumReferenceFrames.Description")

/// Rate Control
#define AMF_H264_RATECONTROLMETHOD							TEXT_AMF_H264("RateControlMethod")
#define AMF_H264_RATECONTROLMETHOD_DESCRIPTION				TEXT_AMF_H264("RateControlMethod.Description")
#define AMF_H264_RATECONTROLMETHOD_CQP						TEXT_AMF_H264("RateControlMethod.CQP")
#define AMF_H264_RATECONTROLMETHOD_CBR						TEXT_AMF_H264("RateControlMethod.CBR")
#define AMF_H264_RATECONTROLMETHOD_VBR						TEXT_AMF_H264("RateControlMethod.VBR.Peak")
#define AMF_H264_RATECONTROLMETHOD_VBR_LAT					TEXT_AMF_H264("RateControlMethod.VBR.Latency")
#define AMF_H264_PREPASS									TEXT_AMF_H264("PrePass")
#define AMF_H264_PREPASS_DESCRIPTION						TEXT_AMF_H264("PrePass.Description")
#define AMF_H264_PREPASS_QUARTERSCALE						TEXT_AMF_H264("PrePass.QuarterScale")
#define AMF_H264_PREPASS_HALFSCALE							TEXT_AMF_H264("PrePass.HalfScale")
#define AMF_H264_PREPASS_FULLSCALE							TEXT_AMF_H264("PrePass.FullScale")
#define AMF_H264_BITRATE_TARGET								TEXT_AMF_H264("Bitrate.Target")
#define AMF_H264_BITRATE_TARGET_DESCRIPTION					TEXT_AMF_H264("Bitrate.Target.Description")
#define AMF_H264_BITRATE_PEAK								TEXT_AMF_H264("Bitrate.Peak")
#define AMF_H264_BITRATE_PEAK_DESCRIPTION					TEXT_AMF_H264("Bitrate.Peak.Description")
#define AMF_H264_QP_IFRAME									TEXT_AMF_H264("QP.IFrame")
#define AMF_H264_QP_IFRAME_DESCRIPTION						TEXT_AMF_H264("QP.IFrame.Description")
#define AMF_H264_QP_PFRAME									TEXT_AMF_H264("QP.PFrame")
#define AMF_H264_QP_PFRAME_DESCRIPTION						TEXT_AMF_H264("QP.PFrame.Description")
#define AMF_H264_QP_BFRAME									TEXT_AMF_H264("QP.BFrame")
#define AMF_H264_QP_BFRAME_DESCRIPTION						TEXT_AMF_H264("QP.BFrame.Description")
#define AMF_H264_QP_MINIMUM									TEXT_AMF_H264("QP.Minimum")
#define AMF_H264_QP_MINIMUM_DESCRIPTION						TEXT_AMF_H264("QP.Minimum.Description")
#define AMF_H264_QP_MAXIMUM									TEXT_AMF_H264("QP.Maximum")
#define AMF_H264_QP_MAXIMUM_DESCRIPTION						TEXT_AMF_H264("QP.Maximum.Description")
#define AMF_H264_FILLERDATA									TEXT_AMF_H264("FillerData")
#define AMF_H264_FILLERDATA_DESCRIPTION						TEXT_AMF_H264("FillerData.Description")
#define AMF_H264_FRAMESKIPPING								TEXT_AMF_H264("FrameSkipping")
#define AMF_H264_FRAMESKIPPING_DESCRIPTION					TEXT_AMF_H264("FrameSkipping.Description")
#define AMF_H264_VBAQ										TEXT_AMF_H264("VBAQ")
#define AMF_H264_VBAQ_DESCRIPTION							TEXT_AMF_H264("VBAQ.Description")
#define AMF_H264_ENFORCEHRD									TEXT_AMF_H264("EnforceHRDCompatibility")
#define AMF_H264_ENFORCEHRD_DESCRIPTION						TEXT_AMF_H264("EnforceHRDCompatibility.Description")

/// VBV Buffer
#define AMF_H264_VBVBUFFER									TEXT_AMF_H264("VBVBuffer")
#define AMF_H264_VBVBUFFER_DESCRIPTION						TEXT_AMF_H264("VBVBuffer.Description")
#define AMF_H264_VBVBUFFER_STRICTNESS						TEXT_AMF_H264("VBVBuffer.Strictness")
#define AMF_H264_VBVBUFFER_STRICTNESS_DESCRIPTION			TEXT_AMF_H264("VBVBuffer.Strictness.Description")
#define AMF_H264_VBVBUFFER_SIZE								TEXT_AMF_H264("VBVBuffer.Size")
#define AMF_H264_VBVBUFFER_SIZE_DESCRIPTION					TEXT_AMF_H264("VBVBuffer.Size.Description")
#define AMF_H264_VBVBUFFER_FULLNESS							TEXT_AMF_H264("VBVBuffer.Fullness")
#define AMF_H264_VBVBUFFER_FULLNESS_DESCRIPTION				TEXT_AMF_H264("VBVBuffer.Fullness.Description")

/// Picture Control
#define AMF_H264_KEYFRAME_INTERVAL							TEXT_AMF_H264("KeyframeInterval")
#define AMF_H264_KEYFRAME_INTERVAL_DESCRIPTION				TEXT_AMF_H264("KeyframeInterval.Description")
#define AMF_H264_IDR_PERIOD									TEXT_AMF_H264("IDRPeriod")
#define AMF_H264_IDR_PERIOD_DESCRIPTION						TEXT_AMF_H264("IDRPeriod.Description")
#define AMF_H264_BFRAME_PATTERN								TEXT_AMF_H264("BFrame.Pattern")
#define AMF_H264_BFRAME_PATTERN_DESCRIPTION					TEXT_AMF_H264("BFrame.Pattern.Description")
#define AMF_H264_BFRAME_DELTAQP								TEXT_AMF_H264("BFrame.DeltaQP")
#define AMF_H264_BFRAME_DELTAQP_DESCRIPTION					TEXT_AMF_H264("BFrame.DeltaQP.Description")
#define AMF_H264_BFRAME_REFERENCE							TEXT_AMF_H264("BFrame.Reference")
#define AMF_H264_BFRAME_REFERENCE_DESCRIPTION				TEXT_AMF_H264("BFrame.Reference.Description")
#define AMF_H264_BFRAME_REFERENCEDELTAQP					TEXT_AMF_H264("BFrame.ReferenceDeltaQP")
#define AMF_H264_BFRAME_REFERENCEDELTAQP_DESCRIPTION		TEXT_AMF_H264("BFrame.ReferenceDeltaQP.Description")
#define AMF_H264_DEBLOCKINGFILTER							TEXT_AMF_H264("DeblockingFilter")
#define AMF_H264_DEBLOCKINGFILTER_DESCRIPTION				TEXT_AMF_H264("DeblockingFilter.Description")
#define AMF_H264_MOTIONESTIMATION							TEXT_AMF_H264("MotionEstimation")
#define AMF_H264_MOTIONESTIMATION_DESCRIPTION				TEXT_AMF_H264("MotionEstimation.Description")
#define AMF_H264_MOTIONESTIMATION_NONE						TEXT_AMF_H264("MotionEstimation.None")
#define AMF_H264_MOTIONESTIMATION_HALF						TEXT_AMF_H264("MotionEstimation.Half")
#define AMF_H264_MOTIONESTIMATION_QUARTER					TEXT_AMF_H264("MotionEstimation.Quarter")
#define AMF_H264_MOTIONESTIMATION_BOTH						TEXT_AMF_H264("MotionEstimation.Both")

/// System Properties
#define AMF_H264_VIDEOAPI									TEXT_AMF_H264("VideoAPI")
#define AMF_H264_VIDEOAPI_DESCRIPTION						TEXT_AMF_H264("VideoAPI.Description")
#define AMF_H264_VIDEOADAPTER								TEXT_AMF_H264("VideoAdapter")
#define AMF_H264_VIDEOADAPTER_DESCRIPTION					TEXT_AMF_H264("VideoAdapter.Description")
#define AMF_H264_OPENCL										TEXT_AMF_H264("OpenCL")
#define AMF_H264_OPENCL_DESCRIPTION							TEXT_AMF_H264("OpenCL.Description")
#define AMF_H264_VIEW										TEXT_AMF_H264("View")
#define AMF_H264_VIEW_DESCRIPTION							TEXT_AMF_H264("View.Description")
#define AMF_H264_VIEW_BASIC									TEXT_AMF_H264("View.Basic")
#define AMF_H264_VIEW_ADVANCED								TEXT_AMF_H264("View.Advanced")
#define AMF_H264_VIEW_EXPERT								TEXT_AMF_H264("View.Expert")
#define AMF_H264_VIEW_MASTER								TEXT_AMF_H264("View.Master")
#define AMF_H264_DEBUG										TEXT_AMF_H264("Debug")
#define AMF_H264_DEBUG_DESCRIPTION							TEXT_AMF_H264("Debug.Description")
#define AMF_H264_VERSION									TEXT_AMF_H264("Version")

/// Other
//#define AMF_H264_SCANTYPE									TEXT_AMF_H264("ScanType")
//#define AMF_H264_SCANTYPE_DESCRIPTION						TEXT_AMF_H264("ScanType.Description")
//#define AMF_H264_SCANTYPE_PROGRESSIVE						TEXT_AMF_H264("ScanType.Progressive")
//#define AMF_H264_SCANTYPE_INTERLACED						TEXT_AMF_H264("ScanType.Interlaced")
//#define AMF_H264_MAXIMUMLTRFRAMES							TEXT_AMF_H264("MaximumLTRFrames")
//#define AMF_H264_MAXIMUMLTRFRAMES_DESCRIPTION				TEXT_AMF_H264("MaximumLTRFrames.Description")
//#define AMF_H264_MAXIMUMACCESSUNITSIZE						TEXT_AMF_H264("MaximumAccessUnitSize")
//#define AMF_H264_MAXIMUMACCESSUNITSIZE_DESCRIPTION			TEXT_AMF_H264("MaximumAccessUnitSize.Description")
//#define AMF_H264_HEADER_INSERTION_SPACING					TEXT_AMF_H264("HeaderInsertionSpacing")
//#define AMF_H264_HEADER_INSERTION_SPACING_DESCRIPTION		TEXT_AMF_H264("HeaderInsertionSpacing.Description")
//#define AMF_H264_WAITFORTASK								TEXT_AMF_H264("WaitForTask")
//#define AMF_H264_WAITFORTASK_DESCRIPTION					TEXT_AMF_H264("WaitForTask.Description")
//#define AMF_H264_GOPSIZE									TEXT_AMF_H264("GOPSize")
//#define AMF_H264_GOPSIZE_DESCRIPTION						TEXT_AMF_H264("GOPSize.Description")
//#define AMF_H264_GOPALIGNMENT								TEXT_AMF_H264("GOPAlignment")
//#define AMF_H264_GOPALIGNMENT_DESCRIPTION					TEXT_AMF_H264("GOPAlignment.Description")
//#define AMF_H264_SLICESPERFRAME								TEXT_AMF_H264("SlicesPerFrame")
//#define AMF_H264_SLICESPERFRAME_DESCRIPTION					TEXT_AMF_H264("SlicesPerFrame.Description")
//#define AMF_H264_SLICEMODE									TEXT_AMF_H264("SliceMode")
//#define AMF_H264_SLICEMODE_DESCRIPTION						TEXT_AMF_H264("SliceMode.Description")
//#define AMF_H264_MAXIMUMSLICESIZE							TEXT_AMF_H264("MaximumSliceSize")
//#define AMF_H264_MAXIMUMSLICESIZE_DESCRIPTION				TEXT_AMF_H264("MaximumSliceSize.Description")
//#define AMF_H264_SLICECONTROLMODE							TEXT_AMF_H264("SliceControlMode")
//#define AMF_H264_SLICECONTROLMODE_DESCRIPTION				TEXT_AMF_H264("SliceControlMode.Description")
//#define AMF_H264_SLICECONTROLSIZE							TEXT_AMF_H264("SliceControlSize")
//#define AMF_H264_SLICECONTROLSIZE_DESCRIPTION				TEXT_AMF_H264("SliceControlSize.Description")
//#define AMF_H264_INTRAREFRESH_NUMBEROFSTRIPES				TEXT_AMF_H264("IntraRefresh.NumberOfStripes")
//#define AMF_H264_INTRAREFRESH_NUMBEROFSTRIPES_DESCRIPTION	TEXT_AMF_H264("IntraRefresh.NumberOfStripes.Description")
//#define AMF_H264_INTRAREFRESH_MACROBLOCKSPERSLOT			TEXT_AMF_H264("IntraRefresh.MacroblocksPerSlot")
//#define AMF_H264_INTRAREFRESH_MACROBLOCKSPERSLOT_DESCRIPTION	TEXT_AMF_H264("IntraRefresh.MacroblocksPerSlot.Description")

// H265


//////////////////////////////////////////////////////////////////////////
// Properties
//////////////////////////////////////////////////////////////////////////

// Shared
#define PROP_VERSION					"Version"

// H264

// H265