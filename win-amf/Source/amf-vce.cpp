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
#include "amf-vce.h"

// System
#include <exception>
#include <stdexcept>
#include <vector>
#include <chrono>

// AMF
#include "AMD-Media-SDK/1.1/inc/ErrorCodes.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/Component.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderVCE.h"

// Plugin
#include "win-amf.h"
#include "amf-vce-capabilities.h"
#include <mutex>

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////

/**
 * AMF Code Warning:
 * - SetProperty does not do any Type Checking - 64-Bit Integers will be cut in half if retrieved as 32-Bit Integers by AMF.
 * - SetProperty has different types for different values, exercise extreme caution - always try 32-bit size integers first.
 * - Asynchronous access to AMF can cause unexpected crashes, synchronize whenever possible.
 **/


AMFEncoder::VCE::VCE(VCE_Encoder_Type type) {
	AMF_RESULT res;
	VCE_Capabilities::EncoderCaps* encoderCaps;

	// Initialize Class Members to Defaults
	/// For future readers, if you're wondering why this is here, Visual
	///  Studio likes to skip the initial memset when using maximal speed
	///  optimization - even though it shouldn't hurt, we'll see the ef-
	///  fects further down the executation path.
	m_isStarted = false;
	/// Static Properties
	m_encoderType = type;
	m_memoryType = VCE_MEMORY_TYPE_HOST;
	m_surfaceFormat = VCE_SURFACE_FORMAT_NV12;
	m_usage = VCE_USAGE_TRANSCODING;
	m_qualityPreset = VCE_QUALITY_PRESET_BALANCED;
	m_profile = VCE_PROFILE_MAIN;
	m_profileLevel = VCE_PROFILE_LEVEL_42;
	m_maxLTRFrames = 0;
	m_scanType = VCE_SCANTYPE_PROGRESSIVE;
	m_frameSize = std::pair<uint32_t, uint32_t>(1280, 720);
	m_frameRate = std::pair<uint32_t, uint32_t>(1, 60);
	/// Dynamic Properties
	m_rateControlMethod = VCE_RATE_CONTROL_VARIABLE_BITRATE_PEAK_CONSTRAINED;
	m_skipFrameEnabled = false;
	m_fillerDataEnabled = false;
	m_enforceHRDEnabled = false;
	m_GOPSize = m_frameRate.second;
	m_VBVBufferSize = 20000000;
	m_initalVBVBufferFullness = 1.0;
	m_maximumAccessUnitSize = 0;
	m_BPictureDeltaQP = 4;
	m_refBPictureDeltaQP = 2;
	m_minimumQP = 18;
	m_maximumQP = 51;
	m_IFrameQP = 22;
	m_PFrameQP = 22;
	m_BFrameQP = 22;
	m_targetBitrate = 20000000;
	m_peakBitrate = 20000000;
	m_headerInsertionSpacing = 0;
	m_numberOfBPictures = 0; // AMF Default is 3.
	m_deblockingFilterEnabled = true;
	m_referenceToBFrameEnabled = false; // AMF Default is true.
	m_IDRPeriod = m_frameRate.second >> 1;
	m_intraRefreshMBPerSlotInMacrobocks = 0;
	m_numberOfSlicesPerFrame = 1;
	m_halfPixelMotionEstimationEnabled = true;
	m_quarterPixelMotionEstimationEnabled = true;
	m_numberOfTemporalEnhancementLayers = 0;
	/// Binding: AMF
	m_AMFContext = nullptr;
	m_AMFEncoder = nullptr;

	// Attempt to create an AMF Context
	res = AMFCreateContext(&m_AMFContext);
	if (res != AMF_OK)
		throwAMFError("<AMFEncoder::VCE::H264> AMFCreateContext failed, error %s (code %d).", res);

	// Create Video Coding Engine Component
	switch (m_encoderType) { // Depending on given type.
		case VCE_ENCODER_TYPE_AVC:
			AMF_LOG_INFO("<AMFEncoder::VCE::H264> Attempting to create AVC Encoder...");
			encoderCaps = &(VCE_Capabilities::getInstance()->m_AVCCaps);
			if (encoderCaps->acceleration_type != amf::AMF_ACCEL_HARDWARE) {
				AMF_LOG_WARNING("<AMFEncoder::VCE::H264> AVC Encoder is not Hardware-Accelerated!");
			}

			res = AMFCreateComponent(m_AMFContext, AMFVideoEncoderVCE_AVC, &m_AMFEncoder);
			break;
		case VCE_ENCODER_TYPE_SVC:
			AMF_LOG_INFO("<AMFEncoder::VCE::H264> Attempting to create SVC Encoder...");
			encoderCaps = &(VCE_Capabilities::getInstance()->m_SVCCaps);
			if (encoderCaps->acceleration_type != amf::AMF_ACCEL_HARDWARE) {
				AMF_LOG_WARNING("<AMFEncoder::VCE::H264> SVC Encoder is not Hardware-Accelerated!");
			}

			res = AMFCreateComponent(m_AMFContext, AMFVideoEncoderVCE_SVC, &m_AMFEncoder);
			break;
		case VCE_ENCODER_TYPE_HEVC:
			AMF_LOG_INFO("<AMFEncoder::VCE::H264> Attempting to create HEVC Encoder...");

			res = AMFCreateComponent(m_AMFContext, L"AMFVideoEncoderVCE_HEVC", &m_AMFEncoder);
			break;
	}
	if (res != AMF_OK) {
		m_AMFContext->Terminate();
		throwAMFError("<AMFEncoder::VCE::H264> AMFCreateComponent failed, error %s (code %d).", res);
	}
}

AMFEncoder::VCE::~VCE() {
	if (m_AMFEncoder)
		m_AMFEncoder->Terminate();
	if (m_AMFContext)
		m_AMFContext->Terminate();
}

void AMFEncoder::VCE::SetMemoryType(VCE_Memory_Type type) {
	char* memoryTypes[] = {
		"Host",
		"DirectX11",
		"OpenGL"
	};

	// Early-Exception if encoding.
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetMemoryType> Attempted to change while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// ToDo: Check if type is supported. Warn if not native.

	m_memoryType = type;
	AMF_LOG_INFO("<AMFEncoder::VCE::SetMemoryType> Set to %s.", memoryTypes[m_memoryType]);
}

AMFEncoder::VCE_Memory_Type AMFEncoder::VCE::GetMemoryType() {
	return m_memoryType;
}

void AMFEncoder::VCE::SetSurfaceFormat(VCE_Surface_Format format) {
	char* surfaceFormats[] = {
		"NV12",
		"I420",
		"I444",
		"RGB"
	};

	// Early-Exception if encoding.
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetSurfaceFormat> Attempted to change while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// ToDo: Check if format is supported. Warn if not native.

	m_surfaceFormat = format;
	AMF_LOG_INFO("<AMFEncoder::VCE::SetSurfaceFormat> Set to %s.", surfaceFormats[m_surfaceFormat]);
}

AMFEncoder::VCE_Surface_Format AMFEncoder::VCE::GetSurfaceFormat() {
	return m_surfaceFormat;
}

