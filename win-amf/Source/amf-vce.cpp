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

// AMF
#include "AMD-Media-SDK/1.1/inc/ErrorCodes.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/Component.h"
#include "AMD-Media-SDK/1.1/inc/amf/components/VideoEncoderVCE.h"

// Plugin
#include "win-amf.h"
#include "amf-vce-capabilities.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
AMFEncoder::VCE::VCE(VCE_Encoder_Type encoderType) {
	AMF_RESULT res;
	VCE_Capabilities::EncoderCaps* encoderCaps;

		// Set Encoder Type
	m_encoderType = encoderType;

	// Create AMF Context
	res = AMFCreateContext(&m_AMFContext);
	if (res != AMF_OK)
		throwAMFError("<AMFEncoder::VCE::H264> AMFCreateContext failed, error %s (code %d).", res);

	// Create AMF VCE Component depending on Type.
	switch (m_encoderType) {
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
	} else {
		// Apply Defaults
		if (m_encoderType == VCE_ENCODER_TYPE_SVC)
			SetUsage(VCE_USAGE_WEBCAM); // PipelineEncoder Example has this
		else
			SetUsage(VCE_USAGE_TRANSCODING);
		//SetQualityPreset(VCE_QUALITY_PRESET_BALANCED);

		// Gather Information from Encoder
		//GetUsage();
		//GetQualityPreset();
		//GetProfile();
		//GetProfileLevel();
		//GetMaxLTRFrames();
		//GetScanType();
		//GetFrameSize();
		//GetFrameRate();
	}
}

AMFEncoder::VCE::~VCE() {
	if (m_AMFEncoder)
		m_AMFEncoder->Terminate();
	if (m_AMFContext)
		m_AMFContext->Terminate();
}

void AMFEncoder::VCE::SetMemoryType(VCE_Memory_Type memoryType) {
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

	m_memoryType = memoryType;
	AMF_LOG_INFO("<AMFEncoder::VCE::SetMemoryType> Set to %s.", memoryTypes[m_memoryType]);
}

AMFEncoder::VCE_Memory_Type AMFEncoder::VCE::GetMemoryType() {
	return m_memoryType;
}

void AMFEncoder::VCE::SetSurfaceFormat(VCE_Surface_Format surfaceFormat) {
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

	m_surfaceFormat = surfaceFormat;
	AMF_LOG_INFO("<AMFEncoder::VCE::SetSurfaceFormat> Set to %s.", surfaceFormats[m_surfaceFormat]);
}

AMFEncoder::VCE_Surface_Format AMFEncoder::VCE::GetSurfaceFormat() {
	return m_surfaceFormat;
}

void AMFEncoder::VCE::SetUsage(VCE_Usage usage) {
	AMF_RESULT res = AMF_UNEXPECTED;
	char* usages[] = {
		"Transcoding",
		"Ultra Low Latency",
		"Low Latency",
		"Webcam"
	};

	// Early-Exception if encoding.
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetUsage> Attempted to change while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Set usage
	switch (usage) {
		case VCE_USAGE_WEBCAM:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_WEBCAM);
			break;
		case VCE_USAGE_ULTRA_LOW_LATENCY:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
			break;
		case VCE_USAGE_LOW_LATENCY:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY);
			break;
		case VCE_USAGE_TRANSCODING:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
			break;
	}
	if (res == AMF_OK) {
		m_usage = usage;
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

void AMFEncoder::VCE::SetQualityPreset(VCE_Quality_Preset qualityPreset) {
	AMF_RESULT res = AMF_UNEXPECTED;
	char* qualities[] = {
		"Balanced",
		"Speed",
		"Quality"
	};

	// Early-Exception if encoding.
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetQualityPreset> Attempted to change while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Set usage
	switch (qualityPreset) {
		case VCE_QUALITY_PRESET_BALANCED:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED);
			break;
		case VCE_QUALITY_PRESET_SPEED:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED);
			break;
		case VCE_QUALITY_PRESET_QUALITY:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY);
			break;
	}
	if (res == AMF_OK) {
		m_qualityPreset = qualityPreset;
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

void AMFEncoder::VCE::SetProfile(VCE_Profile profile) {
	AMF_RESULT res = AMF_UNEXPECTED;
	char* profiles[] = {
		"Baseline",
		"Main",
		"High"
	};

	// Early-Exception if encoding.
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetProfile> Attempted to change while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Set profile
	switch (profile) {
		case VCE_PROFILE_BASELINE:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_BASELINE);
			break;
		case VCE_PROFILE_MAIN:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_MAIN);
			break;
		case VCE_PROFILE_HIGH:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_HIGH);
			break;
	}
	if (res == AMF_OK) {
		m_profile = profile;
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

void AMFEncoder::VCE::SetProfileLevel(VCE_Profile_Level profileLevel) {
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

	// Early-Exception if encoding.
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetProfileLevel> Attempted to change while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Set profile level
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, profileToAMF[profileLevel]);
	if (res == AMF_OK) {
		m_profileLevel = profileLevel;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetProfile> Set to %s.", profiles[m_profileLevel]);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetProfile> Failed to set to %s, error %s (code %d).", profiles[m_profileLevel], res);
	}
}

