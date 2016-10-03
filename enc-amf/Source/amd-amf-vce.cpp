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
#include "amd-amf-vce.h"
#include "amd-amf-vce-capabilities.h"

#include "OBS-Studio/libobs/util/util_uint128.h"

#if (defined _WIN32) | (defined _WIN64)
#include <windows.h>
#include <VersionHelpers.h>
#endif

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
void Plugin::AMD::VCEEncoder::InputThreadMain(Plugin::AMD::VCEEncoder* p_this) {
	p_this->InputThreadLogic();
}

void Plugin::AMD::VCEEncoder::OutputThreadMain(Plugin::AMD::VCEEncoder* p_this) {
	p_this->OutputThreadLogic();
}

Plugin::AMD::VCEEncoder::VCEEncoder(VCEEncoderType p_Type, VCESurfaceFormat p_SurfaceFormat /*= VCESurfaceFormat_NV12*/, VCEMemoryType p_MemoryType /*= VCEMemoryType_Auto*/, VCEComputeType p_ComputeType /*= VCEComputeType_None*/) {
	AMF_RESULT res;

	AMF_LOG_INFO("<Plugin::AMD::VCEEncoder::VCEEncoder> Initializing...");

	// Solve the optimized away issue.
	m_EncoderType = p_Type;
	m_SurfaceFormat = p_SurfaceFormat;
	m_MemoryType = p_MemoryType;
	m_ComputeType = p_ComputeType;
	m_Flag_IsStarted = false;
	m_Flag_EmergencyQuit = false;
	m_Flag_Threading = true;
	m_FrameSize.first = 64;	m_FrameSize.second = 64;
	m_FrameRate.first = 30; m_FrameRate.second = 1;
	m_FrameRateDivisor = ((double_t)m_FrameRate.first / (double_t)m_FrameRate.second);
	m_FrameRateReverseDivisor = ((double_t)m_FrameRate.second / (double_t)m_FrameRate.first);
	m_InputQueueLimit = (uint32_t)(m_FrameRateDivisor * 3);
	m_TimerPeriod = 1;
	m_DecodeTimestamp = 0;

	// AMF
	m_AMF = AMF::GetInstance();
	m_AMFFactory = m_AMF->GetFactory();
	/// AMF Context
	res = m_AMFFactory->CreateContext(&m_AMFContext);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::VCEEncoder> Creating a context object failed with error %ls (code %d).", res);
	}

	// Initialize Memory
	if (m_MemoryType == VCEMemoryType_Auto) { /// Autodetect best setting depending on platform.
		#if (defined _WIN32) | (defined _WIN64)
		if (IsWindows8OrGreater()) {
			m_MemoryType = VCEMemoryType_DirectX11;
		} else if (IsWindowsXPOrGreater()) {
			m_MemoryType = VCEMemoryType_DirectX9;
		} else {
			m_MemoryType = VCEMemoryType_Host;
		}
		#else
		m_MemoryType = VCEMemoryType_OpenGL;
		#endif
	}

	switch (m_MemoryType) {
		case VCEMemoryType_Host:
			res = AMF_OK;
			break;
		case VCEMemoryType_DirectX11:
			if (IsWindows8OrGreater()) {
				res = m_AMFContext->InitDX11(nullptr);
			} else {
				AMF_LOG_ERROR("<Plugin::AMD::VCEEncoder::VCEEncoder> DirectX 11 is only supported on Windows 8 or newer, using Host Memory Type instead.");
				m_MemoryType = VCEMemoryType_Host;
			}
			break;
		case VCEMemoryType_DirectX9:
			if (IsWindowsXPOrGreater()) {
				res = m_AMFContext->InitDX9(nullptr);
			} else {
				AMF_LOG_ERROR("<Plugin::AMD::VCEEncoder::VCEEncoder> DirectX 11 is only supported on Windows 8 or newer, using Host Memory Type instead.");
				m_MemoryType = VCEMemoryType_Host;
			}
			break;
		case VCEMemoryType_OpenGL:
			res = m_AMFContext->InitOpenGL(nullptr, nullptr, nullptr);
			break;
	}
	if (res != AMF_OK)
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::VCEEncoder> Initializing 3D queue failed with error %ls (code %d)", res);

	switch (m_ComputeType) {
		case VCEComputeType_None:
			break;
		case VCEComputeType_OpenCL:
			res = m_AMFContext->InitOpenCL(nullptr);
			if (res != AMF_OK)
				ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::VCEEncoder> InitOpenCL failed with error %ls (code %d)", res);
			m_AMFContext->GetCompute(amf::AMF_MEMORY_OPENCL, &m_AMFCompute);
			break;
	}

	/// AMF Component (Encoder)
	switch (p_Type) {
		case VCEEncoderType_AVC:
			res = m_AMFFactory->CreateComponent(m_AMFContext, AMFVideoEncoderVCE_AVC, &m_AMFEncoder);
			break;
		case VCEEncoderType_SVC:
			res = m_AMFFactory->CreateComponent(m_AMFContext, AMFVideoEncoderVCE_SVC, &m_AMFEncoder);
			break;
		case VCEEncoderType_HEVC:
			res = m_AMFFactory->CreateComponent(m_AMFContext, L"AMFVideoEncoderHW_HEVC", &m_AMFEncoder);
			break;
	}
	if (res != AMF_OK)
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::VCEEncoder> Creating a component object failed with error %ls (code %d).", res);

	AMF_LOG_INFO("<Plugin::AMD::VCEEncoder::VCEEncoder> Initialization complete!");
}

Plugin::AMD::VCEEncoder::~VCEEncoder() {
	if (m_Flag_IsStarted)
		Stop();

	// AMF
	if (m_AMFEncoder) {
		m_AMFEncoder->Terminate();
		m_AMFEncoder = nullptr;
	}
	if (m_AMFContext) {
		m_AMFContext->Terminate();
		m_AMFContext = nullptr;
	}
	m_AMFFactory = nullptr;
}

void Plugin::AMD::VCEEncoder::Start() {
	static amf::AMF_SURFACE_FORMAT surfaceFormatToAMF[] = {
		// 4:2:0 Formats
		amf::AMF_SURFACE_NV12,
		amf::AMF_SURFACE_YUV420P,
		// 4:2:2 Formats
		amf::AMF_SURFACE_YUY2,
		// Uncompressed
		amf::AMF_SURFACE_BGRA,
		amf::AMF_SURFACE_RGBA,
		// Other
		amf::AMF_SURFACE_GRAY8,
	};

	// Set proper Timer resolution.
	m_TimerPeriod = 1;
	while (timeBeginPeriod(m_TimerPeriod) == TIMERR_NOCANDO) {
		++m_TimerPeriod;
	}

	// Create Encoder
	AMF_RESULT res = m_AMFEncoder->Init(surfaceFormatToAMF[m_SurfaceFormat], m_FrameSize.first, m_FrameSize.second);
	if (res != AMF_OK)
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::Start> Initialization failed with error %ls (code %d).", res);

	m_Flag_IsStarted = true;

	if (m_Flag_Threading) { // Threading
		m_Input.thread = std::thread(&(Plugin::AMD::VCEEncoder::InputThreadMain), this);
		m_Output.thread = std::thread(&(Plugin::AMD::VCEEncoder::OutputThreadMain), this);
	}
}