void AMFEncoder::VCE::SetUsage(VCE_Usage value) {
	AMF_RESULT res = AMF_UNEXPECTED;
	char* usages[] = {
		"Transcoding",
		"Ultra Low Latency",
		"Low Latency",
		"Webcam"
	};

	// Set
	switch (value) {
		case VCE_USAGE_TRANSCODING:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
			break;
		case VCE_USAGE_ULTRA_LOW_LATENCY:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
			break;
		case VCE_USAGE_LOW_LATENCY:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY);
			break;
		case VCE_USAGE_WEBCAM:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_WEBCAM);
			break;
	}
	if (res == AMF_OK) {
		m_usage = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetUsage> Set to %s.", usages[m_usage]);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetUsage> Failed to set to %s, error %s (code %d).", usages[m_usage], res);
	}
}

AMFEncoder::VCE_Usage AMFEncoder::VCE::GetUsage() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_USAGE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		switch (variant.ToInt64()) {
			case AMF_VIDEO_ENCODER_USAGE_TRANSCONDING:
				m_usage = VCE_USAGE_TRANSCODING;
				break;
			case AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY:
				m_usage = VCE_USAGE_ULTRA_LOW_LATENCY;
				break;
			case AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY:
				m_usage = VCE_USAGE_LOW_LATENCY;
				break;
			case AMF_VIDEO_ENCODER_USAGE_WEBCAM:
				m_usage = VCE_USAGE_WEBCAM;
				break;
		}
	} else {
		throwAMFError("<AMFEncoder::VCE::GetUsage> Failed to retrieve, error %s (code %d).", res);
	}

	return m_usage;
}

void AMFEncoder::VCE::SetQualityPreset(VCE_Quality_Preset value) {
	AMF_RESULT res = AMF_UNEXPECTED;
	char* qualities[] = {
		"Balanced",
		"Speed",
		"Quality"
	};

	// Set
	switch (value) {
		case VCE_QUALITY_PRESET_BALANCED:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, (int32_t)AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED);
			break;
		case VCE_QUALITY_PRESET_SPEED:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, (int32_t)AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
			break;
		case VCE_QUALITY_PRESET_QUALITY:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, (int32_t)AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY);
			break;
	}
	if (res == AMF_OK) {
		m_qualityPreset = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetQualityPreset> Set to %s.", qualities[m_qualityPreset]);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetQualityPreset> Failed to set to %s, error %s (code %d).", qualities[m_qualityPreset], res);
	}
}

AMFEncoder::VCE_Quality_Preset AMFEncoder::VCE::GetQualityPreset() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		switch (variant.ToInt64()) {
			case AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED:
				m_qualityPreset = VCE_QUALITY_PRESET_SPEED;
				break;
			case AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED:
				m_qualityPreset = VCE_QUALITY_PRESET_BALANCED;
				break;
			case AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY:
				m_qualityPreset = VCE_QUALITY_PRESET_QUALITY;
				break;
		}
	} else {
		throwAMFError("<AMFEncoder::VCE::GetQualityPreset> Failed to retrieve, error %s (code %d).", res);
	}

	return m_qualityPreset;
}

void AMFEncoder::VCE::SetProfile(VCE_Profile value) {
	AMF_RESULT res = AMF_UNEXPECTED;
	char* profiles[] = {
		"Baseline",
		"Main",
		"High"
	};

	// Set
	switch (value) {
		case VCE_PROFILE_BASELINE:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, (int32_t)AMF_VIDEO_ENCODER_PROFILE_BASELINE);
			break;
		case VCE_PROFILE_MAIN:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, (int32_t)AMF_VIDEO_ENCODER_PROFILE_MAIN);
			break;
		case VCE_PROFILE_HIGH:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, (int32_t)AMF_VIDEO_ENCODER_PROFILE_HIGH);
			break;
	}
	if (res == AMF_OK) {
		m_profile = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetProfile> Set to %s.", profiles[m_profile]);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetProfile> Failed to set to %s, error %s (code %d).", profiles[m_profile], res);
	}
}

AMFEncoder::VCE_Profile AMFEncoder::VCE::GetProfile() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PROFILE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		switch (variant.ToInt64()) {
			case AMF_VIDEO_ENCODER_PROFILE_BASELINE:
				m_profile = VCE_PROFILE_BASELINE;
				break;
			case AMF_VIDEO_ENCODER_PROFILE_MAIN:
				m_profile = VCE_PROFILE_MAIN;
				break;
			case AMF_VIDEO_ENCODER_PROFILE_HIGH:
				m_profile = VCE_PROFILE_HIGH;
				break;
		}
	} else {
		throwAMFError("<AMFEncoder::VCE::GetProfile> Failed to retrieve, error %s (code %d).", res);
	}

	return m_profile;
}

void AMFEncoder::VCE::SetProfileLevel(VCE_Profile_Level value) {
	AMF_RESULT res = AMF_UNEXPECTED;
	char* profiles[] = {
		"1.0", "1.1", "1.2", "1.3",
		"2.0", "2.1", "2.2",
		"3.0", "3.1", "3.2",
		"4.0", "4.1", "4.2",
		"5.0", "5.1", "5.2"
	};
	uint32_t profileToAMF[] = {
		10, 11, 12, 13,
		20, 21, 22,
		30, 31, 32,
		40, 41, 42,
		50, 51, 52
	};

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, (int32_t)profileToAMF[value]);
	if (res == AMF_OK) {
		m_profileLevel = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetProfileLevel> Set to %s.", profiles[value]);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetProfileLevel> Failed to set to %s, error %s (code %d).", profiles[value], res);
	}
}

AMFEncoder::VCE_Profile_Level AMFEncoder::VCE::GetProfileLevel() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		switch (variant.ToInt64()) {
			case 10:case 11:case 12:case 13:
				m_profileLevel = (VCE_Profile_Level)(variant.ToInt64() - 10 + VCE_PROFILE_LEVEL_10);
				break;
			case 20:case 21:case 22:
				m_profileLevel = (VCE_Profile_Level)(variant.ToInt64() - 20 + VCE_PROFILE_LEVEL_20);
				break;
			case 30:case 31:case 32:
				m_profileLevel = (VCE_Profile_Level)(variant.ToInt64() - 30 + VCE_PROFILE_LEVEL_30);
				break;
			case 40:case 41:case 42:
				m_profileLevel = (VCE_Profile_Level)(variant.ToInt64() - 40 + VCE_PROFILE_LEVEL_40);
				break;
			case 50:case 51:case 52:
				m_profileLevel = (VCE_Profile_Level)(variant.ToInt64() - 50 + VCE_PROFILE_LEVEL_50);
				break;
			default:
				AMF_LOG_WARNING("<AMFEncoder::VCE::GetProfileLevel> Unknown Profile Level %d", variant.ToInt64());
				m_profileLevel = VCE_PROFILE_LEVEL_42; // Determined by leaving at default and looking at the output.
				break;
		}
	} else {
		throwAMFError("<AMFEncoder::VCE::GetProfileLevel> Failed to retrieve, error %s (code %d).", res);
	}

	return m_profileLevel;
}

void AMFEncoder::VCE::SetMaxLTRFrames(uint32_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, (int32_t)value);
	if (res == AMF_OK) {
		m_maxLTRFrames = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetMaxOfLTRFrames> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetMaxOfLTRFrames> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint32_t AMFEncoder::VCE::GetMaxLTRFrames() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_maxLTRFrames = variant.ToInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetMaxOfLTRFrames> Failed to retrieve, error %s (code %d).", res);
	}

	return m_maxLTRFrames;
}