AMFEncoder::VCE_Profile_Level AMFEncoder::VCE::GetProfileLevel() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PROFILE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		switch (variant.ToInt64()) {
			case 10:case 11:case 12:case 13:
				m_profileLevel = (VCE_Profile_Level)(variant.ToInt64() - 10);
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
		}
	} else {
		throwAMFError("<AMFEncoder::VCE::GetProfileLevel> Failed to retrieve, error %s (code %d).", res);
	}

	return m_profileLevel;
}

void AMFEncoder::VCE::SetMaxLTRFrames(uint32_t maxLTRFrames) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Early-Exception if encoding.
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetMaxOfLTRFrames> Attempted to change while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Set usage
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, maxLTRFrames);
	if (res == AMF_OK) {
		m_maxLTRFrames = maxLTRFrames;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetMaxOfLTRFrames> Set to %d.", maxLTRFrames);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetMaxOfLTRFrames> Failed to set to %d, error %s (code %d).", maxLTRFrames, res);
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

void AMFEncoder::VCE::SetScanType(VCE_ScanType scanType) {
	AMF_RESULT res = AMF_UNEXPECTED;
	char* scanTypes[] = {
		"Progressive",
		"Interlaced"
	};

	// Early-Exception if encoding.
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetScanType> Attempted to change while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Set usage
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_SCANTYPE, scanType);
	if (res == AMF_OK) {
		m_scanType = scanType;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetScanType> Set to %s.", scanTypes[scanType]);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetScanType> Failed to set to %s, error %s (code %d).", scanTypes[scanType], res);
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

void AMFEncoder::VCE::SetFrameSize(std::pair<uint32_t, uint32_t>& framesize) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Early-Exception if encoding.
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetFrameSize> Attempted to change while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// ToDo: Verify with Capabilities.

	// Set frame size
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(framesize.first, framesize.second));
	if (res == AMF_OK) {
		m_frameSize.first = framesize.first;
		m_frameSize.second = framesize.second;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetFrameSize> Set to %dx%d.", framesize.first, framesize.second);
	} else { // Not OK? Then throw an error instead.
		std::vector<char> sizebuf(256);
		sprintf(sizebuf.data(), "%dx%d", framesize.first, framesize.second);
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

void AMFEncoder::VCE::SetFrameRate(std::pair<uint32_t, uint32_t>& framerate) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Early-Exception if encoding.
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetFrameRate> Attempted to change while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Set frame size
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(framerate.first, framerate.second));
	if (res == AMF_OK) {
		m_frameRate.first = framerate.first;
		m_frameRate.second = framerate.second;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetFrameRate> Set to %d/%d.", m_frameRate.first, m_frameRate.second);
	} else { // Not OK? Then throw an error instead.
		std::vector<char> sizebuf(96);
		sprintf(sizebuf.data(), "%d/%d", framerate.first, framerate.second);
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

void AMFEncoder::VCE::SetRateControlMethod(VCE_Rate_Control_Method method) {
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

	// Set frame size
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, methodToAMF[method]);
	if (res == AMF_OK) {
		m_rateControlMethod = method;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetRateControlMethod> Set to %s.", methodToName[method]);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetRateControlMethod> Failed to set to %s, error %s (code %d).", methodToName[method], res);
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

void AMFEncoder::VCE::SetFrameSkippingEnabled(bool enable) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, enable);
	if (res == AMF_OK) {
		m_skipFrameEnabled = enable;
		AMF_LOG_INFO("<AMFEncoder::VCE::EnableFrameSkipping> Set to %s.", enable ? "Enabled" : "Disabled");
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::EnableFrameSkipping> Failed to set to %s, error %s (code %d).", enable ? "Enabled" : "Disabled", res);
	}
}

