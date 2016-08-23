AMD hides certain parameters from us, but i think some of them should be accessible. Might need some in depth IDA work first.

## The following Encoder Types have been discovered:
* AMFVideoEncoderVCE_AVC
* AMFVideoEncoderVCE_SVC
* AMFVideoEncoderHW_AVC
* AMFVideoEncoderHW_HEVC(AMFEncoderHEVC)

## The following Properties have been discovered:
* **Tier**
* ProfileLevel
* MaxOfLTRFrames
* **MaxNumRefFrames**
* TargetBitrate
* PeakBitrate
* RateControlMethod
* **MinQP_I**
* **MaxQP_I**
* QP_I
* **MinQP_P**
* **MaxQP_P**
* QP_P
* **QPCBOFFSET**
* **QPCROFFSET**
* **GOPType**
* **GOPPerIDR**
* **GOPSize**
* **GOPSizeMin**
* **GOPSizeMax**
* **AspectRatio**
* **NominalRange**
* VBVBufferSize
* InitialVBVBufferFullness
* MaxAUSize
* HeaderInsertionSpacing
* IDRPeriod
* DeBlockingFilter
* **IntraRefreshFrameNum**
* **IntraRefreshMode**
* SlicesPerFrame
* BPicturesPattern
* BReferenceEnable
* **LowLatencyInternal**
* **CommonLowLatencyInternal**
* **EnableGOPAlignment**
* **SliceControlMode**
* **SliceControlSize**
* **CABACEnable**
* ScanType
* HalfPixel
* QuarterPixel
* **UniqueInstance**
* EncoderMaxInstances
* **MultiInstanceMode**
* **MultiInstanceCurrentQueue**
* MaxNumOfTemporalLayers
* NumOfTemporalLayers
* **TemporalLayerSelect**
* InsertSPS
* InsertPPS
* ForcePictureType
* InsertAUD
* EndOfSequence
* EndOfStream
* MarkCurrentWithLTRIndex
* ForceLTRReferenceBitfield

## Constants found
* AHEVC_PARAMETER_NAME__TEMPORAL_LAYER_SELECT
* AHEVC_PARAMETER_NAME__NUM_TEMPORAL_LAYERS
* AHEVC_PARAMETER_NAME__MAX_NUM_TEMPORAL_LAYE
* AHEVC_PARAMETER_NAME__TIER
* and some more.

## Other Strings in the same list:
* GOP_ALIGNED
* IDR_ALIGNED
* Cabac
* Calv
* Undefined
* XVBA
* AHEVCEncode
* AHEHVCApplySpeedQualityPreset is bypassed, because it is not implemented

# More Discoveries

## Profile Level
Libraries support as high as 6.2, which is most likely for HEVC. Full List:  
1.0 2.0 2.1 3.0 3.1 4.0 4.1 5.0 5.1 5.2 6.0 6.1 6.2

## SubmitInput, QueryOutput
There are references to deque, which is something in std::queue. Or BufferQueue.

## Typos
* CheckRes**oul**tion
