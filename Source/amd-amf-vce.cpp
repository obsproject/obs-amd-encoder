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

//////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////
#include <chrono>

#include "amd-amf-vce.h"
#include "amd-amf-vce-capabilities.h"
#include "misc-util.cpp"
#include "api-base.h"

// AMF
#include "components/VideoEncoderVCE.h"
#include "components/VideoConverter.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////

// Logging and Exception Helpers
static void FormatTextWithAMFError(std::vector<char>* buffer, const char* format, AMF_RESULT res) {
	sprintf(buffer->data(), format, Plugin::AMD::AMF::GetInstance()->GetTrace()->GetResultText(res), res);
}

template<typename _T>
static void FormatTextWithAMFError(std::vector<char>* buffer, const char* format, _T other, AMF_RESULT res) {
	sprintf(buffer->data(), format, other, Plugin::AMD::AMF::GetInstance()->GetTrace()->GetResultText(res), res);
}

// Class
#ifdef _DEBUG
static void fastPrintVariant(const char* text, amf::AMFVariantStruct variant) {
	std::vector<char> buf(1024);
	switch (variant.type) {
		case amf::AMF_VARIANT_EMPTY:
			sprintf(buf.data(), "%s%s", text, "Empty");
			break;
		case amf::AMF_VARIANT_BOOL:
			sprintf(buf.data(), "%s%s", text, variant.boolValue ? "true" : "false");
			break;
		case amf::AMF_VARIANT_INT64:
			sprintf(buf.data(), "%s%lld", text, variant.int64Value);
			break;
		case amf::AMF_VARIANT_DOUBLE:
			sprintf(buf.data(), "%s%f", text, variant.doubleValue);
			break;
		case amf::AMF_VARIANT_RECT:
			sprintf(buf.data(), "%s[%ld,%ld,%ld,%ld]", text,
				variant.rectValue.top, variant.rectValue.left,
				variant.rectValue.bottom, variant.rectValue.right);
			break;
		case amf::AMF_VARIANT_SIZE:
			sprintf(buf.data(), "%s%ldx%ld", text,
				variant.sizeValue.width, variant.sizeValue.height);
			break;
		case amf::AMF_VARIANT_POINT:
			sprintf(buf.data(), "%s[%ld,%ld]", text,
				variant.pointValue.x, variant.pointValue.y);
			break;
		case amf::AMF_VARIANT_RATE:
			sprintf(buf.data(), "%s%ld/%ld", text,
				variant.rateValue.num, variant.rateValue.den);
			break;
		case amf::AMF_VARIANT_RATIO:
			sprintf(buf.data(), "%s%ld:%ld", text,
				variant.ratioValue.num, variant.ratioValue.den);
			break;
		case amf::AMF_VARIANT_COLOR:
			sprintf(buf.data(), "%s(%d,%d,%d,%d)", text,
				variant.colorValue.r,
				variant.colorValue.g,
				variant.colorValue.b,
				variant.colorValue.a);
			break;
		case amf::AMF_VARIANT_STRING:
			sprintf(buf.data(), "%s'%s'", text,
				variant.stringValue);
			break;
		case amf::AMF_VARIANT_WSTRING:
			sprintf(buf.data(), "%s'%ls'", text,
				variant.wstringValue);
			break;
	}
	AMF_LOG_INFO("%s", buf.data());
};

static void printDebugInfo(amf::AMFComponentPtr m_AMFEncoder) {
	amf::AMFPropertyInfo* pInfo;
	size_t propCount = m_AMFEncoder->GetPropertyCount();
	AMF_LOG_INFO("-- Internal AMF Encoder Properties --");
	for (size_t propIndex = 0; propIndex < propCount; propIndex++) {
		static const char* typeToString[] = {
			"Empty",
			"Boolean",
			"Int64",
			"Double",
			"Rect",
			"Size",
			"Point",
			"Rate",
			"Ratio",
			"Color",
			"String",
			"WString",
			"Interface"
		};

		AMF_RESULT res = m_AMFEncoder->GetPropertyInfo(propIndex, (const amf::AMFPropertyInfo**) &pInfo);
		if (res != AMF_OK)
			continue;
		AMF_LOG_INFO(" [%ls] %ls (Type: %s, Index %d)",
			pInfo->name, pInfo->desc, typeToString[pInfo->type], propIndex);
		AMF_LOG_INFO("  Content Type: %d",
			pInfo->contentType);
		AMF_LOG_INFO("  Access: %s%s%s",
			(pInfo->accessType & amf::AMF_PROPERTY_ACCESS_READ) ? "R" : "",
			(pInfo->accessType & amf::AMF_PROPERTY_ACCESS_WRITE) ? "W" : "",
			(pInfo->accessType & amf::AMF_PROPERTY_ACCESS_WRITE_RUNTIME) ? "X" : "");

		AMF_LOG_INFO("  Values:");
		amf::AMFVariantStruct curStruct = amf::AMFVariantStruct();
		m_AMFEncoder->GetProperty(pInfo->name, &curStruct);
		fastPrintVariant("    Current: ", curStruct);
		fastPrintVariant("    Default: ", pInfo->defaultValue);
		fastPrintVariant("    Minimum: ", pInfo->minValue);
		fastPrintVariant("    Maximum: ", pInfo->maxValue);
		if (pInfo->pEnumDescription) {
			AMF_LOG_INFO("  Enumeration: ");
			const amf::AMFEnumDescriptionEntry* pEnumEntry = pInfo->pEnumDescription;
			while (pEnumEntry->name != nullptr) {
				AMF_LOG_INFO("    %ls (%ld)",
					pEnumEntry->name,
					pEnumEntry->value);
				pEnumEntry++;
			}
		}
	}
}
#endif

Plugin::AMD::VCEEncoder::VCEEncoder(
	VCEEncoderType p_Type,
	std::string p_VideoAPI,
	uint64_t p_VideoAdapterId,
	bool p_OpenCL,
	VCEColorFormat p_SurfaceFormat/* = VCESurfaceFormat_NV12*/
) {
	#pragma region Assign Default Values
	m_EncoderType = p_Type;
	m_SurfaceFormat = p_SurfaceFormat;
	m_OpenCL = p_OpenCL;
	m_Flag_IsStarted = false;
	m_Flag_FirstFrameReceived = false;
	m_Flag_FirstFrameSubmitted = false;
	m_FrameSize.first = 64;	m_FrameSize.second = 64;
	m_FrameRate.first = 30; m_FrameRate.second = 1;
	m_FrameRateDivisor = ((double_t)m_FrameRate.first / (double_t)m_FrameRate.second);
	m_FrameRateReverseDivisor = ((double_t)m_FrameRate.second / (double_t)m_FrameRate.first);
	m_InputQueueLimit = (uint32_t)(m_FrameRateDivisor);
	m_InputQueueLastSize = 0;
	m_TimerPeriod = 1;
	#pragma endregion Assign Default Values

	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Initializing...");

	AMF_RESULT res = AMF_OK;
	// AMF
	m_AMF = AMF::GetInstance();
	m_AMFFactory = m_AMF->GetFactory();
	/// Create an AMF context.
	if (m_AMFFactory->CreateContext(&m_AMFContext) != AMF_OK)
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> CreateContext failed with error %ls (code %ld).", res);
	/// Initialize to a specific API.
	m_API = Plugin::API::Base::GetAPIByName(p_VideoAPI);
	m_APIAdapter = m_API->GetAdapterById(p_VideoAdapterId & UINT_MAX, (p_VideoAdapterId >> 32) & UINT_MAX);
	m_APIInstance = m_API->CreateInstanceOnAdapter(m_APIAdapter);
	switch (m_API->GetType()) {
		case Plugin::API::APIType_Direct3D11:
			m_MemoryType = VCEMemoryType_DirectX11;
			res = m_AMFContext->InitDX11(m_API->GetContextFromInstance(m_APIInstance));
			break;
		case Plugin::API::APIType_Direct3D9:
			m_MemoryType = VCEMemoryType_DirectX9;
			res = m_AMFContext->InitDX9(m_API->GetContextFromInstance(m_APIInstance));
			break;
		case Plugin::API::APIType_OpenGL:
			m_MemoryType = VCEMemoryType_OpenGL;
			res = m_AMFContext->InitOpenGL(m_API->GetContextFromInstance(m_APIInstance), GetDesktopWindow(), nullptr);
			break;
		case Plugin::API::APIType_Host:
			m_MemoryType = VCEMemoryType_Host;
			m_OpenCL = false;
			break;
	}
	if (res != AMF_OK)
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Initializing Video API failed with error %ls (code %ld).", res);

	/// Initialize OpenCL if user selected it.
	if (m_OpenCL) {
		res = m_AMFContext->InitOpenCL(nullptr);
		if (res != AMF_OK)
			ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> InitOpenCL failed with error %ls (code %ld).", res);
		m_AMFContext->GetCompute(amf::AMF_MEMORY_OPENCL, &m_AMFCompute);
	}

	/// Create the AMF Encoder component.
	if (m_AMFFactory->CreateComponent(m_AMFContext, Plugin::Utility::VCEEncoderTypeAsAMF(p_Type), &m_AMFEncoder) != AMF_OK)
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Creating a component object failed with error %ls (code %ld).", res);

	/// Create the AMF Converter component.
	if (m_AMFFactory->CreateComponent(m_AMFContext, AMFVideoConverter, &m_AMFConverter) != AMF_OK)
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Unable to create VideoConverter component, error %ls (code %ld).", res);
	if (m_AMFConverter->SetProperty(AMF_VIDEO_CONVERTER_MEMORY_TYPE, Utility::MemoryTypeAsAMF(m_MemoryType)) != AMF_OK)
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Memory Type not supported by VideoConverter component, error %ls (code %ld).", res);
	if (m_AMFConverter->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_FORMAT, Utility::SurfaceFormatAsAMF(m_SurfaceFormat)))
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Color Format not supported by VideoConverter component, error %ls (code %ld).", res);

	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Initialized.");
}