bool AMFEncoder::VCE::IsFrameSkippingEnabled() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_BOOL) {
		m_skipFrameEnabled = variant.ToBool();
	} else {
		throwAMFError("<AMFEncoder::VCE::IsFrameSkippingEnabled> Failed to retrieve, error %s (code %d).", res);
	}
	return m_skipFrameEnabled;
}

void AMFEncoder::VCE::SetFillerDataEnabled(bool enable) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, enable);
	if (res == AMF_OK) {
		m_fillerDataEnabled = enable;
		AMF_LOG_INFO("<AMFEncoder::VCE::EnableFillerData> Set to %s.", enable ? "Enabled" : "Disabled");
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::EnableFillerData> Failed to set to %s, error %s (code %d).", enable ? "Enabled" : "Disabled", res);
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

void AMFEncoder::VCE::SetEnforceHRDEnabled(bool force) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_ENFORCE_HRD, force);
	if (res == AMF_OK) {
		m_enforceHRDEnabled = force;
		AMF_LOG_INFO("<AMFEncoder::VCE::EnableEnforceHRD> Set to %s.", force ? "Enabled" : "Disabled");
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::EnableEnforceHRD> Failed to set to %s, error %s (code %d).", force ? "Enabled" : "Disabled", res);
	}
}

bool AMFEncoder::VCE::IsEnforceHRDEnabled() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_ENFORCE_HRD, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_BOOL) {
		m_enforceHRDEnabled = variant.ToBool();
	} else {
		throwAMFError("<AMFEncoder::VCE::IsEnforceHRDEnabled> Failed to retrieve, error %s (code %d).", res);
	}
	return m_enforceHRDEnabled;
}

void AMFEncoder::VCE::SetGOPSize(uint32_t GOPSize) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_GOP_SIZE, GOPSize);
	if (res == AMF_OK) {
		m_GOPSize = GOPSize;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetGOPSize> Set to %d.", GOPSize);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetGOPSize> Failed to set to %d, error %s (code %d).", GOPSize, res);
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

void AMFEncoder::VCE::SetVBVBufferSize(uint32_t VBVBufferSize) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, VBVBufferSize);
	if (res == AMF_OK) {
		m_VBVBufferSize = VBVBufferSize;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetVBVBufferSize> Set to %d.", VBVBufferSize);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetVBVBufferSize> Failed to set to %d, error %s (code %d).", VBVBufferSize, res);
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

void AMFEncoder::VCE::SetInitialVBVBufferFullness(double_t initialVBVBufferFullness) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS, (int32_t)(initialVBVBufferFullness * 64));
	if (res == AMF_OK) {
		m_initalVBVBufferFullness = initialVBVBufferFullness;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetInitialVBVBufferFullness> Set to %f.", initialVBVBufferFullness);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetInitialVBVBufferFullness> Failed to set to %f, error %s (code %d).", initialVBVBufferFullness, res);
	}
}

double_t AMFEncoder::VCE::GetInitialVBVBufferFullness() {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_initalVBVBufferFullness = (variant.ToInt64() / 1.0);
	} else {
		throwAMFError("<AMFEncoder::VCE::GetInitialVBVBufferFullness> Failed to retrieve, error %s (code %d).", res);
	}
	return m_initalVBVBufferFullness;
}

void AMFEncoder::VCE::SetMaximumAccessUnitSize(uint32_t maximumAccessUnitSize) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_AU_SIZE, maximumAccessUnitSize);
	if (res == AMF_OK) {
		m_maximumAccessUnitSize = maximumAccessUnitSize;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetMaximumAccessUnitSize> Set to %d.", maximumAccessUnitSize);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetMaximumAccessUnitSize> Failed to set to %d, error %s (code %d).", maximumAccessUnitSize, res);
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