void AMFEncoder::VCE::SetScanType(VCE_ScanType value) {
	AMF_RESULT res = AMF_UNEXPECTED;
	char* scanTypes[] = {
		"Progressive",
		"Interlaced"
	};

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_SCANTYPE, (int32_t)value);
	if (res == AMF_OK) {
		m_scanType = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetScanType> Set to %s.", scanTypes[value]);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetScanType> Failed to set to %s, error %s (code %d).", scanTypes[value], res);
	}
}

AMFEncoder::VCE_ScanType AMFEncoder::VCE::GetScanType() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_SCANTYPE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_scanType = (VCE_ScanType)variant.ToInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetScanType> Failed to retrieve, error %s (code %d).", res);
	}

	return m_scanType;
}

void AMFEncoder::VCE::SetFrameSize(std::pair<uint32_t, uint32_t>& value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// ToDo: Verify with Capabilities.

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(value.first, value.second));
	if (res == AMF_OK) {
		m_frameSize.first = value.first;
		m_frameSize.second = value.second;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetFrameSize> Set to %dx%d.", value.first, value.second);
	} else { // Not OK? Then throw an error instead.
		std::vector<char> sizebuf(256);
		sprintf(sizebuf.data(), "%dx%d", value.first, value.second);
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetFrameSize> Failed to set to %s, error %s (code %d).", sizebuf.data(), res);
	}
}

std::pair<uint32_t, uint32_t> AMFEncoder::VCE::GetFrameSize() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_SIZE) {
		AMFSize size = variant.ToSize();
		m_frameSize.first = size.width;
		m_frameSize.second = size.height;
	} else {
		throwAMFError("<AMFEncoder::VCE::GetFrameSize> Failed to retrieve, error %s (code %d).", res);
	}

	return std::pair<uint32_t, uint32_t>(m_frameSize);
}

void AMFEncoder::VCE::SetFrameRate(std::pair<uint32_t, uint32_t>& value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(value.first, value.second));
	if (res == AMF_OK) {
		m_frameRate.first = value.first;
		m_frameRate.second = value.second;
		m_frameRateDiv = (double_t)m_frameRate.first / (double_t)m_frameRate.second; //avoid re-calc
		AMF_LOG_INFO("<AMFEncoder::VCE::SetFrameRate> Set to %d/%d.", m_frameRate.first, m_frameRate.second);
	} else { // Not OK? Then throw an error instead.
		std::vector<char> sizebuf(96);
		sprintf(sizebuf.data(), "%d/%d", value.first, value.second);
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetFrameRate> Failed to set to %s, error %s (code %d).", sizebuf.data(), res);
	}
}

std::pair<uint32_t, uint32_t> AMFEncoder::VCE::GetFrameRate() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FRAMERATE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_RATE) {
		AMFRate rate = variant.ToRate();
		m_frameRate.first = rate.num;
		m_frameRate.second = rate.den;
	} else {
		throwAMFError("<AMFEncoder::VCE::GetFrameRate> Failed to retrieve, error %s (code %d).", res);
	}

	return std::pair<uint32_t, uint32_t>(m_frameRate);
}

void AMFEncoder::VCE::SetRateControlMethod(VCE_Rate_Control_Method value) {
	AMF_RESULT res = AMF_UNEXPECTED;
	AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM methodToAMF[] = {
		AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTRAINED_QP,
		AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR,
		AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR,
		AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR
	};
	const char* methodToName[] = {
		"Constrained QP",
		"Constant Bitrate",
		"Variable Bitrate (Peak Constrained)",
		"Variable Bitrate (Latency Constrained)"
	};

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, (int32_t)methodToAMF[value]);
	if (res == AMF_OK) {
		m_rateControlMethod = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetRateControlMethod> Set to %s.", methodToName[value]);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetRateControlMethod> Failed to set to %s, error %s (code %d).", methodToName[value], res);
	}
}

AMFEncoder::VCE_Rate_Control_Method AMFEncoder::VCE::GetRateControlMethod() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;
	VCE_Rate_Control_Method amfToMethod[] = {
		VCE_RATE_CONTROL_CONSTRAINED_QUANTIZATION_PARAMETER,
		VCE_RATE_CONTROL_CONSTANT_BITRATE,
		VCE_RATE_CONTROL_VARIABLE_BITRATE_PEAK_CONSTRAINED,
		VCE_RATE_CONTROL_VARIABLE_BITRATE_LATENCY_CONSTRAINED
	};

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_rateControlMethod = amfToMethod[variant.ToInt64()];
	} else {
		throwAMFError("<AMFEncoder::VCE::GetRateControlMethod> Failed to retrieve, error %s (code %d).", res);
	}
	return m_rateControlMethod;
}

void AMFEncoder::VCE::SetFrameSkippingEnabled(bool value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, value);
	if (res == AMF_OK) {
		m_skipFrameEnabled = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::EnableFrameSkipping> Set to %s.", value ? "Enabled" : "Disabled");
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::EnableFrameSkipping> Failed to set to %s, error %s (code %d).", value ? "Enabled" : "Disabled", res);
	}
}

bool AMFEncoder::VCE::IsFrameSkippingEnabled() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_BOOL) {
		m_skipFrameEnabled = variant.ToBool();
	} else {
		throwAMFError("<AMFEncoder::VCE::IsFrameSkippingEnabled> Failed to retrieve, error %s (code %d).", res);
	}
	return m_skipFrameEnabled;
}

void AMFEncoder::VCE::SetFillerDataEnabled(bool value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, value);
	if (res == AMF_OK) {
		m_fillerDataEnabled = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::EnableFillerData> Set to %s.", value ? "Enabled" : "Disabled");
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::EnableFillerData> Failed to set to %s, error %s (code %d).", value ? "Enabled" : "Disabled", res);
	}
}

bool AMFEncoder::VCE::IsFillerDataEnabled() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_BOOL) {
		m_fillerDataEnabled = variant.ToBool();
	} else {
		throwAMFError("<AMFEncoder::VCE::IsFillerDataEnabled> Failed to retrieve, error %s (code %d).", res);
	}
	return m_fillerDataEnabled;
}

void AMFEncoder::VCE::SetEnforceHRDEnabled(bool value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_ENFORCE_HRD, (int64_t)value);
	if (res == AMF_OK) {
		m_enforceHRDEnabled = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::EnableEnforceHRD> Set to %s.", value ? "Enabled" : "Disabled");
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::EnableEnforceHRD> Failed to set to %s, error %s (code %d).", value ? "Enabled" : "Disabled", res);
	}
}

bool AMFEncoder::VCE::IsEnforceHRDEnabled() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_ENFORCE_HRD, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_BOOL) {
		m_enforceHRDEnabled = variant.ToBool();
	} else {
		AMF_LOG_INFO("%d", variant.type);
		throwAMFError("<AMFEncoder::VCE::IsEnforceHRDEnabled> Failed to retrieve, error %s (code %d).", res);
	}
	return m_enforceHRDEnabled;
}