Plugin::AMD::VCEEncoder::~VCEEncoder() {
	if (m_Flag_IsStarted)
		Stop();

	// AMF
	if (m_AMFConverter)
		m_AMFConverter->Terminate();
	if (m_AMFEncoder)
		m_AMFEncoder->Terminate();
	if (m_AMFContext)
		m_AMFContext->Terminate();
	m_AMFConverter = nullptr;
	m_AMFEncoder = nullptr;
	m_AMFContext = nullptr;

	// API
	if (m_APIInstance)
		m_API->DestroyInstance(m_APIInstance);
	m_APIInstance = nullptr;
	m_API = nullptr;
}

void Plugin::AMD::VCEEncoder::Start() {
	// Set proper Timer resolution.
	m_TimerPeriod = 1;
	while (timeBeginPeriod(m_TimerPeriod) == TIMERR_NOCANDO) {
		++m_TimerPeriod;
	}

	// Create Encoder
	AMF_RESULT res = m_AMFEncoder->Init(Utility::SurfaceFormatAsAMF(m_SurfaceFormat),
		m_FrameSize.first, m_FrameSize.second);
	if (res != AMF_OK)
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Encoder initialization failed with error %ls (code %ld).", res);

	// Create Converter
	m_AMFConverter->SetProperty(AMF_VIDEO_CONVERTER_COLOR_PROFILE, AMF_VIDEO_CONVERTER_COLOR_PROFILE_709);
	//m_AMFConverter->SetProperty(L"NominalRange", this->IsFullColorRangeEnabled());
	res = m_AMFConverter->Init(Utility::SurfaceFormatAsAMF(m_SurfaceFormat), m_FrameSize.first, m_FrameSize.second);
	if (res != AMF_OK)
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Converter initialization failed with error %ls (code %ld).", res);

	m_Flag_IsStarted = true;

	// Threading
	m_Input.thread = std::thread(&(Plugin::AMD::VCEEncoder::InputThreadMain), this);
	m_Output.thread = std::thread(&(Plugin::AMD::VCEEncoder::OutputThreadMain), this);
}

void Plugin::AMD::VCEEncoder::Restart() {
	if (!m_Flag_IsStarted)
		return;

	std::unique_lock<std::mutex> ilock(m_Input.mutex);
	std::unique_lock<std::mutex> olock(m_Output.mutex);

	// Create Encoder
	AMF_RESULT res = m_AMFEncoder->ReInit(m_FrameSize.first, m_FrameSize.second);
	if (res != AMF_OK)
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Initialization failed with error %ls (code %ld).", res);
}

void Plugin::AMD::VCEEncoder::Stop() {
	// Restore Timer precision.
	if (m_TimerPeriod != 0) {
		timeEndPeriod(m_TimerPeriod);
	}

	m_Flag_IsStarted = false;

	// Threading
	m_Output.condvar.notify_all();
	#if defined _WIN32 || defined _WIN64
	{ // Windows: Force terminate Thread after 1 second of waiting.
		std::thread::native_handle_type hnd = m_Output.thread.native_handle();

		uint32_t res = WaitForSingleObject((HANDLE)hnd, 1000);
		switch (res) {
			case WAIT_OBJECT_0:
				m_Output.thread.join();
				break;
			default:
				m_Output.thread.detach();
				TerminateThread((HANDLE)hnd, 0);
				break;
		}
	}
	#else
	m_Output.thread.join();
	#endif

	m_Input.condvar.notify_all();
	#if defined _WIN32 || defined _WIN64
	{ // Windows: Force terminate Thread after 1 second of waiting.
		std::thread::native_handle_type hnd = m_Input.thread.native_handle();

		uint32_t res = WaitForSingleObject((HANDLE)hnd, 1000);
		switch (res) {
			case WAIT_OBJECT_0:
				m_Input.thread.join();
				break;
			default:
				m_Input.thread.detach();
				TerminateThread((HANDLE)hnd, 0);
				break;
		}
	}
	#else
	m_Input.thread.join();
	#endif

	// Stop AMF Encoder
	if (m_AMFEncoder) {
		m_AMFEncoder->Drain();
		m_AMFEncoder->Flush();
	}

	// Clear Queues, Data
	std::queue<amf::AMFSurfacePtr>().swap(m_Input.queue);
	std::queue<amf::AMFDataPtr>().swap(m_Output.queue);
	m_PacketDataBuffer.clear();
	m_ExtraDataBuffer.clear();
}

bool Plugin::AMD::VCEEncoder::IsStarted() {
	return m_Flag_IsStarted;
}

