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

#include "amd-amf-h264-capabilities.h"

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////
#define AMF_SYNC_LOCK(x) { \
		x; \
	};
//#define AMF_SYNC_LOCK(x) { \
//		std::unique_lock<std::mutex> amflock(m_AMFSyncLock); \
//		x; \
//	};

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
		/*m_AMFEncoder->Release();
		m_AMFEncoder = nullptr;*/
	}
	if (m_AMFContext) {
		m_AMFContext->Terminate();
		/*m_AMFContext->Release();
		m_AMFContext = nullptr;*/
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
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::Start> Initialization failed with error %s (code %d).", res);

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
		const char* error = "<Plugin::AMD::H264VideoEncoder::SendInput> Attempted to send input while not running.";
		AMF_LOG_ERROR("%s", error);
		throw std::exception(error);
	}

	// Submit Input
	amf::AMFSurfacePtr pSurface = CreateSurfaceFromFrame(frame);
	{/// Queue Frame 
		std::unique_lock<std::mutex> lock(m_ThreadedInput.mutex);
		if (m_ThreadedInput.queue.size() < (size_t)ceil(m_FrameRateDivisor * 2)) {
			m_ThreadedInput.queue.push(pSurface);
			/// Signal Thread Wakeup
			m_ThreadedInput.condvar.notify_all();
		} else {
			AMF_LOG_ERROR("<Plugin::AMD::H264VideoEncoder::SendInput> Input Queue is full, aborting...");
			return false;
		}

		#ifdef DEBUG
		if (m_ThreadedInput.queue.size() % 5 == 4)
			AMF_LOG_WARNING("<Plugin::AMD::H264VideoEncoder::InputThreadLogic> Input Queue is filling up. (%d of %d)", m_ThreadedInput.queue.size(), (size_t)(m_FrameRateDivisor));
		#endif
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

	// Queue: Check if a Packet is available.
	ThreadData pkt;
	{
		std::unique_lock<std::mutex> lock(m_ThreadedOutput.queuemutex);
		if (m_ThreadedOutput.queue.size() == 0) {
			*received_packet = false;
			return;
		}

		pkt = m_ThreadedOutput.queue.front();
	}

	// Submit Packet to OBS
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
	*received_packet = true;

	// Queue: Remove submitted element.
	{
		std::unique_lock<std::mutex> lock(m_ThreadedOutput.queuemutex);
		m_ThreadedOutput.queue.pop();
	}

	// Debug: Packet Information
	#ifdef DEBUG
	AMF_LOG_INFO("Packet: Priority(%d), DropPriority(%d), PTS(%d), Size(%d)", packet->priority, packet->drop_priority, packet->pts, packet->size);
	#endif
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

		//buf->Release();

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

	uint32_t usage;
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

	uint32_t profile;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PROFILE, &profile);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetProfile> Retrieving Property failed with error %s (code %d).", res);
	}
	switch ((AMF_VIDEO_ENCODER_PROFILE_ENUM)profile) {
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

	return H264Profile_Unknown;
}

void Plugin::AMD::H264VideoEncoder::SetProfileLevel(H264ProfileLevel level) {
	static uint32_t customToAMF[] = {
		10, 11, 12, 13,
		20, 21, 22,
		30, 31, 32,
		40, 41, 42,
		50, 51, 52,
	};

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, customToAMF[level]);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetProfile> Setting to %d failed with error %s (code %d).", customToAMF[level], res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetProfile> Set to %d.", customToAMF[level]);
}

