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
#include <stdint.h>
#include <inttypes.h>
#include <vector>

// OBS
#include "OBS-Studio/libobs/obs-module.h"
#include "OBS-Studio/libobs/obs-encoder.h"

// Plugin
#include "amf-vce.h"

//////////////////////////////////////////////////////////////////////////
// Defines
//////////////////////////////////////////////////////////////////////////
#define AMF_TEXT_H264(x) (AMF_TEXT("H264." ## x))
#define AMF_TEXT_H264_T(x) obs_module_text(AMF_TEXT_H264(x))

#define AMF_VCE_H264_NAME												AMF_TEXT_H264("Name")
#define AMF_VCE_H264_RESET												AMF_TEXT_H264("Reset")
#define AMF_VCE_H264_UPDATE												AMF_TEXT_H264("Update")
#define AMF_VCE_H264_TYPE												AMF_TEXT_H264("Type")
#define AMF_VCE_H264_TYPE_AVC											AMF_TEXT_H264("Type.AVC")
#define AMF_VCE_H264_TYPE_SVC											AMF_TEXT_H264("Type.SVC")
#define AMF_VCE_H264_TYPE_HEVC											AMF_TEXT_H264("Type.HEVC")
#define AMF_VCE_H264_USAGE												AMF_TEXT_H264("Usage")
#define AMF_VCE_H264_USAGE_TRANSCODING									AMF_TEXT_H264("Usage.Transcoding")
#define AMF_VCE_H264_USAGE_ULTRALOWLATENCY								AMF_TEXT_H264("Usage.UltraLowLatency")
#define AMF_VCE_H264_USAGE_LOWLATENCY									AMF_TEXT_H264("Usage.LowLatency")
#define AMF_VCE_H264_USAGE_WEBCAM										AMF_TEXT_H264("Usage.Webcam")
#define AMF_VCE_H264_QUALITY_PRESET										AMF_TEXT_H264("QualityPreset")
#define AMF_VCE_H264_QUALITY_PRESET_NONE								AMF_TEXT_H264("QualityPreset.None")
#define AMF_VCE_H264_QUALITY_PRESET_SPEED								AMF_TEXT_H264("QualityPreset.Speed")
#define AMF_VCE_H264_QUALITY_PRESET_BALANCED							AMF_TEXT_H264("QualityPreset.Balanced")
#define AMF_VCE_H264_QUALITY_PRESET_QUALITY								AMF_TEXT_H264("QualityPreset.Quality")
#define AMF_VCE_H264_PROFILE											AMF_TEXT_H264("Profile")
#define AMF_VCE_H264_PROFILE_DEFAULT									AMF_TEXT_H264("Profile.Default")
#define AMF_VCE_H264_PROFILE_BASELINE									AMF_TEXT_H264("Profile.Baseline")
#define AMF_VCE_H264_PROFILE_MAIN										AMF_TEXT_H264("Profile.Main")
#define AMF_VCE_H264_PROFILE_HIGH										AMF_TEXT_H264("Profile.High")
#define AMF_VCE_H264_PROFILE_LEVEL										AMF_TEXT_H264("ProfileLevel")
#define AMF_VCE_H264_PROFILE_LEVEL2(x)									AMF_TEXT_H264("ProfileLevel." ## x)
#define AMF_VCE_H264_MAX_LTR_FRAMES										AMF_TEXT_H264("MaxLTRFrames")
#define AMF_VCE_H264_SCAN_TYPE											AMF_TEXT_H264("ScanType")
#define AMF_VCE_H264_SCAN_TYPE_DEFAULT									AMF_TEXT_H264("ScanType.Default")
#define AMF_VCE_H264_SCAN_TYPE_PROGRESSIVE								AMF_TEXT_H264("ScanType.Progressive")
#define AMF_VCE_H264_SCAN_TYPE_INTERLACED								AMF_TEXT_H264("ScanType.Interlaced")
#define AMF_VCE_H264_RATECONTROL_METHOD									AMF_TEXT_H264("RateControl.Method")
#define AMF_VCE_H264_RATECONTROL_METHOD_DEFAULT							AMF_TEXT_H264("RateControl.Method.Default")
#define AMF_VCE_H264_RATECONTROL_METHOD_CQP								AMF_TEXT_H264("RateControl.Method.CQP")
#define AMF_VCE_H264_RATECONTROL_METHOD_CBR								AMF_TEXT_H264("RateControl.Method.CBR")
#define AMF_VCE_H264_RATECONTROL_METHOD_VBR_PEAK_CONSTRAINED			AMF_TEXT_H264("RateControl.Method.VBR.Peak")
#define AMF_VCE_H264_RATECONTROL_METHOD_VBR_LATENCY_CONSTRAINED			AMF_TEXT_H264("RateControl.Method.VBR.Latency")
#define AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING							AMF_TEXT_H264("RateControl.FrameSkipping")
#define AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING_DEFAULT					AMF_TEXT_H264("RateControl.FrameSkipping.Default")
#define AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING_DISABLED				AMF_TEXT_H264("RateControl.FrameSkipping.Disabled")
#define AMF_VCE_H264_RATECONTROL_FRAME_SKIPPING_ENABLED					AMF_TEXT_H264("RateControl.FrameSkipping.Enabled")
#define AMF_VCE_H264_FILLERDATA											AMF_TEXT_H264("FillerData")
#define AMF_VCE_H264_FILLERDATA_DEFAULT									AMF_TEXT_H264("FillerData.Default")
#define AMF_VCE_H264_FILLERDATA_DISABLED								AMF_TEXT_H264("FillerData.Disabled")
#define AMF_VCE_H264_FILLERDATA_ENABLED									AMF_TEXT_H264("FillerData.Enabled")
#define AMF_VCE_H264_ENFORCEHRD											AMF_TEXT_H264("EnforceHRD")
#define AMF_VCE_H264_ENFORCEHRD_DEFAULT									AMF_TEXT_H264("EnforceHRD.Default")
#define AMF_VCE_H264_ENFORCEHRD_DISABLED								AMF_TEXT_H264("EnforceHRD.Disabled")
#define AMF_VCE_H264_ENFORCEHRD_ENABLED									AMF_TEXT_H264("EnforceHRD.Enabled")
#define AMF_VCE_H264_GOP_SIZE											AMF_TEXT_H264("GOPSize")
#define AMF_VCE_H264_VBVBUFFER_SIZE										AMF_TEXT_H264("VBVBuffer.Size")
#define AMF_VCE_H264_VBVBUFFER_FULLNESS									AMF_TEXT_H264("VBVBuffer.Fullness")
#define AMF_VCE_H264_MAX_AU_SIZE										AMF_TEXT_H264("MaxAUSize")
#define AMF_VCE_H264_BPIC_DELTA_QP										AMF_TEXT_H264("BPicDeltaQP")
#define AMF_VCE_H264_REF_BPIC_DELTA_QP									AMF_TEXT_H264("RefBPicDeltaQP")
#define AMF_VCE_H264_QP_MINIMUM											AMF_TEXT_H264("QP.Minimum")
#define AMF_VCE_H264_QP_MAXIMUM											AMF_TEXT_H264("QP.Maximum")
#define AMF_VCE_H264_QP_IFRAME											AMF_TEXT_H264("QP.IFrame")
#define AMF_VCE_H264_QP_PFRAME											AMF_TEXT_H264("QP.PFrame")
#define AMF_VCE_H264_QP_BFRAME											AMF_TEXT_H264("QP.BFrame")
#define AMF_VCE_H264_BITRATE_TARGET										AMF_TEXT_H264("Bitrate.Target")
#define AMF_VCE_H264_BITRATE_PEAK										AMF_TEXT_H264("Bitrate.Peak")
#define AMF_VCE_H264_HEADER_INSERTION_SPACING							AMF_TEXT_H264("HeaderInsertionSpacing")
#define AMF_VCE_H264_NUMBER_OF_BPICTURES								AMF_TEXT_H264("NumberOfBPictures")
#define AMF_VCE_H264_DEBLOCKING_FILTER									AMF_TEXT_H264("DeblockingFilter")
#define AMF_VCE_H264_DEBLOCKING_FILTER_DEFAULT							AMF_TEXT_H264("DeblockingFilter.Default")
#define AMF_VCE_H264_DEBLOCKING_FILTER_DISABLED							AMF_TEXT_H264("DeblockingFilter.Disabled")
#define AMF_VCE_H264_DEBLOCKING_FILTER_ENABLED							AMF_TEXT_H264("DeblockingFilter.Enabled")
#define AMF_VCE_H264_BREFERENCE											AMF_TEXT_H264("BReference")
#define AMF_VCE_H264_BREFERENCE_DEFAULT									AMF_TEXT_H264("BReference.Default")
#define AMF_VCE_H264_BREFERENCE_DISABLED								AMF_TEXT_H264("BReference.Disabled")
#define AMF_VCE_H264_BREFERENCE_ENABLED									AMF_TEXT_H264("BReference.Enabled")
#define AMF_VCE_H264_IDR_PERIOD											AMF_TEXT_H264("IDRPeriod")
#define AMF_VCE_H264_INTRAREFRESHNUMMBPERSLOT							AMF_TEXT_H264("IntraRefreshNumMBPerSlot")
#define AMF_VCE_H264_SLICESPERFRAME										AMF_TEXT_H264("SlicesPerFrame")
#define AMF_VCE_H264_MOTIONESTIMATION									AMF_TEXT_H264("MotionEstimation")
#define AMF_VCE_H264_MOTIONESTIMATION_DEFAULT							AMF_TEXT_H264("MotionEstimation.Default")
#define AMF_VCE_H264_MOTIONESTIMATION_NONE								AMF_TEXT_H264("MotionEstimation.None")
#define AMF_VCE_H264_MOTIONESTIMATION_HALF								AMF_TEXT_H264("MotionEstimation.Half")
#define AMF_VCE_H264_MOTIONESTIMATION_QUARTER							AMF_TEXT_H264("MotionEstimation.Quarter")
#define AMF_VCE_H264_MOTIONESTIMATION_BOTH								AMF_TEXT_H264("MotionEstimation.Both")
#define AMF_VCE_H264_NUMBEROFTEMPORALENHANCEMENTLAYERS					AMF_TEXT_H264("NumberOfTemporalEnhancementLayers")