void AMFEncoder::VCE::SetGOPSize(uint32_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_GOP_SIZE, (int32_t)value);
	if (res == AMF_OK) {
		m_GOPSize = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetGOPSize> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetGOPSize> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint32_t AMFEncoder::VCE::GetGOPSize() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_GOP_SIZE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_GOPSize = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetGOPSize> Failed to retrieve, error %s (code %d).", res);
	}
	return m_GOPSize;
}

void AMFEncoder::VCE::SetVBVBufferSize(uint32_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	uint32_t maxBitrate = VCE_Capabilities::getInstance()->getEncoderCaps(m_encoderType)->maxBitrate;
	if (value > maxBitrate) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetTargetBitrate> Bitrate value %d is out of range, limiting to 0 - %d...", maxBitrate);
		value = maxBitrate;
	}

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, (int32_t)value);
	if (res == AMF_OK) {
		m_VBVBufferSize = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetVBVBufferSize> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetVBVBufferSize> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint32_t AMFEncoder::VCE::GetVBVBufferSize() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_VBVBufferSize = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetVBVBufferSize> Failed to retrieve, error %s (code %d).", res);
	}
	return m_VBVBufferSize;
}

void AMFEncoder::VCE::SetInitialVBVBufferFullness(double_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Limit value
	if (value < 0) {
		value = 0;
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetInitialVBVBufferFullness> QP value %f is out of range, limiting to 0 - 1...", value);
	} else if (value > 1) {
		value = 1;
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetInitialVBVBufferFullness> QP value %f is out of range, limiting to 0 - 1...", value);
	}

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS, (int32_t)(value * 64));
	if (res == AMF_OK) {
		m_initalVBVBufferFullness = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetInitialVBVBufferFullness> Set to %f.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetInitialVBVBufferFullness> Failed to set to %f, error %s (code %d).", value, res);
	}
}

double_t AMFEncoder::VCE::GetInitialVBVBufferFullness() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_initalVBVBufferFullness = (variant.ToInt64() / 64.0);
	} else {
		throwAMFError("<AMFEncoder::VCE::GetInitialVBVBufferFullness> Failed to retrieve, error %s (code %d).", res);
	}
	return m_initalVBVBufferFullness;
}

void AMFEncoder::VCE::SetMaximumAccessUnitSize(uint32_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Limit value
	if (value > 1000) {
		value = 1000;
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetMaximumAccessUnitSize> AU Size %d is out of range, limiting to 0 - 1000...", value);
	}

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_AU_SIZE, (int32_t)value);
	if (res == AMF_OK) {
		m_maximumAccessUnitSize = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetMaximumAccessUnitSize> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetMaximumAccessUnitSize> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint32_t AMFEncoder::VCE::GetMaximumAccessUnitSize() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_AU_SIZE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_maximumAccessUnitSize = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetMaximumAccessUnitSize> Failed to retrieve, error %s (code %d).", res);
	}
	return m_maximumAccessUnitSize;
}

void AMFEncoder::VCE::SetBPictureDeltaQP(int8_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Limit value
	if (value < -10) {
		value = -10;
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetReferenceBPictureDeltaQP> QP value %d is out of range, limiting to -10 - 10...", value);
	} else if (value > 10) {
		value = 10;
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetReferenceBPictureDeltaQP> QP value %d is out of range, limiting to -10 - 10...", value);
	}

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, (int32_t)value);
	if (res == AMF_OK) {
		m_BPictureDeltaQP = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetBPictureDeltaQP> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetBPictureDeltaQP> Failed to set to %d, error %s (code %d).", value, res);
	}
}

int8_t AMFEncoder::VCE::GetBPictureDeltaQP() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_BPictureDeltaQP = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetBPictureDeltaQP> Failed to retrieve, error %s (code %d).", res);
	}
	return m_BPictureDeltaQP;
}

void AMFEncoder::VCE::SetReferenceBPictureDeltaQP(int8_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Limit value
	if (value < -10) {
		value = -10;
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetReferenceBPictureDeltaQP> QP value %d is out of range, limiting to -10 - 10...", value);
	} else if (value > 10) {
		value = 10;
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetReferenceBPictureDeltaQP> QP value %d is out of range, limiting to -10 - 10...", value);
	}

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, (int32_t)value);
	if (res == AMF_OK) {
		m_refBPictureDeltaQP = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetReferenceBPictureDeltaQP> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetReferenceBPictureDeltaQP> Failed to set to %d, error %s (code %d).", value, res);
	}
}

int8_t AMFEncoder::VCE::GetReferenceBPictureDeltaQP() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_refBPictureDeltaQP = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetReferenceBPictureDeltaQP> Failed to retrieve, error %s (code %d).", res);
	}
	return m_refBPictureDeltaQP;
}

void AMFEncoder::VCE::SetMinimumQP(uint8_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	if (value > 51) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetMaximumQP> QP value %d is out of range, limiting to 0 - 51...", value);
		value = 51;
	}

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MIN_QP, (int32_t)value);
	if (res == AMF_OK) {
		m_minimumQP = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetMinimumQP> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetMinimumQP> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint8_t AMFEncoder::VCE::GetMinimumQP() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MIN_QP, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_minimumQP = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetMinimumQP> Failed to retrieve, error %s (code %d).", res);
	}
	return m_minimumQP;
}

void AMFEncoder::VCE::SetMaximumQP(uint8_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	if (value > 51) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetMaximumQP> QP value %d is out of range, limiting to 0 - 51...", value);
		value = 51;
	}

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_QP, (int32_t)value);
	if (res == AMF_OK) {
		m_maximumQP = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetMaximumQP> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetMaximumQP> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint8_t AMFEncoder::VCE::GetMaximumQP() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_QP, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_maximumQP = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetMaximumQP> Failed to retrieve, error %s (code %d).", res);
	}
	return m_maximumQP;
}

void AMFEncoder::VCE::SetIFrameQP(uint8_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	if (value > 51) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetIFrameQP> QP value %d is out of range, limiting to 0 - 51...", value);
		value = 51;
	}

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_I, (int32_t)value);
	if (res == AMF_OK) {
		m_IFrameQP = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetIFrameQP> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetIFrameQP> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint8_t AMFEncoder::VCE::GetIFrameQP() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_I, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_IFrameQP = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetIFrameQP> Failed to retrieve, error %s (code %d).", res);
	}
	return m_IFrameQP;
}

void AMFEncoder::VCE::SetPFrameQP(uint8_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	if (value > 51) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetPFrameQP> QP value %d is out of range, limiting to 0 - 51...", value);
		value = 51;
	}

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_P, (int32_t)value);
	if (res == AMF_OK) {
		m_PFrameQP = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetPFrameQP> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetPFrameQP> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint8_t AMFEncoder::VCE::GetPFrameQP() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_P, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_PFrameQP = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetPFrameQP> Failed to retrieve, error %s (code %d).", res);
	}
	return m_PFrameQP;
}

void AMFEncoder::VCE::SetBFrameQP(uint8_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	if (value > 51) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetBFrameQP> QP value %d is out of range, limiting to 0 - 51...", value);
		value = 51;
	}

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_B, (int32_t)value);
	if (res == AMF_OK) {
		m_BFrameQP = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetBFrameQP> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetBFrameQP> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint8_t AMFEncoder::VCE::GetBFrameQP() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_B, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_BFrameQP = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetReferenceBPictureDeltaQP> Failed to retrieve, error %s (code %d).", res);
	}
	return m_BFrameQP;
}