void AMFEncoder::VCE::SetBPictureDeltaQP(uint8_t BPictureDeltaQP) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, BPictureDeltaQP);
	if (res == AMF_OK) {
		m_BPictureDeltaQP = BPictureDeltaQP;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetBPictureDeltaQP> Set to %d.", BPictureDeltaQP);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetBPictureDeltaQP> Failed to set to %d, error %s (code %d).", BPictureDeltaQP, res);
	}
}

uint8_t AMFEncoder::VCE::GetBPictureDeltaQP() {
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

void AMFEncoder::VCE::SetReferenceBPictureDeltaQP(uint8_t refBPictureDeltaQP) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, refBPictureDeltaQP);
	if (res == AMF_OK) {
		m_refBPictureDeltaQP = refBPictureDeltaQP;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetReferenceBPictureDeltaQP> Set to %d.", refBPictureDeltaQP);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetReferenceBPictureDeltaQP> Failed to set to %d, error %s (code %d).", refBPictureDeltaQP, res);
	}
}

uint8_t AMFEncoder::VCE::GetReferenceBPictureDeltaQP() {
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

void AMFEncoder::VCE::SetMinimumQP(uint8_t qp) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	if (qp > 51) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetMaximumQP> QP value %d is out of range, limiting to 0 - 51...", qp);
		qp = 51;
	}

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MIN_QP, qp);
	if (res == AMF_OK) {
		m_minimumQP = qp;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetMinimumQP> Set to %d.", qp);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetMinimumQP> Failed to set to %d, error %s (code %d).", qp, res);
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

void AMFEncoder::VCE::SetMaximumQP(uint8_t qp) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	if (qp > 51) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetMaximumQP> QP value %d is out of range, limiting to 0 - 51...", qp);
		qp = 51;
	}

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_QP, qp);
	if (res == AMF_OK) {
		m_maximumQP = qp;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetMaximumQP> Set to %d.", qp);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetMaximumQP> Failed to set to %d, error %s (code %d).", qp, res);
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

void AMFEncoder::VCE::SetIFrameQP(uint8_t qp) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	if (qp > 51) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetIFrameQP> QP value %d is out of range, limiting to 0 - 51...", qp);
		qp = 51;
	}

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_I, qp);
	if (res == AMF_OK) {
		m_IFrameQP = qp;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetIFrameQP> Set to %d.", qp);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetIFrameQP> Failed to set to %d, error %s (code %d).", qp, res);
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

void AMFEncoder::VCE::SetPFrameQP(uint8_t qp) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	if (qp > 51) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetPFrameQP> QP value %d is out of range, limiting to 0 - 51...", qp);
		qp = 51;
	}

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_P, qp);
	if (res == AMF_OK) {
		m_PFrameQP = qp;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetPFrameQP> Set to %d.", qp);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetPFrameQP> Failed to set to %d, error %s (code %d).", qp, res);
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

void AMFEncoder::VCE::SetBFrameQP(uint8_t qp) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	if (qp > 51) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetBFrameQP> QP value %d is out of range, limiting to 0 - 51...", qp);
		qp = 51;
	}

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_B, qp);
	if (res == AMF_OK) {
		m_BFrameQP = qp;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetBFrameQP> Set to %d.", qp);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetBFrameQP> Failed to set to %d, error %s (code %d).", qp, res);
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

void AMFEncoder::VCE::SetTargetBitrate(uint32_t bitrate) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	uint32_t maxBitrate = VCE_Capabilities::getInstance()->getEncoderCaps(m_encoderType)->maxBitrate;
	if (bitrate > maxBitrate) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetTargetBitrate> Bitrate value %d is out of range, limiting to 0 - %d...", maxBitrate);
		bitrate = maxBitrate;
	}

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, bitrate);
	if (res == AMF_OK) {
		m_targetBitrate = bitrate;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetTargetBitrate> Set to %d.", bitrate);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetTargetBitrate> Failed to set to %d, error %s (code %d).", bitrate, res);
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

