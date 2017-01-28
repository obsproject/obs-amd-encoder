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

#include "amf-encoder.h"
#include "misc-util.cpp"

using namespace Plugin;
using namespace Plugin::AMD;

static const wchar_t* fullColorParams[] = {
	L"FullRangeColor",
	L"NominalRange",
};

Plugin::AMD::Encoder::Encoder(Codec codec,
	std::string videoAPI, uint64_t videoAdapterId, bool useOpenCL,
	ColorFormat colorFormat, ColorSpace colorSpace, bool fullRangeColor) {
	#pragma region Null Values
	m_UniqueId = Utility::GetUniqueIdentifier();
	/// AMF Internals
	m_AMF = nullptr;
	m_AMFFactory = nullptr;
	m_AMFContext = nullptr;
	m_AMFEncoder = nullptr;
	m_AMFConverter = nullptr;
	m_AMFMemoryType = amf::AMF_MEMORY_UNKNOWN;
	m_AMFSurfaceFormat = amf::AMF_SURFACE_UNKNOWN;
	/// API Related
	m_API = nullptr;
	m_APIDevice = nullptr;
	m_OpenCLSubmission = false;
	/// Properties
	m_Codec = codec;
	m_ColorFormat = colorFormat;
	m_ColorSpace = colorSpace;
	m_FullColorRange = fullRangeColor;
	m_Resolution = std::make_pair<uint32_t, uint32_t>(0, 0);
	m_FrameRate = std::make_pair<uint32_t, uint32_t>(0, 0);
	m_FrameRateTimeStepAMF = 0;
	/// Flags
	m_Started = false;
	m_OpenCLConversion = false;
	#pragma endregion Null Values

	// Initialize selected API on Video Adapter
	m_API = API::Base::GetAPIByName(videoAPI);
	m_APIAdapter = m_API->GetAdapterById(videoAdapterId & UINT_MAX, (videoAdapterId >> 32) & UINT_MAX);
	m_APIDevice = m_API->CreateInstanceOnAdapter(m_APIAdapter);

	// Initialize Advanced Media Framework
	m_AMF = AMF::GetInstance();
	/// Retrieve Factory
	m_AMFFactory = m_AMF->GetFactory();

	// Create Context for Conversion and Encoding
	AMF_RESULT res = m_AMFFactory->CreateContext(&m_AMFContext);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg,
			"<Id: %lld> Creating a AMF Context failed, error %ls (code %d).",
			m_UniqueId,
			m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	/// Initialize Context using selected API
	m_OpenCLSubmission = useOpenCL;
	switch (m_API->GetType()) {
		case API::Type::Host:
			m_AMFMemoryType = amf::AMF_MEMORY_HOST;
			m_OpenCLSubmission = false;
			break;
		case API::Type::OpenGL:
			m_AMFMemoryType = amf::AMF_MEMORY_OPENGL;
			res = m_AMFContext->InitOpenGL(m_API->GetContextFromInstance(m_APIDevice), 0, 0);
			break;
		case API::Type::Direct3D9:
			m_AMFMemoryType = amf::AMF_MEMORY_DX9;
			res = m_AMFContext->InitDX9(m_API->GetContextFromInstance(m_APIDevice));
			break;
		case API::Type::Direct3D11:
			m_AMFMemoryType = amf::AMF_MEMORY_DX11;
			res = m_AMFContext->InitDX11(m_API->GetContextFromInstance(m_APIDevice));
			break;
	}
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg,
			"<Id: %lld> Initializing %s API with Adapter '%s' failed, error %ls (code %d).",
			m_UniqueId,
			m_API->GetName().c_str(), m_APIAdapter.Name.c_str(),
			m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	// Initialize OpenCL (if possible)
	res = m_AMFContext->InitOpenCL();
	if (res != AMF_OK) {
		m_OpenCLSubmission = false;
		m_OpenCLConversion = false;

		QUICK_FORMAT_MESSAGE(errMsg,
			"<Id: %lld> Initialising OpenCL failed, error %ls (code %d)",
			m_UniqueId,
			m_AMF->GetTrace()->GetResultText(res), res);
		AMF_LOG_WARNING("%s", errMsg.data());
	} else {
		m_OpenCLConversion = true;

		if (m_OpenCLSubmission) {
			res = m_AMFContext->GetCompute(amf::AMF_MEMORY_OPENCL, &m_AMFCompute);
			if (res != AMF_OK) {
				m_OpenCLSubmission = false;

				QUICK_FORMAT_MESSAGE(errMsg,
					"<Id: %lld> Retrieving Compute object failed, error %ls (code %d)",
					m_UniqueId,
					m_AMF->GetTrace()->GetResultText(res), res);
				AMF_LOG_WARNING("%s", errMsg.data());
			}
		}
	}

	// Create Converter
	res = m_AMFFactory->CreateComponent(m_AMFContext, AMFVideoConverter, &m_AMFConverter);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg,
			"<Id: %lld> Creating frame converter component failed, error %ls (code %d)",
			m_UniqueId,
			m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	res = m_AMFConverter->SetProperty(AMF_VIDEO_CONVERTER_MEMORY_TYPE, m_AMFMemoryType);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg,
			"<Id: %lld> Unable to set converter memory type, error %ls (code %d)",
			m_UniqueId,
			m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	res = m_AMFConverter->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_FORMAT, amf::AMF_SURFACE_NV12);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg,
			"<Id: %lld> Unable to set converter output format, error %ls (code %d)",
			m_UniqueId,
			m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	res = m_AMFConverter->SetProperty(AMF_VIDEO_CONVERTER_COLOR_PROFILE, Utility::ColorSpaceToAMFConverter(m_ColorSpace));
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg,
			"<Id: %lld> Unable to set convertor color profile, error %ls (code %d)",
			m_UniqueId,
			m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	for (const wchar_t* par : fullColorParams) {
		res = m_AMFConverter->SetProperty(par, m_FullColorRange);
		if (res == AMF_OK)
			break;
	}
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg,
			"<Id: %lld> Unable to set converter color range, error %ls (code %d)",
			m_UniqueId,
			m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	res = m_AMFConverter->Init(amf::AMF_SURFACE_NV12, 0, 0);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg,
			"<Id: %lld> Unable to intialize converter, error %ls (code %d)",
			m_UniqueId,
			m_AMF->GetTrace()->GetResultText(res),
			res);
		throw std::exception(errMsg.data());
	}

	// Create Encoder
	res = m_AMFFactory->CreateComponent(m_AMFContext, Utility::CodecToAMF(codec), &m_AMFEncoder);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg,
			"<Id: %lld> Unable to create %s encoder, error %ls (code %d)",
			m_UniqueId,
			Utility::CodecToString(codec),
			m_AMF->GetTrace()->GetResultText(res),
			res);
		throw std::exception(errMsg.data());
	}

	// Full Range Color Stuff
	for (const wchar_t* par : fullColorParams) {
		res = m_AMFEncoder->SetProperty(par, m_FullColorRange);
		if (res == AMF_OK)
			break;
	}
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg,
			"<Id: %lld> Failed to set encoder color range, error %ls (code %d)",
			m_UniqueId,
			m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}

	// Show complete initialization in log.
	QUICK_FORMAT_MESSAGE(notice,
		"<Id: %lld> Initialized.",
		m_UniqueId);
	AMF_LOG_DEBUG("%s", notice.data());
}

