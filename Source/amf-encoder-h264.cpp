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

#include "amf-encoder-h264.h"
#include "misc-util.cpp"

#undef QUICK_FORMAT_MESSAGE
#define QUICK_FORMAT_MESSAGE(var, ...) std::vector<char> var(1024); \
	sprintf_s(var.data(), var.size(), "[H264/AVC] " __VA_ARGS__);
#define QUICK_THROW_ERROR(format, ...) {\
	QUICK_FORMAT_MESSAGE(errMsg, __FUNCTION_NAME__ format, \
		m_UniqueId,  ##__VA_ARGS__, \
		m_AMF->GetTrace()->GetResultText(res), res); \
	throw std::exception(errMsg.data()); \
}
//QUICK_THROW_ERROR(" <%s: '%s'> Failed to set to %s, error %ls (code %d)",
//	Utility::UsageToString(v));

using namespace Plugin;
using namespace Plugin::AMD;

Plugin::AMD::EncoderH264::EncoderH264(std::string videoAPI, uint64_t videoAdapterId, bool useOpenCL,
	ColorFormat colorFormat, ColorSpace colorSpace, bool fullRangeColor)
	: Encoder(Codec::H264AVC, videoAPI, videoAdapterId, useOpenCL, colorFormat, colorSpace, fullRangeColor) {}

Plugin::AMD::EncoderH264::~EncoderH264() {}

// Properties - Initialization
std::vector<Usage> Plugin::AMD::EncoderH264::CapsUsage() {
	const amf::AMFPropertyInfo* var;
	AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(AMF_VIDEO_ENCODER_USAGE, &var);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Querying capabilities failed, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	std::vector<Usage> ret;
	for (const amf::AMFEnumDescriptionEntry* enm = var->pEnumDescription; enm != nullptr; enm++) {
		ret.push_back(Utility::UsageFromAMFH264((AMF_VIDEO_ENCODER_USAGE_ENUM)enm->value));
	}
	return ret;
}

void Plugin::AMD::EncoderH264::SetUsage(Usage v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, Utility::UsageToAMFH264(v));
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %s, error %ls (code %d)",
			m_UniqueId, Utility::UsageToString(v), m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

Plugin::AMD::Usage Plugin::AMD::EncoderH264::GetUsage() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_USAGE, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return Utility::UsageFromAMFH264((AMF_VIDEO_ENCODER_USAGE_ENUM)e);
}

// Properties - Static
std::vector<QualityPreset> Plugin::AMD::EncoderH264::CapsQualityPreset() {
	const amf::AMFPropertyInfo* var;
	AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(AMF_VIDEO_ENCODER_USAGE, &var);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Querying capabilities failed, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	std::vector<QualityPreset> ret;
	for (const amf::AMFEnumDescriptionEntry* enm = var->pEnumDescription; enm != nullptr; enm++) {
		ret.push_back(Utility::QualityPresetFromAMFH264((AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM)enm->value));
	}
	return ret;
}

void Plugin::AMD::EncoderH264::SetQualityPreset(QualityPreset v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET,
		Utility::QualityPresetToAMFH264(v));
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %s, error %ls (code %d)",
			m_UniqueId, Utility::QualityPresetToString(v), m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

Plugin::AMD::QualityPreset Plugin::AMD::EncoderH264::GetQualityPreset() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return Utility::QualityPresetFromAMFH264((AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM)e);
}

std::vector<Profile> Plugin::AMD::EncoderH264::CapsProfile() {
	const amf::AMFPropertyInfo* var;
	AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(AMF_VIDEO_ENCODER_PROFILE, &var);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Querying capabilities failed, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	std::vector<Profile> ret;
	for (const amf::AMFEnumDescriptionEntry* enm = var->pEnumDescription; enm != nullptr; enm++) {
		ret.push_back(Utility::ProfileFromAMFH264((AMF_VIDEO_ENCODER_PROFILE_ENUM)enm->value));
	}
	return ret;
}