//#define AMF_VCE_H264_FRAMESIZE			AMF_TEXT_H264("FrameSize")
//#define AMF_VCE_H264_FRAMERATE			AMF_TEXT_H264("FrameRate")

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
namespace AMFEncoder {
	class VCE_H264_Encoder {
		public:

		// h264 Profiles
		enum PROFILES {
			PROFILE_Baseline,
			PROFILE_Main,
			PROFILE_High,
			PROFILE_COUNT_MAX
		};
		static const char* PROFILE_NAMES[PROFILES::PROFILE_COUNT_MAX];
		static const VCE_Profile PROFILE_VALUES[PROFILES::PROFILE_COUNT_MAX];

		// h264 Levels
		enum LEVELS {
			LEVEL_1, LEVEL_1_1, LEVEL_1_2, LEVEL_1_3,
			LEVEL_2, LEVEL_2_1, LEVEL_2_2,
			LEVEL_3, LEVEL_3_1, LEVEL_3_2,
			LEVEL_4, LEVEL_4_1, LEVEL_4_2,
			LEVEL_5, LEVEL_5_1, LEVEL_5_2,
			LEVEL_COUNT_MAX
		};
		static const char* LEVEL_NAMES[LEVELS::LEVEL_COUNT_MAX];
		static const VCE_Profile_Level LEVEL_VALUES[LEVELS::LEVEL_COUNT_MAX];

