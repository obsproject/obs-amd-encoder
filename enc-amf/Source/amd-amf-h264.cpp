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
#include "amd-amf-h264.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
static char* AMF_RESULT_AS_TEXT[] = {
	"AMF_OK",
	"AMF_FAIL",
	// common errors
	"AMF_UNEXPECTED",
	"AMF_ACCESS_DENIED",
	"AMF_INVALID_ARG",
	"AMF_OUT_OF_RANGE",
	"AMF_OUT_OF_MEMORY",
	"AMF_INVALID_POINTER",
	"AMF_NO_INTERFACE",
	"AMF_NOT_IMPLEMENTED",
	"AMF_NOT_SUPPORTED",
	"AMF_NOT_FOUND",
	"AMF_ALREADY_INITIALIZED",
	"AMF_NOT_INITIALIZED",
	"AMF_INVALID_FORMAT",
	"AMF_WRONG_STATE",
	"AMF_FILE_NOT_OPEN",
	// device common codes
	"AMF_NO_DEVICE",
	// device directx
	"AMF_DIRECTX_FAILED",
	// device opencl 
	"AMF_OPENCL_FAILED",
	// device opengl 
	"AMF_GLX_FAILED",
	// device XV 
	"AMF_XV_FAILED",
	// device alsa
	"AMF_ALSA_FAILED",
	// component common codes
	//		result codes
	"AMF_EOF",
	"AMF_REPEAT",
	"AMF_INPUT_FULL",
	"AMF_RESOLUTION_CHANGED",
	"AMF_RESOLUTION_UPDATED",
	//		error codes
	"AMF_INVALID_DATA_TYPE",
	"AMF_INVALID_RESOLUTION",
	"AMF_CODEC_NOT_SUPPORTED",
	"AMF_SURFACE_FORMAT_NOT_SUPPORTED",
	"AMF_SURFACE_MUST_BE_SHARED",
	// component video decoder
	"AMF_DECODER_NOT_PRESENT",
	"AMF_DECODER_SURFACE_ALLOCATION_FAILED",
	"AMF_DECODER_NO_FREE_SURFACES",
	// component video encoder
	"AMF_ENCODER_NOT_PRESENT",
	// component video processor
	// component video conveter
	// component dem
	"AMF_DEM_ERROR",
	"AMF_DEM_PROPERTY_READONLY",
	"AMF_DEM_REMOTE_DISPLAY_CREATE_FAILED",
	"AMF_DEM_START_ENCODING_FAILED",
	"AMF_DEM_QUERY_OUTPUT_FAILED",
	// component TAN
	"AMF_TAN_CLIPPING_WAS_REQUIRED",
	"AMF_TAN_UNSUPPORTED_VERSION",
	"AMF_NEED_MORE_INPUT",
};

// Logging and Exception Helpers
static void FormatTextWithAMFError(std::vector<char>* buffer, const char* format, AMF_RESULT res) {
	sprintf(buffer->data(), format, AMF_RESULT_AS_TEXT[res], res);
}

static void ThrowExceptionWithAMFError(const char* errorMsg, AMF_RESULT res) {
	std::vector<char> msgBuf(1024);
	FormatTextWithAMFError(&msgBuf, errorMsg, res);
	AMF_LOG_ERROR("%s", msgBuf.data());
	throw std::exception(msgBuf.data());
}

template<typename _T>
static void FormatTextWithAMFError(std::vector<char>* buffer, const char* format, _T other, AMF_RESULT res) {
	sprintf(buffer->data(), format, other, AMF_RESULT_AS_TEXT[res], res);
}

template<typename _T>
static void ThrowExceptionWithAMFError(const char* errorMsg, _T other, AMF_RESULT res) {
	std::vector<char> msgBuf(1024);
	FormatTextWithAMFError(&msgBuf, errorMsg, other, res);
	AMF_LOG_ERROR("%s", msgBuf.data());
	throw std::exception(msgBuf.data());
}