void Plugin::AMD::VCEEncoder::Stop() {
	// Restore Timer precision.
	if (m_TimerPeriod != 0) {
		timeEndPeriod(m_TimerPeriod);
	}

	// Stop AMF Encoder
	if (m_AMFEncoder && m_Flag_IsStarted) {
		m_AMFEncoder->Drain();
		m_AMFEncoder->Flush();
	}

	m_Flag_IsStarted = false;
	if (m_Flag_Threading) { // Threading
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
		#endif
	}

	// Clear Queues, Data
	std::queue<amf::AMFSurfacePtr>().swap(m_Input.queue);
	std::queue<amf::AMFDataPtr>().swap(m_Output.queue);
	m_PacketDataBuffer.clear();
	m_ExtraDataBuffer.clear();
}

bool Plugin::AMD::VCEEncoder::SendInput(struct encoder_frame* frame) {
	// Early-Exception if not encoding.
	if (!m_Flag_IsStarted) {
		const char* error = "<Plugin::AMD::VCEEncoder::SendInput> Attempted to send input while not running.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Convert Frame into a Surface and queue it.
	if (frame != nullptr) {
		amf::AMFSurfacePtr pAMFSurface = CreateSurfaceFromFrame(frame);
		if (pAMFSurface) {
			std::unique_lock<std::mutex> qlock(m_Input.queuemutex);
			size_t uiQueueSize = m_Input.queue.size();

			if (uiQueueSize >= m_InputQueueLimit) {
				AMF_LOG_WARNING("<Plugin::AMD::VCEEncoder::SendInput> Input Queue is full, dropping frame...");
			} else {
				m_Input.queue.push(pAMFSurface);

				if (m_InputQueueLastSize != uiQueueSize) {
					int32_t delta = ((int32_t)uiQueueSize - (int32_t)m_InputQueueLastSize);
					if (uiQueueSize == 0) {
						AMF_LOG_INFO("<Plugin::AMD::VCEEncoder::SendInput> Input Queue is empty. (%d/%d/%d)", uiQueueSize, (uiQueueSize - m_InputQueueLastSize), m_InputQueueLimit);
						m_InputQueueLastSize = uiQueueSize;
					} else if (delta >= 5) {
						AMF_LOG_WARNING("<Plugin::AMD::VCEEncoder::SendInput> Input Queue is growing. (%d/%d/%d)", uiQueueSize, (uiQueueSize - m_InputQueueLastSize), m_InputQueueLimit);
						m_InputQueueLastSize = uiQueueSize;
					} else if (delta <= -5) {
						AMF_LOG_INFO("<Plugin::AMD::VCEEncoder::SendInput> Input Queue is shrinking. (%d/%d/%d)", uiQueueSize, (uiQueueSize - m_InputQueueLastSize), m_InputQueueLimit);
						m_InputQueueLastSize = uiQueueSize;
					}
				}
			}
		} else {
			AMF_LOG_ERROR("<Plugin::AMD::VCEEncoder::SendInput> Critical error while trying to create a frame.");
			m_Flag_EmergencyQuit = true;
			return false;
		}
	}

	if (m_Flag_Threading) { // Threading
		/// Signal Thread Wakeup
		m_Input.condvar.notify_all();
	} else { // No Threading
		AMF_RESULT res = AMF_OK;
		while (res == AMF_OK) {
			if (m_Input.queue.size() == 0) {
				res = AMF_NEED_MORE_INPUT;
				break;
			}

			// Dequeue a Surface.
			amf::AMFSurfacePtr pAMFSurface = m_Input.queue.front();
			if (pAMFSurface) {
				res = m_AMFEncoder->SubmitInput(pAMFSurface);
				if (res == AMF_OK) {
					m_Input.queue.pop();
				} else if (res == AMF_INPUT_FULL) {
					if (frame != nullptr)
						GetOutput(nullptr, nullptr);
				} else {
					AMF_LOG_ERROR("<Plugin::AMD::VCEEncoder::SendInput> SubmitInput returned error %ls (code %d).", m_AMF->GetTrace()->GetResultText(res), res);
				}
			}
		}
	}

	return true;
}

void Plugin::AMD::VCEEncoder::GetOutput(struct encoder_packet* packet, bool* received_packet) {
	// Early-Exception if not encoding.
	if (!m_Flag_IsStarted) {
		const char* error = "<Plugin::AMD::VCEEncoder::GetOutput> Attempted to send input while not running.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	if (m_Flag_Threading) { // Threading
		// Query Output
		m_Output.condvar.notify_all();
	} else { // No Threading
		int64_t __amf_repeat_count = 0;

		AMF_RESULT res = AMF_OK;
		while (res == AMF_OK) {
			amf::AMFDataPtr pAMFData;
			res = m_AMFEncoder->QueryOutput(&pAMFData);
			if (res == AMF_OK) {
				std::unique_lock<std::mutex> qlock(m_Output.queuemutex);
				m_Output.queue.push(pAMFData);
			} else if (res == AMF_REPEAT) {
				if (m_Input.queue.size() == 0)
					continue;
				if (__amf_repeat_count < 5) {
					if (received_packet != nullptr) {
						SendInput(nullptr);

						res = AMF_OK;
						__amf_repeat_count++;
					}
				} else {
					__amf_repeat_count = 0;
				}
			} else {
				AMF_LOG_ERROR("<Plugin::AMD::VCEEncoder::GetOutput> QueryOutput returned error %ls (code %d).", m_AMF->GetTrace()->GetResultText(res), res);
			}
		}
	}

	if (received_packet != nullptr) {
		amf::AMFDataPtr pAMFData;
		{ // Attempt to dequeue an Item.
			std::unique_lock<std::mutex> qlock(m_Output.queuemutex);
			if (m_Output.queue.size() == 0)
				return;

			pAMFData = m_Output.queue.front();
			m_Output.queue.pop();
		}

		amf::AMFBufferPtr pAMFBuffer = amf::AMFBufferPtr(pAMFData);

		// Assemble Packet
		packet->type = OBS_ENCODER_VIDEO;
		size_t uiBufferSize = pAMFBuffer->GetSize();
		if (m_PacketDataBuffer.size() < uiBufferSize) {
			size_t newSize = (size_t)exp2(ceil(log2(uiBufferSize)));
			m_PacketDataBuffer.resize(newSize);
			AMF_LOG_WARNING("<AMFEncoder::VCE::GetOutput> Resized Packet Buffer to %d.", newSize);
		}
		packet->data = m_PacketDataBuffer.data();
		packet->size = uiBufferSize;
		std::memcpy(packet->data, pAMFBuffer->GetNative(), packet->size);

		{ // What Type of Frame is it?
			// Read Packet Type
			uint64_t pktType;
			pAMFData->GetProperty(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, &pktType);

			switch ((AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM)pktType) {
				case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_IDR://
					packet->keyframe = true;				// IDR-Frames are Key-Frames that contain a lot of information.
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

		{ // Timestamps
			//uint32_t frameTimeStep = (int64_t)(m_FrameRateReverseDivisor * 1e7);
			//int64_t dtsTimeOffset = frameTimeStep * 2;
			//
			/// Retrieve Decode-Timestamp from AMF and convert it to micro-seconds.
			//int64_t dts_usec = (pAMFData->GetPts() / 10);
			/// Decode Timestamp
			//packet->dts_usec = (dts_usec - dtsTimeOffset);
			packet->dts = m_DecodeTimestamp++ - 2;
			/// Presentation Timestamp
			pAMFBuffer->GetProperty(L"Frame", &packet->pts);

			// See: https://stackoverflow.com/questions/6044330/ffmpeg-c-what-are-pts-and-dts-what-does-this-code-block-do-in-ffmpeg-c
			// PTS may not be needed: https://github.com/GPUOpen-LibrariesAndSDKs/AMF/issues/17
		}
		*received_packet = true;

		// Debug: Packet Information
		std::vector<wchar_t> fileName(128);
		mbstowcs(fileName.data(), __FILE__, fileName.size());
		std::vector<wchar_t> functionName(128);
		mbstowcs(functionName.data(), __FUNCTION__, functionName.size());
		m_AMF->GetTrace()->TraceW(fileName.data(), __LINE__, AMF_TRACE_TRACE, L"Plugin::GetOutput", 4, L"Packet: Type(%lld), PTS(%4lld), DTS(%4lld), Size(%8lld)", (int64_t)packet->priority, (int64_t)packet->pts, (int64_t)packet->dts, (int64_t)packet->size);
		//AMF_LOG_INFO("Packet: Type(%lld), PTS(%4lld), DTS(%4lld), Size(%8lld)", packet->priority, packet->pts, packet->dts, packet->size);
	}
}

bool Plugin::AMD::VCEEncoder::GetExtraData(uint8_t**& extra_data, size_t*& extra_data_size) {
	if (!m_AMFContext || !m_AMFEncoder)
		throw std::exception("<Plugin::AMD::VCEEncoder::GetExtraData> Called while not initialized.");

	if (!m_Flag_IsStarted)
		throw std::exception("<Plugin::AMD::VCEEncoder::GetExtraData> Called while not encoding.");

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
		throw std::exception("<Plugin::AMD::VCEEncoder::GetVideoInfo> Called while not initialized.");

	if (!m_Flag_IsStarted)
		throw std::exception("<Plugin::AMD::VCEEncoder::GetVideoInfo> Called while not encoding.");

	switch (m_SurfaceFormat) {
		// 4:2:0 Formats
		case VCESurfaceFormat_NV12:
			vsi->format = VIDEO_FORMAT_NV12;
			break;
		case VCESurfaceFormat_I420:
			vsi->format = VIDEO_FORMAT_I420;
			break;
		// 4:2:2 Formats
		case VCESurfaceFormat_YUY2:
			vsi->format = VIDEO_FORMAT_YUY2;
			break;
		// Uncompressed
		case VCESurfaceFormat_RGBA:
			vsi->format = VIDEO_FORMAT_RGBA;
			break;
		case VCESurfaceFormat_BGRA:
			vsi->format = VIDEO_FORMAT_BGRA;
			break;
		// Other
		case VCESurfaceFormat_GRAY:
			vsi->format = VIDEO_FORMAT_Y800;
			break;
	}

	// AMF requires Partial Range for some reason.
	// Also Colorspace is automatic, see: https://github.com/GPUOpen-LibrariesAndSDKs/AMF/issues/6#issuecomment-243473568
	vsi->range = VIDEO_RANGE_PARTIAL;
	if (vsi->height <= 780)
		vsi->colorspace = VIDEO_CS_601;
	else
		vsi->colorspace = VIDEO_CS_709;
}

void Plugin::AMD::VCEEncoder::InputThreadLogic() {	// Thread Loop that handles Surface Submission
	// Assign Thread Name
	static const char* __threadName = "enc-amf Input Thread";
	SetThreadName(__threadName);

	// Core Loop
	std::unique_lock<std::mutex> lock(m_Input.mutex);
	do {
		m_Input.condvar.wait(lock);

		// Skip to check if isStarted is false.
		if (!m_Flag_IsStarted)
			continue;

		// Skip to next wait if queue is empty.
		AMF_RESULT res = AMF_OK;
		int32_t __amf_input_full_repeat = 0;
		while (res == AMF_OK) { // Repeat until impossible.
			amf::AMFSurfacePtr surface;

			{ // Dequeue Surface
				std::unique_lock<std::mutex> qlock(m_Input.queuemutex);
				if (m_Input.queue.size() == 0)
					break;
				surface = m_Input.queue.front();
			}

			/// Submit to AMF
			res = m_AMFEncoder->SubmitInput(surface);
			if (res == AMF_OK) {
				{ // Remove Surface from Queue
					std::unique_lock<std::mutex> qlock(m_Input.queuemutex);
					m_Input.queue.pop();
				}

				// Reset AMF_INPUT_FULL retries.
				__amf_input_full_repeat = 0;
			} else if (res == AMF_INPUT_FULL) { // Try submitting for 5 milliseconds
				m_Output.condvar.notify_all(); // Signal Querying Thread

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				if (__amf_input_full_repeat < 5) {
					res = AMF_OK;
					__amf_input_full_repeat++;
					continue;
				}
			} else {
				std::vector<char> msgBuf(128);
				FormatTextWithAMFError(&msgBuf, "%ls (code %d)", Plugin::AMD::AMF::GetInstance()->GetTrace()->GetResultText(res), res);
				AMF_LOG_WARNING("<Plugin::AMD::VCEEncoder::InputThreadLogic> SubmitInput failed with error %s.", msgBuf.data());
				continue;
			}
		}
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

		// Skip to check if isStarted is false.
		if (!m_Flag_IsStarted)
			continue;

		AMF_RESULT res = AMF_OK;
		while (res == AMF_OK) { // Repeat until impossible.
			amf::AMFDataPtr pData;

			res = m_AMFEncoder->QueryOutput(&pData);
			if (res == AMF_OK) {
				/*if (pData->GetMemoryType() != amf::AMF_MEMORY_HOST) {
					pData->Convert(amf::AMF_MEMORY_HOST);
				}*/

				// Queue
				{
					std::unique_lock<std::mutex> qlock(m_Output.queuemutex);
					m_Output.queue.push(pData);
				}
			} else if (res == AMF_REPEAT) {
				m_Input.condvar.notify_all();
			} else {
				std::vector<char> msgBuf(128);
				FormatTextWithAMFError(&msgBuf, "%s (code %d)", res);
				AMF_LOG_WARNING("<Plugin::AMD::VCEEncoder::OutputThreadLogic> QueryOutput failed with error %s.", msgBuf.data());
			}
		}
	} while (m_Flag_IsStarted);
}

amf::AMFSurfacePtr Plugin::AMD::VCEEncoder::CreateSurfaceFromFrame(struct encoder_frame*& frame) {
	static amf::AMF_SURFACE_FORMAT surfaceFormatToAMF[] = {
		// 4:2:0 Formats
		amf::AMF_SURFACE_NV12,
		amf::AMF_SURFACE_YUV420P,
		// 4:2:2 Formats
		amf::AMF_SURFACE_YUY2,
		// Uncompressed
		amf::AMF_SURFACE_BGRA,
		amf::AMF_SURFACE_RGBA,
		// Other
		amf::AMF_SURFACE_GRAY8,
	};
	amf::AMF_MEMORY_TYPE memoryTypeToAMF[] = {
		amf::AMF_MEMORY_HOST,
		amf::AMF_MEMORY_DX9,
		amf::AMF_MEMORY_DX11,
		amf::AMF_MEMORY_OPENGL,
	};

	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFSurfacePtr pSurface = nullptr;
	if (m_ComputeType != VCEComputeType_None) {
		amf_size l_origin[] = { 0, 0, 0 };
		amf_size l_size0[] = { m_FrameSize.first, m_FrameSize.second, 1 };
		amf_size l_size1[] = { m_FrameSize.first >> 1, m_FrameSize.second >> 1, 1 };

		amf::AMFComputeSyncPointPtr pSyncPoint;
		res = m_AMFContext->AllocSurface(memoryTypeToAMF[m_MemoryType], surfaceFormatToAMF[m_SurfaceFormat], m_FrameSize.first, m_FrameSize.second, &pSurface);
		if (m_ComputeType == VCEComputeType_OpenCL) {
			pSurface->Convert(amf::AMF_MEMORY_OPENCL);
		}
		m_AMFCompute->PutSyncPoint(&pSyncPoint);
		m_AMFCompute->CopyPlaneFromHost(frame->data[0], l_origin, l_size0, frame->linesize[0], pSurface->GetPlaneAt(0), false);
		m_AMFCompute->CopyPlaneFromHost(frame->data[1], l_origin, l_size1, frame->linesize[1], pSurface->GetPlaneAt(1), false);
		m_AMFCompute->FinishQueue();
		pSyncPoint->Wait();

		// Convert to AMF native type.
		pSurface->Convert(memoryTypeToAMF[m_MemoryType]);
	} else {
		res = m_AMFContext->AllocSurface(amf::AMF_MEMORY_HOST, surfaceFormatToAMF[m_SurfaceFormat], m_FrameSize.first, m_FrameSize.second, &pSurface);
		if (res != AMF_OK) // Unable to create Surface
			ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::CreateSurfaceFromFrame> Unable to create AMFSurface, error %ls (code %d).", res);

		size_t planeCount = pSurface->GetPlanesCount();
		for (uint8_t i = 0; i < planeCount; i++) {
			amf::AMFPlanePtr plane = pSurface->GetPlaneAt(i);
			void* plane_nat = plane->GetNative();
			int32_t height = plane->GetHeight();
			int32_t hpitch = plane->GetHPitch();

			for (int32_t py = 0; py < height; py++) {
				int32_t plane_off = py * hpitch;
				int32_t frame_off = py * frame->linesize[i];
				std::memcpy(static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off), static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
			}
		}

		// Convert to AMF native type.
		pSurface->Convert(memoryTypeToAMF[m_MemoryType]);
	}

	// Convert Frame Index to Nanoseconds.
	amf_pts amfPts = (int64_t)(frame->pts * (m_FrameRateReverseDivisor * 1e7));
	pSurface->SetPts(amfPts);
	pSurface->SetProperty(L"Frame", frame->pts);

	return pSurface;
}

//////////////////////////////////////////////////////////////////////////
// AMF Properties
//////////////////////////////////////////////////////////////////////////

void Plugin::AMD::VCEEncoder::LogProperties() {
	static const char* memoryTypeToString[] = {
		"Host",
		"DirectX9",
		"DirectX11",
		"OpenGL",
	};
	static const char* computeTypeToString[] = {
		"None",
		"OpenCL",
	};
	static const char* surfaceFormatToString[] = {
		"NV12",
		"I420",
		"RGBA",
	};
	static const char* usageToString[] = {
		"Transcoding",
		"Ultra Low Latency",
		"Low Latency",
		"Webcam"
	};
	static const char* qualityPresetToString[] = {
		"Speed",
		"Balanced",
		"Quality"
	};
	static const char* profileToString[] = {
		"Baseline",
		"Main",
		"High"
	};
	static const char* rateControlMethodToString[] = {
		"Constant Quantization Parameter (CQP)",
		"Constant Bitrate (CBR)",
		"Peak Constrained Variable Bitrate (VBR)",
		"Latency Constrained Variable Bitrate (VBR_LAT)"
	};

	AMF_LOG_INFO("-- AMD Advanced Media Framework VCE Encoder --");
	AMF_LOG_INFO("Initialization Parameters: ");
	AMF_LOG_INFO("  Memory Type: %s", memoryTypeToString[m_MemoryType]);
	AMF_LOG_INFO("  Compute Type: %s", computeTypeToString[m_ComputeType]);
	AMF_LOG_INFO("  Surface Format: %s", surfaceFormatToString[m_SurfaceFormat]);
	AMF_LOG_INFO("Static Parameters: ");
	AMF_LOG_INFO("  Usage: %s", usageToString[this->GetUsage()]);
	AMF_LOG_INFO("  Quality Preset: %s", qualityPresetToString[this->GetQualityPreset()]);
	AMF_LOG_INFO("  Profile: %s %d.%d", this->GetProfile() == 66 ? "Baseline" : (this->GetProfile() == 77 ? "Main" : "High"), this->GetProfileLevel() / 10, this->GetProfileLevel() % 10);
	AMF_LOG_INFO("  Maximum Long-Term Reference Frames: %d", this->GetMaximumLongTermReferenceFrames());
	AMF_LOG_INFO("Frame Parameters: ");
	AMF_LOG_INFO("  Frame Size: %dx%d", this->GetFrameSize().first, this->GetFrameSize().second);
	AMF_LOG_INFO("  Frame Rate: %d/%d", this->GetFrameRate().first, this->GetFrameRate().second);
	AMF_LOG_INFO("Rate Control Parameters: ");
	AMF_LOG_INFO("  Method: %s", rateControlMethodToString[this->GetRateControlMethod()]);
	AMF_LOG_INFO("  Frame Skipping Enabled: %s", this->IsRateControlSkipFrameEnabled() ? "Yes" : "No");
	AMF_LOG_INFO("  Filler Data Enabled: %s", this->IsFillerDataEnabled() ? "Yes" : "No");
	AMF_LOG_INFO("  Enforcce HRD Restrictions: %s", this->IsEnforceHRDRestrictionsEnabled() ? "Yes" : "No");
	AMF_LOG_INFO("  Maximum Access Unit Size: %d bits", this->GetMaximumAccessUnitSize());
	AMF_LOG_INFO("  Bitrate: ");
	AMF_LOG_INFO("    Target: %d bits", this->GetTargetBitrate());
	AMF_LOG_INFO("    Peak: %d bits", this->GetPeakBitrate());
	AMF_LOG_INFO("  Quantization Parameter: ");
	AMF_LOG_INFO("    Minimum: %d", this->GetMinimumQP());
	AMF_LOG_INFO("    Maximum: %d", this->GetMaximumQP());
	AMF_LOG_INFO("    I-Frame: %d", this->GetIFrameQP());
	AMF_LOG_INFO("    P-Frame: %d", this->GetPFrameQP());
	AMF_LOG_INFO("    B-Frame: %d", this->GetBFrameQP());
	try {
		AMF_LOG_INFO("    B-Picture Delta QP: %d", this->GetBPictureDeltaQP());
	} catch (...) {
		AMF_LOG_INFO("    B-Picture Delta QP: N/A");
	}
	try {
		AMF_LOG_INFO("    Reference B-Picture Delta QP: %d", this->GetReferenceBPictureDeltaQP());
	} catch (...) {
		AMF_LOG_INFO("    Reference B-Picture Delta QP: N/A");
	}
	AMF_LOG_INFO("  VBV Buffer: ");
	AMF_LOG_INFO("    Size: %d bits", this->GetVBVBufferSize());
	AMF_LOG_INFO("    Initial Fullness: %f%%", this->GetInitialVBVBufferFullness() * 100.0);
	AMF_LOG_INFO("Picture Control Parameters: ");
	AMF_LOG_INFO("  IDR Period: %d frames", this->GetIDRPeriod());
	AMF_LOG_INFO("  Header Insertion Spacing: %d frames", this->GetHeaderInsertionSpacing());
	AMF_LOG_INFO("  Deblocking Filter Enabled: %s", this->IsDeBlockingFilterEnabled() ? "Yes" : "No");
	AMF_LOG_INFO("  B-Picture Pattern: %d", this->GetBPicturePattern());
	AMF_LOG_INFO("  B-Picture Reference Enabled: %s", this->IsBPictureReferenceEnabled() ? "Yes" : "No");
	AMF_LOG_INFO("  Intra-Refresh MBs Number per Slot: %d", this->GetIntraRefreshMBsNumberPerSlot());
	AMF_LOG_INFO("  Slices Per Frame: %d", this->GetSlicesPerFrame());
	AMF_LOG_INFO("  Scan Type: %s", this->GetScanType() == VCEScanType_Progressive ? "Progressive" : "Interlaced");
	AMF_LOG_INFO("Motion Estimation Parameters: ");
	AMF_LOG_INFO("  Half Pixel: %s", this->IsHalfPixelMotionEstimationEnabled() ? "Enabled" : "Disabled");
	AMF_LOG_INFO("  Quarter Pixel: %s", this->IsQuarterPixelMotionEstimationEnabled() ? "Enabled" : "Disabled");
	AMF_LOG_INFO("Experimental Parameters: ");
	AMF_LOG_INFO("  Nominal Range: %s", this->GetNominalRange() ? "Enabled" : "Disabled");
	AMF_LOG_INFO("  Wait For Task: %s", this->GetWaitForTask() ? "Enabled" : "Disabled");
	AMF_LOG_INFO("  GOP Size: %d frames", this->GetGOPSize());
	AMF_LOG_INFO("  Aspect Ratio: %d:%d", this->GetAspectRatio().first, this->GetAspectRatio().second);
	AMF_LOG_INFO("  CABAC: %s", this->IsCABACEnabled() ? "Enabled" : "Disabled");
}

void Plugin::AMD::VCEEncoder::SetUsage(VCEUsage usage) {
	static AMF_VIDEO_ENCODER_USAGE_ENUM customToAMF[] = {
		AMF_VIDEO_ENCODER_USAGE_TRANSCONDING,
		AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY,
		AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY,
		AMF_VIDEO_ENCODER_USAGE_WEBCAM,
	};
	static char* customToName[] = {
		"Transcoding",
		"Ultra Low Latency",
		"Low Latency",
		"WebCam"
	};

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, (uint32_t)customToAMF[usage]);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetUsage> Setting to %s failed with error %ls (code %d).", res, customToName[usage]);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetUsage> Set to %s.", customToName[usage]);
}

Plugin::AMD::VCEUsage Plugin::AMD::VCEEncoder::GetUsage() {
	static VCEUsage AMFToCustom[] = {
		VCEUsage_Transcoding,
		VCEUsage_UltraLowLatency,
		VCEUsage_LowLatency,
		VCEUsage_Webcam
	};
	static char* customToName[] = {
		"Transcoding",
		"Ultra Low Latency",
		"Low Latency",
		"WebCam"
	};

	uint32_t usage;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_USAGE, &usage);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetUsage> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetUsage> Value is %s.", customToName[AMFToCustom[usage]]);
	return AMFToCustom[usage];
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
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetQualityPreset> Setting to %s failed with error %ls (code %d).", res, CustomToName[preset]);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetQualityPreset> Set to %s.", CustomToName[preset]);
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
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetQualityPreset> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetQualityPreset> Value is %s.", CustomToName[AMFToCustom[preset]]);
	return AMFToCustom[preset];
}