bool Plugin::AMD::VCEEncoder::SendInput(struct encoder_frame* frame) {
	// Early-Exception if not encoding.
	if (!m_Flag_IsStarted) {
		const char* error = "<" __FUNCTION_NAME__ "> Attempted to send input while not running.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Attempt to queue for 1 second (forces "Encoding overloaded" message to appear).
	bool queueSuccessful = false;
	auto queueStart = std::chrono::high_resolution_clock::now();
	auto queueDuration = std::chrono::nanoseconds((uint64_t)floor(m_FrameRateReverseDivisor * 1000000));
	size_t queueSize = m_InputQueueLimit;
	do {
		// Wake up submission thread.
		m_Input.condvar.notify_all();

		{
			std::unique_lock<std::mutex> qlock(m_Input.queuemutex);
			queueSize = m_Input.queue.size();
		}

		// Push into queue if it has room.
		if (queueSize < m_InputQueueLimit) {
			amf::AMFSurfacePtr pAMFSurface = CreateSurfaceFromFrame(frame);
			if (!pAMFSurface) {
				AMF_LOG_ERROR("Unable copy frame for submission, terminating...");
				return false;
			} else {
				pAMFSurface->SetPts(frame->pts / m_FrameRate.second);
				pAMFSurface->SetProperty(L"Frame", frame->pts);
				pAMFSurface->SetDuration((uint64_t)ceil(m_FrameRateReverseDivisor * AMF_SECOND));
			}

			{
				std::unique_lock<std::mutex> qlock(m_Input.queuemutex);
				m_Input.queue.push(pAMFSurface);
				queueSize++;
			}
			queueSuccessful = true;
			break;
		}

		// Sleep
		std::this_thread::sleep_for(queueDuration / 4);
	} while ((queueSuccessful == false) && (std::chrono::high_resolution_clock::now() - queueStart <= queueDuration));

	// Report status.
	if (queueSuccessful) {
		int32_t queueSizeDelta = ((int32_t)m_InputQueueLastSize - (int32_t)queueSize);
		if (queueSizeDelta < -5) {
			AMF_LOG_DEBUG("Queue is shrinking.");
			m_InputQueueLastSize = queueSize;
		} else if (queueSizeDelta > 5) {
			AMF_LOG_WARNING("GPU Encoder overloaded, queue is growing... (%ld,%ld,%ld)",
				m_InputQueueLastSize, queueSizeDelta, queueSize);
			m_InputQueueLastSize = queueSize;
		}
	} else {
		AMF_LOG_ERROR("GPU Encoder overloaded, dropping frame instead...");
	}

	/// Signal Thread Wakeup
	m_Input.condvar.notify_all();

	// WORKAROUND: Block for at most 5 seconds until the first frame has been submitted.
	if (!m_Flag_FirstFrameSubmitted) {
		auto startsubmit = std::chrono::high_resolution_clock::now();
		auto diff = std::chrono::high_resolution_clock::now() - startsubmit;
		do {
			diff = std::chrono::high_resolution_clock::now() - startsubmit;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		} while ((diff <= std::chrono::seconds(5)) && !m_Flag_FirstFrameSubmitted);
		if (!m_Flag_FirstFrameSubmitted)
			throw std::exception("Unable to submit first frame, terminating...");
		else
			AMF_LOG_INFO("First submission took %d nanoseconds.", diff.count());
	}

	return true;
}

bool Plugin::AMD::VCEEncoder::GetOutput(struct encoder_packet* packet, bool* received_packet) {
	// Early-Exception if not encoding.
	if (!m_Flag_IsStarted) {
		const char* error = "<" __FUNCTION_NAME__ "> Attempted to send input while not running.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Signal Output Thread to wake up.
	m_Output.condvar.notify_all();

	// Dequeue a Packet
	{
		std::unique_lock<std::mutex> qlock(m_Output.queuemutex);
		if (m_Output.queue.size() == 0)
			return true;
	}

	// We've got a DataPtr, let's use it.
	{
		amf::AMFDataPtr pAMFData;
		{
			std::unique_lock<std::mutex> qlock(m_Output.queuemutex);
			pAMFData = m_Output.queue.front();
			m_Output.queue.pop();
		}
		amf::AMFBufferPtr pAMFBuffer = amf::AMFBufferPtr(pAMFData);

		// Assemble Packet
		packet->type = OBS_ENCODER_VIDEO;
		/// Data
		packet->size = pAMFBuffer->GetSize();
		if (m_PacketDataBuffer.size() < packet->size) {
			size_t newBufferSize = (size_t)exp2(ceil(log2(packet->size)));
			AMF_LOG_DEBUG("Packet Buffer was resized to %d byte from %d byte.", newBufferSize, m_PacketDataBuffer.size());
			m_PacketDataBuffer.resize(newBufferSize);
		}
		packet->data = m_PacketDataBuffer.data();
		std::memcpy(packet->data, pAMFBuffer->GetNative(), packet->size);
		/// Timestamps
		packet->dts = (pAMFData->GetPts() - 2) * m_FrameRate.second; // Offset by 2 to support B-Frames
		pAMFBuffer->GetProperty(L"Frame", &packet->pts);
		{ /// Packet Priority & Keyframe
			uint64_t pktType;
			pAMFData->GetProperty(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, &pktType);

			switch ((AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM)pktType) {
				case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_IDR://
					packet->keyframe = true;				// IDR-Frames are Key-Frames that contain a lot of information.
					packet->priority = 3;					// Highest priority, always continue streaming with these.
					//packet->drop_priority = 3;				// Dropped IDR-Frames can only be replaced by the next IDR-Frame.
					break;
				case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_I:	// I-Frames need only a previous I- or IDR-Frame.
					packet->priority = 2;					// I- and IDR-Frames will most likely be present.
				//	packet->drop_priority = 2;				// So we can continue with a I-Frame when streaming.
				//	break;
				case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_P:	// P-Frames need either a previous P-, I- or IDR-Frame.
					packet->priority = 1;					// We can safely assume that at least one of these is present.
				//	packet->drop_priority = 1;				// So we can continue with a P-Frame when streaming.
				//	break;
				case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_B:	// B-Frames need either a parent B-, P-, I- or IDR-Frame.
					packet->priority = 0;					// We don't know if the last non-dropped frame was a B-Frame.
				//	packet->drop_priority = 1;				// So require a P-Frame or better to continue streaming.
				//	break;
			}
		}
		*received_packet = true;

		// Debug: Packet Information
		std::vector<wchar_t> fileName(128);
		mbstowcs(fileName.data(), __FILE__, fileName.size());
		std::vector<wchar_t> functionName(128);
		mbstowcs(functionName.data(), __FUNCTION__, functionName.size());
		m_AMF->GetTrace()->TraceW(fileName.data(), __LINE__, AMF_TRACE_TRACE, L"Plugin::GetOutput", 4, L"Packet: Type(%lld), PTS(%4lld), DTS(%4lld), Size(%8lld)", (int64_t)packet->priority, (int64_t)packet->pts, (int64_t)packet->dts, (int64_t)packet->size);
	}

	return true;
}

bool Plugin::AMD::VCEEncoder::GetExtraData(uint8_t**& extra_data, size_t*& extra_data_size) {
	if (!m_AMFContext || !m_AMFEncoder)
		throw std::exception("<" __FUNCTION_NAME__ "> Called while not initialized.");

	if (!m_Flag_IsStarted)
		throw std::exception("<" __FUNCTION_NAME__ "> Called while not encoding.");

	amf::AMFVariant var;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_EXTRADATA, &var);
	if (res == AMF_OK && var.type == amf::AMF_VARIANT_INTERFACE) {
		amf::AMFBufferPtr buf(var.pInterface);

		*extra_data_size = buf->GetSize();
		m_ExtraDataBuffer.resize(*extra_data_size);
		std::memcpy(m_ExtraDataBuffer.data(), buf->GetNative(), *extra_data_size);
		*extra_data = m_ExtraDataBuffer.data();

		return true;
	}
	return false;
}

void Plugin::AMD::VCEEncoder::GetVideoInfo(struct video_scale_info*& vsi) {
	if (!m_AMFContext || !m_AMFEncoder)
		throw std::exception("<" __FUNCTION_NAME__ "> Called while not initialized.");

	if (!m_Flag_IsStarted)
		throw std::exception("<" __FUNCTION_NAME__ "> Called while not encoding.");

	switch (m_SurfaceFormat) {
		// 4:2:0 Formats
		case VCEColorFormat_NV12:
			vsi->format = VIDEO_FORMAT_NV12;
			break;
		case VCEColorFormat_I420:
			vsi->format = VIDEO_FORMAT_I420;
			break;
		// 4:2:2 Formats
		case VCEColorFormat_YUY2:
			vsi->format = VIDEO_FORMAT_YUY2;
			break;
		// Uncompressed
		case VCEColorFormat_RGBA:
			vsi->format = VIDEO_FORMAT_RGBA;
			break;
		case VCEColorFormat_BGRA:
			vsi->format = VIDEO_FORMAT_BGRA;
			break;
		// Other
		case VCEColorFormat_GRAY:
			vsi->format = VIDEO_FORMAT_Y800;
			break;
	}

	// AMF requires Partial Range for some reason.
	if (this->IsFullColorRangeEnabled()) { // Only use Full range if actually enabled.
		vsi->range = VIDEO_RANGE_FULL;
	} else {
		vsi->range = VIDEO_RANGE_PARTIAL;
	}
}

void Plugin::AMD::VCEEncoder::InputThreadMain(Plugin::AMD::VCEEncoder* p_this) {
	p_this->InputThreadLogic();
}

void Plugin::AMD::VCEEncoder::OutputThreadMain(Plugin::AMD::VCEEncoder* p_this) {
	p_this->OutputThreadLogic();
}

void Plugin::AMD::VCEEncoder::InputThreadLogic() {	// Thread Loop that handles Surface Submission
	// Assign Thread Name
	static const char* __threadName = "enc-amf Input Thread";
	SetThreadName(__threadName);

	// Core Loop
	std::unique_lock<std::mutex> lock(m_Input.mutex);
	uint32_t repeatSurfaceSubmission = 0;
	do {
		m_Input.condvar.wait(lock);

		// Assign Thread Name
		static const char* __threadName = "enc-amf Input Thread";
		SetThreadName(__threadName);

		// Skip to check if isStarted is false.
		if (!m_Flag_IsStarted)
			continue;

		// Dequeue Surface
		amf::AMFSurfacePtr surface;
		{
			std::unique_lock<std::mutex> qlock(m_Input.queuemutex);
			if (m_Input.queue.size() == 0)
				continue; // Queue is empty, 
			surface = m_Input.queue.front();
		}

		/// Convert Frame
		AMF_RESULT res;
		amf::AMFDataPtr outbuf;

		res = m_AMFConverter->SubmitInput(surface);
		if (res != AMF_OK)
			ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Unable to submit Frame to Converter, error %ls (code %ld).", res);
		res = m_AMFConverter->QueryOutput(&outbuf);
		if (res != AMF_OK)
			ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Unable to retrieve Frame from Converter, error %ls (code %ld).", res);

		/// Submit to AMF
		res = m_AMFEncoder->SubmitInput(outbuf);
		if (res == AMF_OK) {
			m_Flag_FirstFrameSubmitted = true;

			{ // Remove Surface from Queue
				std::unique_lock<std::mutex> qlock(m_Input.queuemutex);
				m_Input.queue.pop();
			}

			// Reset AMF_INPUT_FULL retries.
			repeatSurfaceSubmission = 0;

			// Continue with next Surface.
			m_Input.condvar.notify_all();
		} else if (res == AMF_INPUT_FULL) {
			m_Output.condvar.notify_all();
			if (repeatSurfaceSubmission < 5) {
				repeatSurfaceSubmission++;
				m_Input.condvar.notify_all();
			}
		} else if (res == AMF_EOF) {
			// This should never happen, but on the off-chance that it does, just straight up leave the loop.
			break;
		} else {
			// Unknown, unexpected return code.
			std::vector<char> msgBuf(128);
			FormatTextWithAMFError(&msgBuf, "%ls (code %d)", Plugin::AMD::AMF::GetInstance()->GetTrace()->GetResultText(res), res);
			AMF_LOG_WARNING("<" __FUNCTION_NAME__ "> SubmitInput failed with error %s.", msgBuf.data());
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(m_TimerPeriod));
	} while (m_Flag_IsStarted);
}

void Plugin::AMD::VCEEncoder::OutputThreadLogic() {	// Thread Loop that handles Querying
	// Assign Thread Name
	static const char* __threadName = "enc-amf Output Thread";
	SetThreadName(__threadName);

	// Core Loop
	std::unique_lock<std::mutex> lock(m_Output.mutex);
	do {
		m_Output.condvar.wait(lock);

		// Assign Thread Name
		static const char* __threadName = "enc-amf Output Thread";
		SetThreadName(__threadName);

		// Skip to check if isStarted is false.
		if (!m_Flag_IsStarted)
			continue;

		amf::AMFDataPtr pData = amf::AMFDataPtr();
		AMF_RESULT res = m_AMFEncoder->QueryOutput(&pData);
		if (res == AMF_OK) {
			m_Flag_FirstFrameReceived = true;

			{ // Queue
				std::unique_lock<std::mutex> qlock(m_Output.queuemutex);
				m_Output.queue.push(pData);
			}

			// Continue querying until nothing is left.
			m_Output.condvar.notify_all();
		} else if (res == AMF_REPEAT) {
			m_Input.condvar.notify_all();
		} else if (res == AMF_EOF) {
			// This should never happen, but on the off-chance that it does, just straight up leave the loop.
			break;
		} else {
			// Unknown, unexpected return code.
			std::vector<char> msgBuf(128);
			FormatTextWithAMFError(&msgBuf, "%s (code %d)", res);
			AMF_LOG_WARNING("<" __FUNCTION_NAME__ "> QueryOutput failed with error %s.", msgBuf.data());
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(m_TimerPeriod));
	} while (m_Flag_IsStarted);
}

inline amf::AMFSurfacePtr Plugin::AMD::VCEEncoder::CreateSurfaceFromFrame(struct encoder_frame*& frame) {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFSurfacePtr pSurface = nullptr;
	if (m_OpenCL) {
		amf_size l_origin[] = { 0, 0, 0 };
		amf_size l_size0[] = { m_FrameSize.first, m_FrameSize.second, 1 };
		amf_size l_size1[] = { m_FrameSize.first >> 1, m_FrameSize.second >> 1, 1 };

		res = m_AMFContext->AllocSurface(Utility::MemoryTypeAsAMF(m_MemoryType),
			Utility::SurfaceFormatAsAMF(m_SurfaceFormat),
			m_FrameSize.first, m_FrameSize.second, &pSurface);
		if (res != AMF_OK) // Unable to create Surface
			ThrowExceptionWithAMFError("AllocSurface failed with error %ls (code %d).", res);

		amf::AMFComputeSyncPointPtr pSyncPoint;
		m_AMFCompute->PutSyncPoint(&pSyncPoint);
		pSurface->Convert(amf::AMF_MEMORY_OPENCL);
		m_AMFCompute->CopyPlaneFromHost(frame->data[0], l_origin, l_size0, frame->linesize[0], pSurface->GetPlaneAt(0), false);
		m_AMFCompute->CopyPlaneFromHost(frame->data[1], l_origin, l_size1, frame->linesize[1], pSurface->GetPlaneAt(1), false);
		m_AMFCompute->FinishQueue();
		pSurface->Convert(Utility::MemoryTypeAsAMF(m_MemoryType));
		pSyncPoint->Wait();
	} else {
		res = m_AMFContext->AllocSurface(amf::AMF_MEMORY_HOST, Utility::SurfaceFormatAsAMF(m_SurfaceFormat),
			m_FrameSize.first, m_FrameSize.second, &pSurface);
		if (res != AMF_OK) // Unable to create Surface
			ThrowExceptionWithAMFError("AllocSurface failed with error %ls (code %d).", res);

		size_t planeCount = pSurface->GetPlanesCount();
		#pragma loop(hint_parallel(2))
		for (uint8_t i = 0; i < planeCount; i++) {
			amf::AMFPlanePtr plane = pSurface->GetPlaneAt(i);
			void* plane_nat = plane->GetNative();
			int32_t height = plane->GetHeight();
			int32_t hpitch = plane->GetHPitch();

			for (int32_t py = 0; py < height; py++) {
				int32_t plane_off = py * hpitch;
				int32_t frame_off = py * frame->linesize[i];
				std::memcpy(
					static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off),
					static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
			}
		}

		// Convert to AMF native type.
		pSurface->Convert(Utility::MemoryTypeAsAMF(m_MemoryType));
	}

	return pSurface;
}

//////////////////////////////////////////////////////////////////////////
// AMF Properties
//////////////////////////////////////////////////////////////////////////

void Plugin::AMD::VCEEncoder::LogProperties() {
	AMF_LOG_INFO("-- AMD Advanced Media Framework Encoder --");
	AMF_LOG_INFO("Initialization Parameters: ");
	AMF_LOG_INFO("  Memory Type: %s", Utility::MemoryTypeAsString(m_MemoryType));
	if (m_MemoryType != VCEMemoryType_Host) {
		AMF_LOG_INFO("  Device: %s", m_APIAdapter.Name.c_str());
		AMF_LOG_INFO("  OpenCL: %s", m_OpenCL ? "Enabled" : "Disabled");
	}
	AMF_LOG_INFO("  Surface Format: %s", Utility::SurfaceFormatAsString(m_SurfaceFormat));
	try { AMF_LOG_INFO("  Color Profile: %s", this->GetColorProfile() == VCEColorProfile_709 ? "709" : "601"); } catch (...) {}
	try { AMF_LOG_INFO("  Color Range: %s", this->IsFullColorRangeEnabled() ? "Full" : "Partial"); } catch (...) {}
	AMF_LOG_INFO("Static Parameters: ");
	AMF_LOG_INFO("  Usage: %s", Utility::UsageAsString(this->GetUsage()));
	AMF_LOG_INFO("  Quality Preset: %s", Utility::QualityPresetAsString(this->GetQualityPreset()));
	AMF_LOG_INFO("  Profile: %s %d.%d", Utility::ProfileAsString(this->GetProfile()), this->GetProfileLevel() / 10, this->GetProfileLevel() % 10);
	AMF_LOG_INFO("  Frame Size: %dx%d", this->GetFrameSize().first, this->GetFrameSize().second);
	AMF_LOG_INFO("  Frame Rate: %d/%d", this->GetFrameRate().first, this->GetFrameRate().second);
	AMF_LOG_INFO("  Scan Type: %s", this->GetScanType() == VCEScanType_Progressive ? "Progressive" : "Interlaced");
	AMF_LOG_INFO("Rate Control Parameters: ");
	AMF_LOG_INFO("  Method: %s", Utility::RateControlMethodAsString(this->GetRateControlMethod()));
	AMF_LOG_INFO("  Bitrate: ");
	AMF_LOG_INFO("    Target: %d bits", this->GetTargetBitrate());
	AMF_LOG_INFO("    Peak: %d bits", this->GetPeakBitrate());
	AMF_LOG_INFO("  Quantization Parameter: ");
	AMF_LOG_INFO("    Minimum: %d", this->GetMinimumQP());
	AMF_LOG_INFO("    Maximum: %d", this->GetMaximumQP());
	AMF_LOG_INFO("    I-Frame: %d", this->GetIFrameQP());
	AMF_LOG_INFO("    P-Frame: %d", this->GetPFrameQP());
	if (VCECapabilities::GetInstance()->GetAdapterCapabilities(m_API, m_APIAdapter, VCEEncoderType_AVC).supportsBFrames) {
		try { AMF_LOG_INFO("    B-Frame: %d", this->GetBFrameQP()); } catch (...) {}
	} else {
		AMF_LOG_INFO("    B-Frame: N/A");
	}
	AMF_LOG_INFO("  VBV Buffer: ");
	AMF_LOG_INFO("    Size: %d bits", this->GetVBVBufferSize());
	AMF_LOG_INFO("    Initial Fullness: %f%%", this->GetInitialVBVBufferFullness() * 100.0);
	AMF_LOG_INFO("  Flags: ");
	AMF_LOG_INFO("    Filler Data: %s", this->IsFillerDataEnabled() ? "Enabled" : "Disabled");
	AMF_LOG_INFO("    Frame Skipping: %s", this->IsFrameSkippingEnabled() ? "Enabled" : "Disabled");
	AMF_LOG_INFO("    Enforce HRD Restrictions: %s", this->IsEnforceHRDRestrictionsEnabled() ? "Enabled" : "Disabled");
	AMF_LOG_INFO("Frame Control Parameters: ");
	AMF_LOG_INFO("  IDR Period: %d frames", this->GetIDRPeriod());
	AMF_LOG_INFO("  Deblocking Filter: %s", this->IsDeblockingFilterEnabled() ? "Enabled" : "Disabled");
	if (VCECapabilities::GetInstance()->GetAdapterCapabilities(m_API, m_APIAdapter, VCEEncoderType_AVC).supportsBFrames) {
		AMF_LOG_INFO("  B-Frame Pattern: %d", this->GetBFramePattern());
		try { AMF_LOG_INFO("  B-Frame Delta QP: %d", this->GetBFrameDeltaQP()); } catch (...) {}
		AMF_LOG_INFO("  B-Frame Reference: %s", this->IsBFrameReferenceEnabled() ? "Enabled" : "Disabled");
		try { AMF_LOG_INFO("  B-Frame Reference Delta QP: %d", this->GetBFrameReferenceDeltaQP()); } catch (...) {}
	} else {
		AMF_LOG_INFO("  B-Frame Pattern: N/A");
		AMF_LOG_INFO("  B-Frame Delta QP: N/A");
		AMF_LOG_INFO("  B-Frame Reference: N/A");
		AMF_LOG_INFO("  B-Frame Reference Delta QP: N/A");
	}
	AMF_LOG_INFO("Motion Estimation Parameters: ");
	AMF_LOG_INFO("  Half Pixel: %s", this->IsHalfPixelMotionEstimationEnabled() ? "Enabled" : "Disabled");
	AMF_LOG_INFO("  Quarter Pixel: %s", this->IsQuarterPixelMotionEstimationEnabled() ? "Enabled" : "Disabled");
	AMF_LOG_INFO("Experimental Parameters: ");
	AMF_LOG_INFO("  Maximum Long-Term Reference Frames: %d", this->GetMaximumLongTermReferenceFrames());
	try { AMF_LOG_INFO("  Coding Type: %s", Utility::CodingTypeAsString(this->GetCodingType())); } catch (...) {}
	AMF_LOG_INFO("  Maximum Access Unit Size: %d bits", this->GetMaximumAccessUnitSize());
	AMF_LOG_INFO("  Header Insertion Spacing: %d frames", this->GetHeaderInsertionSpacing());
	AMF_LOG_INFO("  Slices Per Frame: %d", this->GetSlicesPerFrame());
	AMF_LOG_INFO("  Intra-Refresh MBs Number per Slot: %d", this->GetIntraRefreshMacroblocksPerSlot());
	try { AMF_LOG_INFO("  Wait For Task: %s", this->IsWaitForTaskEnabled() ? "Enabled" : "Disabled"); } catch (...) {}
	try { AMF_LOG_INFO("  Pre-Analysis Pass: %s", this->IsPreAnalysisPassEnabled() ? "Enabled" : "Disabled"); } catch (...) {}
	try { AMF_LOG_INFO("  VBAQ: %s", this->IsVBAQEnabled() ? "Enabled" : "Disabled"); } catch (...) {}
	try { AMF_LOG_INFO("  Maximum Reference Frames: %d", this->GetMaximumReferenceFrames()); } catch (...) {}
	try { AMF_LOG_INFO("  MaxMBPerSec: %d", this->GetMaxMBPerSec()); } catch (...) {}
	try { AMF_LOG_INFO("  Aspect Ratio: %d:%d", this->GetAspectRatio().first, this->GetAspectRatio().second); } catch (...) {}
	//try { AMF_LOG_INFO("  Quality Enhancement Mode: %s", Utility::QualityEnhancementModeAsString(this->GetQualityEnhancementMode())); } catch (...) {}

	Plugin::AMD::VCECapabilities::ReportAdapterCapabilities(m_API, m_APIAdapter);

	#ifdef _DEBUG
	printDebugInfo(m_AMFEncoder);
	#endif

	AMF_LOG_INFO("-- AMD Advanced Media Framework VCE Encoder --");
}

void Plugin::AMD::VCEEncoder::SetUsage(VCEUsage usage) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE,
		(uint32_t)Utility::UsageAsAMF(usage));
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).",
			res, Utility::UsageAsString(usage));
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", Utility::UsageAsString(usage));
}

