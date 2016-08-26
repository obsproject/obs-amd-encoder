AMD hides certain parameters from us, but i think some of them should be accessible. Might need some in depth IDA work first.

# Sorted Discoveries

## Encoder Types
* AMFVideoEncoderVCE_AVC
* AMFVideoEncoderVCE_SVC
* AMFVideoEncoderHW_AVC
* AMFVideoEncoderHW_HEVC (AMFEncoderHEVC Component)

## Parameters

### Capabilities
* MaxNumRefFrames
* EncoderMaxInstances
* MaxNumOfTemporalLayers

### Encoder
* Tier
* ProfileLevel
* MaxOfLTRFrames
* TargetBitrate
* PeakBitrate
* RateControlMethod
* QP_I
* QP_P
* VBVBufferSize
* InitialVBVBufferFullness
* MaxAUSize
* HeaderInsertionSpacing
* IDRPeriod
* DeBlockingFilter
* SlicesPerFrame
* BPicturesPattern
* BReferenceEnable
* ScanType
* HalfPixel
* QuarterPixel
* NumOfTemporalLayers
* **MinQP_I**
* **MaxQP_I**
* **MinQP_P**
* **MaxQP_P**
* **QPCBOFFSET**
* **QPCROFFSET**
* **GOPType**
* **GOPPerIDR**
* **GOPSize**
* **GOPSizeMin**
* **GOPSizeMax**
* **AspectRatio**
* **NominalRange**
* **IntraRefreshMode**
* **LowLatencyInternal**
* **CommonLowLatencyInternal**
* **EnableGOPAlignment**
* **SliceControlMode**
* **SliceControlSize**
* **CABACEnable**
* **UniqueInstance**
* **MultiInstanceMode**
* **MultiInstanceCurrentQueue**

### Per Submission
* EndOfSequence = AMF_VIDEO_ENCODER_END_OF_SEQUENCE
* EndOfStream = AMF_VIDEO_ENCODER_END_OF_STREAM
* ForcePictureType = AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE
* InsertAUD = AMF_VIDEO_ENCODER_INSERT_AUD
* InsertSPS = AMF_VIDEO_ENCODER_INSERT_SPS
* InsertPPS = AMF_VIDEO_ENCODER_INSERT_PPS
* MarkCurrentWithLTRIndex = AMF_VIDEO_ENCODER_MARK_CURRENT_WITH_LTR_INDEX
* ForceLTRReferenceBitfield = AMF_VIDEO_ENCODER_FORCE_LTR_REFERENCE_BITFIELD
* **IntraRefreshFrameNum**
* **TemporalLayerSelect** (Per Frame?)

### Per Output

### Unknown

## Values

### Constants found
* AHEVC_PARAMETER_NAME__TEMPORAL_LAYER_SELECT
* AHEVC_PARAMETER_NAME__NUM_TEMPORAL_LAYERS
* AHEVC_PARAMETER_NAME__MAX_NUM_TEMPORAL_LAYE
* AHEVC_PARAMETER_NAME__TIER
* and some more.

### Other Strings in the same list:
* GOP_ALIGNED
* IDR_ALIGNED
* Cabac
* Calv
* Undefined
* XVBA
* AHEVCEncode
* AHEHVCApplySpeedQualityPreset is bypassed, because it is not implemented

### Profile Levels
Libraries support as high as 6.2, which is most likely for HEVC. Full List:  
1.0 2.0 2.1 3.0 3.1 4.0 4.1 5.0 5.1 5.2 6.0 6.1 6.2

# Other Discoveries

## SubmitInput, QueryOutput
There are references to deque, which is something in std::queue. Or BufferQueue.

## Typos
* CheckRes**oul**tion