void Plugin::AMD::VCEEncoder::SetProfile(VCEProfile profile) {
	if ((profile != VCEProfile_High) && (profile != VCEProfile_Main) && (profile != VCEProfile_Baseline)) {
		profile = VCEProfile_Baseline;
	}

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, (uint32_t)profile);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetProfile> Setting to %s failed with error %ls (code %d).", res, (profile == 100 ? "High" : (profile == 77 ? "Main" : "Baseline")));
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetProfile> Set to %s.", (profile == 100 ? "High" : (profile == 77 ? "Main" : "Baseline")));
}

Plugin::AMD::VCEProfile Plugin::AMD::VCEEncoder::GetProfile() {
	uint32_t profile;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PROFILE, &profile);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetProfile> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetProfile> Value is %s.", (profile == 100 ? "High" : (profile == 77 ? "Main" : "Baseline")));
	return (VCEProfile)profile;
}

void Plugin::AMD::VCEEncoder::SetProfileLevel(VCEProfileLevel level) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, (uint32_t)level);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetProfile> Setting to %d failed with error %ls (code %d).", res, level);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetProfile> Set to %d.", level);
}

Plugin::AMD::VCEProfileLevel Plugin::AMD::VCEEncoder::GetProfileLevel() {
	uint32_t profileLevel;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, &profileLevel);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetProfileLevel> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetProfileLevel> Value is %d.", profileLevel);
	return (VCEProfileLevel)(profileLevel);
}