		//////////////////////////////////////////////////////////////////////////
		// Static Code
		//////////////////////////////////////////////////////////////////////////
		public:

		static obs_encoder_info* encoder_info;
		static void encoder_register();
		static const char* get_name(void* type_data);
		static void get_defaults(obs_data_t *settings);
		static obs_properties_t* get_properties(void* data);
		static bool reset_callback(obs_properties_t *props, obs_property_t *property, obs_data_t *settings);
		static bool update_from_amf(obs_properties_t *props, obs_property_t *property, obs_data_t *settings);

		static void* create(obs_data_t* settings, obs_encoder_t* encoder);
		static void destroy(void* data);
		static bool update(void *data, obs_data_t *settings);
		static bool encode(void *data, struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet);
		static void get_video_info(void *data, struct video_scale_info *info);
		static bool get_extra_data(void *data, uint8_t** extra_data, size_t* size);

		///**
		//* Gets the SEI data, if any
		//*
		//* @param       data      Data associated with this encoder context
		//* @param[out]  sei_data  Pointer to receive the SEI data
		//* @param[out]  size      Pointer to receive the SEI data size
		//* @return                true if SEI data available, false otherwise
		//*/
		//bool(*get_sei_data)(void *data, uint8_t **sei_data, size_t *size);
		
		//void *type_data;
		//void(*free_type_data)(void *type_data);
		
		//////////////////////////////////////////////////////////////////////////
		// Module Code
		//////////////////////////////////////////////////////////////////////////
		public:

		VCE_H264_Encoder(obs_data_t* settings, obs_encoder_t* encoder);
		~VCE_H264_Encoder();

		bool update(obs_data_t* settings);
		bool encode(struct encoder_frame * frame, struct encoder_packet * packet, bool * received_packet);
		void get_video_info(struct video_scale_info* info);
		bool get_extra_data(uint8_t** extra_data, size_t* size);

		bool update_properties(obs_data_t* settings);

		//////////////////////////////////////////////////////////////////////////
		// Storage
		//////////////////////////////////////////////////////////////////////////
		private:
		// Settings
		int m_cfgWidth, m_cfgHeight;
		int m_cfgFPSnum, m_cfgFPSden;

		// Encoder
		AMFEncoder::VCE* m_VCE;
	};
}