Plugin::AMD::VCEUsage Plugin::AMD::VCEEncoder::GetUsage() {
	uint32_t usage;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_USAGE, &usage);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.",
		Utility::UsageAsString(Utility::UsageFromAMF(usage)));
	return Utility::UsageFromAMF(usage);
}

void Plugin::AMD::VCEEncoder::SetQualityPreset(VCEQualityPreset preset) {
	static AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM CustomToAMF[] = {
		AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED,
		AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED,
		AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY,
	};
	static char* CustomToName[] = {
		"Speed",
		"Balanced",
		"Quality",
	};

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, (uint32_t)CustomToAMF[preset]);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, CustomToName[preset]);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", CustomToName[preset]);
}

Plugin::AMD::VCEQualityPreset Plugin::AMD::VCEEncoder::GetQualityPreset() {
	static VCEQualityPreset AMFToCustom[] = {
		VCEQualityPreset_Balanced,
		VCEQualityPreset_Speed,
		VCEQualityPreset_Quality,
	};
	static char* CustomToName[] = {
		"Speed",
		"Balanced",
		"Quality",
	};

	uint32_t preset;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, &preset);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", CustomToName[AMFToCustom[preset]]);
	return AMFToCustom[preset];
}

void Plugin::AMD::VCEEncoder::SetProfile(VCEProfile profile) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, (uint32_t)profile);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, Utility::ProfileAsString(profile));
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", Utility::ProfileAsString(profile));
}