void AMFEncoder::VCE::SetPeakBitrate(uint32_t bitrate) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Validate Input
	uint32_t maxBitrate = VCE_Capabilities::getInstance()->getEncoderCaps(m_encoderType)->maxBitrate;
	if (bitrate > maxBitrate) { // Warn and limit
		AMF_LOG_WARNING("<AMFEncoder::VCE::SetPeakBitrate> Bitrate value %d is out of range, limiting to 0 - %d...", maxBitrate);
		bitrate = maxBitrate;
	}

	// Set Frame Skipping
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, bitrate);
	if (res == AMF_OK) {
		m_peakBitrate = bitrate;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetTargetBitrate> Set to %d.", bitrate);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetPeakBitrate> Failed to set to %d, error %s (code %d).", bitrate, res);
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

	//ToDo: Support for ReInit at different Framesize? How does that even work?
	res = m_AMFEncoder->Init(surfaceFormatToAMF[m_surfaceFormat], m_frameSize.first, m_frameSize.second);
	if (res != AMF_OK) {
		throwAMFError("<AMFEncoder::VCE::Start> Unable to start, error %s (code %d).", res);
	} else {
		m_isStarted = true;
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

	//ToDo: Support for ReInit at different Framesize? How does that even work?
	res = m_AMFEncoder->Terminate();
	if (res != AMF_OK) {
		throwAMFError("<AMFEncoder::VCE::Stop> Unable to stop, error %s (code %d).", res);
	} else {
		m_isStarted = false;
	}
}