void Plugin::AMD::H264VideoEncoder::InputThreadMain(Plugin::AMD::H264VideoEncoder* p_this) {
	p_this->InputThreadLogic();
}

void Plugin::AMD::H264VideoEncoder::OutputThreadMain(Plugin::AMD::H264VideoEncoder* p_this) {
	p_this->OutputThreadLogic();
}

Plugin::AMD::H264VideoEncoder::H264VideoEncoder(H264EncoderType p_Type, H264MemoryType p_MemoryType) {
	AMF_RESULT res;

	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::H264VideoEncoder> Initializing...");

	// AMF
	m_AMF = AMF::GetInstance();
	m_AMFFactory = m_AMF->GetFactory();
	/// AMF Context
	res = m_AMFFactory->CreateContext(&m_AMFContext);
	if (res != AMF_OK) {
		AMF_LOG_ERROR("<Plugin::AMD::H264VideoEncoder::H264VideoEncoder> Creating a context object failed with error code %d.", res);
		throw;
	}
	switch (p_MemoryType) {
		case H264MemoryType_Host:
			break;
		case H264MemoryType_DirectX9:
			m_AMFContext->InitDX9(nullptr);
			break;
		case H264MemoryType_DirectX11:
			m_AMFContext->InitDX11(nullptr);
			break;
		case H264MemoryType_OpenGL:
			m_AMFContext->InitOpenGL(nullptr, nullptr, nullptr);
			break;
		case H264MemoryType_OpenCL:
			m_AMFContext->InitOpenCL(nullptr);
			break;
	}
	/// AMF Component (Encoder)
	switch (p_Type) {
		case H264EncoderType_AVC:
			res = m_AMFFactory->CreateComponent(m_AMFContext, AMFVideoEncoderVCE_AVC, &m_AMFEncoder);
			break;
		/*case H264EncoderType_SVC:
			res = m_AMFFactory->CreateComponent(m_AMFContext, AMFVideoEncoderVCE_SVC, &m_AMFEncoder);
			break;*/
	}
	if (res != AMF_OK) {
		AMF_LOG_ERROR("<Plugin::AMD::H264VideoEncoder::H264VideoEncoder> Creating a component object failed with error code %d.", res);
		throw;
	}

	m_EncoderType = p_Type;
	m_MemoryType = p_MemoryType;

	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::H264VideoEncoder> Initialization complete!");
}

Plugin::AMD::H264VideoEncoder::~H264VideoEncoder() {
	if (m_IsStarted)
		Stop();

	// AMF
	if (m_AMFEncoder) {
		m_AMFEncoder->Terminate();
		m_AMFEncoder->Release();
		m_AMFEncoder = nullptr;
	}
	if (m_AMFContext) {
		m_AMFContext->Terminate();
		m_AMFContext->Release();
		m_AMFContext = nullptr;
	}
	m_AMFFactory = nullptr;
}

void Plugin::AMD::H264VideoEncoder::Start() {
	AMF_RESULT res;
	switch (m_InputSurfaceFormat) {
		case H264SurfaceFormat_NV12:
			res = m_AMFEncoder->Init(amf::AMF_SURFACE_NV12, m_FrameSize.first, m_FrameSize.second);
			break;
		case H264SurfaceFormat_I420:
			res = m_AMFEncoder->Init(amf::AMF_SURFACE_YUV420P, m_FrameSize.first, m_FrameSize.second);
			break;
		case H264SurfaceFormat_RGBA:
			res = m_AMFEncoder->Init(amf::AMF_SURFACE_RGBA, m_FrameSize.first, m_FrameSize.second);
			break;
	}
	if (res != AMF_OK)
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::Start> ", res);

	// Threading
	/// Create and start Threads
	m_ThreadedInput.thread = std::thread(&(Plugin::AMD::H264VideoEncoder::InputThreadMain), this);
	m_ThreadedOutput.thread = std::thread(&(Plugin::AMD::H264VideoEncoder::OutputThreadMain), this);

	m_IsStarted = true;
}