void Plugin::AMD::VCEEncoder::SetMaximumLongTermReferenceFrames(uint32_t maximumLTRFrames) {
	// Clamp Parameter Value
	maximumLTRFrames = max(min(maximumLTRFrames, 2), 0);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, (uint32_t)maximumLTRFrames);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetMaxLTRFrames> Setting to %d failed with error %ls (code %d).", res, maximumLTRFrames);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetMaxLTRFrames> Set to %d.", maximumLTRFrames);
}

uint32_t Plugin::AMD::VCEEncoder::GetMaximumLongTermReferenceFrames() {
	uint32_t maximumLTRFrames;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, &maximumLTRFrames);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetMaxLTRFrames> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetMaxLTRFrames> Value is %d.", maximumLTRFrames);
	return maximumLTRFrames;
}

void Plugin::AMD::VCEEncoder::SetFrameSize(uint32_t width, uint32_t height) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(width, height));
	if (res != AMF_OK) {
		std::vector<char> msgBuf(128);
		sprintf(msgBuf.data(), "%dx%d", width, height);
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetFrameSize> Setting to %s failed with error %ls (code %d).", res, msgBuf.data());
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetFrameSize> Set to %dx%d.", width, height);
}

std::pair<uint32_t, uint32_t> Plugin::AMD::VCEEncoder::GetFrameSize() {
	AMFSize frameSize;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, &frameSize);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetFrameSize> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetFrameSize> Value is %dx%d.", frameSize.width, frameSize.height);
	m_FrameSize.first = frameSize.width;
	m_FrameSize.second = frameSize.height;
	return std::pair<uint32_t, uint32_t>(m_FrameSize);
}

