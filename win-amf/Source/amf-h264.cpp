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

AMFEncoder::H264::H264(H264_Usage Usage, H264_Quality_Preset QualityPreset,
	std::pair<uint32_t, uint32_t>& framesize, std::pair<int, int>& framerate,
	H264_Profile profile, H264_Profile_Level profileLevel,
	int maxOfLTRFrames, H264_ScanType scanType) {
	AMF_RESULT res;

	// Create AMF Context
	res = AMFCreateContext(&m_amfContext);
	if (res != AMF_OK) {
		std::vector<char> msgBuf(1024);
		tempFormatAMFError(&msgBuf, "<AMFEncoder::H264::H264> AMFCreateContext failed, error %s (code %d).", res);
		AMF_LOG_ERROR("%s", msgBuf.data());
		throw std::exception(msgBuf.data());
	}

	// Create AMF VCE Component depending on Profile.
	if ((profile == H264_PROFILE_BASELINE_SCALABLE)
		|| (profile == H264_PROFILE_HIGH_SCALABLE)) {
		res = AMFCreateComponent(m_amfContext, AMFVideoEncoderVCE_SVC, &m_amfVCEEncoder);
		AMF_LOG_INFO("<AMFEncoder::H264::H264> Attempting to create SVC Encoder...");
	} else {
		res = AMFCreateComponent(m_amfContext, AMFVideoEncoderVCE_AVC, &m_amfVCEEncoder);
		AMF_LOG_INFO("<AMFEncoder::H264::H264> Attempting to create AVC Encoder...");
	}
	if (res != AMF_OK) {
		m_amfContext->Terminate(); m_amfContext = nullptr;

		std::vector<char> msgBuf(1024);
		tempFormatAMFError(&msgBuf, "<AMFEncoder::H264::H264> AMFCreateComponent failed, error %s (code %d).", res);
		AMF_LOG_ERROR("%s", msgBuf.data());
		throw std::exception(msgBuf.data());
	}
}

AMFEncoder::H264::~H264() {
	if (m_amfVCEEncoder)
		m_amfVCEEncoder->Terminate();
	if (m_amfContext)
		m_amfContext->Terminate();
}

void AMFEncoder::H264::tempFormatAMFError(std::vector<char>* buffer, const char* format, AMF_RESULT res) {
	std::vector<char> errBuf(1024);
	wcstombs(errBuf.data(), amf::AMFGetResultText(res), errBuf.size());
	sprintf(buffer->data(), format, errBuf, res);
}