void AMFEncoder::VCE::SetTargetBitrate(uint32_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	uint32_t maxBitrate = VCE_Capabilities::getInstance()->getEncoderCaps(m_encoderType)->maxBitrate;
	if (value > maxBitrate) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetTargetBitrate> Bitrate value %d is out of range, limiting to 0 - %d...", maxBitrate);
		value = maxBitrate;
	}

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, (uint32_t)value);
	if (res == AMF_OK) {
		m_targetBitrate = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetTargetBitrate> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetTargetBitrate> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint32_t AMFEncoder::VCE::GetTargetBitrate() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_targetBitrate = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetTargetBitrate> Failed to retrieve, error %s (code %d).", res);
	}
	return m_targetBitrate;
}

void AMFEncoder::VCE::SetPeakBitrate(uint32_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	uint32_t maxBitrate = VCE_Capabilities::getInstance()->getEncoderCaps(m_encoderType)->maxBitrate;
	if (value > maxBitrate) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetPeakBitrate> Bitrate value %d is out of range, limiting to 0 - %d...", maxBitrate);
		value = maxBitrate;
	}

	// Set
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, (uint32_t)value);
	if (res == AMF_OK) {
		m_peakBitrate = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetPeakBitrate> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetPeakBitrate> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint32_t AMFEncoder::VCE::GetPeakBitrate() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_peakBitrate = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetPeakBitrate> Failed to retrieve, error %s (code %d).", res);
	}
	return m_peakBitrate;
}

void AMFEncoder::VCE::SetHeaderInsertionSpacing(uint16_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING, (int32_t)value);
	if (res == AMF_OK) {
		m_headerInsertionSpacing = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetHeaderInsertionSpacing> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetHeaderInsertionSpacing> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint16_t AMFEncoder::VCE::GetHeaderInsertionSpacing() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_headerInsertionSpacing = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetHeaderInsertionSpacing> Failed to retrieve, error %s (code %d).", res);
	}
	return m_headerInsertionSpacing;
}

void AMFEncoder::VCE::SetNumberOfBPictures(uint8_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, (int32_t)value);
	if (res == AMF_OK) {
		m_numberOfBPictures = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetNumberOfBPictures> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetNumberOfBPictures> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint8_t AMFEncoder::VCE::GetNumberOfBPictures() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_numberOfBPictures = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetNumberOfBPictures> Failed to retrieve, error %s (code %d).", res);
	}
	return m_numberOfBPictures;
}

void AMFEncoder::VCE::SetDeblockingFilterEnabled(bool value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER, value);
	if (res == AMF_OK) {
		m_deblockingFilterEnabled = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetDeblockingFilterEnabled> Set to %s.", value ? "Enabled" : "Disabled");
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetDeblockingFilterEnabled> Failed to set to %s, error %s (code %d).", value ? "Enabled" : "Disabled", res);
	}
}

bool AMFEncoder::VCE::IsDeblockingFilterEnabled() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_BOOL) {
		m_deblockingFilterEnabled = variant.ToBool();
	} else {
		throwAMFError("<AMFEncoder::VCE::IsDeblockingFilterEnabled> Failed to retrieve, error %s (code %d).", res);
	}
	return m_deblockingFilterEnabled;
}

void AMFEncoder::VCE::SetReferenceToBFrameEnabled(bool value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, value);
	if (res == AMF_OK) {
		m_referenceToBFrameEnabled = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetReferenceToBFrameEnabled> Set to %s.", value ? "Enabled" : "Disabled");
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetReferenceToBFrameEnabled> Failed to set to %s, error %s (code %d).", value ? "Enabled" : "Disabled", res);
	}
}

bool AMFEncoder::VCE::IsReferenceToBFrameEnabled() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_BOOL) {
		m_referenceToBFrameEnabled = variant.ToBool();
	} else {
		throwAMFError("<AMFEncoder::VCE::IsReferenceToBFrameEnabled> Failed to retrieve, error %s (code %d).", res);
	}
	return m_referenceToBFrameEnabled;
}

void AMFEncoder::VCE::SetIDRPeriod(uint32_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Up to 1000 according to AMF

	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, (uint32_t)value);
	if (res == AMF_OK) {
		m_IDRPeriod = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetIDRPeriod> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetIDRPeriod> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint32_t AMFEncoder::VCE::GetIDRPeriod() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_IDRPeriod = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetIDRPeriod> Failed to retrieve, error %s (code %d).", res);
	}
	return m_IDRPeriod;
}

void AMFEncoder::VCE::SetInfraRefreshMBsPerSlotInMacroblocks(uint32_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, (int32_t)value);
	if (res == AMF_OK) {
		m_intraRefreshMBPerSlotInMacrobocks = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetInfraRefreshMBsPerSlotInMacroblocks> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetInfraRefreshMBsPerSlotInMacroblocks> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint32_t AMFEncoder::VCE::GetInfraRefreshMBsPerSlotInMacroblocks() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_intraRefreshMBPerSlotInMacrobocks = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetInfraRefreshMBsPerSlotInMacroblocks> Failed to retrieve, error %s (code %d).", res);
	}
	return m_intraRefreshMBPerSlotInMacrobocks;
}

void AMFEncoder::VCE::SetNumberOfSlicesPerFrame(uint32_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, (int32_t)value);
	if (res == AMF_OK) {
		m_numberOfSlicesPerFrame = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetNumberOfSlicesPerFrame> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetNumberOfSlicesPerFrame> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint32_t AMFEncoder::VCE::GetNumberOfSlicesPerFrame() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_numberOfSlicesPerFrame = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetNumberOfSlicesPerFrame> Failed to retrieve, error %s (code %d).", res);
	}
	return m_numberOfSlicesPerFrame;
}

void AMFEncoder::VCE::SetHalfPixelMotionEstimationEnabled(bool value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL, value);
	if (res == AMF_OK) {
		m_halfPixelMotionEstimationEnabled = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetHalfPixelMotionEstimationEnabled> Set to %s.", value ? "Enabled" : "Disabled");
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetHalfPixelMotionEstimationEnabled> Failed to set to %s, error %s (code %d).", value ? "Enabled" : "Disabled", res);
	}
}

bool AMFEncoder::VCE::GetHalfPixelMotionEstimationEnabled() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_BOOL) {
		m_halfPixelMotionEstimationEnabled = variant.ToBool();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetHalfPixelMotionEstimationEnabled> Failed to retrieve, error %s (code %d).", res);
	}
	return m_halfPixelMotionEstimationEnabled;
}

void AMFEncoder::VCE::SetQuarterPixelMotionEstimationEnabled(bool value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL, value);
	if (res == AMF_OK) {
		m_quarterPixelMotionEstimationEnabled = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetQuarterPixelMotionEstimationEnabled> Set to %s.", value ? "Enabled" : "Disabled");
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetQuarterPixelMotionEstimationEnabled> Failed to set to %s, error %s (code %d).", value ? "Enabled" : "Disabled", res);
	}
}