Plugin::AMD::Encoder::~Encoder() {
	// Destroy AMF Encoder
	if (m_AMFEncoder) {
		m_AMFEncoder->Terminate();
		m_AMFEncoder = nullptr;
	}

	// Destroy AMF Converter
	if (m_AMFConverter) {
		m_AMFConverter->Terminate();
		m_AMFConverter = nullptr;
	}

	// Destroy AMF Context
	if (m_AMFContext) {
		m_AMFContext->Terminate();
		m_AMFContext = nullptr;
	}

	// Destroy API
	if (m_API) {
		m_API->DestroyInstance(m_APIDevice);
		m_APIDevice = nullptr;
		m_API = nullptr;
	}

	m_AMF = nullptr;

	// Show complete initialization in log.
	QUICK_FORMAT_MESSAGE(notice,
		"<Id: %lld> Finalized.",
		m_UniqueId);
	AMF_LOG_DEBUG("%s", notice.data());
}

void Plugin::AMD::Encoder::UpdateFrameRateValues() {
	// 1			Second
	// 1000			Millisecond
	// 1000000		Microsecond
	// 10000000		amf_pts
	// 1000000000	Nanosecond
	m_FrameRateTimeStep = 1.0 / ((double_t)m_FrameRate.first / (double_t)m_FrameRate.second);
	m_FrameRateTimeStepAMF = (uint64_t)round(m_FrameRateTimeStep * AMF_SECOND);
}

void Plugin::AMD::Encoder::Start() {
	AMF_RESULT res = m_AMFEncoder->Init(Utility::ColorFormatToAMF(m_ColorFormat), m_Resolution.first, m_Resolution.second);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg,
			"<Id: %lld> Failed to initialize encoder, error %ls (code %d)",
			m_UniqueId,
			m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
	m_Started = true;
}