void Plugin::AMD::VCEEncoder::SetTargetBitrate(uint32_t bitrate) {
	// Clamp Value
	bitrate = min(max(bitrate, 10000), Plugin::AMD::VCECapabilities::GetInstance()->GetEncoderCaps(m_EncoderType)->maxBitrate);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, bitrate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetTargetBitrate> Setting to %d bits failed with error %ls (code %d).", res, bitrate);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetTargetBitrate> Set to %d bits.", bitrate);
}

uint32_t Plugin::AMD::VCEEncoder::GetTargetBitrate() {
	uint32_t bitrate;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, &bitrate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetTargetBitrate> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetTargetBitrate> Value is %d bits.", bitrate);
	return bitrate;
}

void Plugin::AMD::VCEEncoder::SetPeakBitrate(uint32_t bitrate) {
	// Clamp Value
	bitrate = min(max(bitrate, 10000), Plugin::AMD::VCECapabilities::GetInstance()->GetEncoderCaps(m_EncoderType)->maxBitrate);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, (uint32_t)bitrate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetPeakBitrate> Setting to %d bits failed with error %ls (code %d).", res, bitrate);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetPeakBitrate> Set to %d bits.", bitrate);
}

uint32_t Plugin::AMD::VCEEncoder::GetPeakBitrate() {
	uint32_t bitrate;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, &bitrate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetPeakBitrate> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetPeakBitrate> Value is %d bits.", bitrate);
	return bitrate;
}