void Plugin::AMD::EncoderH264::SetProfile(Profile v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE,
		Utility::ProfileToAMFH264(v));
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %s, error %ls (code %d)",
			m_UniqueId, Utility::ProfileToString(v), m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

Plugin::AMD::Profile Plugin::AMD::EncoderH264::GetProfile() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PROFILE, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return Utility::ProfileFromAMFH264((AMF_VIDEO_ENCODER_PROFILE_ENUM)e);
}

std::vector<ProfileLevel> Plugin::AMD::EncoderH264::CapsProfileLevel() {
	const amf::AMFPropertyInfo* var;
	AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(AMF_VIDEO_ENCODER_PROFILE_LEVEL, &var);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Querying capabilities failed, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	std::vector<ProfileLevel> ret;
	for (const amf::AMFEnumDescriptionEntry* enm = var->pEnumDescription; enm != nullptr; enm++) {
		ret.push_back((ProfileLevel)enm->value);
	}
	return ret;
}

void Plugin::AMD::EncoderH264::SetProfileLevel(ProfileLevel v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL,
		(int64_t)v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %lld, error %ls (code %d)",
			m_UniqueId, (int64_t)v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

Plugin::AMD::ProfileLevel Plugin::AMD::EncoderH264::GetProfileLevel() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return (ProfileLevel)e;
}

std::pair<uint64_t, uint64_t> Plugin::AMD::EncoderH264::CapsMaximumReferenceFrames() {
	const amf::AMFPropertyInfo* var;
	AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(AMF_VIDEO_ENCODER_MAX_NUM_REFRAMES, &var);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Querying capabilities failed, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	return std::make_pair(var->minValue.int64Value, var->maxValue.int64Value);
}

void Plugin::AMD::EncoderH264::SetMaximumReferenceFrames(uint64_t v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_NUM_REFRAMES, v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %lld, error %ls (code %d)",
			m_UniqueId, v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

uint64_t Plugin::AMD::EncoderH264::GetMaximumReferenceFrames() {
	uint64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_NUM_REFRAMES, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, __FUNCTION_NAME__ "<%s: '%s'> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return e;
}

std::pair<std::pair<uint32_t, uint32_t>, std::pair<uint32_t, uint32_t>> Plugin::AMD::EncoderH264::CapsResolution() {
	const amf::AMFPropertyInfo* var;
	AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(AMF_VIDEO_ENCODER_FRAMESIZE, &var);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Querying capabilities failed, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	return std::make_pair(
		std::make_pair(var->minValue.sizeValue.width, var->maxValue.sizeValue.width),
		std::make_pair(var->minValue.sizeValue.height, var->maxValue.sizeValue.height)
	);
}

void Plugin::AMD::EncoderH264::SetResolution(std::pair<uint32_t, uint32_t> v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(v.first, v.second));
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %ldx%ld, error %ls (code %d)",
			m_UniqueId, v.first, v.second, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	m_Resolution.first = v.first;
	m_Resolution.second = v.second;
}

std::pair<uint32_t, uint32_t> Plugin::AMD::EncoderH264::GetResolution() {
	AMFSize e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	m_Resolution.first = e.width;
	m_Resolution.second = e.height;
	return std::make_pair(e.width, e.height);
}

void Plugin::AMD::EncoderH264::SetAspectRatio(std::pair<uint32_t, uint32_t> v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_ASPECT_RATIO, ::AMFConstructRatio(v.first, v.second));
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %ld:%ld, error %ls (code %d)",
			m_UniqueId, v.first, v.second, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

std::pair<uint32_t, uint32_t> Plugin::AMD::EncoderH264::GetAspectRatio() {
	AMFRatio e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_ASPECT_RATIO, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return std::make_pair(e.num, e.den);
}