void Plugin::AMD::H264VideoEncoder::Stop() {
	m_IsStarted = false;

	// Threading
	m_ThreadedOutput.condvar.notify_all();
	m_ThreadedOutput.thread.join();
	m_ThreadedInput.condvar.notify_all();
	m_ThreadedInput.thread.join();

	// Stop AMF Encoder
	m_AMFEncoder->Drain();
	m_AMFEncoder->Flush();
}

bool Plugin::AMD::H264VideoEncoder::SendInput(struct encoder_frame*& frame) {
	AMF_RESULT res = AMF_UNEXPECTED;

	// Early-Exception if not encoding.
	if (!m_IsStarted) {
		const char* error = "<AMFEncoder::VCE::SendInput> Attempted to send input while not running.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Submit Input
	amf::AMFSurfacePtr pSurface = CreateSurfaceFromFrame(frame);
	{/// Queue Frame 
		std::unique_lock<std::mutex> lock(m_ThreadedInput.mutex);
		if (m_ThreadedInput.queue.size() < (size_t)ceil((double_t)m_FrameRate.first / (double_t)m_FrameRate.second)) {
			m_ThreadedInput.queue.push(pSurface);
			/// Signal Thread Wakeup
			m_ThreadedInput.condvar.notify_all();
		} else {
			AMF_LOG_ERROR("<Plugin::AMD::H264VideoEncoder::SendInput> Input Queue is full, aborting...");
			pSurface->Release();
			return false;
		}
	}

	return true;
}

void Plugin::AMD::H264VideoEncoder::GetOutput(struct encoder_packet*& packet, bool*& received_packet) {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFDataPtr pData;

	// Early-Exception if not encoding.
	if (!m_IsStarted) {
		const char* error = "<Plugin::AMD::H264VideoEncoder::GetOutput> Attempted to send input while not running.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Query Output
	m_ThreadedOutput.condvar.notify_all();

	// Check Queue
	{
		std::unique_lock<std::mutex> lock(m_ThreadedOutput.mutex);
		if (m_ThreadedOutput.queue.size() == 0) {
			*received_packet = false;
			return;
		}

		// Submit Packet to OBS
		ThreadData pkt = m_ThreadedOutput.queue.front();
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
		packet->pts = packet->dts = pkt.frame;
		switch (pkt.type) {
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

		// Remove front() element.
		m_ThreadedOutput.queue.pop();
	}

	#ifdef DEBUG
	AMF_LOG_INFO("Packet: Priority(%d), DropPriority(%d), PTS(%d), Size(%d)", packet->priority, packet->drop_priority, packet->pts, packet->size);
	#endif

	*received_packet = true;
}

bool Plugin::AMD::H264VideoEncoder::GetExtraData(uint8_t**& extra_data, size_t*& extra_data_size) {
	if (!m_AMFContext || !m_AMFEncoder)
		throw std::exception("<Plugin::AMD::H264VideoEncoder::GetExtraData> Called while not initialized.");

	if (!m_IsStarted)
		throw std::exception("<Plugin::AMD::H264VideoEncoder::GetExtraData> Called while not encoding.");

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

void Plugin::AMD::H264VideoEncoder::GetVideoInfo(struct video_scale_info*& vsi) {
	if (!m_AMFContext || !m_AMFEncoder)
		throw std::exception("<Plugin::AMD::H264VideoEncoder::GetVideoInfo> Called while not initialized.");

	if (!m_IsStarted)
		throw std::exception("<Plugin::AMD::H264VideoEncoder::GetVideoInfo> Called while not encoding.");

	switch (m_InputSurfaceFormat) {
		case H264SurfaceFormat_NV12:
			vsi->format = VIDEO_FORMAT_NV12;
			break;
		case H264SurfaceFormat_I420:
			vsi->format = VIDEO_FORMAT_I420;
			break;
		case H264SurfaceFormat_RGBA:
			vsi->format = VIDEO_FORMAT_RGBA;
			break;
		default: 
			vsi->format = VIDEO_FORMAT_NV12;
			break;
	}

	//ToDo: Figure out Color Range and Color Profile conversion (AMD worker says there is a default Video Converter attached?)
}

void Plugin::AMD::H264VideoEncoder::SetInputSurfaceFormat(H264SurfaceFormat p_Format) {
	m_InputSurfaceFormat = p_Format;
}

Plugin::AMD::H264SurfaceFormat Plugin::AMD::H264VideoEncoder::GetInputSurfaceFormat() {
	return m_InputSurfaceFormat;
}

void Plugin::AMD::H264VideoEncoder::SetOutputSurfaceFormat(H264SurfaceFormat p_Format) {
	m_OutputSurfaceFormat = p_Format;
}

Plugin::AMD::H264SurfaceFormat Plugin::AMD::H264VideoEncoder::GetOutputSurfaceFormat() {
	return m_OutputSurfaceFormat;
}

void Plugin::AMD::H264VideoEncoder::SetUsage(H264Usage usage) {
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

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_USAGE, customToAMF[usage]);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetUsage> Setting to %s failed with error %s (code %d).", customToName[usage], res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetUsage> Set to %s.", customToName[usage]);
}

Plugin::AMD::H264Usage Plugin::AMD::H264VideoEncoder::GetUsage() {
	static H264Usage AMFToCustom[] = {
		H264Usage_Transcoding,
		H264Usage_UltraLowLatency,
		H264Usage_LowLatency,
		H264Usage_Webcam
	};
	static char* customToName[] = {
		"Transcoding",
		"Ultra Low Latency",
		"Low Latency",
		"WebCam"
	};
	
	AMF_VIDEO_ENCODER_USAGE_ENUM usage;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_USAGE, &usage);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetUsage> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetUsage> Retrieved Property, Value is %s.", customToName[AMFToCustom[usage]]);
	return AMFToCustom[usage];
}

void Plugin::AMD::H264VideoEncoder::SetProfile(H264Profile profile) {
	static AMF_VIDEO_ENCODER_PROFILE_ENUM customToAMF[] = {
		AMF_VIDEO_ENCODER_PROFILE_BASELINE,
		AMF_VIDEO_ENCODER_PROFILE_MAIN,
		AMF_VIDEO_ENCODER_PROFILE_HIGH,
	};
	static char* customToName[] = {
		"Baseline",
		"Main",
		"High",
	};

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE, customToAMF[profile]);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetProfile> Setting to %s failed with error %s (code %d).", customToName[profile], res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetProfile> Set to %s.", customToName[profile]);
}