void Plugin::AMD::VCEEncoder::SetRateControlMethod(VCERateControlMethod method) {
	static AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM CustomToAMF[] = {
		AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP,
		AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR,
		AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR,
		AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR,
	};
	static char* CustomToName[] = {
		"Constant Quantization Parameter",
		"Constant Bitrate",
		"Peak Constrained Variable Bitrate",
		"Latency Constrained Variable Bitrate",
	};

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, (uint32_t)CustomToAMF[method]);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetRateControlMethod> Setting to %s failed with error %ls (code %d).", res, CustomToName[method]);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetRateControlMethod> Set to %s.", CustomToName[method]);
}

Plugin::AMD::VCERateControlMethod Plugin::AMD::VCEEncoder::GetRateControlMethod() {
	static VCERateControlMethod AMFToCustom[] = {
		VCERateControlMethod_ConstantQP,
		VCERateControlMethod_ConstantBitrate,
		VCERateControlMethod_VariableBitrate_PeakConstrained,
		VCERateControlMethod_VariableBitrate_LatencyConstrained,
	};
	static char* CustomToName[] = {
		"Constant Quantization Parameter",
		"Constant Bitrate",
		"Peak Constrained Variable Bitrate",
		"Latency Constrained Variable Bitrate",
	};

	uint32_t method;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, &method);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetRateControlMethod> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetRateControlMethod> Value is %s.", CustomToName[AMFToCustom[method]]);
	return AMFToCustom[method];
}

void Plugin::AMD::VCEEncoder::SetRateControlSkipFrameEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetRateControlSkipFrameEnabled> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetRateControlSkipFrameEnabled> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsRateControlSkipFrameEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::IsRateControlSkipFrameEnabled> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::IsRateControlSkipFrameEnabled> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetMinimumQP(uint8_t qp) {
	// Clamp Value
	qp = max(min(qp, 51), 0);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MIN_QP, (uint32_t)qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetMinimumQP> Setting to %d failed with error %ls (code %d).", res, qp);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetMinimumQP> Set to %d.", qp);
}

uint8_t Plugin::AMD::VCEEncoder::GetMinimumQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MIN_QP, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetMinimumQP> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetMinimumQP> Value is %d.", qp);
	return (uint8_t)qp;
}

void Plugin::AMD::VCEEncoder::SetMaximumQP(uint8_t qp) {
	// Clamp Value
	qp = max(min(qp, 51), 0);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_QP, (uint32_t)qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetMaximumQP> Setting to %d failed with error %ls (code %d).", res, qp);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetMaximumQP> Set to %d.", qp);
}

uint8_t Plugin::AMD::VCEEncoder::GetMaximumQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_QP, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetMaximumQP> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetMaximumQP> Value is %d.", qp);
	return (uint8_t)qp;
}

void Plugin::AMD::VCEEncoder::SetIFrameQP(uint8_t qp) {
	// Clamp Value
	qp = max(min(qp, 51), 0);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_I, (uint32_t)qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetIFrameQP> Setting to %d failed with error %ls (code %d).", res, qp);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetIFrameQP> Set to %d.", qp);
}

uint8_t Plugin::AMD::VCEEncoder::GetIFrameQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_I, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetIFrameQP> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetIFrameQP> Value is %d.", qp);
	return (uint8_t)qp;
}

void Plugin::AMD::VCEEncoder::SetPFrameQP(uint8_t qp) {
	// Clamp Value
	qp = max(min(qp, 51), 0);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_P, (uint32_t)qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetPFrameQP> Setting to %d failed with error %ls (code %d).", res, qp);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetPFrameQP> Set to %d.", qp);
}

uint8_t Plugin::AMD::VCEEncoder::GetPFrameQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_P, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetPFrameQP> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetPFrameQP> Value is %d.", qp);
	return (uint8_t)qp;
}

