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
#include "amf-h264.h"

AMFEncoder::VCE::VCE(H264_Encoder_Type encoderType) {
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
		case H264_ENCODER_TYPE_AVC:
			AMF_LOG_INFO("<AMFEncoder::VCE::H264> Attempting to create AVC Encoder...");
			encoderCaps = &(VCE_Capabilities::getInstance()->m_AVCCaps);
			if (encoderCaps->acceleration_type != amf::AMF_ACCEL_HARDWARE) {
				AMF_LOG_WARNING("<AMFEncoder::VCE::H264> AVC Encoder is not Hardware-Accelerated!");
			}

			res = AMFCreateComponent(m_AMFContext, AMFVideoEncoderVCE_AVC, &m_AMFEncoder);
			break;
		case H264_ENCODER_TYPE_SVC:
			AMF_LOG_INFO("<AMFEncoder::VCE::H264> Attempting to create SVC Encoder...");
			encoderCaps = &(VCE_Capabilities::getInstance()->m_SVCCaps);
			if (encoderCaps->acceleration_type != amf::AMF_ACCEL_HARDWARE) {
				AMF_LOG_WARNING("<AMFEncoder::VCE::H264> SVC Encoder is not Hardware-Accelerated!");
			}

			res = AMFCreateComponent(m_AMFContext, AMFVideoEncoderVCE_SVC, &m_AMFEncoder);
			break;
		case H264_ENCODER_TYPE_HEVC:
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

void AMFEncoder::VCE::SetMemoryType(H264_Memory_Type memoryType) {
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

AMFEncoder::H264_Memory_Type AMFEncoder::VCE::GetMemoryType() {
	return m_memoryType;
}

void AMFEncoder::VCE::SetSurfaceFormat(H264_Surface_Format surfaceFormat) {
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

AMFEncoder::H264_Surface_Format AMFEncoder::VCE::GetSurfaceFormat() {
	return m_surfaceFormat;
}

void AMFEncoder::VCE::SetUsage(H264_Usage usage) {
	AMF_RESULT res;
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
		case H264_USAGE_WEBCAM:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_WEBCAM);
			break;
		case H264_USAGE_ULTRA_LOW_LATENCY:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
			break;
		case H264_USAGE_LOW_LATENCY:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY);
			break;
		case H264_USAGE_TRANSCODING:
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

AMFEncoder::H264_Usage AMFEncoder::VCE::GetUsage() {
	AMF_RESULT res;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_USAGE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		switch (variant.ToInt64()) {
			case AMF_VIDEO_ENCODER_USAGE_TRANSCONDING:
				m_usage = H264_USAGE_TRANSCODING;
				break;
			case AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY:
				m_usage = H264_USAGE_ULTRA_LOW_LATENCY;
				break;
			case AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY:
				m_usage = H264_USAGE_LOW_LATENCY;
				break;
			case AMF_VIDEO_ENCODER_USAGE_WEBCAM:
				m_usage = H264_USAGE_WEBCAM;
				break;
		}
	} else {
		throwAMFError("<AMFEncoder::VCE::GetUsage> Failed to retrieve, error %s (code %d).", res);
	}

	return m_usage;
}

void AMFEncoder::VCE::SetQualityPreset(H264_Quality_Preset qualityPreset) {
	AMF_RESULT res;
	char* qualities[] = {
		"Speed",
		"Balanced",
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
		case H264_USAGE_WEBCAM:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_WEBCAM);
			break;
		case H264_USAGE_ULTRA_LOW_LATENCY:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY);
			break;
		case H264_USAGE_LOW_LATENCY:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY);
			break;
		case H264_USAGE_TRANSCODING:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
			break;
	}
	if (res == AMF_OK) {
		m_qualityPreset = qualityPreset;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetQualityPreset> Set to %s.", qualities[m_qualityPreset]);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetQualityPreset> Failed to set to %s, error %s (code %d).", qualities[m_qualityPreset], res);
	}
}

