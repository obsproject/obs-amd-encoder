For an unknown reason, there are hidden parameters in the Runtime that can be set.

## AVC Properties
### Static
* FrameSize
* FrameRate
* ExtraData (Read-Only)
* Usage
* Profile<br/>Two new profiles are in the runtime, Constrained Base and Constrained High.
* Profile Level
* MaxOfLTRFrames
* ScanType
* QualityPreset

### Dynamic
* RateControlMethod
* TargetBitrate
* PeakBitrate
* RateControlSkipFrameEnable
* MinQP
* MaxQP
* QPI
* QPP
* QPB
* VBVBufferSize
* VBVBufferFullness
* EnforceHRD
* MaxAUSize
* FillerDataEnable
* BPicturesDeltaQP
* ReferenceBPicturesDeltaQP
* HeaderInsertionSpacing
* IDRPeriod
* DeBlockingFilter
* IntraRefreshMBsNumberPerSlot
* SlicesPerFrame
* BPicturesPattern
* BReferenceEnable
* HalfPixel
* QuarterPixel

### Unknown / New

* **QualityEnhancementMode**<br/>So far always 0.
* **MaxNumRefFrames**<br/>Shouldn't this be a Capability instead? Could perhaps be used to control B-Pictures more.
* **MaxMBPerSec**<br/>Unknown value.
* **InstanceID**
* **EnableVBAQ**<br/>Unknown meaning.
* **RateControlPreanalysisEnable**<br/>Is this Two-Pass encoding?
* **GOPSize**<br/>Technically ignored, but still there in code.
* **AspectRatio**
* **NominalRange**
* **IntraRefreshNumOfStripes**<br/>New, was not here before.
* **SliceMode**<br/>New, was not here before.
* **MaxSliceSize**<br/>New, was not here before.
* **LowLatencyInternal**
* **CommonLowLatencyInternal**
* **SliceControlMode**
* **SliceControlSize**
* **CABACEnable**<br/>Not yet publicly available, but 16.10.3 always uses it anyway.
* **UniqueInstance**
* **EncoderMaxInstances**
* **MultiInstanceMode**
* **MultiInstanceCurrentQueue**
* **WaitForTask**