void Plugin::AMD::EncoderH264::SetFrameRate(std::pair<uint32_t, uint32_t> v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(v.first, v.second));
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %ld/%ld, error %ls (code %d)",
			m_UniqueId, v.first, v.second, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	m_FrameRate = std::make_pair(v.first, v.second);
	UpdateFrameRateValues();
}

std::pair<uint32_t, uint32_t> Plugin::AMD::EncoderH264::GetFrameRate() {
	AMFRate e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FRAMERATE, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Unable to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	m_FrameRate = std::make_pair(e.num, e.den);
	UpdateFrameRateValues();
	return m_FrameRate;
}

std::vector<CodingType> Plugin::AMD::EncoderH264::CapsCodingType() {
	const amf::AMFPropertyInfo* var;
	AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(AMF_VIDEO_ENCODER_CABAC_ENABLE, &var);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Querying capabilities failed, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	std::vector<CodingType> ret;
	for (const amf::AMFEnumDescriptionEntry* enm = var->pEnumDescription; enm != nullptr; enm++) {
		ret.push_back(Utility::CodingTypeFromAMFH264((AMF_VIDEO_ENCODER_CODING_ENUM)enm->value));
	}
	return ret;
}

void Plugin::AMD::EncoderH264::SetCodingType(CodingType v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_CABAC_ENABLE, Utility::CodingTypeToAMFH264(v));
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %s, error %ls (code %d)",
			m_UniqueId, Utility::CodingTypeToString(v), m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

Plugin::AMD::CodingType Plugin::AMD::EncoderH264::GetCodingType() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_CABAC_ENABLE, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Unable to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return Utility::CodingTypeFromAMFH264((AMF_VIDEO_ENCODER_CODING_ENUM)e);
}

// Properties - Dynamic
std::vector<RateControlMethod> Plugin::AMD::EncoderH264::CapsRateControlMethod() {
	const amf::AMFPropertyInfo* var;
	AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, &var);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Querying capabilities failed, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	std::vector<RateControlMethod> ret;
	for (const amf::AMFEnumDescriptionEntry* enm = var->pEnumDescription; enm != nullptr; enm++) {
		ret.push_back(Utility::RateControlMethodFromAMFH264((AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM)enm->value));
	}
	return ret;
}

void Plugin::AMD::EncoderH264::SetRateControlMethod(RateControlMethod v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, Utility::RateControlMethodToAMFH264(v));
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %s, error %ls (code %d)",
			m_UniqueId, Utility::RateControlMethodToString(v), m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

Plugin::AMD::RateControlMethod Plugin::AMD::EncoderH264::GetRateControlMethod() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return Utility::RateControlMethodFromAMFH264((AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM)e);
}

std::vector<PrePassMode> Plugin::AMD::EncoderH264::CapsPrePassMode() {
	const amf::AMFPropertyInfo* var;
	AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(AMF_VIDEO_ENCODER_RATE_CONTROL_PREANALYSIS_ENABLE, &var);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Querying capabilities failed, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	std::vector<PrePassMode> ret;
	for (const amf::AMFEnumDescriptionEntry* enm = var->pEnumDescription; enm != nullptr; enm++) {
		ret.push_back(Utility::PrePassModeFromAMFH264((AMF_VIDEO_ENCODER_PREENCODE_MODE_ENUM)enm->value));
	}
	return ret;
}

void Plugin::AMD::EncoderH264::SetPrePassMode(PrePassMode v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_PREANALYSIS_ENABLE, Utility::PrePassModeToAMFH264(v));
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %s, error %ls (code %d)",
			m_UniqueId, Utility::PrePassModeToString(v), m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

Plugin::AMD::PrePassMode Plugin::AMD::EncoderH264::GetPrePassMode() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_PREANALYSIS_ENABLE, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Unable to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return Utility::PrePassModeFromAMFH264((AMF_VIDEO_ENCODER_PREENCODE_MODE_ENUM)e);
}