Plugin::AMD::H264Profile Plugin::AMD::H264VideoEncoder::GetProfile() {
	static char* customToName[] = {
		"Baseline",
		"Main",
		"High",
	};

	AMF_VIDEO_ENCODER_PROFILE_ENUM profile;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PROFILE, &profile);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetProfile> Retrieving Property failed with error %s (code %d).", res);
	}
	switch (profile) {
		case AMF_VIDEO_ENCODER_PROFILE_BASELINE:
			AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetProfile> Retrieved Property, Value is %s.", customToName[H264Profile_Baseline]);
			return H264Profile_Baseline;
			break;
		case AMF_VIDEO_ENCODER_PROFILE_MAIN:
			AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetProfile> Retrieved Property, Value is %s.", customToName[H264Profile_Main]);
			return H264Profile_Main;
			break;
		case AMF_VIDEO_ENCODER_PROFILE_HIGH:
			AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetProfile> Retrieved Property, Value is %s.", customToName[H264Profile_High]);
			return H264Profile_High;
			break;
	}
}

void Plugin::AMD::H264VideoEncoder::SetProfileLevel(H264ProfileLevel level) {

}

Plugin::AMD::H264ProfileLevel Plugin::AMD::H264VideoEncoder::GetProfileLevel() {

}

void Plugin::AMD::H264VideoEncoder::SetMaxLTRFrames(uint32_t maximumLTRFrames) {
	// Clamp Parameter Value
	maximumLTRFrames = max(min(maximumLTRFrames, 2), 0);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, maximumLTRFrames);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetMaxLTRFrames> Setting to %d failed with error %s (code %d).", maximumLTRFrames, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetMaxLTRFrames> Set to %d.", maximumLTRFrames);
}