Plugin::AMD::VCEProfile Plugin::AMD::VCEEncoder::GetProfile() {
	uint32_t profile;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PROFILE, &profile);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", Utility::ProfileAsString((VCEProfile)profile));
	return (VCEProfile)profile;
}

void Plugin::AMD::VCEEncoder::SetProfileLevel(VCEProfileLevel level) {
	// Automatic Detection
	if (level == VCEProfileLevel_Automatic) {
		auto frameSize = this->GetFrameSize();
		auto frameRate = this->GetFrameRate();
		level = Plugin::Utility::GetMinimumProfileLevel(frameSize, frameRate);
	}

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, (uint32_t)level);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, level);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", level);
}

Plugin::AMD::VCEProfileLevel Plugin::AMD::VCEEncoder::GetProfileLevel() {
	uint32_t profileLevel;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, &profileLevel);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", profileLevel);
	return (VCEProfileLevel)(profileLevel);
}

void Plugin::AMD::VCEEncoder::SetColorProfile(VCEColorProfile profile) {
	AMF_VIDEO_CONVERTER_COLOR_PROFILE_ENUM pluginToAMF[] = {
		AMF_VIDEO_CONVERTER_COLOR_PROFILE_601,
		AMF_VIDEO_CONVERTER_COLOR_PROFILE_709,
		AMF_VIDEO_CONVERTER_COLOR_PROFILE_2020,
	};
	const char* pluginToString[] = {
		"601",
		"709",
		"2020",
	};

	AMF_RESULT res = m_AMFConverter->SetProperty(AMF_VIDEO_CONVERTER_COLOR_PROFILE,
		pluginToAMF[profile]);
	if (res != AMF_OK)
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Unable to set Color Profile, error %ls (code %ld).", res);
	m_ColorProfile = profile;
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", pluginToString[profile]);
}