Plugin::AMD::H264ProfileLevel Plugin::AMD::H264VideoEncoder::GetProfileLevel() {
	uint32_t profileLevel;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PROFILE_LEVEL, &profileLevel);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetProfileLevel> Retrieving Property failed with error %s (code %d).", profileLevel, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetProfileLevel> Retrieved Property, Value is %d.", profileLevel);
	switch (profileLevel) {
		case 10: case 11: case 12: case 13:
			return (H264ProfileLevel)(H264ProfileLevel_10 + (profileLevel - 10));
			break;
		case 20: case 21: case 22:
			return (H264ProfileLevel)(H264ProfileLevel_20 + (profileLevel - 20));
			break;
		case 30: case 31: case 32:
			return (H264ProfileLevel)(H264ProfileLevel_30 + (profileLevel - 30));
			break;
		case 40: case 41: case 42:
			return (H264ProfileLevel)(H264ProfileLevel_40 + (profileLevel - 40));
			break;
		case 50: case 51: case 52:
			return (H264ProfileLevel)(H264ProfileLevel_50 + (profileLevel - 50));
			break;
	}
	return H264ProfileLevel_Unknown;
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
	uint32_t maximumLTRFrames;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, &maximumLTRFrames);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetMaxLTRFrames> Retrieving Property failed with error %s (code %d).", maximumLTRFrames, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetMaxLTRFrames> Retrieved Property, Value is %d.", maximumLTRFrames);
	return maximumLTRFrames;
}

void Plugin::AMD::H264VideoEncoder::SetFrameSize(uint32_t width, uint32_t height) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, ::AMFConstructSize(width, height));
	if (res != AMF_OK) {
		std::vector<char> msgBuf;
		sprintf(msgBuf.data(), "%dx%d", width, height);
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetFrameSize> Setting to %s failed with error %s (code %d).", msgBuf.data(), res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetFrameSize> Set to %dx%d.", width, height);
}

std::pair<uint32_t, uint32_t> Plugin::AMD::H264VideoEncoder::GetFrameSize() {
	AMFSize frameSize;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, &frameSize);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetFrameSize> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetFrameSize> Retrieved Property, Value is %dx%d.", frameSize.width, frameSize.height);
	m_FrameSize.first = frameSize.width;
	m_FrameSize.second = frameSize.height;
	return std::pair<uint32_t, uint32_t>(m_FrameSize);
}

void Plugin::AMD::H264VideoEncoder::SetTargetBitrate(uint32_t bitrate) {
	// Clamp Value
	bitrate = min(max(bitrate, 10000), Plugin::AMD::H264Capabilities::getInstance()->getEncoderCaps(m_EncoderType)->maxBitrate);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, bitrate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetTargetBitrate> Setting to %d bits failed with error %s (code %d).", bitrate, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetTargetBitrate> Set to %d bits.", bitrate);
}

uint32_t Plugin::AMD::H264VideoEncoder::GetTargetBitrate() {
	uint32_t bitrate;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_TARGET_BITRATE, &bitrate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetTargetBitrate> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetTargetBitrate> Retrieved Property, Value is %d bits.", bitrate);
	return bitrate;
}

void Plugin::AMD::H264VideoEncoder::SetPeakBitrate(uint32_t bitrate) {
	// Clamp Value
	bitrate = min(max(bitrate, 10000), Plugin::AMD::H264Capabilities::getInstance()->getEncoderCaps(m_EncoderType)->maxBitrate);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, bitrate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetPeakBitrate> Setting to %d bits failed with error %s (code %d).", bitrate, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetPeakBitrate> Set to %d bits.", bitrate);
}

uint32_t Plugin::AMD::H264VideoEncoder::GetPeakBitrate() {
	uint32_t bitrate;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_PEAK_BITRATE, &bitrate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetPeakBitrate> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetPeakBitrate> Retrieved Property, Value is %d bits.", bitrate);
	return bitrate;
}

void Plugin::AMD::H264VideoEncoder::SetRateControlMethod(H264RateControlMethod method) {
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

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, CustomToAMF[method]);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetRateControlMethod> Setting to %s failed with error %s (code %d).", CustomToName[method], res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetRateControlMethod> Set to %s.", CustomToName[method]);
}