uint32_t Plugin::AMD::H264VideoEncoder::GetMaxLTRFrames() {

}

void Plugin::AMD::H264VideoEncoder::SetFrameSize(uint32_t width, uint32_t height) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(width, height));
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetFrameSize> Setting to %dx%d failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetFrameSize> Set to %dx%d.", width, height);
}

std::pair<uint32_t, uint32_t> Plugin::AMD::H264VideoEncoder::GetFrameSize() {
	
}

void Plugin::AMD::H264VideoEncoder::SetFrameRate(uint32_t num, uint32_t den) {
	
	m_FrameRateDivisor = (double_t)num / (double_t)den;
}

std::pair<uint32_t, uint32_t> Plugin::AMD::H264VideoEncoder::GetFrameRate() {
	return std::pair<uint32_t, uint32_t>(m_FrameRate);
}

void Plugin::AMD::H264VideoEncoder::InputThreadLogic() {
	// Thread Loop that handles Surface Submission
	std::unique_lock<std::mutex> lock(m_ThreadedInput.mutex);
	do {
		m_ThreadedInput.condvar.wait(lock);

		// Skip to check if isStarted is false.
		if (!m_IsStarted)
			continue;

		// Skip to next wait if queue is empty.
		AMF_RESULT res = AMF_OK;
		while ((m_ThreadedInput.queue.size() > 0) && res == AMF_OK) { // Repeat until impossible.
			amf::AMFSurfacePtr surface = m_ThreadedInput.queue.front();

			AMF_RESULT res = m_AMFEncoder->SubmitInput(surface);
			if (res == AMF_OK) {
				surface->Release();
				m_ThreadedInput.queue.pop();
			} else if (res != AMF_INPUT_FULL) {
				std::vector<char> msgBuf(128);
				FormatTextWithAMFError(&msgBuf, "%s (code %d)", res);
				AMF_LOG_WARNING("<AMFENcoder::VCE::InputThreadMethod> SubmitInput failed with error %s.", msgBuf.data());
			}

			if (m_ThreadedInput.queue.size() > 10) // Magic number for now.
				AMF_LOG_WARNING("<AMFENcoder::VCE::InputThreadMethod> Input Queue is filling up. (%d of %d)", m_ThreadedInput.queue.size(), (size_t)((double_t)m_FrameRate.first / (double_t)m_FrameRate.second) * 60);
		}
	} while (m_IsStarted);
}