Plugin::AMD::VCEColorProfile Plugin::AMD::VCEEncoder::GetColorProfile() {
	return m_ColorProfile;
}

void Plugin::AMD::VCEEncoder::SetFullColorRangeEnabled(bool enabled) {
	// Info from Mikhail:
	// - Name may change in the future
	// - Use GetProperty or GetPropertyDescription to test for older or newer drivers.
	const wchar_t* names[] = {
		L"NominalRange", // 16.11.2 and below.
		L"FullRange"
	};

	bool enabledTest;
	AMF_RESULT res = AMF_INVALID_ARG;
	for (size_t i = 0; i < 2; i++) {
		if (m_AMFEncoder->GetProperty(names[i], &enabledTest) == AMF_OK) {
			m_AMFConverter->SetProperty(names[i], enabled);
			res = m_AMFEncoder->SetProperty(names[i], enabled);
			break;
		}
	}
	if (res != AMF_OK)
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsFullColorRangeEnabled() {
	// Info from Mikhail:
	// - Name may change in the future
	// - Use GetProperty or GetPropertyDescription to test for older or newer drivers.
	const wchar_t* names[] = {
		L"NominalRange", // 16.11.2 and below.
		L"FullRange"
	};

	bool enabled;
	AMF_RESULT res = AMF_INVALID_ARG;
	for (size_t i = 0; i < 2; i++) {
		res = m_AMFEncoder->GetProperty(names[i], &enabled);
		if (res == AMF_OK) {
			break;
		}
	}
	if (res != AMF_OK)
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetResolution(uint32_t width, uint32_t height) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(width, height));
	if (res != AMF_OK) {
		std::vector<char> msgBuf(128);
		sprintf(msgBuf.data(), "%dx%d", width, height);
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, msgBuf.data());
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %dx%d.", width, height);
	m_FrameSize.first = width;
	m_FrameSize.second = height;

	if (this->GetProfileLevel() == VCEProfileLevel_Automatic)
		this->SetProfileLevel(VCEProfileLevel_Automatic);
}

std::pair<uint32_t, uint32_t> Plugin::AMD::VCEEncoder::GetFrameSize() {
	AMFSize frameSize;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, &frameSize);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %dx%d.", frameSize.width, frameSize.height);
	m_FrameSize.first = frameSize.width;
	m_FrameSize.second = frameSize.height;

	if (this->GetProfileLevel() == VCEProfileLevel_Automatic)
		this->SetProfileLevel(VCEProfileLevel_Automatic);

	return std::pair<uint32_t, uint32_t>(m_FrameSize);
}

void Plugin::AMD::VCEEncoder::SetFrameRate(uint32_t num, uint32_t den) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(num, den));
	if (res != AMF_OK) {
		std::vector<char> msgBuf;
		sprintf(msgBuf.data(), "%d/%d", num, den);
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, msgBuf.data());
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d/%d.", num, den);
	m_FrameRate.first = num;
	m_FrameRate.second = den;
	m_FrameRateDivisor = (double_t)m_FrameRate.first / (double_t)m_FrameRate.second;
	m_FrameRateReverseDivisor = ((double_t)m_FrameRate.second / (double_t)m_FrameRate.first);
	m_InputQueueLimit = (uint32_t)ceil(m_FrameRateDivisor);

	if (this->GetProfileLevel() == VCEProfileLevel_Automatic)
		this->SetProfileLevel(VCEProfileLevel_Automatic);
}

std::pair<uint32_t, uint32_t> Plugin::AMD::VCEEncoder::GetFrameRate() {
	AMFRate frameRate;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FRAMERATE, &frameRate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d/%d.", frameRate.num, frameRate.den);
	m_FrameRate.first = frameRate.num;
	m_FrameRate.second = frameRate.den;
	m_FrameRateDivisor = (double_t)frameRate.num / (double_t)frameRate.den;
	m_InputQueueLimit = (uint32_t)ceil(m_FrameRateDivisor);

	if (this->GetProfileLevel() == VCEProfileLevel_Automatic)
		this->SetProfileLevel(VCEProfileLevel_Automatic);

	return std::pair<uint32_t, uint32_t>(m_FrameRate);
}

void Plugin::AMD::VCEEncoder::SetScanType(VCEScanType scanType) {
	static AMF_VIDEO_ENCODER_SCANTYPE_ENUM CustomToAMF[] = {
		AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE,
		AMF_VIDEO_ENCODER_SCANTYPE_INTERLACED,
	};
	static char* CustomToName[] = {
		"Progressive",
		"Interlaced",
	};

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_SCANTYPE, (uint32_t)CustomToAMF[scanType]);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, CustomToName[scanType]);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", CustomToName[scanType]);
}

Plugin::AMD::VCEScanType Plugin::AMD::VCEEncoder::GetScanType() {
	static char* CustomToName[] = {
		"Progressive",
		"Interlaced",
	};

	uint32_t scanType;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_SCANTYPE, &scanType);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", CustomToName[scanType]);
	return (Plugin::AMD::VCEScanType)scanType;
}

void Plugin::AMD::VCEEncoder::SetRateControlMethod(VCERateControlMethod method) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD,
		(uint64_t)Utility::RateControlMethodAsAMF(method));
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).",
			res, Utility::RateControlMethodAsString(method));
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.",
		Utility::RateControlMethodAsString(method));
}

Plugin::AMD::VCERateControlMethod Plugin::AMD::VCEEncoder::GetRateControlMethod() {
	uint32_t method;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, &method);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.",
		Utility::RateControlMethodAsString(Utility::RateControlMethodFromAMF(method)));
	return Utility::RateControlMethodFromAMF(method);
}

void Plugin::AMD::VCEEncoder::SetTargetBitrate(uint32_t bitrate) {
	// Clamp Value
	bitrate = clamp(bitrate, 10000,
		Plugin::AMD::VCECapabilities::GetInstance()->GetAdapterCapabilities(m_API, m_APIAdapter, VCEEncoderType_AVC).maxBitrate);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, bitrate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d bits failed with error %ls (code %d).", res, bitrate);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d bits.", bitrate);
}

uint32_t Plugin::AMD::VCEEncoder::GetTargetBitrate() {
	uint32_t bitrate;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, &bitrate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d bits.", bitrate);
	return bitrate;
}

void Plugin::AMD::VCEEncoder::SetPeakBitrate(uint32_t bitrate) {
	// Clamp Value
	bitrate = clamp(bitrate, 10000,
		Plugin::AMD::VCECapabilities::GetInstance()->GetAdapterCapabilities(m_API, m_APIAdapter, VCEEncoderType_AVC).maxBitrate);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, (uint32_t)bitrate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d bits failed with error %ls (code %d).", res, bitrate);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d bits.", bitrate);
}

uint32_t Plugin::AMD::VCEEncoder::GetPeakBitrate() {
	uint32_t bitrate;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, &bitrate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d bits.", bitrate);
	return bitrate;
}

void Plugin::AMD::VCEEncoder::SetMinimumQP(uint8_t qp) {
	// Clamp Value
	qp = clamp(qp, 0, 51);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MIN_QP, (uint32_t)qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, qp);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", qp);
}

uint8_t Plugin::AMD::VCEEncoder::GetMinimumQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MIN_QP, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", qp);
	return (uint8_t)qp;
}