bool AMFEncoder::VCE::GetQuarterPixelMotionEstimationEnabled() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_BOOL) {
		m_quarterPixelMotionEstimationEnabled = variant.ToBool();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetQuarterPixelMotionEstimationEnabled> Failed to retrieve, error %s (code %d).", res);
	}
	return m_quarterPixelMotionEstimationEnabled;
}

void AMFEncoder::VCE::SetNumberOfTemporalEnhancementLayers(uint32_t value) {
	AMF_RESULT res = AMF_UNEXPECTED;

	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS, (int32_t)value);
	if (res == AMF_OK) {
		m_numberOfTemporalEnhancementLayers = value;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetNumberOfTemporalEnhancementLayers> Set to %d.", value);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetNumberOfTemporalEnhancementLayers> Failed to set to %d, error %s (code %d).", value, res);
	}
}

uint32_t AMFEncoder::VCE::GetNumberOfTemporalEnhancementLayers() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_numberOfTemporalEnhancementLayers = variant.ToUInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetNumberOfTemporalEnhancementLayers> Failed to retrieve, error %s (code %d).", res);
	}
	return m_numberOfTemporalEnhancementLayers;
}

void AMFEncoder::VCE::Start() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMF_SURFACE_FORMAT surfaceFormatToAMF[] = {
		amf::AMF_SURFACE_NV12,
		amf::AMF_SURFACE_YUV420P,
		(amf::AMF_SURFACE_FORMAT)(amf::AMF_SURFACE_LAST + 1), // YUV444, but missing from SDK. (Experimental)
		amf::AMF_SURFACE_RGBA
	};

	// Early-Exception if encoding.
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::Start> Attempted to start again while already running.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Initialize AMF Encoder.
	res = m_AMFEncoder->Init(surfaceFormatToAMF[m_surfaceFormat], m_frameSize.first, m_frameSize.second);
	if (res != AMF_OK) {
		throwAMFError("<AMFEncoder::VCE::Start> Unable to start, error %s (code %d).", res);
	} else {
		m_isStarted = true;

		#ifdef THREADED // Threading
		m_InputThread = std::thread(&InputThreadMain, this);
		m_OutputThread = std::thread(&OutputThreadMain, this);
		#endif
	}
}