void Plugin::AMD::H264VideoEncoder::OutputThreadLogic() {
	// Thread Loop that handles Querying
	uint64_t lastFrameIndex = -1;

	std::unique_lock<std::mutex> lock(m_ThreadedOutput.mutex);
	do {
		m_ThreadedOutput.condvar.wait(lock);

		// Skip to check if isStarted is false.
		if (!m_IsStarted)
			continue;

		AMF_RESULT res = AMF_OK;
		while (res == AMF_OK) { // Repeat until impossible.
			amf::AMFDataPtr pData;

			res = m_AMFEncoder->QueryOutput(&pData);
			if (res == AMF_OK) {
				amf::AMFVariant variant;
				ThreadData pkt;

				// Acquire Buffer
				amf::AMFBufferPtr pBuffer(pData);

				// Create a Packet
				pkt.data.resize(pBuffer->GetSize());
				std::memcpy(pkt.data.data(), pBuffer->GetNative(), pkt.data.size());
				pkt.frame = uint64_t(pData->GetPts() * m_FrameRateDivisor); // (m_frameRateDiv / 1e7)); // Not sure what way around the accuracy is better.
				AMF_RESULT res2 = pData->GetProperty(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, &variant);
				if (res2 == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
					pkt.type = (AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM)variant.ToUInt64();
				}
				if ((lastFrameIndex >= pkt.frame) && (lastFrameIndex != 0))
					AMF_LOG_ERROR("<AMFENcoder::VCE::OutputThreadMethod> Detected out of order packet. Frame Index is %d, expected %d.", pkt.frame, lastFrameIndex + 1);
				lastFrameIndex = pkt.frame;
				
				// Release Buffer and Data
				pBuffer->Release();
				pData->Release();

				// Queue
				m_ThreadedOutput.queue.push(pkt);
			} else if (res != AMF_REPEAT) {
				std::vector<char> msgBuf(128);
				FormatTextWithAMFError(&msgBuf, "%s (code %d)", res);
				AMF_LOG_WARNING("<AMFENcoder::VCE::OutputThreadMethod> QueryOutput failed with error %s.", msgBuf.data());
			}
		}
	} while (m_IsStarted);
}

amf::AMFSurfacePtr Plugin::AMD::H264VideoEncoder::CreateSurfaceFromFrame(struct encoder_frame*& frame) {
	AMF_RESULT res = AMF_UNEXPECTED;
	amf::AMFSurfacePtr pSurface = nullptr;
	amf::AMF_SURFACE_FORMAT surfaceFormatToAMF[] = {
		amf::AMF_SURFACE_NV12,
		amf::AMF_SURFACE_YUV420P,
		amf::AMF_SURFACE_RGBA
	};
	amf::AMF_MEMORY_TYPE memoryTypeToAMF[] = {
		amf::AMF_MEMORY_HOST,
		amf::AMF_MEMORY_DX11,
		amf::AMF_MEMORY_OPENGL
	};

	if (m_MemoryType == H264MemoryType_Host) {
		#pragma region Host Memory Type
		size_t planeCount;

		res = m_AMFContext->AllocSurface(
			memoryTypeToAMF[m_MemoryType], surfaceFormatToAMF[m_InputSurfaceFormat],
			m_FrameSize.first, m_FrameSize.second,
			&pSurface);
		if (res != AMF_OK) // Unable to create Surface
			ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::CreateSurfaceFromFrame> Unable to create AMFSurface, error %s (code %d).", res);

		planeCount = pSurface->GetPlanesCount();

		#pragma loop(hint_parallel(2))
		for (uint8_t i = 0; i < planeCount; i++) {
			amf::AMFPlane* plane;
			void* plane_nat;
			int32_t height;
			size_t hpitch;

			plane = pSurface->GetPlaneAt(i);
			plane_nat = plane->GetNative();
			height = plane->GetHeight();
			hpitch = plane->GetHPitch();

			#pragma loop(hint_parallel(2))
			for (int32_t py = 0; py < height; py++) {
				size_t plane_off = py * hpitch;
				size_t frame_off = py * frame->linesize[i];
				std::memcpy(static_cast<void*>(static_cast<uint8_t*>(plane_nat) + plane_off), static_cast<void*>(frame->data[i] + frame_off), frame->linesize[i]);
			}
		}
		#pragma endregion Host Memory Type

		amf_pts amfPts = (int64_t)ceil((frame->pts / ((double_t)m_FrameRate.first / (double_t)m_FrameRate.second)) * 10000000l);
			//(1 * 1000 * 1000 * 10)
		pSurface->SetPts(amfPts);
		pSurface->SetProperty(L"Frame", frame->pts);
	}

	return pSurface;
}