void Plugin::AMD::VCEEncoder::SetBFrameQP(uint8_t qp) {
	// Clamp Value
	qp = max(min(qp, 51), 0);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_B, (uint32_t)qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetBFrameQP> Setting to %d failed with error %ls (code %d).", res, qp);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetBFrameQP> Set to %d.", qp);
}

uint8_t Plugin::AMD::VCEEncoder::GetBFrameQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_B, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetBFrameQP> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetBFrameQP> Value is %d.", qp);
	return (uint8_t)qp;
}

void Plugin::AMD::VCEEncoder::SetFrameRate(uint32_t num, uint32_t den) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(num, den));
	if (res != AMF_OK) {
		std::vector<char> msgBuf;
		sprintf(msgBuf.data(), "%d/%d", num, den);
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetFrameRate> Setting to %s failed with error %ls (code %d).", res, msgBuf.data());
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetFrameRate> Set to %d/%d.", num, den);
	m_FrameRate.first = num;
	m_FrameRate.second = den;
	m_FrameRateDivisor = (double_t)m_FrameRate.first / (double_t)m_FrameRate.second;
	m_FrameRateReverseDivisor = ((double_t)m_FrameRate.second / (double_t)m_FrameRate.first);
	m_InputQueueLimit = (uint32_t)ceil(m_FrameRateDivisor * 3);
	//AMF_LOG_DEBUG("%f div, %f revdiv", m_FrameRateDivisor, m_FrameRateReverseDivisor);

	if (m_Flag_IsStarted) { // Change Timer precision if encoding.
		if (m_TimerPeriod != 0) {
			// Restore Timer precision.
			timeEndPeriod(m_TimerPeriod);
		}

		m_TimerPeriod = 1;
		while (timeBeginPeriod(m_TimerPeriod) == TIMERR_NOCANDO) {
			++m_TimerPeriod;
		}
	}
}

std::pair<uint32_t, uint32_t> Plugin::AMD::VCEEncoder::GetFrameRate() {
	AMFRate frameRate;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FRAMERATE, &frameRate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetFrameRate> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetFrameRate> Value is %d/%d.", frameRate.num, frameRate.den);
	m_FrameRate.first = frameRate.num;
	m_FrameRate.second = frameRate.den;
	m_FrameRateDivisor = (double_t)frameRate.num / (double_t)frameRate.den;
	m_InputQueueLimit = (uint32_t)ceil(m_FrameRateDivisor * 3);
	return std::pair<uint32_t, uint32_t>(m_FrameRate);
}

void Plugin::AMD::VCEEncoder::SetVBVBufferSize(uint32_t size) {
	// Clamp Value
	size = max(min(size, 100000000), 1000); // 1kbit to 100mbit.

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, (uint32_t)size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetVBVBufferSize> Setting to %d bits failed with error %ls (code %d).", res, size);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetVBVBufferSize> Set to %d bits.", size);
}

uint32_t Plugin::AMD::VCEEncoder::GetVBVBufferSize() {
	uint32_t size;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, &size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetVBVBufferSize> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetVBVBufferSize> Value is %d.", size);
	return size;
}

void Plugin::AMD::VCEEncoder::SetInitialVBVBufferFullness(double_t fullness) {
	// Clamp Value
	fullness = max(min(fullness, 1), 0); // 0 to 100 %

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS, (uint32_t)(fullness * 64));
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetVBVBufferFullness> Setting to %f%% failed with error %ls (code %d).", res, fullness * 100);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetVBVBufferFullness> Set to %f%%.", fullness * 100);
}

double_t Plugin::AMD::VCEEncoder::GetInitialVBVBufferFullness() {
	uint32_t vbvBufferFullness;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS, &vbvBufferFullness);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetInitialVBVBufferFullness> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetInitialVBVBufferFullness> Value is %f%%.", vbvBufferFullness / 64.0 * 100.0);
	return ((double_t)vbvBufferFullness / 64.0);
}

void Plugin::AMD::VCEEncoder::SetEnforceHRDRestrictionsEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_ENFORCE_HRD, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetEnforceHRD> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetEnforceHRD> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsEnforceHRDRestrictionsEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_ENFORCE_HRD, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetEnforceHRD> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetEnforceHRD> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetFillerDataEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetFillerDataEnabled> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetFillerDataEnabled> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsFillerDataEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::IsFillerDataEnabled> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::IsFillerDataEnabled> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetMaximumAccessUnitSize(uint32_t size) {
	// Clamp Value
	size = max(min(size, 100000000), 0); // 1kbit to 100mbit.

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_AU_SIZE, (uint32_t)size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetMaxAUSize> Setting to %d bits failed with error %ls (code %d).", res, size);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetMaxAUSize> Set to %d bits.", size);
}

uint32_t Plugin::AMD::VCEEncoder::GetMaximumAccessUnitSize() {
	uint32_t size;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_AU_SIZE, &size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetMaxAUSize> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetMaxAUSize> Value is %d.", size);
	return size;
}

void Plugin::AMD::VCEEncoder::SetBPictureDeltaQP(int8_t qp) {
	// Clamp Value
	qp = max(min(qp, 10), -10);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, (int32_t)qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetBPictureDeltaQP> Setting to %d failed with error %ls (code %d).", res, qp);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetBPictureDeltaQP> Set to %d.", qp);
}

int8_t Plugin::AMD::VCEEncoder::GetBPictureDeltaQP() {
	int32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetBPictureDeltaQP> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetBPictureDeltaQP> Value is %d.", qp);
	return (int8_t)qp;
}

void Plugin::AMD::VCEEncoder::SetReferenceBPictureDeltaQP(int8_t qp) {
	// Clamp Value
	qp = max(min(qp, 10), -10);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, (int32_t)qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetReferenceBPictureDeltaQP> Setting to %d failed with error %ls (code %d).", res, qp);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetReferenceBPictureDeltaQP> Set to %d.", qp);
}

int8_t Plugin::AMD::VCEEncoder::GetReferenceBPictureDeltaQP() {
	int32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetReferenceBPictureDeltaQP> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetReferenceBPictureDeltaQP> Value is %d.", qp);
	return (int8_t)qp;
}

void Plugin::AMD::VCEEncoder::SetHeaderInsertionSpacing(uint32_t spacing) {
	// Clamp Value
	spacing = max(min(spacing, m_FrameRate.second * 1000), 0);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING, (uint32_t)spacing);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetHeaderInsertionSpacing> Setting to %d failed with error %ls (code %d).", res, spacing);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetHeaderInsertionSpacing> Set to %d.", spacing);
}

uint32_t Plugin::AMD::VCEEncoder::GetHeaderInsertionSpacing() {
	int32_t headerInsertionSpacing;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING, &headerInsertionSpacing);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetHeaderInsertionSpacing> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetHeaderInsertionSpacing> Value is %d.", headerInsertionSpacing);
	return headerInsertionSpacing;
}