void AMFEncoder::VCE::Stop() {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Early-Exception if encoding.
	if (!m_isStarted) {
		const char* error = "<AMFEncoder::VCE::Stop> Attempted to stop while not running.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Reset Started State
	m_isStarted = false;

	#ifdef THREADED // Threading
	m_InputThreadData.m_condVar.notify_all();
	m_InputThread.join();
	m_OutputThreadData.m_condVar.notify_all();
	m_OutputThread.join();
	#endif

	// Terminate AMF Encoder Session
	res = m_AMFEncoder->Terminate();
	if (res != AMF_OK) {
		throwAMFError("<AMFEncoder::VCE::Stop> Unable to stop, error %s (code %d).", res);
	}

}

bool AMFEncoder::VCE::SendInput(struct encoder_frame*& frame) {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFSurfacePtr pSurface;

	// Early-Exception if not encoding.
	if (!m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SendInput> Attempted to send input while not running.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Submit Input
	pSurface = CreateSurfaceFromFrame(frame);
	#ifdef THREADED
	/// Queue Frame 
	{ /// Open a new Scope to quickly unlock the mutex again.
		std::unique_lock<std::mutex> lock(m_InputThreadData.m_mutex);
		if (m_InputThreadData.m_queue.size() < AMF_VCE_MAX_QUEUED_FRAMES) {
			m_InputThreadData.m_queue.push(pSurface);
			/// Signal Thread Wakeup
			m_InputThreadData.m_condVar.notify_all();
		} else {
			AMF_LOG_ERROR("<AMFEncoder::VCE::SendInput> Input Queue is full, aborting...");
			return false;
		}
	}
	#else
	res = m_AMFEncoder->SubmitInput(pSurface);
	if (res == AMF_INPUT_FULL) { // Input Queue is Full
		std::vector<char> msgBuf(1024);
		formatAMFError(&msgBuf, "<AMFEncoder::VCE::SendInput> Input Queue is full, error %s (code %d).", res);
		AMF_LOG_ERROR("%s", msgBuf.data());
	} else {// Unable to submit Surface
		std::vector<char> msgBuf(1024);
		formatAMFError(&msgBuf, "<AMFEncoder::VCE::SendInput> Unable to submit input, error %s (code %d).", res);
		AMF_LOG_ERROR("%s", msgBuf.data());
		return false;
	}
	#endif

	return true;
}

void AMFEncoder::VCE::GetOutput(struct encoder_packet*& packet, bool*& received_packet) {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFDataPtr pData;

	// Early-Exception if not encoding.
	if (!m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SendInput> Attempted to send input while not running.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Query Output
	#ifdef THREADED // Use Threaded Model
	// Signal Thread Wakeup
	m_OutputThreadData.m_condVar.notify_all();

	// Check Queue
	{
		std::unique_lock<std::mutex> lock(m_OutputThreadData.m_mutex);
		if (m_OutputThreadData.m_queue.size() == 0) {
			*received_packet = false;
			return;
		}

		// Submit Packet to OBS
		OutputThreadPacket pkt = m_OutputThreadData.m_queue.front();
		/// Copy to Static Buffer
		size_t bufferSize = pkt.data.size();
		if (m_PacketDataBuffer.size() < bufferSize) {
			size_t newSize = (size_t)exp2(ceil(log2(bufferSize)));
			m_PacketDataBuffer.resize(newSize);
			AMF_LOG_WARNING("<AMFEncoder::VCE::GetOutput> Resized Packet Buffer to %d.", newSize);
		}
		if ((bufferSize > 0) && (m_PacketDataBuffer.data()))
			std::memcpy(m_PacketDataBuffer.data(), pkt.data.data(), bufferSize);

		/// Set up Packet Information
		packet->type = OBS_ENCODER_VIDEO;
		packet->size = bufferSize;
		packet->data = m_PacketDataBuffer.data();
		packet->pts = pkt.frameIndex;
		packet->dts = packet->pts;
		switch (pkt.dataType) {
			case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_IDR://
				packet->keyframe = true;				// IDR-Frames are Keyframes that contain a lot of information.
				packet->priority = 3;					// Highest priority, always continue streaming with these.
				packet->drop_priority = 3;				// Dropped IDR-Frames can only be replaced by the next IDR-Frame.
				break;
			case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_I:	// I-Frames need only a previous I- or IDR-Frame.
				packet->priority = 2;					// I- and IDR-Frames will most likely be present.
				packet->drop_priority = 2;				// So we can continue with a I-Frame when streaming.
				break;
			case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_P:	// P-Frames need either a previous P-, I- or IDR-Frame.
				packet->priority = 1;					// We can safely assume that at least one of these is present.
				packet->drop_priority = 1;				// So we can continue with a P-Frame when streaming.
				break;
			case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_B:	// B-Frames need either a parent B-, P-, I- or IDR-Frame.
				packet->priority = 0;					// We don't know if the last non-dropped frame was a B-Frame.
				packet->drop_priority = 0;				// So require a P-Frame or better to continue streaming.
				break;
		}

		// Remove front() element.
		m_OutputThreadData.m_queue.pop();
	}
	#else
	// Query Output
	res = m_AMFEncoder->QueryOutput(&pData);
	if (res != AMF_OK) {
		std::vector<char> msgBuf(1024);
		formatAMFError(&msgBuf, "<AMFEncoder::VCE::GetOutput> Unable to query output, error %s (code %d).", res);
		AMF_LOG_ERROR("%s", msgBuf.data());
		*received_packet = false;
		return;
	}

	// Send to OBS
	amf::AMFBufferPtr pBuffer(pData);

	/// Copy to Static Buffer
	size_t bufferSize = pBuffer->GetSize();
	if (m_PacketDataBuffer.size() < bufferSize) {
		size_t newSize = (size_t)exp2(ceil(log2(bufferSize)));
		m_PacketDataBuffer.resize(newSize);
		std::vector<char> msgBuf(1024);
		formatAMFError(&msgBuf, "<AMFEncoder::VCE::GetOutput> Resized Packet Buffer, error %s (code %d).", res);
		AMF_LOG_WARNING("%s", msgBuf.data());
	}
	if ((bufferSize > 0) && (m_PacketDataBuffer.data()))
		std::memcpy(m_PacketDataBuffer.data(), pBuffer->GetNative(), bufferSize);

	/// Set up Packet Information
	packet->type = OBS_ENCODER_VIDEO;
	packet->size = bufferSize;
	packet->data = m_PacketDataBuffer.data();
	packet->pts = int64_t(pData->GetPts() * m_frameRateDiv / 1e7);
	packet->dts = packet->pts;
	{
		amf::AMFVariant variant;
		res = pData->GetProperty(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, &variant);
		if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
			AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM dataType = (AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM)variant.ToUInt64();
			switch (dataType) {
				case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_IDR://
					packet->keyframe = true;				// IDR-Frames are Keyframes that contain a lot of information.
					packet->priority = 3;					// Highest priority, always continue streaming with these.
					packet->drop_priority = 3;				// Dropped IDR-Frames can only be replaced by the next IDR-Frame.
					break;
				case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_I:	// I-Frames need only a previous I- or IDR-Frame.
					packet->priority = 2;					// I- and IDR-Frames will most likely be present.
					packet->drop_priority = 2;				// So we can continue with a I-Frame when streaming.
					break;
				case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_P:	// P-Frames need either a previous P-, I- or IDR-Frame.
					packet->priority = 1;					// We can safely assume that at least one of these is present.
					packet->drop_priority = 1;				// So we can continue with a P-Frame when streaming.
					break;
				case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_B:	// B-Frames need either a parent B-, P-, I- or IDR-Frame.
					packet->priority = 0;					// We don't know if the last non-dropped frame was a B-Frame.
					packet->drop_priority = 0;				// So require a P-Frame or better to continue streaming.
					break;
			}
		}
	}
	#endif

	#ifdef DEBUG
	AMF_LOG_INFO("Packet: Priority(%d), DropPriority(%d), PTS(%d), Size(%d)", packet->priority, packet->drop_priority, packet->pts, packet->size);
	#endif

	*received_packet = true;
}

bool AMFEncoder::VCE::GetExtraData(uint8_t**& extra_data, size_t*& extra_data_size) {
	if (!m_AMFContext || !m_AMFEncoder)
		throw std::exception("<AMFEncoder::VCE::GetExtraData> Called while not initialized.");

	if (!m_isStarted)
		throw std::exception("<AMFEncoder::VCE::GetExtraData> Called while not encoding.");

	amf::AMFVariant var;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_EXTRADATA, &var);
	if (res == AMF_OK && var.type == amf::AMF_VARIANT_INTERFACE) {
		amf::AMFBufferPtr buf(var.pInterface);

		*extra_data_size = buf->GetSize();
		m_ExtraDataBuffer.resize(*extra_data_size);
		std::memcpy(m_ExtraDataBuffer.data(), buf->GetNative(), *extra_data_size);
		*extra_data = m_ExtraDataBuffer.data();

		buf->Release();

		return true;
	}
	return false;
}

void AMFEncoder::VCE::GetVideoInfo(struct video_scale_info*& info) {
	if (!m_AMFContext || !m_AMFEncoder)
		throw std::exception("<AMFEncoder::VCE::GetVideoInfo> Called while not initialized.");

	if (!m_isStarted)
		throw std::exception("<AMFEncoder::VCE::GetVideoInfo> Called while not encoding.");

	switch (m_surfaceFormat) {
		case VCE_SURFACE_FORMAT_NV12:
			info->format = VIDEO_FORMAT_NV12;
			break;
		case VCE_SURFACE_FORMAT_I420:
			info->format = VIDEO_FORMAT_I420;
			break;
		case VCE_SURFACE_FORMAT_I444:
			info->format = VIDEO_FORMAT_I444;
			break;
		case VCE_SURFACE_FORMAT_RGB:
			info->format = VIDEO_FORMAT_RGBA;
			break;
	}
	//info->range = VIDEO_RANGE_PARTIAL;
}

amf::AMFSurfacePtr inline AMFEncoder::VCE::CreateSurfaceFromFrame(struct encoder_frame*& frame) {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFSurfacePtr pSurface = nullptr;
	amf::AMF_SURFACE_FORMAT surfaceFormatToAMF[] = {
		amf::AMF_SURFACE_NV12,
		amf::AMF_SURFACE_YUV420P,
		(amf::AMF_SURFACE_FORMAT)(amf::AMF_SURFACE_LAST + 1), // YUV444, but missing from SDK. (Experimental)
		amf::AMF_SURFACE_RGBA
	};
	amf::AMF_MEMORY_TYPE memoryTypeToAMF[] = {
		amf::AMF_MEMORY_HOST,
		amf::AMF_MEMORY_DX11,
		amf::AMF_MEMORY_OPENGL
	};

	if (m_memoryType == VCE_MEMORY_TYPE_HOST) {
		#pragma region Host Memory Type
		size_t planeCount;

		AMF_SYNC_LOCK(
			res = m_AMFContext->AllocSurface(
				memoryTypeToAMF[m_memoryType], surfaceFormatToAMF[m_surfaceFormat],
				m_frameSize.first, m_frameSize.second,
				&pSurface);
		planeCount = pSurface->GetPlanesCount();
		);

		switch (m_surfaceFormat) {
			case VCE_SURFACE_FORMAT_NV12:
			{ // NV12, Y:U+V, Two Plane
				#pragma loop(hint_parallel(2))
				for (uint8_t i = 0; i < planeCount; i++) {
					amf::AMFPlane* plane;
					void* plane_nat;
					int32_t height;
					size_t hpitch;

					AMF_SYNC_LOCK(
						plane = pSurface->GetPlaneAt(i);
					plane_nat = plane->GetNative();
					height = plane->GetHeight();
					hpitch = plane->GetHPitch();
					);

					for (int32_t py = 0; py < height; py++) {
						size_t plane_off = py * hpitch;
						size_t frame_off = py * frame->linesize[i];
						std::memcpy(static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off), static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
					}
				}
				break;
			}
			case VCE_SURFACE_FORMAT_I420:
			{	// YUV 4:2:0, Y, subsampled U, subsampled V
				#pragma loop(hint_parallel(3))
				for (uint8_t i = 0; i < planeCount; i++) {
					amf::AMFPlane* plane;
					void* plane_nat;
					int32_t height;
					size_t hpitch;

					AMF_SYNC_LOCK(
						plane = pSurface->GetPlaneAt(i);
					plane_nat = plane->GetNative();
					height = plane->GetHeight();
					hpitch = plane->GetHPitch();
					);

					for (int32_t py = 0; py < height; py++) {
						size_t plane_off = py * hpitch;
						size_t frame_off = py * frame->linesize[i];
						std::memcpy(static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off), static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
					}
				}
				break;
			}
			case VCE_SURFACE_FORMAT_I444:
			{
				break;
			}
			case VCE_SURFACE_FORMAT_RGB:
			{ // RGBA, Single Plane
				for (uint8_t i = 0; i < planeCount; i++) {
					amf::AMFPlane* plane;
					void* plane_nat;
					int32_t height;
					size_t hpitch;

					AMF_SYNC_LOCK(
						plane = pSurface->GetPlaneAt(i);
					plane_nat = plane->GetNative();
					height = plane->GetHeight();
					hpitch = plane->GetHPitch();
					);

					#pragma loop(hint_parallel(4))
					for (int32_t py = 0; py < height; py++) {
						size_t plane_off = py * hpitch;
						size_t frame_off = py * frame->linesize[i];
						std::memcpy(static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off), static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
					}
				}
				break;
			}
		}
		#pragma endregion Host Memory Type
		if (res != AMF_OK) // Unable to create Surface
			throwAMFError("<AMFEncoder::VCE::SendInput> Unable to create AMFSurface, error %s (code %d).", res);

		AMF_SYNC_LOCK(
			amf_pts amfPts = (int64_t)ceil((frame->pts / ((double_t)m_frameRate.first / (double_t)m_frameRate.second)) * 10000000l);//(1 * 1000 * 1000 * 10)
		pSurface->SetPts(amfPts);
		pSurface->SetProperty(AMF_VCE_PROPERTY_FRAMEINDEX, frame->pts);
		);
	}

	return pSurface;
}

#ifdef THREADED // Threading
void AMFEncoder::VCE::InputThreadMain(VCE* cls) {
	cls->InputThreadMethod();
}

void AMFEncoder::VCE::OutputThreadMain(VCE* cls) {
	cls->OutputThreadMethod();
}

void AMFEncoder::VCE::InputThreadMethod() {
	// Thread Loop that handles Surface Submission
	std::unique_lock<std::mutex> lock(m_InputThreadData.m_mutex);
	do {
		m_InputThreadData.m_condVar.wait(lock);

		// Skip to check if isStarted is false.
		if (!m_isStarted)
			continue;

		// Skip to next wait if queue is empty.
		AMF_RESULT res = AMF_OK;
		while ((m_InputThreadData.m_queue.size() > 0) && res == AMF_OK) { // Repeat until impossible.
			amf::AMFSurfacePtr surface = m_InputThreadData.m_queue.front();

			AMF_SYNC_LOCK(AMF_RESULT res = m_AMFEncoder->SubmitInput(surface););
			if (res == AMF_OK) {
				m_InputThreadData.m_queue.pop();
			} else if (res != AMF_INPUT_FULL) {
				std::vector<char> msgBuf(128);
				formatAMFError(&msgBuf, "%s (code %d)", res);
				AMF_LOG_WARNING("<AMFENcoder::VCE::InputThreadMethod> SubmitInput failed with error %s.", msgBuf.data());
			}

			if (m_InputThreadData.m_queue.size() > 10) // Magic number for now.
				AMF_LOG_WARNING("<AMFENcoder::VCE::InputThreadMethod> Input Queue is filling up. (%d of %d)", m_InputThreadData.m_queue.size(), AMF_VCE_MAX_QUEUED_FRAMES);
		}
	} while (m_isStarted);
}

void AMFEncoder::VCE::OutputThreadMethod() {
	// Thread Loop that handles Querying
	uint64_t lastFrameIndex = -1;

	std::unique_lock<std::mutex> lock(m_InputThreadData.m_mutex);
	do {
		m_InputThreadData.m_condVar.wait(lock);

		// Skip to check if isStarted is false.
		if (!m_isStarted)
			continue;

		AMF_RESULT res = AMF_OK;
		while (res == AMF_OK) { // Repeat until impossible.
			amf::AMFDataPtr pData;

			AMF_SYNC_LOCK(res = m_AMFEncoder->QueryOutput(&pData););
			if (res == AMF_OK) {
				amf::AMFBufferPtr pBuffer(pData);
				amf::AMFVariant variant;
				OutputThreadPacket pkt;

				// Create a Packet
				pkt.data.resize(pBuffer->GetSize());
				std::memcpy(pkt.data.data(), pBuffer->GetNative(), pkt.data.size());
				pkt.frameIndex = uint64_t(pData->GetPts() * (m_frameRateDiv / 1e7)); // Not sure what way around the accuracy is better.
				AMF_SYNC_LOCK(AMF_RESULT res2 = pData->GetProperty(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, &variant);
				if (res2 == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
					pkt.dataType = (AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM)variant.ToUInt64();
				});
				if (lastFrameIndex >= pkt.frameIndex)
					AMF_LOG_ERROR("<AMFENcoder::VCE::OutputThreadMethod> Detected out of order packet. Frame Index is %d, expected %d.", pkt.frameIndex, lastFrameIndex + 1);
				lastFrameIndex = pkt.frameIndex;

				// Queue
				m_OutputThreadData.m_queue.push(pkt);
			} else if (res != AMF_REPEAT) {
				std::vector<char> msgBuf(128);
				formatAMFError(&msgBuf, "%s (code %d)", res);
				AMF_LOG_WARNING("<AMFENcoder::VCE::OutputThreadMethod> QueryOutput failed with error %s.", msgBuf.data());
			}
		}
	} while (m_isStarted);
}
#endif

void AMFEncoder::VCE::throwAMFError(const char* errorMsg, AMF_RESULT res) {
	std::vector<char> msgBuf(1024);
	formatAMFError(&msgBuf, errorMsg, res);
	AMF_LOG_ERROR("%s", msgBuf.data());
	throw std::exception(msgBuf.data());
}

void AMFEncoder::VCE::formatAMFError(std::vector<char>* buffer, const char* format, AMF_RESULT res) {
	std::vector<char> errBuf(1024);
	wcstombs(errBuf.data(), amf::AMFGetResultText(res), errBuf.size());
	sprintf(buffer->data(), format, errBuf.data(), res);
}

template<typename _T>
void AMFEncoder::VCE::formatAMFErrorAdvanced(std::vector<char>* buffer, const char* format, _T other, AMF_RESULT res) {
	std::vector<char> errBuf(1024);
	wcstombs(errBuf.data(), amf::AMFGetResultText(res), errBuf.size());
	sprintf(buffer->data(), format, other, errBuf.data(), res);
}

template<typename _T>
void AMFEncoder::VCE::throwAMFErrorAdvanced(const char* errorMsg, _T other, AMF_RESULT res) {
	std::vector<char> msgBuf(1024);
	formatAMFErrorAdvanced(&msgBuf, errorMsg, other, res);
	AMF_LOG_ERROR("%s", msgBuf.data());
	throw std::exception(msgBuf.data());
}
