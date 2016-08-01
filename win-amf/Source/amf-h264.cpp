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
		throwAMFError("<AMFEncoder::VCE::SetUsage> Failed to set to %s, error %s (code %d).", res);
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
		throwAMFError("<AMFEncoder::VCE::SetQualityPreset> Failed to set to %s, error %s (code %d).", res);
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
			m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_BASELINE);
			break;
		case H264_PROFILE_MAIN:
			m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_MAIN);
			break;
		case H264_PROFILE_HIGH:
			m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, AMF_VIDEO_ENCODER_PROFILE_HIGH);
			break;
	}
	if (res == AMF_OK) {
		m_profile = profile;
		AMF_LOG_INFO("<AMFEncoder::VCE::SetProfile> Set to %s.", profiles[m_profile]);
	} else { // Not OK? Then throw an error instead.
		throwAMFError("<AMFEncoder::VCE::SetProfile> Failed to set to %s, error %s (code %d).", res);
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

}

AMFEncoder::H264_Profile_Level AMFEncoder::VCE::GetProfileLevel() {

}

void AMFEncoder::VCE::SetMaxOfLTRFrames(uint32_t maxOfLTRFrames) {

}

uint32_t AMFEncoder::VCE::GetMaxOfLTRFrames() {

}

void AMFEncoder::VCE::SetScanType(H264_ScanType scanType) {

}

AMFEncoder::H264_ScanType AMFEncoder::VCE::GetScanType() {

}

void AMFEncoder::VCE::SetFrameSize(std::pair<uint32_t, uint32_t>& framesize) {

}

std::pair<uint32_t, uint32_t> AMFEncoder::VCE::GetFrameSize() {

}

void AMFEncoder::VCE::SetFrameRate(std::pair<uint32_t, uint32_t>& framerate) {

}

std::pair<uint32_t, uint32_t> AMFEncoder::VCE::GetFrameRate() {

}

void AMFEncoder::VCE::throwAMFError(const char* errorMsg, AMF_RESULT res) {
	std::vector<char> msgBuf(1024);
	tempFormatAMFError(&msgBuf, errorMsg, res);
	AMF_LOG_ERROR("%s", msgBuf.data());
	throw std::exception(msgBuf.data());
}

void AMFEncoder::VCE::tempFormatAMFError(std::vector<char>* buffer, const char* format, AMF_RESULT res) {
	std::vector<char> errBuf(1024);
	wcstombs(errBuf.data(), amf::AMFGetResultText(res), errBuf.size());
	sprintf(buffer->data(), format, errBuf, res);
}