AMFEncoder::H264_Quality_Preset AMFEncoder::VCE::GetQualityPreset() {
	AMF_RESULT res;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		switch (variant.ToInt64()) {
			case AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED:
				m_qualityPreset = H264_QUALITY_PRESET_SPEED;
				break;
			case AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED:
				m_qualityPreset = H264_QUALITY_PRESET_BALANCED;
				break;
			case AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY:
				m_qualityPreset = H264_QUALITY_PRESET_QUALITY;
				break;
		}
	} else {
		throwAMFError("<AMFEncoder::VCE::GetQualityPreset> Failed to retrieve, error %s (code %d).", res);
	}

	return m_qualityPreset;
}

void AMFEncoder::VCE::SetProfile(H264_Profile profile) {
	AMF_RESULT res;
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
		case H264_PROFILE_BASELINE:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_BASELINE);
			break;
		case H264_PROFILE_MAIN:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_MAIN);
			break;
		case H264_PROFILE_HIGH:
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

AMFEncoder::H264_Profile AMFEncoder::VCE::GetProfile() {
	AMF_RESULT res;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PROFILE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		switch (variant.ToInt64()) {
			case AMF_VIDEO_ENCODER_PROFILE_BASELINE:
				m_profile = H264_PROFILE_BASELINE;
				break;
			case AMF_VIDEO_ENCODER_PROFILE_MAIN:
				m_profile = H264_PROFILE_MAIN;
				break;
			case AMF_VIDEO_ENCODER_PROFILE_HIGH:
				m_profile = H264_PROFILE_HIGH;
				break;
		}
	} else {
		throwAMFError("<AMFEncoder::VCE::GetProfile> Failed to retrieve, error %s (code %d).", res);
	}

	return m_profile;
}

void AMFEncoder::VCE::SetProfileLevel(H264_Profile_Level profileLevel) {
	AMF_RESULT res;
	char* profiles[] = {
		"1.0", "1.1", "1.2", "1.3",
		"2.0", "2.1", "2.2",
		"3.0", "3.1", "3.2",
		"4.0", "4.1", "4.2",
		"5.0", "5.1", "5.2"
	};

	// Early-Exception if encoding.
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetProfileLevel> Attempted to change while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Set profile level
	switch (profileLevel) {
		case H264_PROFILE_LEVEL_1:
		case H264_PROFILE_LEVEL_11:
		case H264_PROFILE_LEVEL_12:
		case H264_PROFILE_LEVEL_13:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, profileLevel + 10);
			break;
		case H264_PROFILE_LEVEL_2:
		case H264_PROFILE_LEVEL_21:
		case H264_PROFILE_LEVEL_22:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, profileLevel + 20);
			break;
		case H264_PROFILE_LEVEL_3:
		case H264_PROFILE_LEVEL_31:
		case H264_PROFILE_LEVEL_32:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, profileLevel + 30);
			break;
		case H264_PROFILE_LEVEL_4:
		case H264_PROFILE_LEVEL_41:
		case H264_PROFILE_LEVEL_42:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, profileLevel + 40);
			break;
		case H264_PROFILE_LEVEL_5:
		case H264_PROFILE_LEVEL_51:
		case H264_PROFILE_LEVEL_52:
			res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, profileLevel + 50);
			break;
	}
	if (res == AMF_OK) {
		m_profileLevel = profileLevel;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetProfile> Set to %s.", profiles[m_profileLevel]);
	} else { // Not OK? Then throw an error instead.
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetProfile> Failed to set to %s, error %s (code %d).", profiles[m_profileLevel], res);
	}
}