Plugin::AMD::H264RateControlMethod Plugin::AMD::H264VideoEncoder::GetRateControlMethod() {
	static H264RateControlMethod AMFToCustom[] = {
		H264RateControlMethod_CQP,
		H264RateControlMethod_CBR,
		H264RateControlMethod_VBR,
		H264RateControlMethod_VBR_LAT,
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
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetRateControlMethod> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetRateControlMethod> Retrieved Property, Value is %s.", CustomToName[AMFToCustom[method]]);
	return AMFToCustom[method];
}

void Plugin::AMD::H264VideoEncoder::SetRateControlSkipFrameEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetRateControlSkipFrameEnabled> Setting to %s failed with error %s (code %d).", enabled ? "Enabled" : "Disabled", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetRateControlSkipFrameEnabled> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::H264VideoEncoder::IsRateControlSkipFrameEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::IsRateControlSkipFrameEnabled> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::IsRateControlSkipFrameEnabled> Retrieved Property, Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::H264VideoEncoder::SetMinimumQP(uint8_t qp) {
	// Clamp Value
	qp = max(min(qp, 51), 0); // 0-51? That must be a documentation error...

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MIN_QP, qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetMinimumQP> Setting to %d failed with error %s (code %d).", qp, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetMinimumQP> Set to %d.", qp);
}

uint8_t Plugin::AMD::H264VideoEncoder::GetMinimumQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MIN_QP, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetMinimumQP> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetMinimumQP> Retrieved Property, Value is %d.", qp);
	return qp;
}

void Plugin::AMD::H264VideoEncoder::SetMaximumQP(uint8_t qp) {
	// Clamp Value
	qp = max(min(qp, 51), 0); // 0-51? That must be a documentation error...

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_QP, qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetMaximumQP> Setting to %d failed with error %s (code %d).", qp, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetMaximumQP> Set to %d.", qp);
}

uint8_t Plugin::AMD::H264VideoEncoder::GetMaximumQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_QP, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetMaximumQP> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetMaximumQP> Retrieved Property, Value is %d.", qp);
	return qp;
}

void Plugin::AMD::H264VideoEncoder::SetIFrameQP(uint8_t qp) {
	// Clamp Value
	qp = max(min(qp, 51), 0); // 0-51? That must be a documentation error...

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_I, qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetIFrameQP> Setting to %d failed with error %s (code %d).", qp, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetIFrameQP> Set to %d.", qp);
}

uint8_t Plugin::AMD::H264VideoEncoder::GetIFrameQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_I, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetIFrameQP> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetIFrameQP> Retrieved Property, Value is %d.", qp);
	return qp;
}

void Plugin::AMD::H264VideoEncoder::SetPFrameQP(uint8_t qp) {
	// Clamp Value
	qp = max(min(qp, 51), 0); // 0-51? That must be a documentation error...

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_P, qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetPFrameQP> Setting to %d failed with error %s (code %d).", qp, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetPFrameQP> Set to %d.", qp);
}

uint8_t Plugin::AMD::H264VideoEncoder::GetPFrameQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_P, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetPFrameQP> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetPFrameQP> Retrieved Property, Value is %d.", qp);
	return qp;
}

void Plugin::AMD::H264VideoEncoder::SetBFrameQP(uint8_t qp) {
	// Clamp Value
	qp = max(min(qp, 51), 0); // 0-51? That must be a documentation error...

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QP_B, qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetBFrameQP> Setting to %d failed with error %s (code %d).", qp, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetBFrameQP> Set to %d.", qp);
}

uint8_t Plugin::AMD::H264VideoEncoder::GetBFrameQP() {
	uint32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QP_B, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetBFrameQP> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetBFrameQP> Retrieved Property, Value is %d.", qp);
	return qp;
}

void Plugin::AMD::H264VideoEncoder::SetFrameRate(uint32_t num, uint32_t den) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FRAMERATE, ::AMFConstructRate(num, den));
	if (res != AMF_OK) {
		std::vector<char> msgBuf;
		sprintf(msgBuf.data(), "%d/%d", num, den);
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetFrameRate> Setting to %s failed with error %s (code %d).", msgBuf.data(), res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetFrameRate> Set to %d/%d.", num, den);
	m_FrameRate.first = num;
	m_FrameRate.second = den;
	m_FrameRateDivisor = (double_t)num / (double_t)den;
	m_InputQueueLimit = (uint32_t)ceil(m_FrameRateDivisor * 3);
}