void Plugin::AMD::VCEEncoder::SetIDRPeriod(uint32_t period) {
	// Clamp Value
	period = max(min(period, m_FrameRate.second * 1000), 1); // 1-1000 so that OBS can actually quit.

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, (uint32_t)period);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetIDRPeriod> Setting to %d failed with error %ls (code %d).", res, period);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetIDRPeriod> Set to %d.", period);
}

uint32_t Plugin::AMD::VCEEncoder::GetIDRPeriod() {
	int32_t period;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, &period);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetIDRPeriod> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetIDRPeriod> Value is %d.", period);
	return period;
}

void Plugin::AMD::VCEEncoder::SetDeBlockingFilterEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetDeBlockingFilterEnabled> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetDeBlockingFilterEnabled> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsDeBlockingFilterEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::IsDeBlockingFilterEnabled> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::IsDeBlockingFilterEnabled> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetIntraRefreshMBsNumberPerSlot(uint32_t mbs) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, (uint32_t)mbs);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetIntraRefreshMBsNumberPerSlot> Setting to %d failed with error %ls (code %d).", res, mbs);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetIntraRefreshMBsNumberPerSlot> Set to %d.", mbs);
}

uint32_t Plugin::AMD::VCEEncoder::GetIntraRefreshMBsNumberPerSlot() {
	int32_t mbs;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, &mbs);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetIntraRefreshMBsNumberPerSlot> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetIntraRefreshMBsNumberPerSlot> Value is %d.", mbs);
	return mbs;
}

void Plugin::AMD::VCEEncoder::SetSlicesPerFrame(uint32_t slices) {
	slices = max(slices, 1);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, (uint32_t)slices);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetSlicesPerFrame> Setting to %d failed with error %ls (code %d).", res, slices);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetSlicesPerFrame> Set to %d.", slices);
}

uint32_t Plugin::AMD::VCEEncoder::GetSlicesPerFrame() {
	uint32_t slices;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, &slices);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetSlicesPerFrame> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetSlicesPerFrame> Value is %d.", slices);
	return slices;
}

void Plugin::AMD::VCEEncoder::SetBPicturePattern(VCEBPicturePattern pattern) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, (uint32_t)pattern);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetBPicturesPattern> Setting to %d failed with error %ls (code %d).", res, pattern);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetBPicturesPattern> Set to %d.", pattern);
}

Plugin::AMD::VCEBPicturePattern Plugin::AMD::VCEEncoder::GetBPicturePattern() {
	uint32_t pattern;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, &pattern);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetBPicturesPattern> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetBPicturesPattern> Value is %d.", pattern);
	return (Plugin::AMD::VCEBPicturePattern)pattern;
}

void Plugin::AMD::VCEEncoder::SetBPictureReferenceEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetBReferenceEnabled> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetBReferenceEnabled> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsBPictureReferenceEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::IsBReferenceEnabled> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::IsBReferenceEnabled> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
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
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetScanType> Setting to %s failed with error %ls (code %d).", res, CustomToName[scanType]);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetScanType> Set to %s.", CustomToName[scanType]);
}

Plugin::AMD::VCEScanType Plugin::AMD::VCEEncoder::GetScanType() {
	static char* CustomToName[] = {
		"Progressive",
		"Interlaced",
	};

	uint32_t scanType;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_SCANTYPE, &scanType);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetScanType> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetScanType> Value is %s.", CustomToName[scanType]);
	return (Plugin::AMD::VCEScanType)scanType;
}

void Plugin::AMD::VCEEncoder::SetHalfPixelMotionEstimationEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetHalfPixelMotionEstimationEnabled> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetHalfPixelMotionEstimationEnabled> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsHalfPixelMotionEstimationEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::IsHalfPixelMotionEstimationEnabled> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::IsHalfPixelMotionEstimationEnabled> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetQuarterPixelMotionEstimationEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetQuarterPixelMotionEstimationEnabled> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetQuarterPixelMotionEstimationEnabled> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsQuarterPixelMotionEstimationEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::IsQuarterPixelMotionEstimationEnabled> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::IsQuarterPixelMotionEstimationEnabled> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetNumberOfTemporalEnhancementLayers(uint32_t layers) {
	layers = min(layers, 2);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS, (uint32_t)layers);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetNumberOfTemporalEnhancementLayers> Setting to %d failed with error %ls (code %d).", res, layers);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetNumberOfTemporalEnhancementLayers> Set to %d.", layers);
}

uint32_t Plugin::AMD::VCEEncoder::GetNumberOfTemporalEnhancementLayers() {
	uint32_t layers;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS, &layers);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetNumberOfTemporalEnhancementLayers> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetNumberOfTemporalEnhancementLayers> Value is %d.", layers);
	return layers;
}

void Plugin::AMD::VCEEncoder::SetNominalRange(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"NominalRange", enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetNominalRange> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetNominalRange> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::GetNominalRange() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"NominalRange", &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetNominalRange> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetNominalRange> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetWaitForTask(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"WaitForTask", enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetWaitForTask> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetWaitForTask> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::GetWaitForTask() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"WaitForTask", &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetWaitForTask> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetWaitForTask> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::VCEEncoder::SetGOPSize(uint32_t size) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"GOPSize", (uint32_t)size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetGOPSize> Setting to %d failed with error %ls (code %d).", res, size);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetGOPSize> Set to %d.", size);
}

uint32_t Plugin::AMD::VCEEncoder::GetGOPSize() {
	uint32_t size;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"GOPSize", &size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetGOPSize> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetGOPSize> Value is %d.", size);
	return size;
}

void Plugin::AMD::VCEEncoder::SetAspectRatio(uint32_t num, uint32_t den) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"AspectRatio", ::AMFConstructRate(num, den));
	if (res != AMF_OK) {
		std::vector<char> msgBuf;
		sprintf(msgBuf.data(), "%d:%d", num, den);
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetAspectRatio> Setting to %s failed with error %ls (code %d).", res, msgBuf.data());
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetAspectRatio> Set to %d:%d.", num, den);
}

std::pair<uint32_t, uint32_t> Plugin::AMD::VCEEncoder::GetAspectRatio() {
	AMFRate aspectRatio;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"AspectRatio", &aspectRatio);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::GetAspectRatio> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::GetAspectRatio> Value is %d:%d.", aspectRatio.num, aspectRatio.den);
	return std::pair<uint32_t, uint32_t>(aspectRatio.num, aspectRatio.den);
}

void Plugin::AMD::VCEEncoder::SetCABACEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(L"CABACEnable", enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::SetCABACEnabled> Setting to %s failed with error %ls (code %d).", res, enabled ? "Enabled" : "Disabled");
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::SetCABACEnabled> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::VCEEncoder::IsCABACEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(L"CABACEnable", &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::VCEEncoder::IsCABACEnabled> Failed with error %ls (code %d).", res);
	}
	AMF_LOG_DEBUG("<Plugin::AMD::VCEEncoder::IsCABACEnabled> Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}