AMFEncoder::H264_Profile_Level AMFEncoder::VCE::GetProfileLevel() {
	AMF_RESULT res;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PROFILE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		switch (variant.ToInt64()) {
			case 10:case 11:case 12:case 13:
				m_profileLevel = (H264_Profile_Level)(variant.ToInt64() - 10);
				break;
			case 20:case 21:case 22:
				m_profileLevel = (H264_Profile_Level)(variant.ToInt64() - 20 + H264_PROFILE_LEVEL_2);
				break;
			case 30:case 31:case 32:
				m_profileLevel = (H264_Profile_Level)(variant.ToInt64() - 30 + H264_PROFILE_LEVEL_3);
				break;
			case 40:case 41:case 42:
				m_profileLevel = (H264_Profile_Level)(variant.ToInt64() - 40 + H264_PROFILE_LEVEL_4);
				break;
			case 50:case 51:case 52:
				m_profileLevel = (H264_Profile_Level)(variant.ToInt64() - 50 + H264_PROFILE_LEVEL_5);
				break;
		}
	} else {
		throwAMFError("<AMFEncoder::VCE::GetProfileLevel> Failed to retrieve, error %s (code %d).", res);
	}

	return m_profileLevel;
}

void AMFEncoder::VCE::SetMaxLTRFrames(uint32_t maxLTRFrames) {
	AMF_RESULT res;

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
	AMF_RESULT res;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_maxLTRFrames = variant.ToInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetMaxOfLTRFrames> Failed to retrieve, error %s (code %d).", res);
	}

	return m_maxLTRFrames;
}

void AMFEncoder::VCE::SetScanType(H264_ScanType scanType) {
	AMF_RESULT res;
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

AMFEncoder::H264_ScanType AMFEncoder::VCE::GetScanType() {
	AMF_RESULT res;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_SCANTYPE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
		m_scanType = (H264_ScanType)variant.ToInt32();
	} else {
		throwAMFError("<AMFEncoder::VCE::GetScanType> Failed to retrieve, error %s (code %d).", res);
	}

	return m_scanType;
}

void AMFEncoder::VCE::SetFrameSize(std::pair<uint32_t, uint32_t>& framesize) {
	AMF_RESULT res;

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
	AMF_RESULT res;
	amf::AMFVariant variant;

	res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, &variant);
	if (res == AMF_OK && variant.type == amf::AMF_VARIANT_SIZE) {
		AMFSize size = variant.ToSize();
		m_frameSize.first = size.width;
		m_frameSize.second = size.height;
	} else {
		throwAMFError("<AMFEncoder::VCE::GetScanType> Failed to retrieve, error %s (code %d).", res);
	}

	return std::pair<uint32_t, uint32_t>(m_frameSize);
}

void AMFEncoder::VCE::SetFrameRate(std::pair<uint32_t, uint32_t>& framerate) {
	AMF_RESULT res;

	// Early-Exception if encoding.
	if (m_isStarted) {
		const char* error = "<AMFEncoder::VCE::SetFrameRate> Attempted to change while encoding.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// ToDo: Verify with Capabilities.

	// Set frame size
	res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(framerate.first, framerate.second));
	if (res == AMF_OK) {
		m_frameRate.first = framerate.first;
		m_frameRate.second = framerate.second;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetFrameRate> Set to %d/%d.", framerate.first, framerate.second);
	} else { // Not OK? Then throw an error instead.
		std::vector<char> sizebuf(256);
		sprintf(sizebuf.data(), "%d/%d", framerate.first, framerate.second);
		throwAMFErrorAdvanced("<AMFEncoder::VCE::SetFrameRate> Failed to set to %s, error %s (code %d).", sizebuf.data(), res);
	}
}

std::pair<uint32_t, uint32_t> AMFEncoder::VCE::GetFrameRate() {

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
	sprintf(buffer->data(), format, errBuf, res);
}

template<typename _T>
void AMFEncoder::VCE::formatAMFErrorAdvanced(std::vector<char>* buffer, const char* format, _T other, AMF_RESULT res) {
	std::vector<char> errBuf(1024);
	wcstombs(errBuf.data(), amf::AMFGetResultText(res), errBuf.size());
	sprintf(buffer->data(), format, other, errBuf, res);
}

template<typename _T>
void AMFEncoder::VCE::throwAMFErrorAdvanced(const char* errorMsg, _T other, AMF_RESULT res) {
	std::vector<char> msgBuf(1024);
	formatAMFErrorAdvanced(&msgBuf, errorMsg, other, res);
	AMF_LOG_ERROR("%s", msgBuf.data());
	throw std::exception(msgBuf.data());
}