std::pair<uint32_t, uint32_t> Plugin::AMD::H264VideoEncoder::GetFrameRate() {
	AMFRate frameRate;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FRAMERATE, &frameRate);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetFrameRate> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetFrameRate> Retrieved Property, Value is %d/%d.", frameRate.num, frameRate.den);
	m_FrameRate.first = frameRate.num;
	m_FrameRate.second = frameRate.den;
	m_FrameRateDivisor = (double_t)frameRate.num / (double_t)frameRate.den;
	m_InputQueueLimit = (uint32_t)ceil(m_FrameRateDivisor * 3);
	return std::pair<uint32_t, uint32_t>(m_FrameRate);
}

void Plugin::AMD::H264VideoEncoder::SetVBVBufferSize(uint32_t size) {
	// Clamp Value
	size = max(min(size, 100000000), 1000); // 1kbit to 100mbit.

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetVBVBufferSize> Setting to %d bits failed with error %s (code %d).", size, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetVBVBufferSize> Set to %d bits.", size);
}

uint32_t Plugin::AMD::H264VideoEncoder::GetVBVBufferSize() {
	uint32_t size;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, &size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetVBVBufferSize> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetVBVBufferSize> Retrieved Property, Value is %d.", size);
	return size;
}

void Plugin::AMD::H264VideoEncoder::SetInitialVBVBufferFullness(double_t fullness) {
	// Clamp Value
	fullness = max(min(fullness, 1), 0); // 0 to 100 %

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS, (uint32_t)(fullness * 64));
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetVBVBufferFullness> Setting to %f%% failed with error %s (code %d).", fullness * 100, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetVBVBufferFullness> Set to %f%%.", fullness * 100);
}

double_t Plugin::AMD::H264VideoEncoder::GetInitialVBVBufferFullness() {
	uint32_t vbvBufferFullness;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS, &vbvBufferFullness);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetVBVBufferSize> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetVBVBufferSize> Retrieved Property, Value is %d.", vbvBufferFullness);
	return ((double_t)vbvBufferFullness / 64.0);
}

void Plugin::AMD::H264VideoEncoder::SetEnforceHRDRestrictionsEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_ENFORCE_HRD, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetEnforceHRD> Setting to %s failed with error %s (code %d).", enabled ? "Enabled" : "Disabled", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetEnforceHRD> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::H264VideoEncoder::IsEnforceHRDRestrictionsEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_ENFORCE_HRD, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetEnforceHRD> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetEnforceHRD> Retrieved Property, Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::H264VideoEncoder::SetFillerDataEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetFillerDataEnabled> Setting to %s failed with error %s (code %d).", enabled ? "Enabled" : "Disabled", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetFillerDataEnabled> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::H264VideoEncoder::IsFillerDataEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::IsFillerDataEnabled> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::IsFillerDataEnabled> Retrieved Property, Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::H264VideoEncoder::SetMaximumAccessUnitSize(uint32_t size) {
	// Clamp Value
	size = max(min(size, 100000000), 0); // 1kbit to 100mbit.

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MAX_AU_SIZE, size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetMaxAUSize> Setting to %d bits failed with error %s (code %d).", size, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetMaxAUSize> Set to %d bits.", size);
}

uint32_t Plugin::AMD::H264VideoEncoder::GetMaximumAccessUnitSize() {
	uint32_t size;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MAX_AU_SIZE, &size);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetMaxAUSize> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetMaxAUSize> Retrieved Property, Value is %d.", size);
	return size;
}

void Plugin::AMD::H264VideoEncoder::SetBPictureDeltaQP(int8_t qp) {
	// Clamp Value
	qp = max(min(qp, 10), -10);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetBPictureDeltaQP> Setting to %d failed with error %s (code %d).", qp, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetBPictureDeltaQP> Set to %d.", qp);
}

int8_t Plugin::AMD::H264VideoEncoder::GetBPictureDeltaQP() {
	int32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetBPictureDeltaQP> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetBPictureDeltaQP> Retrieved Property, Value is %d.", qp);
	return qp;
}