void Plugin::AMD::VCEEncoder::SetMaximumQP(uint8_t qp) {
	// Clamp Value
	qp = clamp(qp, 0, 51);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_QP, (uint32_t)qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, qp);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", qp);
}

uint8_t Plugin::AMD::VCEEncoder::GetMaximumQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_QP, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", qp);
	return (uint8_t)qp;
}

void Plugin::AMD::VCEEncoder::SetIFrameQP(uint8_t qp) {
	// Clamp Value
	qp = clamp(qp, 0, 51);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_I, (uint32_t)qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, qp);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", qp);
}

uint8_t Plugin::AMD::VCEEncoder::GetIFrameQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_I, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", qp);
	return (uint8_t)qp;
}

void Plugin::AMD::VCEEncoder::SetPFrameQP(uint8_t qp) {
	// Clamp Value
	qp = clamp(qp, 0, 51);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_P, (uint32_t)qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, qp);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", qp);
}

uint8_t Plugin::AMD::VCEEncoder::GetPFrameQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_P, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", qp);
	return (uint8_t)qp;
}

void Plugin::AMD::VCEEncoder::SetBFrameQP(uint8_t qp) {
	// Clamp Value
	qp = clamp(qp, 0, 51);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_B, (uint32_t)qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, qp);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", qp);
}

uint8_t Plugin::AMD::VCEEncoder::GetBFrameQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_B, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", qp);
	return (uint8_t)qp;
}

void Plugin::AMD::VCEEncoder::SetVBVBufferSize(uint32_t size) {
	// Clamp Value
	size = clamp(size, 1000, 100000000); // 1kbit to 100mbit.

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, (uint32_t)size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d bits failed with error %ls (code %d).", res, size);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d bits.", size);
}

void Plugin::AMD::VCEEncoder::SetVBVBufferAutomatic(double_t strictness) {
	uint32_t strictBitrate = 1000, looseBitrate = 100000000;

	// Strict VBV Buffer Size = Bitrate / FPS
	// Loose VBV Buffer Size = Bitrate

	switch (this->GetRateControlMethod()) {
		case VCERateControlMethod_ConstantBitrate:
		case VCERateControlMethod_VariableBitrate_LatencyConstrained:
			looseBitrate = this->GetTargetBitrate();
			break;
		case VCERateControlMethod_VariableBitrate_PeakConstrained:
			looseBitrate = max(this->GetTargetBitrate(), this->GetPeakBitrate());
			break;
		case VCERateControlMethod_ConstantQP:
			// When using Constant QP, one will have to pick a QP that is decent
			//  in both quality and bitrate. We can easily calculate both the QP
			//  required for an average bitrate and the average bitrate itself 
			//  with these formulas:
			// BITRATE = ((1 - (QP / 51)) ^ 2) * ((Width * Height) * 1.5 * (FPSNumerator / FPSDenumerator))
			// QP = (1 - sqrt(BITRATE / ((Width * Height) * 1.5 * (FPSNumerator / FPSDenumerator)))) * 51

			auto frameSize = this->GetFrameSize();
			auto frameRate = this->GetFrameRate();

			double_t bitrate = frameSize.first * frameSize.second;
			switch (this->m_SurfaceFormat) {
				case VCEColorFormat_NV12:
				case VCEColorFormat_I420:
					bitrate *= 1.5;
					break;
				case VCEColorFormat_YUY2:
					bitrate *= 4;
					break;
				case VCEColorFormat_BGRA:
				case VCEColorFormat_RGBA:
					bitrate *= 3;
					break;
				case VCEColorFormat_GRAY:
					bitrate *= 1;
					break;
			}
			bitrate *= frameRate.first / frameRate.second;

			uint8_t qp_i, qp_p, qp_b;
			qp_i = this->GetIFrameQP();
			qp_p = this->GetPFrameQP();
			try { qp_b = this->GetBFrameQP(); } catch (...) { qp_b = 51; }
			double_t qp = 1 - ((double_t)(min(min(qp_i, qp_p), qp_b)) / 51.0);
			qp = max(qp * qp, 0.001); // Needs to be at least 0.001.

			looseBitrate = static_cast<uint32_t>(bitrate * qp);
			break;
	}
	strictBitrate = static_cast<uint32_t>(looseBitrate * m_FrameRateReverseDivisor);

	#define PI 3.14159265
	double_t interpVal = (sin(max(min(strictness, 1.0), 0.0) * 90 * (PI / 180))); // sin curve?
	uint32_t realBitrate = static_cast<uint32_t>(ceil((strictBitrate * interpVal) + (looseBitrate * (1.0 - interpVal))));
	this->SetVBVBufferSize(realBitrate);
}

uint32_t Plugin::AMD::VCEEncoder::GetVBVBufferSize() {
	uint32_t size;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, &size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", size);
	return size;
}

void Plugin::AMD::VCEEncoder::SetInitialVBVBufferFullness(double_t fullness) {
	// Clamp Value
	fullness = max(min(fullness, 1), 0); // 0 to 100 %

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS, (uint32_t)(fullness * 64));
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %f%% failed with error %ls (code %d).", res, fullness * 100);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %f%%.", fullness * 100);
}

double_t Plugin::AMD::VCEEncoder::GetInitialVBVBufferFullness() {
	uint32_t vbvBufferFullness;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS, &vbvBufferFullness);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %f%%.", vbvBufferFullness / 64.0 * 100.0);
	return ((double_t)vbvBufferFullness / 64.0);
}

void Plugin::AMD::VCEEncoder::SetFillerDataEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsFillerDataEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetFrameSkippingEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsFrameSkippingEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetEnforceHRDRestrictionsEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_ENFORCE_HRD, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsEnforceHRDRestrictionsEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_ENFORCE_HRD, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetIDRPeriod(uint32_t period) {
	// Clamp Value
	period = max(min(period, 1000), 1); // 1-1000 so that OBS can actually quit.

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, (uint32_t)period);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, period);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", period);
}

uint32_t Plugin::AMD::VCEEncoder::GetIDRPeriod() {
	int32_t period;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, &period);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", period);
	return period;
}

void Plugin::AMD::VCEEncoder::SetBFramePattern(VCEBFramePattern pattern) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, (uint32_t)pattern);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, pattern);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", pattern);
}

Plugin::AMD::VCEBFramePattern Plugin::AMD::VCEEncoder::GetBFramePattern() {
	uint32_t pattern;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, &pattern);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", pattern);
	return (Plugin::AMD::VCEBFramePattern)pattern;
}

void Plugin::AMD::VCEEncoder::SetBFrameDeltaQP(int8_t qp) {
	// Clamp Value
	qp = clamp(qp, -10, 10);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, (int32_t)qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, qp);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", qp);
}

int8_t Plugin::AMD::VCEEncoder::GetBFrameDeltaQP() {
	int32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", qp);
	return (int8_t)qp;
}

void Plugin::AMD::VCEEncoder::SetBFrameReferenceEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsBFrameReferenceEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetBFrameReferenceDeltaQP(int8_t qp) {
	// Clamp Value
	qp = clamp(qp, -10, 10);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, (int32_t)qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, qp);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", qp);
}

int8_t Plugin::AMD::VCEEncoder::GetBFrameReferenceDeltaQP() {
	int32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", qp);
	return (int8_t)qp;
}

void Plugin::AMD::VCEEncoder::SetDeblockingFilterEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsDeblockingFilterEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetHalfPixelMotionEstimationEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsHalfPixelMotionEstimationEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetQuarterPixelMotionEstimationEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsQuarterPixelMotionEstimationEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

//////////////////////////////////////////////////////////////////////////
// EXPERIMENTAL PROPERTIES - MAY BREAK AT ANY POINT IN TIME!
//////////////////////////////////////////////////////////////////////////
// Their effect may vary from driver to driver, card to card.

uint32_t Plugin::AMD::VCEEncoder::GetMaxMBPerSec() {
	uint32_t maxMBPerSec;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"MaxMBPerSec", &maxMBPerSec);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", maxMBPerSec);
	return maxMBPerSec;
}

void Plugin::AMD::VCEEncoder::SetHeaderInsertionSpacing(uint32_t spacing) {
	// Clamp Value
	spacing = max(min(spacing, m_FrameRate.second * 1000), 0);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING, (uint32_t)spacing);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, spacing);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", spacing);
}