void Plugin::AMD::Encoder::Restart() {
	AMF_RESULT res = m_AMFEncoder->ReInit(m_Resolution.first, m_Resolution.second);
	if (res != AMF_OK) {
		QUICK_FORMAT_MESSAGE(errMsg,
			"<Id: %lld> Could not re-initialize encoder, error %ls (code %d)",
			m_UniqueId,
			m_AMF->GetTrace()->GetResultText(res), res);
		throw std::exception(errMsg.data());
	}
}

void Plugin::AMD::Encoder::Stop() {
	if (!m_Started)
		throw std::logic_error("Can't stop an encoder that isn't running!");

	m_AMFConverter->Drain();
	m_AMFConverter->Flush();
	m_AMFEncoder->Drain();
	m_AMFEncoder->Flush();

	m_Started = false;
}

bool Plugin::AMD::Encoder::Encode(struct encoder_frame* frame, struct encoder_packet* packet, bool* received_packet) {
	if (!m_Started)
		return false;

	AMF_RESULT res;
	amf::AMFSurfacePtr pSurface = nullptr;
	amf::AMFDataPtr pData = nullptr;

	// Allocate Surface
	{
		res = m_AMFContext->AllocSurface(m_AMFMemoryType, m_AMFSurfaceFormat,
			m_Resolution.first, m_Resolution.second, &pSurface);
		if (res != AMF_OK) {
			QUICK_FORMAT_MESSAGE(errMsg,
				"<Id: %lld> Unable to allocate Surface, error %ls (code %d)",
				m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
			AMF_LOG_WARNING("%s", errMsg.data());
			return false;
		}
	}

	// Copy Information
	{
		amf::AMFComputeSyncPointPtr pSyncPoint;
		if (m_OpenCLSubmission) {
			m_AMFCompute->PutSyncPoint(&pSyncPoint);
			res = pSurface->Convert(amf::AMF_MEMORY_OPENCL);
			if (res != AMF_OK) {
				QUICK_FORMAT_MESSAGE(errMsg,
					"<Id: %lld> Conversion of Surface to OpenCL failed, error %ls (code %d)",
					m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
				AMF_LOG_WARNING("%s", errMsg.data());
				return false;
			}
		}

		size_t planeCount = pSurface->GetPlanesCount();
		for (uint8_t i = 0; i < planeCount; i++) {
			amf::AMFPlanePtr plane = pSurface->GetPlaneAt(i);
			int32_t width = plane->GetWidth();
			int32_t height = plane->GetHeight();
			int32_t hpitch = plane->GetHPitch();

			if (m_OpenCLSubmission) {
				static const amf_size l_origin[] = { 0, 0, 0 };
				const amf_size l_size[] = { (amf_size)width, (amf_size)height, 1 };
				res = m_AMFCompute->CopyPlaneFromHost(frame->data[i], l_origin, l_size, frame->linesize[i], pSurface->GetPlaneAt(i), false);
				if (res != AMF_OK) {
					QUICK_FORMAT_MESSAGE(errMsg,
						"<Id: %lld> Unable to copy plane %d with OpenCL, error %ls (code %d)",
						m_UniqueId, i, m_AMF->GetTrace()->GetResultText(res), res);
					AMF_LOG_WARNING("%s", errMsg.data());
					return false;
				}
			} else {
				void* plane_nat = plane->GetNative();
				#pragma loop(hint_parallel(32))
				for (int32_t py = 0; py < height; py++) {
					int32_t plane_off = py * hpitch;
					int32_t frame_off = py * frame->linesize[i];
					std::memcpy(
						static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off),
						static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
				}
			}
		}

		if (m_OpenCLSubmission) {
			res = m_AMFCompute->FinishQueue();
			if (res != AMF_OK) {
				QUICK_FORMAT_MESSAGE(errMsg,
					"<Id: %lld> Failed to finish OpenCL queue, error %ls (code %d)",
					m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
				AMF_LOG_WARNING("%s", errMsg.data());
				return false;
			}
			pSyncPoint->Wait();
			res = pSurface->Convert(m_AMFMemoryType);
			if (res != AMF_OK) {
				QUICK_FORMAT_MESSAGE(errMsg,
					"<Id: %lld> Conversion of Surface from OpenCL failed, error %ls (code %d)",
					m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
				AMF_LOG_WARNING("%s", errMsg.data());
				return false;
			}
		}
	}

	// Color Conversion
	if (m_OpenCLConversion) {
		pSurface->Convert(amf::AMF_MEMORY_OPENCL);
		m_AMFConverter->SubmitInput(pSurface);
		m_AMFConverter->QueryOutput(&pData);
		pSurface->Convert(m_AMFMemoryType);
	}

	// Submit Frame
	{
		if (pData != nullptr)
			pSurface = (amf::AMFSurfacePtr)pData;
		/// Presentation Timestamp
		pSurface->SetPts(frame->pts * m_FrameRateTimeStepAMF);
		/// Decode Timestamp
		pSurface->SetProperty(AMF_PRESENT_TIMESTAMP, frame->pts - 2); // -2 Offset for B-Picture Support
		/// Duration
		pSurface->SetDuration((uint64_t)round(frame->pts * m_FrameRateTimeStep * AMF_SECOND));
		/// Performance Monitoring: Submission Timestamp
		auto clk = std::chrono::high_resolution_clock::now();
		pSurface->SetProperty(AMF_SUBMIT_TIMESTAMP, std::chrono::nanoseconds(clk.time_since_epoch()).count());
		/// Submit
		res = m_AMFEncoder->SubmitInput(pSurface);
		switch (res) {
			default:
				{
					QUICK_FORMAT_MESSAGE(errMsg,
						"<Id: %lld> Submitting Surface failed, error %ls (code %d)",
						m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
					AMF_LOG_ERROR("%s", errMsg.data());
				}
				return false;
			case AMF_INPUT_FULL:
				{
					QUICK_FORMAT_MESSAGE(errMsg,
						"<Id: %lld> Submitting Surface failed , error %ls (code %d)",
						m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
					AMF_LOG_ERROR("%s", errMsg.data());
				}
				break;
			case AMF_EOF:
				break; // Swallow
		}
	}

	// Retrieve Frame
	{
		res = m_AMFEncoder->QueryOutput(&pData);
		switch (res) {
			default:
				{
					QUICK_FORMAT_MESSAGE(errMsg,
						"<Id: %lld> Retrieving Packet failed, error %ls (code %d)",
						m_UniqueId, m_AMF->GetTrace()->GetResultText(res), res);
					AMF_LOG_ERROR("%s", errMsg.data());
				}
				return false;
			case AMF_OK:
				{
					amf::AMFBufferPtr pBuffer = amf::AMFBufferPtr(pData);

					packet->type = OBS_ENCODER_VIDEO;
					packet->size = pBuffer->GetSize();
					if (m_PacketDataBuffer.size() < packet->size) {
						size_t newBufferSize = (size_t)exp2(ceil(log2(packet->size)));
						//AMF_LOG_DEBUG("Packet Buffer was resized to %d byte from %d byte.", newBufferSize, m_PacketDataBuffer.size());
						m_PacketDataBuffer.resize(newBufferSize);
					}
					packet->data = m_PacketDataBuffer.data();
					std::memcpy(packet->data, pBuffer->GetNative(), packet->size); // ToDo: Can we make this threaded?
					packet->dts = pData->GetPts() / m_FrameRateTimeStepAMF;
					pData->GetProperty(AMF_PRESENT_TIMESTAMP, &packet->pts); // Not technically needed, but VLC is stupid. :(

					{
						uint64_t pktType;
						if (m_Codec != Codec::HEVC) {
							pData->GetProperty(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, &pktType);
							switch ((AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM)pktType) {
								case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_IDR:
								case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_I:
									packet->keyframe = true;
									packet->priority = 3;
									break;
								case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_P:
									packet->priority = 2;
									break;
								case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_B:
									packet->priority = 0;
									break;
							}
						} else {
							pData->GetProperty(AMF_VIDEO_ENCODER_HEVC_OUTPUT_DATA_TYPE, &pktType);
							switch ((AMF_VIDEO_ENCODER_HEVC_OUTPUT_DATA_TYPE_ENUM)pktType) {
								case AMF_VIDEO_ENCODER_HEVC_OUTPUT_DATA_TYPE_I:
									packet->keyframe = true;
									packet->priority = 3;
									break;
								case AMF_VIDEO_ENCODER_HEVC_OUTPUT_DATA_TYPE_P:
									packet->priority = 1;
									break;
							}
						}
					}

					*received_packet = true;
				}
				break;
			case AMF_REPEAT: // These two just mean that we need to submit more and aren't actually errors.
			case AMF_NEED_MORE_INPUT:
			case AMF_EOF: // Encoder is done encoding.
				break; // Swallow
		}
	}

	return true;
}