void Plugin::AMD::H264VideoEncoder::SetReferenceBPictureDeltaQP(int8_t qp) {
	// Clamp Value
	qp = max(min(qp, 10), -10);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetReferenceBPictureDeltaQP> Setting to %d failed with error %s (code %d).", qp, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetReferenceBPictureDeltaQP> Set to %d.", qp);
}

int8_t Plugin::AMD::H264VideoEncoder::GetReferenceBPictureDeltaQP() {
	int32_t qp;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, &qp);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetReferenceBPictureDeltaQP> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetReferenceBPictureDeltaQP> Retrieved Property, Value is %d.", qp);
	return qp;
}

void Plugin::AMD::H264VideoEncoder::SetHeaderInsertionSpacing(uint32_t spacing) {
	// Clamp Value
	spacing = max(min(spacing, m_FrameRate.second * 1000), 0);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING, spacing);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetHeaderInsertionSpacing> Setting to %d failed with error %s (code %d).", spacing, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetHeaderInsertionSpacing> Set to %d.", spacing);
}

uint32_t Plugin::AMD::H264VideoEncoder::GetHeaderInsertionSpacing() {
	int32_t headerInsertionSpacing;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING, &headerInsertionSpacing);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetHeaderInsertionSpacing> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetHeaderInsertionSpacing> Retrieved Property, Value is %d.", headerInsertionSpacing);
	return headerInsertionSpacing;
}

void Plugin::AMD::H264VideoEncoder::SetIDRPeriod(uint32_t period) {
	// Clamp Value
	period = max(min(period, m_FrameRate.second * 1000), 0);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, period);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetIDRPeriod> Setting to %d failed with error %s (code %d).", period, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetIDRPeriod> Set to %d.", period);
}

uint32_t Plugin::AMD::H264VideoEncoder::GetIDRPeriod() {
	int32_t period;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_IDR_PERIOD, &period);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetIDRPeriod> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetIDRPeriod> Retrieved Property, Value is %d.", period);
	return period;
}

void Plugin::AMD::H264VideoEncoder::SetDeBlockingFilterEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetDeBlockingFilterEnabled> Setting to %s failed with error %s (code %d).", enabled ? "Enabled" : "Disabled", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetDeBlockingFilterEnabled> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::H264VideoEncoder::IsDeBlockingFilterEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::IsDeBlockingFilterEnabled> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::IsDeBlockingFilterEnabled> Retrieved Property, Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::H264VideoEncoder::SetIntraRefreshMBsNumberPerSlot(uint32_t mbs) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, mbs);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetIntraRefreshMBsNumberPerSlot> Setting to %d failed with error %s (code %d).", mbs, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetIntraRefreshMBsNumberPerSlot> Set to %d.", mbs);
}

uint32_t Plugin::AMD::H264VideoEncoder::GetIntraRefreshMBsNumberPerSlot() {
	int32_t mbs;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, &mbs);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetIntraRefreshMBsNumberPerSlot> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetIntraRefreshMBsNumberPerSlot> Retrieved Property, Value is %d.", mbs);
	return mbs;
}

void Plugin::AMD::H264VideoEncoder::SetSlicesPerFrame(uint32_t slices) {
	slices = max(slices, 1);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, slices);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetSlicesPerFrame> Setting to %d failed with error %s (code %d).", slices, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetSlicesPerFrame> Set to %d.", slices);
}

uint32_t Plugin::AMD::H264VideoEncoder::GetSlicesPerFrame() {
	uint32_t slices;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, &slices);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetSlicesPerFrame> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetSlicesPerFrame> Retrieved Property, Value is %d.", slices);
	return slices;
}

void Plugin::AMD::H264VideoEncoder::SetBPicturesPattern(H264BPicturesPattern pattern) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, (uint32_t)pattern);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetBPicturesPattern> Setting to %d failed with error %s (code %d).", pattern, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetBPicturesPattern> Set to %d.", pattern);
}

Plugin::AMD::H264BPicturesPattern Plugin::AMD::H264VideoEncoder::GetBPicturesPattern() {
	uint32_t pattern;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_PIC_PATTERN, &pattern);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetBPicturesPattern> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetBPicturesPattern> Retrieved Property, Value is %d.", pattern);
	return (Plugin::AMD::H264BPicturesPattern)pattern;
}