bool AMFEncoder::VCE::SendInput(struct encoder_frame*& frame) {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFSurfacePtr pSurface;
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

	// Early-Exception if encoding.
	if (!m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SendInput> Attempted to send input while not running.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Memory Type: Host
	#ifdef USE_CreateSurfaceFromHostNative
	m_FrameDataBuffer.resize((frame->linesize[0] * m_frameSize.second) << 1); // Fits all supported formats, I believe.
	#endif
	if (m_memoryType == VCE_MEMORY_TYPE_HOST) {
		#ifndef USE_CreateSurfaceFromHostNative
		res = m_AMFContext->AllocSurface(
			memoryTypeToAMF[m_memoryType], surfaceFormatToAMF[m_surfaceFormat],
			m_frameSize.first, m_frameSize.second,
			&pSurface);
		#endif

		switch (m_surfaceFormat) {
			case VCE_SURFACE_FORMAT_NV12:
			{ // NV12, Y:U+V, Two Plane
				#ifndef USE_CreateSurfaceFromHostNative
				size_t iMax = pSurface->GetPlanesCount();
				#pragma loop(hint_parallel(2))
				for (uint8_t i = 0; i < iMax; i++) {
					amf::AMFPlane* plane = pSurface->GetPlaneAt(i);
					void* plane_nat = plane->GetNative();
					int32_t height = plane->GetHeight();
					size_t hpitch = plane->GetHPitch();

					#pragma loop(hint_parallel(4))
					for (int32_t py = 0; py < height; py++) {
						size_t plane_off = py * hpitch;
						size_t frame_off = py * frame->linesize[i];
						std::memcpy(static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off), static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
					}
				}
				#else
				std::memcpy(m_FrameDataBuffer.data(), frame->data[0], frame->linesize[0] * m_cfgHeight);
				std::memcpy(m_FrameDataBuffer.data() + (frame->linesize[0] * m_cfgHeight), frame->data[1], frame->linesize[0] * (m_cfgHeight >> 1));
				#endif
				break;
			}
			case VCE_SURFACE_FORMAT_I420:
			{	// YUV 4:2:0, Y, subsampled U, subsampled V
				#ifndef USE_CreateSurfaceFromHostNative
				size_t iMax = pSurface->GetPlanesCount();
				#pragma loop(hint_parallel(3))
				for (uint8_t i = 0; i < iMax; i++) {
					amf::AMFPlane* plane = pSurface->GetPlaneAt(i);
					void* plane_nat = plane->GetNative();
					int32_t height = plane->GetHeight();
					size_t hpitch = plane->GetHPitch();

					#pragma loop(hint_parallel(8))
					for (int32_t py = 0; py < height; py++) {
						size_t plane_off = py * hpitch;
						size_t frame_off = py * frame->linesize[i];
						std::memcpy(static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off), static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
					}
				}
				#else
				size_t halfHeight = m_cfgHeight >> 1;
				size_t fullFrame = (frame->linesize[0] * m_cfgHeight);
				size_t halfFrame = frame->linesize[1] * halfHeight;

				std::memcpy(m_FrameDataBuffer.data(), frame->data[0], frame->linesize[0] * m_cfgHeight);
				std::memcpy(m_FrameDataBuffer.data() + fullFrame, frame->data[1], frame->linesize[1] * halfHeight);
				std::memcpy(m_FrameDataBuffer.data() + (fullFrame + halfFrame), frame->data[2], frame->linesize[2] * halfHeight);
				#endif
				break;
			}
			case VCE_SURFACE_FORMAT_I444:
			{
				break;
			}
			case VCE_SURFACE_FORMAT_RGB:
			{ // RGBA, Single Plane
				#ifndef USE_CreateSurfaceFromHostNative
				size_t iMax = pSurface->GetPlanesCount();
				for (uint8_t i = 0; i < iMax; i++) {
					amf::AMFPlane* plane = pSurface->GetPlaneAt(i);
					void* plane_nat = plane->GetNative();
					int32_t height = plane->GetHeight();
					size_t hpitch = plane->GetHPitch();

					#pragma loop(hint_parallel(8))
					for (int32_t py = 0; py < height; py++) {
						size_t plane_off = py * hpitch;
						size_t frame_off = py * frame->linesize[i];
						std::memcpy(static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off), static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
					}
				}
				#else
				std::memcpy(m_FrameDataBuffer.data(), frame->data[0], frame->linesize[0] * m_cfgHeight);
				#endif
				break;
			}

		}
		#ifdef USE_CreateSurfaceFromHostNative
		res = m_AMFContext->CreateSurfaceFromHostNative(m_AMFSurfaceFormat, m_cfgWidth, m_cfgHeight, m_cfgWidth, m_cfgHeight, myFrame->surfaceBuffer.data(), &surfaceIn, NULL);
		#endif
	}
	if (res != AMF_OK) // Unable to create Surface
		throwAMFError("<AMFEncoder::VCE::SendInput> Unable to create AMFSurface, error %s (code %d).", res);

	// ToDo: This needs to be in converted time (frame index to amf_pts) - conversion varies.
	//  Otherwise Low Latency, Ultra Low Latency and Latency Constrained VBR are completely broken
	//  and will not output the expected bitstream.
	pSurface->SetPts(frame->pts * OBS_PTS_TO_AMF_PTS);
	pSurface->SetProperty(AMFVCE_PROPERTY_FRAME, frame->pts);
	res = m_AMFEncoder->SubmitInput(pSurface);
	if (res != AMF_OK) {// Unable to submit Surface
		std::vector<char> msgBuf(1024);
		formatAMFError(&msgBuf, "<AMFEncoder::VCE::SendInput> Unable to submit input, error %s (code %d).", res);
		AMF_LOG_ERROR("%s", msgBuf.data());
		return false;
		//throwAMFError("<AMFEncoder::VCE::SendInput> Unable to submit AMFSurface, error %s (code %d).", res);
	}

	return true;
}

void AMFEncoder::VCE::GetOutput(struct encoder_packet*& packet, bool*& received_packet) {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFDataPtr pData;

	res = m_AMFEncoder->QueryOutput(&pData);
	if (res != AMF_OK) {
		std::vector<char> msgBuf(1024);
		formatAMFError(&msgBuf, "<AMFEncoder::VCE::GetOutput> Unable to query output, error %s (code %d).", res);
		AMF_LOG_ERROR("%s", msgBuf.data());
	} else {
		amf::AMFBufferPtr pBuffer(pData);

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

		packet->type = OBS_ENCODER_VIDEO;
		packet->size = bufferSize;
		packet->data = m_PacketDataBuffer.data();
		if (pBuffer->GetProperty(AMFVCE_PROPERTY_FRAME, &packet->pts) == AMF_OK) {
			packet->dts = packet->pts;
		}
		{ // If it is a Keyframe or not, the light will tell you... the light being this integer here.
			int t_frameDataType = -1;
			pBuffer->GetProperty(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, &t_frameDataType);

			switch (t_frameDataType) {
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
					packet->drop_priority = 1;				// So require a P-Frame or better to continue streaming.
					break;
			}
		}

		*received_packet = true;
	}
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
}

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