void Plugin::AMD::EncoderH264::SetVariableBitrateAverageQualityEnabled(bool v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_ENABLE_VBAQ, v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %s, error %ls (code %d)",
			m_UniqueId, v ? "Enabled" : "Disabled", m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

bool Plugin::AMD::EncoderH264::IsVariableBitrateAverageQualityEnabled() {
	bool e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_ENABLE_VBAQ, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return e;
}

void Plugin::AMD::EncoderH264::SetFrameSkippingEnabled(bool v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %s, error %ls (code %d)",
			m_UniqueId, v ? "Enabled" : "Disabled", m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

bool Plugin::AMD::EncoderH264::IsFrameSkippingEnabled() {
	bool e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return e;
}

void Plugin::AMD::EncoderH264::SetEnforceHRDEnabled(bool v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_ENFORCE_HRD, v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %s, error %ls (code %d)",
			m_UniqueId, v ? "Enabled" : "Disabled", m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

bool Plugin::AMD::EncoderH264::IsEnforceHRDEnabled() {
	bool e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_ENFORCE_HRD, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return e;
}

void Plugin::AMD::EncoderH264::SetQPMinimum(uint8_t v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MIN_QP, (int64_t)v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %d, error %ls (code %d)",
			m_UniqueId, v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

uint8_t Plugin::AMD::EncoderH264::GetQPMinimum() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MIN_QP, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return (uint8_t)e;
}

void Plugin::AMD::EncoderH264::SetQPMaximum(uint8_t v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_QP, (int64_t)v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %d, error %ls (code %d)",
			m_UniqueId, v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

uint8_t Plugin::AMD::EncoderH264::GetQPMaximum() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_QP, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return (uint8_t)e;
}

std::pair<uint64_t, uint64_t> Plugin::AMD::EncoderH264::CapsTargetBitrate() {
	const amf::AMFPropertyInfo* var;
	AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(AMF_VIDEO_ENCODER_TARGET_BITRATE, &var);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Querying capabilities failed, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	return std::make_pair(var->minValue.int64Value, var->maxValue.int64Value);
}

void Plugin::AMD::EncoderH264::SetTargetBitrate(uint64_t v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, (int64_t)v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %lld, error %ls (code %d)",
			m_UniqueId, v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

uint64_t Plugin::AMD::EncoderH264::GetTargetBitrate() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return e;
}

std::pair<uint64_t, uint64_t> Plugin::AMD::EncoderH264::CapsPeakBitrate() {
	const amf::AMFPropertyInfo* var;
	AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(AMF_VIDEO_ENCODER_PEAK_BITRATE, &var);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Querying capabilities failed, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	return std::make_pair(var->minValue.int64Value, var->maxValue.int64Value);
}

void Plugin::AMD::EncoderH264::SetPeakBitrate(uint64_t v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, (int64_t)v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %lld, error %ls (code %d)",
			m_UniqueId, v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

uint64_t Plugin::AMD::EncoderH264::GetPeakBitrate() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return e;
}

void Plugin::AMD::EncoderH264::SetIFrameQP(uint8_t v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_I, (int64_t)v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %d, error %ls (code %d)",
			m_UniqueId, v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

uint8_t Plugin::AMD::EncoderH264::GetIFrameQP() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_I, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return (uint8_t)e;
}

void Plugin::AMD::EncoderH264::SetPFrameQP(uint8_t v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_P, (int64_t)v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %d, error %ls (code %d)",
			m_UniqueId, v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

uint8_t Plugin::AMD::EncoderH264::GetPFrameQP() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_P, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return (uint8_t)e;
}

void Plugin::AMD::EncoderH264::SetBFrameQP(uint8_t v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_B, (int64_t)v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %d, error %ls (code %d)",
			m_UniqueId, v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

uint8_t Plugin::AMD::EncoderH264::GetBFrameQP() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_B, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return (uint8_t)e;
}

std::pair<uint64_t, uint64_t> Plugin::AMD::EncoderH264::CapsVBVBufferSize() {
	const amf::AMFPropertyInfo* var;
	AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, &var);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Querying capabilities failed, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	return std::make_pair(var->minValue.int64Value, var->maxValue.int64Value);
}

void Plugin::AMD::EncoderH264::SetVBVBufferSize(uint64_t v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, (int64_t)v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %lld, error %ls (code %d)",
			m_UniqueId, v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

uint64_t Plugin::AMD::EncoderH264::GetVBVBufferSize() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return e;
}

void Plugin::AMD::EncoderH264::SetVBVBufferInitialFullness(float v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, (int64_t)(v * 64));
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %lf, error %ls (code %d)",
			m_UniqueId, v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

float Plugin::AMD::EncoderH264::GetInitialVBVBufferFullness() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return (e / 64.0f);
}

// Properties - Picture Control
void Plugin::AMD::EncoderH264::SetIDRPeriod(uint64_t v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, (int64_t)v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %lld, error %ls (code %d)",
			m_UniqueId, v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

uint64_t Plugin::AMD::EncoderH264::GetIDRPeriod() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return e;
}

void Plugin::AMD::EncoderH264::SetDeblockingFilterEnabled(bool v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER, v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %s, error %ls (code %d)",
			m_UniqueId, v ? "Enabled" : "Disabled", m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

bool Plugin::AMD::EncoderH264::IsDeblockingFilterEnabled() {
	bool e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return e;
}

uint8_t Plugin::AMD::EncoderH264::CapsBFramePattern() {
	const amf::AMFPropertyInfo* var;
	AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(AMF_VIDEO_ENCODER_B_PIC_PATTERN, &var);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Querying capabilities failed, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	return (uint8_t)var->maxValue.int64Value;
}

void Plugin::AMD::EncoderH264::SetBFramePattern(uint8_t v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, (int64_t)v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %d, error %ls (code %d)",
			m_UniqueId, v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

uint8_t Plugin::AMD::EncoderH264::GetBFramePattern() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return (uint8_t)e;
}

void Plugin::AMD::EncoderH264::SetBFrameDeltaQP(int8_t v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_B, (int64_t)v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %d, error %ls (code %d)",
			m_UniqueId, v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

int8_t Plugin::AMD::EncoderH264::GetBFrameDeltaQP() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return (int8_t)e;
}

void Plugin::AMD::EncoderH264::SetBFrameReference(bool v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %s, error %ls (code %d)",
			m_UniqueId, v ? "Enabled" : "Disabled", m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

bool Plugin::AMD::EncoderH264::GetBFrameReference() {
	bool e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return e;
}

void Plugin::AMD::EncoderH264::SetBFrameReferenceDeltaQP(int8_t v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, (int64_t)v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set to %d, error %ls (code %d)",
			m_UniqueId, v, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

int8_t Plugin::AMD::EncoderH264::GetBFrameReferenceDeltaQP() {
	int64_t e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return (int8_t)e;
}

// Properties - Motion Estimation
void Plugin::AMD::EncoderH264::SetMotionEstimationQuarterPixelEnabled(bool v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL, v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set mode to %s, error %ls (code %d)",
			m_UniqueId, v ? "Enabled" : "Disabled", m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

bool Plugin::AMD::EncoderH264::GetMotionEstimationQuarterPixelEnabled() {
	bool e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return e;
}

void Plugin::AMD::EncoderH264::SetMotionEstimationHalfPixelEnabled(bool v) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL, v);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to set mode to %s, error %ls (code %d)",
			m_UniqueId, v ? "Enabled" : "Disabled", m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

bool Plugin::AMD::EncoderH264::GetMotionEstimationHalfPixelEnabled() {
	bool e;

	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL, &e);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg, "<Id: %lld> Failed to retrieve value, error %ls (code %d)",
			m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	return e;
}