uint32_t Plugin::AMD::VCEEncoder::GetHeaderInsertionSpacing() {
	int32_t headerInsertionSpacing;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING, &headerInsertionSpacing);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", headerInsertionSpacing);
	return headerInsertionSpacing;
}

void Plugin::AMD::VCEEncoder::SetMaximumLongTermReferenceFrames(uint32_t maximumLTRFrames) {
	// Clamp Parameter Value
	maximumLTRFrames = max(min(maximumLTRFrames, 2), 0);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, (uint32_t)maximumLTRFrames);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, maximumLTRFrames);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", maximumLTRFrames);
}

uint32_t Plugin::AMD::VCEEncoder::GetMaximumLongTermReferenceFrames() {
	uint32_t maximumLTRFrames;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, &maximumLTRFrames);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", maximumLTRFrames);
	return maximumLTRFrames;
}

void Plugin::AMD::VCEEncoder::SetCodingType(VCECodingType type) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"CABACEnable", type);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, Utility::CodingTypeAsString(type));
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", Utility::CodingTypeAsString(type));
}

VCECodingType Plugin::AMD::VCEEncoder::GetCodingType() {
	uint64_t type;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"CABACEnable", &type);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", Utility::CodingTypeAsString((VCECodingType)type));
	return (VCECodingType)type;
}

void Plugin::AMD::VCEEncoder::SetMaximumAccessUnitSize(uint32_t size) {
	// Clamp Value
	size = max(min(size, 100000000), 0); // 1kbit to 100mbit.

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_AU_SIZE, (uint32_t)size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d bits failed with error %ls (code %d).", res, size);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d bits.", size);
}

uint32_t Plugin::AMD::VCEEncoder::GetMaximumAccessUnitSize() {
	uint32_t size;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_AU_SIZE, &size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", size);
	return size;
}

void Plugin::AMD::VCEEncoder::SetWaitForTaskEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"WaitForTask", enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsWaitForTaskEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"WaitForTask", &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetPreAnalysisPassEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"RateControlPreanalysisEnable", enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsPreAnalysisPassEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"RateControlPreanalysisEnable", &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetVBAQEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"EanbleVBAQ", enabled); // ToDo: Typo in AMF.
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsVBAQEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"EanbleVBAQ", &enabled); // ToDo: Typo in AMF.
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetGOPSize(uint32_t size) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"GOPSize", (uint32_t)size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, size);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", size);
}

uint32_t Plugin::AMD::VCEEncoder::GetGOPSize() {
	uint32_t size;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"GOPSize", &size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", size);
	return size;
}

void Plugin::AMD::VCEEncoder::SetGOPAlignmentEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"EnableGOPAlignment", enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsGOPAlignementEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"EnableGOPAlignment", &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetMaximumReferenceFrames(uint32_t numFrames) {
	auto caps = VCECapabilities::GetInstance()->GetAdapterCapabilities(m_API, m_APIAdapter, VCEEncoderType_AVC);
	numFrames = clamp(numFrames,
		caps.minReferenceFrames,
		caps.maxReferenceFrames);

	AMF_RESULT res = m_AMFEncoder->SetProperty(L"MaxNumRefFrames", (uint32_t)numFrames);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, numFrames);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", numFrames);
}

uint32_t Plugin::AMD::VCEEncoder::GetMaximumReferenceFrames() {
	uint32_t numFrames;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"MaxNumRefFrames", &numFrames);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", numFrames);
	return numFrames;
}

void Plugin::AMD::VCEEncoder::SetAspectRatio(uint32_t num, uint32_t den) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"AspectRatio", ::AMFConstructRate(num, den));
	if (res != AMF_OK) {
		std::vector<char> msgBuf;
		sprintf(msgBuf.data(), "%d:%d", num, den);
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, msgBuf.data());
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d:%d.", num, den);
}

std::pair<uint32_t, uint32_t> Plugin::AMD::VCEEncoder::GetAspectRatio() {
	AMFRate aspectRatio;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"AspectRatio", &aspectRatio);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d:%d.", aspectRatio.num, aspectRatio.den);
	return std::pair<uint32_t, uint32_t>(aspectRatio.num, aspectRatio.den);
}

void Plugin::AMD::VCEEncoder::SetIntraRefreshMacroblocksPerSlot(uint32_t mbs) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, (uint32_t)mbs);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, mbs);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", mbs);
}

uint32_t Plugin::AMD::VCEEncoder::GetIntraRefreshMacroblocksPerSlot() {
	int32_t mbs;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, &mbs);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", mbs);
	return mbs;
}

void Plugin::AMD::VCEEncoder::SetIntraRefreshNumberOfStripes(uint32_t stripes) {
	stripes = clamp(stripes, 0, INT_MAX);

	AMF_RESULT res = m_AMFEncoder->SetProperty(L"IntraRefreshNumOfStripes", (uint32_t)stripes);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, stripes);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", stripes);

}

uint32_t Plugin::AMD::VCEEncoder::GetIntraRefreshNumberOfStripes() {
	uint32_t stripes;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"IntraRefreshNumOfStripes", &stripes);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", stripes);
	return stripes;
}

void Plugin::AMD::VCEEncoder::SetSlicesPerFrame(uint32_t slices) {
	slices = max(slices, 1);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, (uint32_t)slices);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, slices);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", slices);
}

uint32_t Plugin::AMD::VCEEncoder::GetSlicesPerFrame() {
	uint32_t slices;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, &slices);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", slices);
	return slices;
}

void Plugin::AMD::VCEEncoder::SetSliceMode(VCESliceMode mode) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"SliceMode", (uint32_t)mode);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, Utility::SliceModeAsString(mode));
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", Utility::SliceModeAsString(mode));
}

Plugin::AMD::VCESliceMode Plugin::AMD::VCEEncoder::GetSliceMode() {
	uint32_t mode;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"SliceMode", &mode);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", Utility::SliceModeAsString((VCESliceMode)mode));
	return (VCESliceMode)mode;
}

void Plugin::AMD::VCEEncoder::SetMaximumSliceSize(uint32_t size) {
	size = clamp(size, 1, INT_MAX);

	AMF_RESULT res = m_AMFEncoder->SetProperty(L"MaxSliceSize", (uint32_t)size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, size);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", size);
}

uint32_t Plugin::AMD::VCEEncoder::GetMaximumSliceSize() {
	uint32_t size;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"MaxSliceSize", &size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", size);
	return size;
}

void Plugin::AMD::VCEEncoder::SetSliceControlMode(VCESliceControlMode mode) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"SliceControlMode", (uint32_t)mode);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %s failed with error %ls (code %d).", res, Utility::SliceControlModeAsString(mode));
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %s.", Utility::SliceControlModeAsString(mode));
}

Plugin::AMD::VCESliceControlMode Plugin::AMD::VCEEncoder::GetSliceControlMode() {
	uint32_t mode;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"SliceControlMode", &mode);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %s.", Utility::SliceControlModeAsString((VCESliceControlMode)mode));
	return (VCESliceControlMode)mode;
}

void Plugin::AMD::VCEEncoder::SetSliceControlSize(uint32_t size) {
	// If GetSliceMode() is VCESliceMode_Vertical, then it outputs nothing with the following settings:
	// - SliceControlMode: VCESliceControlMode_Macroblock
	// - SliceControlSize: < 3600
	// If GetSliceMode() is VCESliceMode_Horizontal, then it outputs nothing with the following settings:
	// - SliceControlMode: VCESliceControlMode_Macroblock
	// - SliceControlSize: < 32

	// H264 Macroblock = 16*16 = 256
	switch (GetSliceControlMode()) {
		case VCESliceControlMode_Off:
			return;
		case VCESliceControlMode_Macroblock:
			size = clamp(size, 0, (uint32_t)(ceil(m_FrameSize.first / 16) * ceil(m_FrameSize.second / 16)));
			break;
		case VCESliceControlMode_Macroblock_Row:
			size = clamp(size, 0, (uint32_t)ceil(m_FrameSize.second / 16));
			break;
	}
	
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"SliceControlSize", (uint32_t)size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Setting to %d failed with error %ls (code %d).", res, size);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Set to %d.", size);
}

uint32_t Plugin::AMD::VCEEncoder::GetSliceControlSize() {
	uint32_t size;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"SliceControlSize", &size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<" __FUNCTION_NAME__ "> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Value is %d.", size);
	return size;
}