void Plugin::AMD::H264VideoEncoder::SetBReferenceEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetBReferenceEnabled> Setting to %s failed with error %s (code %d).", enabled ? "Enabled" : "Disabled", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetBReferenceEnabled> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::H264VideoEncoder::IsBReferenceEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::IsBReferenceEnabled> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::IsBReferenceEnabled> Retrieved Property, Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::H264VideoEncoder::SetScanType(H264ScanType scanType) {
	static AMF_VIDEO_ENCODER_SCANTYPE_ENUM CustomToAMF[] = {
		AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE,
		AMF_VIDEO_ENCODER_SCANTYPE_INTERLACED,
	};
	static char* CustomToName[] = {
		"Progressive",
		"Interlaced",
	};

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_SCANTYPE, CustomToAMF[scanType]);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetScanType> Setting to %s failed with error %s (code %d).", CustomToName[scanType], res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetScanType> Set to %s.", CustomToName[scanType]);
}

Plugin::AMD::H264ScanType Plugin::AMD::H264VideoEncoder::GetScanType() {
	static char* CustomToName[] = {
		"Progressive",
		"Interlaced",
	};

	uint32_t scanType;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_SCANTYPE, &scanType);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetScanType> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetScanType> Retrieved Property, Value is %s.", CustomToName[scanType]);
	return (Plugin::AMD::H264ScanType)scanType;
}

void Plugin::AMD::H264VideoEncoder::SetQualityPreset(H264QualityPreset preset) {
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

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, CustomToAMF[preset]);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetQualityPreset> Setting to %s failed with error %s (code %d).", CustomToName[preset], res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetQualityPreset> Set to %s.", CustomToName[preset]);
}

Plugin::AMD::H264QualityPreset Plugin::AMD::H264VideoEncoder::GetQualityPreset() {
	static H264QualityPreset AMFToCustom[] = {
		H264QualityPreset_Balanced,
		H264QualityPreset_Speed,
		H264QualityPreset_Quality,
	};
	static char* CustomToName[] = {
		"Speed",
		"Balanced",
		"Quality",
	};

	uint32_t preset;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_QUALITY_PRESET, &preset);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetQualityPreset> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetQualityPreset> Retrieved Property, Value is %s.", CustomToName[AMFToCustom[preset]]);
	return AMFToCustom[preset];
}

void Plugin::AMD::H264VideoEncoder::SetHalfPixelMotionEstimationEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetHalfPixelMotionEstimationEnabled> Setting to %s failed with error %s (code %d).", enabled ? "Enabled" : "Disabled", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetHalfPixelMotionEstimationEnabled> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::H264VideoEncoder::IsHalfPixelMotionEstimationEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::IsHalfPixelMotionEstimationEnabled> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::IsHalfPixelMotionEstimationEnabled> Retrieved Property, Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::H264VideoEncoder::SetQuarterPixelMotionEstimationEnabled(bool enabled) {
	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL, enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetQuarterPixelMotionEstimationEnabled> Setting to %s failed with error %s (code %d).", enabled ? "Enabled" : "Disabled", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetQuarterPixelMotionEstimationEnabled> Set to %s.", enabled ? "Enabled" : "Disabled");
}

bool Plugin::AMD::H264VideoEncoder::IsQuarterPixelMotionEstimationEnabled() {
	bool enabled;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL, &enabled);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::IsQuarterPixelMotionEstimationEnabled> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::IsQuarterPixelMotionEstimationEnabled> Retrieved Property, Value is %s.", enabled ? "Enabled" : "Disabled");
	return enabled;
}

void Plugin::AMD::H264VideoEncoder::SetNumberOfTemporalEnhancementLayers(uint32_t layers) {
	layers = min(layers, 2);

	AMF_RESULT res = m_AMFEncoder->SetProperty(AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS, layers);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::SetNumberOfTemporalEnhancementLayers> Setting to %d failed with error %s (code %d).", layers, res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::SetNumberOfTemporalEnhancementLayers> Set to %d.", layers);
}

uint32_t Plugin::AMD::H264VideoEncoder::GetNumberOfTemporalEnhancementLayers() {
	uint32_t layers;
	AMF_RESULT res = m_AMFEncoder->GetProperty(AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS, &layers);
	if (res != AMF_OK) {
		ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::GetNumberOfTemporalEnhancementLayers> Retrieving Property failed with error %s (code %d).", res);
	}
	AMF_LOG_INFO("<Plugin::AMD::H264VideoEncoder::GetNumberOfTemporalEnhancementLayers> Retrieved Property, Value is %d.", layers);
	return layers;
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

			AMF_SYNC_LOCK(res = m_AMFEncoder->SubmitInput(surface););
			if (res == AMF_OK) {
				m_ThreadedInput.queue.pop();
			} else if (res == AMF_INPUT_FULL) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			} else {
				std::vector<char> msgBuf(128);
				FormatTextWithAMFError(&msgBuf, "%s (code %d)", res);
				AMF_LOG_WARNING("<Plugin::AMD::H264VideoEncoder::InputThreadLogic> SubmitInput failed with error %s.", msgBuf.data());
			}
		}
	} while (m_IsStarted);
}

void Plugin::AMD::H264VideoEncoder::OutputThreadLogic() {
	// Thread Loop that handles Querying
	uint64_t lastFrameIndex = 0;

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
				pkt.frame = (uint64_t)(pData->GetPts() * m_FrameRateDivisor / 1e7); // Not sure what way around the accuracy is better.
				AMF_RESULT res2 = AMF_OK;
				AMF_SYNC_LOCK(res2 = pData->GetProperty(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, &variant););
				if (res2 == AMF_OK && variant.type == amf::AMF_VARIANT_INT64) {
					pkt.type = (AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM)variant.ToUInt64();
				}
				if ((lastFrameIndex >= pkt.frame) && (lastFrameIndex > 0))
					AMF_LOG_ERROR("<Plugin::AMD::H264VideoEncoder::OutputThreadLogic> Detected out of order packet. Frame Index is %d, expected %d.", pkt.frame, lastFrameIndex + 1);
				lastFrameIndex = pkt.frame;

				// Release Buffer and Data
				//pBuffer->Release(); // Automatically done?
				//pData->Release();

				// Queue
				{
					std::unique_lock<std::mutex> qlock(m_ThreadedOutput.queuemutex);
					m_ThreadedOutput.queue.push(pkt);
				}
			} else if (res != AMF_REPEAT) {
				std::vector<char> msgBuf(128);
				FormatTextWithAMFError(&msgBuf, "%s (code %d)", res);
				AMF_LOG_WARNING("<Plugin::AMD::H264VideoEncoder::OutputThreadLogic> QueryOutput failed with error %s.", msgBuf.data());

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
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

		AMF_SYNC_LOCK(res = m_AMFContext->AllocSurface(
			memoryTypeToAMF[m_MemoryType], surfaceFormatToAMF[m_InputSurfaceFormat],
			m_FrameSize.first, m_FrameSize.second,
			&pSurface););
		if (res != AMF_OK) // Unable to create Surface
			ThrowExceptionWithAMFError("<Plugin::AMD::H264VideoEncoder::CreateSurfaceFromFrame> Unable to create AMFSurface, error %s (code %d).", res);

		planeCount = pSurface->GetPlanesCount();

		#pragma loop(hint_parallel(2))
		for (uint8_t i = 0; i < planeCount; i++) {
			amf::AMFPlane* plane;
			void* plane_nat;
			int32_t height;
			size_t hpitch;

			AMF_SYNC_LOCK(plane = pSurface->GetPlaneAt(i););
			AMF_SYNC_LOCK(plane_nat = plane->GetNative(););
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
		AMF_SYNC_LOCK(pSurface->SetPts(amfPts););
		AMF_SYNC_LOCK(pSurface->SetProperty(L"Frame", frame->pts););
	}

	return pSurface;
}

