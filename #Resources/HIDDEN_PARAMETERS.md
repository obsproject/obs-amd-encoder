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

23:23:47.388: [AMF Encoder]  [CodecId] CodecId (Type: Int64, Index 0)
23:23:47.389: [AMF Encoder]   Content Type: 0
23:23:47.389: [AMF Encoder]   Access: RW
23:23:47.389: [AMF Encoder]   Values:
23:23:47.389: [AMF Encoder]     Current: 5
23:23:47.389: [AMF Encoder]     Default: 5
23:23:47.389: [AMF Encoder]     Minimum: Empty
23:23:47.389: [AMF Encoder]     Maximum: Empty
23:23:47.389: [AMF Encoder]   Enumeration: 
23:23:47.391: [AMF Encoder]     H264 (5)
23:23:47.391: [AMF Encoder]     UNKNOWN (0)
23:23:47.391: [AMF Encoder]     H264SVC (8)
23:23:47.391: [AMF Encoder]     UNKNOWN (0)
23:23:47.392: [AMF Encoder]  [EngineType] EngineType (Type: Int64, Index 1)
23:23:47.392: [AMF Encoder]   Content Type: 0
23:23:47.392: [AMF Encoder]   Access: RW
23:23:47.392: [AMF Encoder]   Values:
23:23:47.393: [AMF Encoder]     Current: 0
23:23:47.393: [AMF Encoder]     Default: 0
23:23:47.393: [AMF Encoder]     Minimum: Empty
23:23:47.393: [AMF Encoder]     Maximum: Empty
23:23:47.393: [AMF Encoder]   Enumeration: 
23:23:47.393: [AMF Encoder]     Auto (0)
23:23:47.394: [AMF Encoder]     DX9 (1)
23:23:47.394: [AMF Encoder]     DX11 (2)
23:23:47.395: [AMF Encoder]     XVBA (3)
23:23:47.395: [AMF Encoder]  [Usage] Usage (Type: Int64, Index 2)
23:23:47.395: [AMF Encoder]   Content Type: 0
23:23:47.395: [AMF Encoder]   Access: RW
23:23:47.395: [AMF Encoder]   Values:
23:23:47.395: [AMF Encoder]     Current: 0
23:23:47.396: [AMF Encoder]     Default: 0
23:23:47.396: [AMF Encoder]     Minimum: Empty
23:23:47.396: [AMF Encoder]     Maximum: Empty
23:23:47.397: [AMF Encoder]   Enumeration: 
23:23:47.397: [AMF Encoder]     Transcoding (0)
23:23:47.397: [AMF Encoder]     Ultra Low Latency (1)
23:23:47.397: [AMF Encoder]     Low Latency (2)
23:23:47.397: [AMF Encoder]     Webcam (3)
23:23:47.398: [AMF Encoder]  [FrameSize] Width (Type: Size, Index 3)
23:23:47.398: [AMF Encoder]   Content Type: 0
23:23:47.398: [AMF Encoder]   Access: RWX
23:23:47.398: [AMF Encoder]   Values:
23:23:47.398: [AMF Encoder]     Current: 1920x1080
23:23:47.398: [AMF Encoder]     Default: 1920x1080
23:23:47.398: [AMF Encoder]     Minimum: 64x64
23:23:47.398: [AMF Encoder]     Maximum: 4096x2160
23:23:47.399: [AMF Encoder]  [Profile] Profile (Type: Int64, Index 4)
23:23:47.399: [AMF Encoder]   Content Type: 0
23:23:47.399: [AMF Encoder]   Access: RWX
23:23:47.400: [AMF Encoder]   Values:
23:23:47.400: [AMF Encoder]     Current: 77
23:23:47.400: [AMF Encoder]     Default: 77
23:23:47.401: [AMF Encoder]     Minimum: Empty
23:23:47.401: [AMF Encoder]     Maximum: Empty
23:23:47.401: [AMF Encoder]   Enumeration: 
23:23:47.401: [AMF Encoder]     Contrained Base (256)
23:23:47.401: [AMF Encoder]     Baseline (66)
23:23:47.401: [AMF Encoder]     Main (77)
23:23:47.401: [AMF Encoder]     Contrained High (257)
23:23:47.402: [AMF Encoder]     High (100)
23:23:47.402: [AMF Encoder]  [ProfileLevel] Profile-Level (Type: Int64, Index 5)
23:23:47.402: [AMF Encoder]   Content Type: 0
23:23:47.402: [AMF Encoder]   Access: RW
23:23:47.402: [AMF Encoder]   Values:
23:23:47.402: [AMF Encoder]     Current: 42
23:23:47.402: [AMF Encoder]     Default: 42
23:23:47.402: [AMF Encoder]     Minimum: Empty
23:23:47.403: [AMF Encoder]     Maximum: Empty
23:23:47.403: [AMF Encoder]   Enumeration: 
23:23:47.403: [AMF Encoder]     1 (10)
23:23:47.403: [AMF Encoder]     1.1 (11)
23:23:47.403: [AMF Encoder]     1.2 (12)
23:23:47.403: [AMF Encoder]     1.3 (13)
23:23:47.404: [AMF Encoder]     2 (20)
23:23:47.404: [AMF Encoder]     2.1 (21)
23:23:47.404: [AMF Encoder]     2.2 (22)
23:23:47.404: [AMF Encoder]     3 (30)
23:23:47.404: [AMF Encoder]     3.1 (31)
23:23:47.404: [AMF Encoder]     3.2 (32)
23:23:47.404: [AMF Encoder]     4 (40)
23:23:47.404: [AMF Encoder]     4.1 (41)
23:23:47.405: [AMF Encoder]     4.2 (42)
23:23:47.405: [AMF Encoder]     5.0 (50)
23:23:47.405: [AMF Encoder]     5.1 (51)
23:23:47.405: [AMF Encoder]     5.2 (52)
23:23:47.405: [AMF Encoder]  [QualityEnhancementMode] Quality Enhancement Mode (Type: Int64, Index 6)
23:23:47.405: [AMF Encoder]   Content Type: 0
23:23:47.405: [AMF Encoder]   Access: 
23:23:47.405: [AMF Encoder]   Values:
23:23:47.406: [AMF Encoder]     Current: Empty
23:23:47.406: [AMF Encoder]     Default: 0
23:23:47.406: [AMF Encoder]     Minimum: Empty
23:23:47.406: [AMF Encoder]     Maximum: Empty
23:23:47.409: [AMF Encoder]   Enumeration: 
23:23:47.410: [AMF Encoder]     Disable (0)
23:23:47.410: [AMF Encoder]     CGS (1)
23:23:47.410: [AMF Encoder]     CGS Rewrite (2)
23:23:47.411: [AMF Encoder]     MGS (3)
23:23:47.411: [AMF Encoder]  [MaxOfLTRFrames] Max Of LTR (Type: Int64, Index 7)
23:23:47.411: [AMF Encoder]   Content Type: 0
23:23:47.412: [AMF Encoder]   Access: RWX
23:23:47.412: [AMF Encoder]   Values:
23:23:47.412: [AMF Encoder]     Current: 0
23:23:47.412: [AMF Encoder]     Default: 0
23:23:47.413: [AMF Encoder]     Minimum: 0
23:23:47.414: [AMF Encoder]     Maximum: 2
23:23:47.415: [AMF Encoder]  [MaxNumRefFrames] MaxNumRefFrames (Type: Int64, Index 8)
23:23:47.415: [AMF Encoder]   Content Type: 0
23:23:47.416: [AMF Encoder]   Access: RWX
23:23:47.416: [AMF Encoder]   Values:
23:23:47.417: [AMF Encoder]     Current: 4
23:23:47.418: [AMF Encoder]     Default: 4
23:23:47.418: [AMF Encoder]     Minimum: 1
23:23:47.419: [AMF Encoder]     Maximum: 16
23:23:47.419: [AMF Encoder]  [ConstraintSetFlags] ConstraintSetFlags (Type: Int64, Index 9)
23:23:47.419: [AMF Encoder]   Content Type: 0
23:23:47.419: [AMF Encoder]   Access: RW
23:23:47.420: [AMF Encoder]   Values:
23:23:47.420: [AMF Encoder]     Current: 0
23:23:47.420: [AMF Encoder]     Default: 0
23:23:47.420: [AMF Encoder]     Minimum: 0
23:23:47.420: [AMF Encoder]     Maximum: 255
23:23:47.420: [AMF Encoder]  [MaxMBPerSec] MaxMBPerSec (Type: Int64, Index 10)
23:23:47.421: [AMF Encoder]   Content Type: 0
23:23:47.421: [AMF Encoder]   Access: R
23:23:47.421: [AMF Encoder]   Values:
23:23:47.421: [AMF Encoder]     Current: 638704
23:23:47.421: [AMF Encoder]     Default: 638704
23:23:47.421: [AMF Encoder]     Minimum: 0
23:23:47.421: [AMF Encoder]     Maximum: 2147483647
23:23:47.421: [AMF Encoder]  [InstanceID] InstanceID (Type: Int64, Index 11)
23:23:47.421: [AMF Encoder]   Content Type: 0
23:23:47.422: [AMF Encoder]   Access: RW
23:23:47.422: [AMF Encoder]   Values:
23:23:47.422: [AMF Encoder]     Current: -1
23:23:47.422: [AMF Encoder]     Default: -1
23:23:47.422: [AMF Encoder]     Minimum: -1
23:23:47.422: [AMF Encoder]     Maximum: 15
23:23:47.422: [AMF Encoder]  [TargetBitrate] Bitrate (Type: Int64, Index 12)
23:23:47.422: [AMF Encoder]   Content Type: 0
23:23:47.423: [AMF Encoder]   Access: RWX
23:23:47.423: [AMF Encoder]   Values:
23:23:47.423: [AMF Encoder]     Current: 20000000
23:23:47.423: [AMF Encoder]     Default: 20000000
23:23:47.423: [AMF Encoder]     Minimum: 10000
23:23:47.423: [AMF Encoder]     Maximum: 100000000
23:23:47.423: [AMF Encoder]  [PeakBitrate] Peak Bitrate (Type: Int64, Index 13)
23:23:47.423: [AMF Encoder]   Content Type: 0
23:23:47.424: [AMF Encoder]   Access: RWX
23:23:47.424: [AMF Encoder]   Values:
23:23:47.424: [AMF Encoder]     Current: 20000000
23:23:47.424: [AMF Encoder]     Default: 20000000
23:23:47.424: [AMF Encoder]     Minimum: 10000
23:23:47.424: [AMF Encoder]     Maximum: 100000000
23:23:47.424: [AMF Encoder]  [RateControlMethod] Rate Control Method (Type: Int64, Index 14)
23:23:47.424: [AMF Encoder]   Content Type: 0
23:23:47.424: [AMF Encoder]   Access: RWX
23:23:47.425: [AMF Encoder]   Values:
23:23:47.425: [AMF Encoder]     Current: 2
23:23:47.425: [AMF Encoder]     Default: 2
23:23:47.425: [AMF Encoder]     Minimum: Empty
23:23:47.425: [AMF Encoder]     Maximum: Empty
23:23:47.425: [AMF Encoder]   Enumeration: 
23:23:47.425: [AMF Encoder]     Constant QP (0)
23:23:47.425: [AMF Encoder]     CBR (1)
23:23:47.425: [AMF Encoder]     Peak constrained VBR (2)
23:23:47.425: [AMF Encoder]     Latency constrained VBR (3)
23:23:47.425: [AMF Encoder]  [EanbleVBAQ] Eanble VBAQ (Type: Boolean, Index 15)
23:23:47.426: [AMF Encoder]   Content Type: 0
23:23:47.426: [AMF Encoder]   Access: RWX
23:23:47.426: [AMF Encoder]   Values:
23:23:47.426: [AMF Encoder]     Current: false
23:23:47.426: [AMF Encoder]     Default: false
23:23:47.426: [AMF Encoder]     Minimum: Empty
23:23:47.426: [AMF Encoder]     Maximum: Empty
23:23:47.426: [AMF Encoder]  [RateControlPreanalysisEnable] Rate Control Preanalysis Enable (Type: Boolean, Index 16)
23:23:47.427: [AMF Encoder]   Content Type: 0
23:23:47.427: [AMF Encoder]   Access: RWX
23:23:47.427: [AMF Encoder]   Values:
23:23:47.427: [AMF Encoder]     Current: false
23:23:47.427: [AMF Encoder]     Default: false
23:23:47.427: [AMF Encoder]     Minimum: Empty
23:23:47.427: [AMF Encoder]     Maximum: Empty
23:23:47.428: [AMF Encoder]  [RateControlSkipFrameEnable] Rate Control Based Frame Skip (Type: Boolean, Index 17)
23:23:47.428: [AMF Encoder]   Content Type: 0
23:23:47.428: [AMF Encoder]   Access: RWX
23:23:47.428: [AMF Encoder]   Values:
23:23:47.428: [AMF Encoder]     Current: false
23:23:47.429: [AMF Encoder]     Default: false
23:23:47.429: [AMF Encoder]     Minimum: Empty
23:23:47.429: [AMF Encoder]     Maximum: Empty
23:23:47.429: [AMF Encoder]  [MinQP] Min QP (Type: Int64, Index 18)
23:23:47.429: [AMF Encoder]   Content Type: 0
23:23:47.429: [AMF Encoder]   Access: RWX
23:23:47.430: [AMF Encoder]   Values:
23:23:47.430: [AMF Encoder]     Current: 18
23:23:47.430: [AMF Encoder]     Default: 18
23:23:47.430: [AMF Encoder]     Minimum: 0
23:23:47.430: [AMF Encoder]     Maximum: 51
23:23:47.430: [AMF Encoder]  [MaxQP] Max QP (Type: Int64, Index 19)
23:23:47.430: [AMF Encoder]   Content Type: 0
23:23:47.430: [AMF Encoder]   Access: RWX
23:23:47.430: [AMF Encoder]   Values:
23:23:47.431: [AMF Encoder]     Current: 51
23:23:47.431: [AMF Encoder]     Default: 51
23:23:47.431: [AMF Encoder]     Minimum: 0
23:23:47.431: [AMF Encoder]     Maximum: 51
23:23:47.431: [AMF Encoder]  [QPI] QP I (Type: Int64, Index 20)
23:23:47.431: [AMF Encoder]   Content Type: 0
23:23:47.431: [AMF Encoder]   Access: RWX
23:23:47.431: [AMF Encoder]   Values:
23:23:47.431: [AMF Encoder]     Current: 22
23:23:47.431: [AMF Encoder]     Default: 22
23:23:47.432: [AMF Encoder]     Minimum: 0
23:23:47.432: [AMF Encoder]     Maximum: 51
23:23:47.432: [AMF Encoder]  [QPP] QP P (Type: Int64, Index 21)
23:23:47.432: [AMF Encoder]   Content Type: 0
23:23:47.432: [AMF Encoder]   Access: RWX
23:23:47.432: [AMF Encoder]   Values:
23:23:47.432: [AMF Encoder]     Current: 22
23:23:47.432: [AMF Encoder]     Default: 22
23:23:47.432: [AMF Encoder]     Minimum: 0
23:23:47.433: [AMF Encoder]     Maximum: 51
23:23:47.433: [AMF Encoder]  [QPB] QP B (Type: Int64, Index 22)
23:23:47.433: [AMF Encoder]   Content Type: 0
23:23:47.433: [AMF Encoder]   Access: RWX
23:23:47.433: [AMF Encoder]   Values:
23:23:47.433: [AMF Encoder]     Current: 22
23:23:47.433: [AMF Encoder]     Default: 22
23:23:47.433: [AMF Encoder]     Minimum: 0
23:23:47.433: [AMF Encoder]     Maximum: 51
23:23:47.434: [AMF Encoder]  [GOPSize] GOP Size (Type: Int64, Index 23)
23:23:47.434: [AMF Encoder]   Content Type: 0
23:23:47.434: [AMF Encoder]   Access: RWX
23:23:47.434: [AMF Encoder]   Values:
23:23:47.434: [AMF Encoder]     Current: 0
23:23:47.434: [AMF Encoder]     Default: 0
23:23:47.434: [AMF Encoder]     Minimum: 0
23:23:47.434: [AMF Encoder]     Maximum: 2147483647
23:23:47.434: [AMF Encoder]  [FrameRate] Frame Rate (Type: Rate, Index 24)
23:23:47.435: [AMF Encoder]   Content Type: 0
23:23:47.435: [AMF Encoder]   Access: RWX
23:23:47.435: [AMF Encoder]   Values:
23:23:47.435: [AMF Encoder]     Current: 30/1
23:23:47.435: [AMF Encoder]     Default: 30/1
23:23:47.435: [AMF Encoder]     Minimum: Empty
23:23:47.435: [AMF Encoder]     Maximum: Empty
23:23:47.435: [AMF Encoder]  [AspectRatio] AspectRatio (Type: Rate, Index 25)
23:23:47.435: [AMF Encoder]   Content Type: 0
23:23:47.436: [AMF Encoder]   Access: RWX
23:23:47.436: [AMF Encoder]   Values:
23:23:47.436: [AMF Encoder]     Current: 1/1
23:23:47.436: [AMF Encoder]     Default: 1/1
23:23:47.436: [AMF Encoder]     Minimum: Empty
23:23:47.436: [AMF Encoder]     Maximum: Empty
23:23:47.436: [AMF Encoder]  [NominalRange] NominalRange (Type: Boolean, Index 26)
23:23:47.436: [AMF Encoder]   Content Type: 0
23:23:47.436: [AMF Encoder]   Access: RWX
23:23:47.436: [AMF Encoder]   Values:
23:23:47.437: [AMF Encoder]     Current: false
23:23:47.437: [AMF Encoder]     Default: false
23:23:47.437: [AMF Encoder]     Minimum: Empty
23:23:47.438: [AMF Encoder]     Maximum: Empty
23:23:47.438: [AMF Encoder]  [VBVBufferSize] VBV Buffer Size (Type: Int64, Index 27)
23:23:47.438: [AMF Encoder]   Content Type: 0
23:23:47.438: [AMF Encoder]   Access: RWX
23:23:47.438: [AMF Encoder]   Values:
23:23:47.438: [AMF Encoder]     Current: 20000000
23:23:47.438: [AMF Encoder]     Default: 20000000
23:23:47.439: [AMF Encoder]     Minimum: 1000
23:23:47.439: [AMF Encoder]     Maximum: 100000000
23:23:47.439: [AMF Encoder]  [InitialVBVBufferFullness] Initial VBV Buffer Fullness (Type: Int64, Index 28)
23:23:47.439: [AMF Encoder]   Content Type: 0
23:23:47.439: [AMF Encoder]   Access: RWX
23:23:47.439: [AMF Encoder]   Values:
23:23:47.440: [AMF Encoder]     Current: 64
23:23:47.440: [AMF Encoder]     Default: 64
23:23:47.440: [AMF Encoder]     Minimum: 0
23:23:47.440: [AMF Encoder]     Maximum: 64
23:23:47.440: [AMF Encoder]  [EnforceHRD] Enforce HRD (Type: Boolean, Index 29)
23:23:47.440: [AMF Encoder]   Content Type: 0
23:23:47.440: [AMF Encoder]   Access: RWX
23:23:47.440: [AMF Encoder]   Values:
23:23:47.441: [AMF Encoder]     Current: false
23:23:47.441: [AMF Encoder]     Default: false
23:23:47.441: [AMF Encoder]     Minimum: Empty
23:23:47.441: [AMF Encoder]     Maximum: Empty
23:23:47.441: [AMF Encoder]  [MaxAUSize] Max AU Size (Type: Int64, Index 30)
23:23:47.441: [AMF Encoder]   Content Type: 0
23:23:47.441: [AMF Encoder]   Access: RWX
23:23:47.441: [AMF Encoder]   Values:
23:23:47.441: [AMF Encoder]     Current: 0
23:23:47.441: [AMF Encoder]     Default: 0
23:23:47.442: [AMF Encoder]     Minimum: 0
23:23:47.442: [AMF Encoder]     Maximum: 100000000
23:23:47.442: [AMF Encoder]  [FillerDataEnable] Filler Data Enable (Type: Boolean, Index 31)
23:23:47.442: [AMF Encoder]   Content Type: 0
23:23:47.442: [AMF Encoder]   Access: RWX
23:23:47.442: [AMF Encoder]   Values:
23:23:47.442: [AMF Encoder]     Current: false
23:23:47.442: [AMF Encoder]     Default: false
23:23:47.442: [AMF Encoder]     Minimum: Empty
23:23:47.442: [AMF Encoder]     Maximum: Empty
23:23:47.443: [AMF Encoder]  [BPicturesDeltaQP] B-picture Delta (Type: Int64, Index 32)
23:23:47.443: [AMF Encoder]   Content Type: 0
23:23:47.443: [AMF Encoder]   Access: RWX
23:23:47.443: [AMF Encoder]   Values:
23:23:47.443: [AMF Encoder]     Current: 4
23:23:47.443: [AMF Encoder]     Default: 4
23:23:47.443: [AMF Encoder]     Minimum: -10
23:23:47.443: [AMF Encoder]     Maximum: 10
23:23:47.443: [AMF Encoder]  [ReferenceBPicturesDeltaQP] Reference B-picture Delta (Type: Int64, Index 33)
23:23:47.443: [AMF Encoder]   Content Type: 0
23:23:47.444: [AMF Encoder]   Access: RWX
23:23:47.444: [AMF Encoder]   Values:
23:23:47.444: [AMF Encoder]     Current: 2
23:23:47.444: [AMF Encoder]     Default: 2
23:23:47.444: [AMF Encoder]     Minimum: -10
23:23:47.444: [AMF Encoder]     Maximum: 10
23:23:47.444: [AMF Encoder]  [HeaderInsertionSpacing] Header Insertion Spacing (Type: Int64, Index 34)
23:23:47.444: [AMF Encoder]   Content Type: 0
23:23:47.445: [AMF Encoder]   Access: RWX
23:23:47.445: [AMF Encoder]   Values:
23:23:47.445: [AMF Encoder]     Current: 0
23:23:47.445: [AMF Encoder]     Default: 0
23:23:47.445: [AMF Encoder]     Minimum: 0
23:23:47.445: [AMF Encoder]     Maximum: 2147483647
23:23:47.445: [AMF Encoder]  [IDRPeriod] IDR Period (Type: Int64, Index 35)
23:23:47.445: [AMF Encoder]   Content Type: 0
23:23:47.446: [AMF Encoder]   Access: RWX
23:23:47.446: [AMF Encoder]   Values:
23:23:47.446: [AMF Encoder]     Current: 30
23:23:47.446: [AMF Encoder]     Default: 30
23:23:47.446: [AMF Encoder]     Minimum: 0
23:23:47.446: [AMF Encoder]     Maximum: 2147483647
23:23:47.446: [AMF Encoder]  [DeBlockingFilter] De-blocking Filter (Type: Boolean, Index 36)
23:23:47.446: [AMF Encoder]   Content Type: 0
23:23:47.446: [AMF Encoder]   Access: RWX
23:23:47.447: [AMF Encoder]   Values:
23:23:47.447: [AMF Encoder]     Current: true
23:23:47.447: [AMF Encoder]     Default: true
23:23:47.447: [AMF Encoder]     Minimum: Empty
23:23:47.447: [AMF Encoder]     Maximum: Empty
23:23:47.447: [AMF Encoder]  [IntraRefreshMBsNumberPerSlot] Intra Refresh MBs Number Per Slot (Type: Int64, Index 37)
23:23:47.447: [AMF Encoder]   Content Type: 0
23:23:47.447: [AMF Encoder]   Access: RWX
23:23:47.447: [AMF Encoder]   Values:
23:23:47.448: [AMF Encoder]     Current: 0
23:23:47.448: [AMF Encoder]     Default: 0
23:23:47.448: [AMF Encoder]     Minimum: 0
23:23:47.448: [AMF Encoder]     Maximum: 8160
23:23:47.448: [AMF Encoder]  [IntraRefreshNumOfStripes] IntraRefreshNumOfStripes (Type: Int64, Index 38)
23:23:47.448: [AMF Encoder]   Content Type: 0
23:23:47.448: [AMF Encoder]   Access: RWX
23:23:47.448: [AMF Encoder]   Values:
23:23:47.448: [AMF Encoder]     Current: 0
23:23:47.448: [AMF Encoder]     Default: 0
23:23:47.448: [AMF Encoder]     Minimum: 0
23:23:47.449: [AMF Encoder]     Maximum: 2147483647
23:23:47.449: [AMF Encoder]  [SliceMode] Slice Mode (Type: Int64, Index 39)
23:23:47.449: [AMF Encoder]   Content Type: 0
23:23:47.449: [AMF Encoder]   Access: RWX
23:23:47.449: [AMF Encoder]   Values:
23:23:47.449: [AMF Encoder]     Current: 1
23:23:47.449: [AMF Encoder]     Default: 1
23:23:47.450: [AMF Encoder]     Minimum: 1
23:23:47.450: [AMF Encoder]     Maximum: 2
23:23:47.450: [AMF Encoder]  [SlicesPerFrame] Slices Per Frame (Type: Int64, Index 40)
23:23:47.450: [AMF Encoder]   Content Type: 0
23:23:47.450: [AMF Encoder]   Access: RWX
23:23:47.450: [AMF Encoder]   Values:
23:23:47.450: [AMF Encoder]     Current: 1
23:23:47.450: [AMF Encoder]     Default: 1
23:23:47.451: [AMF Encoder]     Minimum: 1
23:23:47.451: [AMF Encoder]     Maximum: 8160
23:23:47.451: [AMF Encoder]  [MaxSliceSize] Max Slice Size (Type: Int64, Index 41)
23:23:47.451: [AMF Encoder]   Content Type: 0
23:23:47.451: [AMF Encoder]   Access: RWX
23:23:47.451: [AMF Encoder]   Values:
23:23:47.451: [AMF Encoder]     Current: 2147483647
23:23:47.451: [AMF Encoder]     Default: 2147483647
23:23:47.452: [AMF Encoder]     Minimum: 1
23:23:47.452: [AMF Encoder]     Maximum: 2147483647
23:23:47.452: [AMF Encoder]  [BPicturesPattern] B-picture Pattern (Type: Int64, Index 42)
23:23:47.452: [AMF Encoder]   Content Type: 0
23:23:47.452: [AMF Encoder]   Access: RWX
23:23:47.452: [AMF Encoder]   Values:
23:23:47.452: [AMF Encoder]     Current: 3
23:23:47.452: [AMF Encoder]     Default: 3
23:23:47.453: [AMF Encoder]     Minimum: 0
23:23:47.453: [AMF Encoder]     Maximum: 3
23:23:47.454: [AMF Encoder]  [BReferenceEnable] Enable B Refrence (Type: Boolean, Index 43)
23:23:47.454: [AMF Encoder]   Content Type: 0
23:23:47.454: [AMF Encoder]   Access: RWX
23:23:47.454: [AMF Encoder]   Values:
23:23:47.454: [AMF Encoder]     Current: true
23:23:47.454: [AMF Encoder]     Default: true
23:23:47.454: [AMF Encoder]     Minimum: Empty
23:23:47.454: [AMF Encoder]     Maximum: Empty
23:23:47.454: [AMF Encoder]  [LowLatencyInternal] LowLatencyInternal (Type: Boolean, Index 44)
23:23:47.454: [AMF Encoder]   Content Type: 0
23:23:47.454: [AMF Encoder]   Access: RWX
23:23:47.455: [AMF Encoder]   Values:
23:23:47.455: [AMF Encoder]     Current: false
23:23:47.455: [AMF Encoder]     Default: false
23:23:47.455: [AMF Encoder]     Minimum: Empty
23:23:47.455: [AMF Encoder]     Maximum: Empty
23:23:47.455: [AMF Encoder]  [CommonLowLatencyInternal] CommonLowLatencyInternal (Type: Boolean, Index 45)
23:23:47.455: [AMF Encoder]   Content Type: 0
23:23:47.455: [AMF Encoder]   Access: RWX
23:23:47.455: [AMF Encoder]   Values:
23:23:47.455: [AMF Encoder]     Current: false
23:23:47.456: [AMF Encoder]     Default: false
23:23:47.456: [AMF Encoder]     Minimum: Empty
23:23:47.456: [AMF Encoder]     Maximum: Empty
23:23:47.456: [AMF Encoder]  [EnableGOPAlignment] EnableGOPAlignment (Type: Boolean, Index 46)
23:23:47.456: [AMF Encoder]   Content Type: 0
23:23:47.456: [AMF Encoder]   Access: RWX
23:23:47.456: [AMF Encoder]   Values:
23:23:47.456: [AMF Encoder]     Current: true
23:23:47.457: [AMF Encoder]     Default: true
23:23:47.457: [AMF Encoder]     Minimum: Empty
23:23:47.457: [AMF Encoder]     Maximum: Empty
23:23:47.457: [AMF Encoder]  [SliceControlMode] SliceControlMode (Type: Int64, Index 47)
23:23:47.457: [AMF Encoder]   Content Type: 0
23:23:47.457: [AMF Encoder]   Access: RWX
23:23:47.457: [AMF Encoder]   Values:
23:23:47.457: [AMF Encoder]     Current: 0
23:23:47.457: [AMF Encoder]     Default: 0
23:23:47.457: [AMF Encoder]     Minimum: 0
23:23:47.457: [AMF Encoder]     Maximum: 3
23:23:47.458: [AMF Encoder]  [SliceControlSize] SliceControlSize (Type: Int64, Index 48)
23:23:47.458: [AMF Encoder]   Content Type: 0
23:23:47.458: [AMF Encoder]   Access: RWX
23:23:47.458: [AMF Encoder]   Values:
23:23:47.458: [AMF Encoder]     Current: 0
23:23:47.458: [AMF Encoder]     Default: 0
23:23:47.458: [AMF Encoder]     Minimum: 0
23:23:47.458: [AMF Encoder]     Maximum: 2147483647
23:23:47.458: [AMF Encoder]  [CABACEnable] CABACEnable (Type: Int64, Index 49)
23:23:47.458: [AMF Encoder]   Content Type: 0
23:23:47.459: [AMF Encoder]   Access: RWX
23:23:47.459: [AMF Encoder]   Values:
23:23:47.459: [AMF Encoder]     Current: 0
23:23:47.459: [AMF Encoder]     Default: 0
23:23:47.459: [AMF Encoder]     Minimum: Empty
23:23:47.459: [AMF Encoder]     Maximum: Empty
23:23:47.459: [AMF Encoder]   Enumeration: 
23:23:47.459: [AMF Encoder]     Undefined (0)
23:23:47.460: [AMF Encoder]     Cabac (1)
23:23:47.460: [AMF Encoder]     Calv (2)
23:23:47.460: [AMF Encoder]  [ScanType] Scan Type (Type: Int64, Index 50)
23:23:47.460: [AMF Encoder]   Content Type: 0
23:23:47.460: [AMF Encoder]   Access: RWX
23:23:47.460: [AMF Encoder]   Values:
23:23:47.460: [AMF Encoder]     Current: 0
23:23:47.460: [AMF Encoder]     Default: 0
23:23:47.460: [AMF Encoder]     Minimum: Empty
23:23:47.461: [AMF Encoder]     Maximum: Empty
23:23:47.461: [AMF Encoder]   Enumeration: 
23:23:47.461: [AMF Encoder]     Progressive (0)
23:23:47.461: [AMF Encoder]     Interlaced (1)
23:23:47.461: [AMF Encoder]  [QualityPreset] Quality Preset (Type: Int64, Index 51)
23:23:47.461: [AMF Encoder]   Content Type: 0
23:23:47.461: [AMF Encoder]   Access: RWX
23:23:47.461: [AMF Encoder]   Values:
23:23:47.461: [AMF Encoder]     Current: 0
23:23:47.461: [AMF Encoder]     Default: 0
23:23:47.461: [AMF Encoder]     Minimum: Empty
23:23:47.462: [AMF Encoder]     Maximum: Empty
23:23:47.462: [AMF Encoder]   Enumeration: 
23:23:47.462: [AMF Encoder]     Balanced (0)
23:23:47.462: [AMF Encoder]     Speed (1)
23:23:47.462: [AMF Encoder]     Quality (2)
23:23:47.462: [AMF Encoder]  [HalfPixel] Half Pixel (Type: Boolean, Index 52)
23:23:47.462: [AMF Encoder]   Content Type: 0
23:23:47.462: [AMF Encoder]   Access: RWX
23:23:47.462: [AMF Encoder]   Values:
23:23:47.462: [AMF Encoder]     Current: true
23:23:47.463: [AMF Encoder]     Default: true
23:23:47.463: [AMF Encoder]     Minimum: Empty
23:23:47.463: [AMF Encoder]     Maximum: Empty
23:23:47.463: [AMF Encoder]  [QuarterPixel] Quarter Pixel (Type: Boolean, Index 53)
23:23:47.463: [AMF Encoder]   Content Type: 0
23:23:47.463: [AMF Encoder]   Access: RWX
23:23:47.463: [AMF Encoder]   Values:
23:23:47.463: [AMF Encoder]     Current: true
23:23:47.463: [AMF Encoder]     Default: true
23:23:47.464: [AMF Encoder]     Minimum: Empty
23:23:47.464: [AMF Encoder]     Maximum: Empty
23:23:47.464: [AMF Encoder]  [ExtraData] ExtraData (Type: Interface, Index 54)
23:23:47.464: [AMF Encoder]   Content Type: 0
23:23:47.464: [AMF Encoder]   Access: RWX
23:23:47.464: [AMF Encoder]   Values:
23:23:47.465: [AMF Encoder] 
23:23:47.465: [AMF Encoder]     Default: Empty
23:23:47.465: [AMF Encoder]     Minimum: Empty
23:23:47.465: [AMF Encoder]     Maximum: ''
23:23:47.465: [AMF Encoder]  [UniqueInstance] Unique Instance (Type: Int64, Index 55)
23:23:47.466: [AMF Encoder]   Content Type: 0
23:23:47.466: [AMF Encoder]   Access: RWX
23:23:47.466: [AMF Encoder]   Values:
23:23:47.466: [AMF Encoder]     Current: 0
23:23:47.466: [AMF Encoder]     Default: 0
23:23:47.466: [AMF Encoder]     Minimum: 0
23:23:47.466: [AMF Encoder]     Maximum: 2147483647
23:23:47.466: [AMF Encoder]  [EncoderMaxInstances] EncoderMaxInstances (Type: Int64, Index 56)
23:23:47.466: [AMF Encoder]   Content Type: 0
23:23:47.466: [AMF Encoder]   Access: RWX
23:23:47.467: [AMF Encoder]   Values:
23:23:47.467: [AMF Encoder]     Current: 1
23:23:47.467: [AMF Encoder]     Default: 1
23:23:47.467: [AMF Encoder]     Minimum: 1
23:23:47.467: [AMF Encoder]     Maximum: 2
23:23:47.467: [AMF Encoder]  [MultiInstanceMode] MultiInstanceMode (Type: Boolean, Index 57)
23:23:47.467: [AMF Encoder]   Content Type: 0
23:23:47.467: [AMF Encoder]   Access: RWX
23:23:47.467: [AMF Encoder]   Values:
23:23:47.467: [AMF Encoder]     Current: false
23:23:47.467: [AMF Encoder]     Default: false
23:23:47.468: [AMF Encoder]     Minimum: Empty
23:23:47.468: [AMF Encoder]     Maximum: Empty
23:23:47.468: [AMF Encoder]  [MultiInstanceCurrentQueue] MultiInstanceCurrentQueue (Type: Int64, Index 58)
23:23:47.468: [AMF Encoder]   Content Type: 0
23:23:47.468: [AMF Encoder]   Access: RWX
23:23:47.468: [AMF Encoder]   Values:
23:23:47.468: [AMF Encoder]     Current: 0
23:23:47.468: [AMF Encoder]     Default: 0
23:23:47.468: [AMF Encoder]     Minimum: 0
23:23:47.468: [AMF Encoder]     Maximum: 1
23:23:47.468: [AMF Encoder]  [WaitForTask] WaitForTask (Type: Boolean, Index 59)
23:23:47.469: [AMF Encoder]   Content Type: 0
23:23:47.469: [AMF Encoder]   Access: RWX
23:23:47.469: [AMF Encoder]   Values:
23:23:47.469: [AMF Encoder]     Current: false
23:23:47.469: [AMF Encoder]     Default: false
23:23:47.469: [AMF Encoder]     Minimum: Empty
23:23:47.470: [AMF Encoder]     Maximum: Empty
23:23:47.470: [AMF Encoder]  [TL0.QL0.BPicturesDeltaQP] TL0.QL0 B-picture Delta (Type: Int64, Index 60)
23:23:47.470: [AMF Encoder]   Content Type: 0
23:23:47.470: [AMF Encoder]   Access: 
23:23:47.470: [AMF Encoder]   Values:
23:23:47.470: [AMF Encoder]     Current: Empty
23:23:47.470: [AMF Encoder]     Default: 4
23:23:47.470: [AMF Encoder]     Minimum: -10
23:23:47.471: [AMF Encoder]     Maximum: 10
23:23:47.471: [AMF Encoder]  [TL1.QL0.BPicturesDeltaQP] TL1.QL0 B-picture Delta (Type: Int64, Index 61)
23:23:47.471: [AMF Encoder]   Content Type: 0
23:23:47.471: [AMF Encoder]   Access: 
23:23:47.471: [AMF Encoder]   Values:
23:23:47.471: [AMF Encoder]     Current: Empty
23:23:47.471: [AMF Encoder]     Default: 4
23:23:47.471: [AMF Encoder]     Minimum: -10
23:23:47.471: [AMF Encoder]     Maximum: 10
23:23:47.472: [AMF Encoder]  [TL2.QL0.BPicturesDeltaQP] TL2.QL0 B-picture Delta (Type: Int64, Index 62)
23:23:47.472: [AMF Encoder]   Content Type: 0
23:23:47.472: [AMF Encoder]   Access: 
23:23:47.472: [AMF Encoder]   Values:
23:23:47.472: [AMF Encoder]     Current: Empty
23:23:47.472: [AMF Encoder]     Default: 4
23:23:47.473: [AMF Encoder]     Minimum: -10
23:23:47.473: [AMF Encoder]     Maximum: 10
23:23:47.473: [AMF Encoder]  [TL3.QL0.BPicturesDeltaQP] TL3.QL0 B-picture Delta (Type: Int64, Index 63)
23:23:47.474: [AMF Encoder]   Content Type: 0
23:23:47.474: [AMF Encoder]   Access: 
23:23:47.474: [AMF Encoder]   Values:
23:23:47.474: [AMF Encoder]     Current: Empty
23:23:47.474: [AMF Encoder]     Default: 4
23:23:47.475: [AMF Encoder]     Minimum: -10
23:23:47.475: [AMF Encoder]     Maximum: 10
23:23:47.475: [AMF Encoder]  [TL0.QL0.EanbleVBAQ] TL0.QL0 Eanble VBAQ (Type: Boolean, Index 64)
23:23:47.475: [AMF Encoder]   Content Type: 0
23:23:47.475: [AMF Encoder]   Access: 
23:23:47.475: [AMF Encoder]   Values:
23:23:47.475: [AMF Encoder]     Current: Empty
23:23:47.475: [AMF Encoder]     Default: false
23:23:47.475: [AMF Encoder]     Minimum: Empty
23:23:47.476: [AMF Encoder]     Maximum: Empty
23:23:47.476: [AMF Encoder]  [TL1.QL0.EanbleVBAQ] TL1.QL0 Eanble VBAQ (Type: Boolean, Index 65)
23:23:47.476: [AMF Encoder]   Content Type: 0
23:23:47.476: [AMF Encoder]   Access: 
23:23:47.476: [AMF Encoder]   Values:
23:23:47.476: [AMF Encoder]     Current: Empty
23:23:47.476: [AMF Encoder]     Default: false
23:23:47.476: [AMF Encoder]     Minimum: Empty
23:23:47.476: [AMF Encoder]     Maximum: Empty
23:23:47.477: [AMF Encoder]  [TL2.QL0.EanbleVBAQ] TL2.QL0 Eanble VBAQ (Type: Boolean, Index 66)
23:23:47.477: [AMF Encoder]   Content Type: 0
23:23:47.477: [AMF Encoder]   Access: 
23:23:47.477: [AMF Encoder]   Values:
23:23:47.477: [AMF Encoder]     Current: Empty
23:23:47.477: [AMF Encoder]     Default: false
23:23:47.477: [AMF Encoder]     Minimum: Empty
23:23:47.477: [AMF Encoder]     Maximum: Empty
23:23:47.477: [AMF Encoder]  [TL3.QL0.EanbleVBAQ] TL3.QL0 Eanble VBAQ (Type: Boolean, Index 67)
23:23:47.477: [AMF Encoder]   Content Type: 0
23:23:47.477: [AMF Encoder]   Access: 
23:23:47.477: [AMF Encoder]   Values:
23:23:47.477: [AMF Encoder]     Current: Empty
23:23:47.478: [AMF Encoder]     Default: false
23:23:47.478: [AMF Encoder]     Minimum: Empty
23:23:47.478: [AMF Encoder]     Maximum: Empty
23:23:47.478: [AMF Encoder]  [TL0.QL0.EnforceHRD] TL0.QL0 Enforce HRD (Type: Boolean, Index 68)
23:23:47.478: [AMF Encoder]   Content Type: 0
23:23:47.478: [AMF Encoder]   Access: 
23:23:47.478: [AMF Encoder]   Values:
23:23:47.478: [AMF Encoder]     Current: Empty
23:23:47.478: [AMF Encoder]     Default: true
23:23:47.478: [AMF Encoder]     Minimum: Empty
23:23:47.478: [AMF Encoder]     Maximum: Empty
23:23:47.478: [AMF Encoder]  [TL1.QL0.EnforceHRD] TL1.QL0 Enforce HRD (Type: Boolean, Index 69)
23:23:47.479: [AMF Encoder]   Content Type: 0
23:23:47.479: [AMF Encoder]   Access: 
23:23:47.479: [AMF Encoder]   Values:
23:23:47.479: [AMF Encoder]     Current: Empty
23:23:47.479: [AMF Encoder]     Default: true
23:23:47.479: [AMF Encoder]     Minimum: Empty
23:23:47.479: [AMF Encoder]     Maximum: Empty
23:23:47.479: [AMF Encoder]  [TL2.QL0.EnforceHRD] TL2.QL0 Enforce HRD (Type: Boolean, Index 70)
23:23:47.479: [AMF Encoder]   Content Type: 0
23:23:47.479: [AMF Encoder]   Access: 
23:23:47.479: [AMF Encoder]   Values:
23:23:47.479: [AMF Encoder]     Current: Empty
23:23:47.480: [AMF Encoder]     Default: true
23:23:47.480: [AMF Encoder]     Minimum: Empty
23:23:47.480: [AMF Encoder]     Maximum: Empty
23:23:47.480: [AMF Encoder]  [TL3.QL0.EnforceHRD] TL3.QL0 Enforce HRD (Type: Boolean, Index 71)
23:23:47.480: [AMF Encoder]   Content Type: 0
23:23:47.480: [AMF Encoder]   Access: 
23:23:47.480: [AMF Encoder]   Values:
23:23:47.480: [AMF Encoder]     Current: Empty
23:23:47.480: [AMF Encoder]     Default: true
23:23:47.480: [AMF Encoder]     Minimum: Empty
23:23:47.480: [AMF Encoder]     Maximum: Empty
23:23:47.481: [AMF Encoder]  [TL0.QL0.FillerDataEnable] TL0.QL0 Filler Data Enable (Type: Boolean, Index 72)
23:23:47.481: [AMF Encoder]   Content Type: 0
23:23:47.481: [AMF Encoder]   Access: 
23:23:47.481: [AMF Encoder]   Values:
23:23:47.481: [AMF Encoder]     Current: Empty
23:23:47.481: [AMF Encoder]     Default: false
23:23:47.481: [AMF Encoder]     Minimum: Empty
23:23:47.481: [AMF Encoder]     Maximum: Empty
23:23:47.481: [AMF Encoder]  [TL1.QL0.FillerDataEnable] TL1.QL0 Filler Data Enable (Type: Boolean, Index 73)
23:23:47.481: [AMF Encoder]   Content Type: 0
23:23:47.481: [AMF Encoder]   Access: 
23:23:47.481: [AMF Encoder]   Values:
23:23:47.482: [AMF Encoder]     Current: Empty
23:23:47.482: [AMF Encoder]     Default: false
23:23:47.482: [AMF Encoder]     Minimum: Empty
23:23:47.482: [AMF Encoder]     Maximum: Empty
23:23:47.482: [AMF Encoder]  [TL2.QL0.FillerDataEnable] TL2.QL0 Filler Data Enable (Type: Boolean, Index 74)
23:23:47.482: [AMF Encoder]   Content Type: 0
23:23:47.482: [AMF Encoder]   Access: 
23:23:47.482: [AMF Encoder]   Values:
23:23:47.482: [AMF Encoder]     Current: Empty
23:23:47.482: [AMF Encoder]     Default: false
23:23:47.482: [AMF Encoder]     Minimum: Empty
23:23:47.483: [AMF Encoder]     Maximum: Empty
23:23:47.483: [AMF Encoder]  [TL3.QL0.FillerDataEnable] TL3.QL0 Filler Data Enable (Type: Boolean, Index 75)
23:23:47.483: [AMF Encoder]   Content Type: 0
23:23:47.483: [AMF Encoder]   Access: 
23:23:47.483: [AMF Encoder]   Values:
23:23:47.483: [AMF Encoder]     Current: Empty
23:23:47.483: [AMF Encoder]     Default: false
23:23:47.483: [AMF Encoder]     Minimum: Empty
23:23:47.483: [AMF Encoder]     Maximum: Empty
23:23:47.483: [AMF Encoder]  [TL0.QL0.FrameRate] TL0.QL0 Frame Rate (Type: Rate, Index 76)
23:23:47.483: [AMF Encoder]   Content Type: 0
23:23:47.484: [AMF Encoder]   Access: 
23:23:47.484: [AMF Encoder]   Values:
23:23:47.484: [AMF Encoder]     Current: Empty
23:23:47.484: [AMF Encoder]     Default: 30/1
23:23:47.484: [AMF Encoder]     Minimum: Empty
23:23:47.484: [AMF Encoder]     Maximum: Empty
23:23:47.484: [AMF Encoder]  [TL1.QL0.FrameRate] TL1.QL0 Frame Rate (Type: Rate, Index 77)
23:23:47.485: [AMF Encoder]   Content Type: 0
23:23:47.485: [AMF Encoder]   Access: 
23:23:47.485: [AMF Encoder]   Values:
23:23:47.485: [AMF Encoder]     Current: Empty
23:23:47.485: [AMF Encoder]     Default: 30/1
23:23:47.485: [AMF Encoder]     Minimum: Empty
23:23:47.485: [AMF Encoder]     Maximum: Empty
23:23:47.485: [AMF Encoder]  [TL2.QL0.FrameRate] TL2.QL0 Frame Rate (Type: Rate, Index 78)
23:23:47.485: [AMF Encoder]   Content Type: 0
23:23:47.485: [AMF Encoder]   Access: 
23:23:47.485: [AMF Encoder]   Values:
23:23:47.485: [AMF Encoder]     Current: Empty
23:23:47.486: [AMF Encoder]     Default: 30/1
23:23:47.486: [AMF Encoder]     Minimum: Empty
23:23:47.486: [AMF Encoder]     Maximum: Empty
23:23:47.486: [AMF Encoder]  [TL3.QL0.FrameRate] TL3.QL0 Frame Rate (Type: Rate, Index 79)
23:23:47.486: [AMF Encoder]   Content Type: 0
23:23:47.486: [AMF Encoder]   Access: 
23:23:47.486: [AMF Encoder]   Values:
23:23:47.486: [AMF Encoder]     Current: Empty
23:23:47.486: [AMF Encoder]     Default: 30/1
23:23:47.486: [AMF Encoder]     Minimum: Empty
23:23:47.486: [AMF Encoder]     Maximum: Empty
23:23:47.486: [AMF Encoder]  [TL0.QL0.GOPSize] TL0.QL0 GOP Size (Type: Int64, Index 80)
23:23:47.487: [AMF Encoder]   Content Type: 0
23:23:47.487: [AMF Encoder]   Access: 
23:23:47.487: [AMF Encoder]   Values:
23:23:47.487: [AMF Encoder]     Current: Empty
23:23:47.487: [AMF Encoder]     Default: 0
23:23:47.487: [AMF Encoder]     Minimum: 0
23:23:47.487: [AMF Encoder]     Maximum: 2147483647
23:23:47.487: [AMF Encoder]  [TL1.QL0.GOPSize] TL1.QL0 GOP Size (Type: Int64, Index 81)
23:23:47.487: [AMF Encoder]   Content Type: 0
23:23:47.487: [AMF Encoder]   Access: 
23:23:47.487: [AMF Encoder]   Values:
23:23:47.487: [AMF Encoder]     Current: Empty
23:23:47.488: [AMF Encoder]     Default: 0
23:23:47.488: [AMF Encoder]     Minimum: 0
23:23:47.488: [AMF Encoder]     Maximum: 2147483647
23:23:47.488: [AMF Encoder]  [TL2.QL0.GOPSize] TL2.QL0 GOP Size (Type: Int64, Index 82)
23:23:47.488: [AMF Encoder]   Content Type: 0
23:23:47.488: [AMF Encoder]   Access: 
23:23:47.488: [AMF Encoder]   Values:
23:23:47.488: [AMF Encoder]     Current: Empty
23:23:47.488: [AMF Encoder]     Default: 0
23:23:47.488: [AMF Encoder]     Minimum: 0
23:23:47.488: [AMF Encoder]     Maximum: 2147483647
23:23:47.488: [AMF Encoder]  [TL3.QL0.GOPSize] TL3.QL0 GOP Size (Type: Int64, Index 83)
23:23:47.489: [AMF Encoder]   Content Type: 0
23:23:47.489: [AMF Encoder]   Access: 
23:23:47.489: [AMF Encoder]   Values:
23:23:47.489: [AMF Encoder]     Current: Empty
23:23:47.489: [AMF Encoder]     Default: 0
23:23:47.489: [AMF Encoder]     Minimum: 0
23:23:47.489: [AMF Encoder]     Maximum: 2147483647
23:23:47.489: [AMF Encoder]  [TL0.QL0.InitialVBVBufferFullness] TL0.QL0 Initial VBV Buffer Fullness (Type: Int64, Index 84)
23:23:47.489: [AMF Encoder]   Content Type: 0
23:23:47.489: [AMF Encoder]   Access: 
23:23:47.489: [AMF Encoder]   Values:
23:23:47.489: [AMF Encoder]     Current: Empty
23:23:47.489: [AMF Encoder]     Default: 64
23:23:47.490: [AMF Encoder]     Minimum: 0
23:23:47.490: [AMF Encoder]     Maximum: 64
23:23:47.490: [AMF Encoder]  [TL1.QL0.InitialVBVBufferFullness] TL1.QL0 Initial VBV Buffer Fullness (Type: Int64, Index 85)
23:23:47.490: [AMF Encoder]   Content Type: 0
23:23:47.490: [AMF Encoder]   Access: 
23:23:47.490: [AMF Encoder]   Values:
23:23:47.490: [AMF Encoder]     Current: Empty
23:23:47.490: [AMF Encoder]     Default: 64
23:23:47.490: [AMF Encoder]     Minimum: 0
23:23:47.490: [AMF Encoder]     Maximum: 64
23:23:47.490: [AMF Encoder]  [TL2.QL0.InitialVBVBufferFullness] TL2.QL0 Initial VBV Buffer Fullness (Type: Int64, Index 86)
23:23:47.490: [AMF Encoder]   Content Type: 0
23:23:47.491: [AMF Encoder]   Access: 
23:23:47.491: [AMF Encoder]   Values:
23:23:47.491: [AMF Encoder]     Current: Empty
23:23:47.491: [AMF Encoder]     Default: 64
23:23:47.491: [AMF Encoder]     Minimum: 0
23:23:47.491: [AMF Encoder]     Maximum: 64
23:23:47.491: [AMF Encoder]  [TL3.QL0.InitialVBVBufferFullness] TL3.QL0 Initial VBV Buffer Fullness (Type: Int64, Index 87)
23:23:47.491: [AMF Encoder]   Content Type: 0
23:23:47.491: [AMF Encoder]   Access: 
23:23:47.491: [AMF Encoder]   Values:
23:23:47.491: [AMF Encoder]     Current: Empty
23:23:47.491: [AMF Encoder]     Default: 64
23:23:47.492: [AMF Encoder]     Minimum: 0
23:23:47.492: [AMF Encoder]     Maximum: 64
23:23:47.492: [AMF Encoder]  [TL0.QL0.MaxAUSize] TL0.QL0 Max AU Size (Type: Int64, Index 88)
23:23:47.492: [AMF Encoder]   Content Type: 0
23:23:47.492: [AMF Encoder]   Access: 
23:23:47.492: [AMF Encoder]   Values:
23:23:47.492: [AMF Encoder]     Current: Empty
23:23:47.492: [AMF Encoder]     Default: 0
23:23:47.492: [AMF Encoder]     Minimum: 0
23:23:47.492: [AMF Encoder]     Maximum: 100000000
23:23:47.492: [AMF Encoder]  [TL1.QL0.MaxAUSize] TL1.QL0 Max AU Size (Type: Int64, Index 89)
23:23:47.492: [AMF Encoder]   Content Type: 0
23:23:47.493: [AMF Encoder]   Access: 
23:23:47.493: [AMF Encoder]   Values:
23:23:47.493: [AMF Encoder]     Current: Empty
23:23:47.493: [AMF Encoder]     Default: 0
23:23:47.493: [AMF Encoder]     Minimum: 0
23:23:47.493: [AMF Encoder]     Maximum: 100000000
23:23:47.493: [AMF Encoder]  [TL2.QL0.MaxAUSize] TL2.QL0 Max AU Size (Type: Int64, Index 90)
23:23:47.493: [AMF Encoder]   Content Type: 0
23:23:47.494: [AMF Encoder]   Access: 
23:23:47.494: [AMF Encoder]   Values:
23:23:47.494: [AMF Encoder]     Current: Empty
23:23:47.494: [AMF Encoder]     Default: 0
23:23:47.494: [AMF Encoder]     Minimum: 0
23:23:47.494: [AMF Encoder]     Maximum: 100000000
23:23:47.494: [AMF Encoder]  [TL3.QL0.MaxAUSize] TL3.QL0 Max AU Size (Type: Int64, Index 91)
23:23:47.494: [AMF Encoder]   Content Type: 0
23:23:47.494: [AMF Encoder]   Access: 
23:23:47.494: [AMF Encoder]   Values:
23:23:47.494: [AMF Encoder]     Current: Empty
23:23:47.495: [AMF Encoder]     Default: 0
23:23:47.495: [AMF Encoder]     Minimum: 0
23:23:47.495: [AMF Encoder]     Maximum: 100000000
23:23:47.495: [AMF Encoder]  [TL0.QL0.MaxQP] TL0.QL0 Max QP (Type: Int64, Index 92)
23:23:47.495: [AMF Encoder]   Content Type: 0
23:23:47.495: [AMF Encoder]   Access: 
23:23:47.495: [AMF Encoder]   Values:
23:23:47.495: [AMF Encoder]     Current: Empty
23:23:47.495: [AMF Encoder]     Default: 51
23:23:47.495: [AMF Encoder]     Minimum: 0
23:23:47.495: [AMF Encoder]     Maximum: 51
23:23:47.496: [AMF Encoder]  [TL1.QL0.MaxQP] TL1.QL0 Max QP (Type: Int64, Index 93)
23:23:47.496: [AMF Encoder]   Content Type: 0
23:23:47.496: [AMF Encoder]   Access: 
23:23:47.496: [AMF Encoder]   Values:
23:23:47.496: [AMF Encoder]     Current: Empty
23:23:47.496: [AMF Encoder]     Default: 51
23:23:47.496: [AMF Encoder]     Minimum: 0
23:23:47.496: [AMF Encoder]     Maximum: 51
23:23:47.496: [AMF Encoder]  [TL2.QL0.MaxQP] TL2.QL0 Max QP (Type: Int64, Index 94)
23:23:47.496: [AMF Encoder]   Content Type: 0
23:23:47.496: [AMF Encoder]   Access: 
23:23:47.496: [AMF Encoder]   Values:
23:23:47.496: [AMF Encoder]     Current: Empty
23:23:47.497: [AMF Encoder]     Default: 51
23:23:47.497: [AMF Encoder]     Minimum: 0
23:23:47.497: [AMF Encoder]     Maximum: 51
23:23:47.497: [AMF Encoder]  [TL3.QL0.MaxQP] TL3.QL0 Max QP (Type: Int64, Index 95)
23:23:47.497: [AMF Encoder]   Content Type: 0
23:23:47.497: [AMF Encoder]   Access: 
23:23:47.497: [AMF Encoder]   Values:
23:23:47.497: [AMF Encoder]     Current: Empty
23:23:47.497: [AMF Encoder]     Default: 51
23:23:47.497: [AMF Encoder]     Minimum: 0
23:23:47.497: [AMF Encoder]     Maximum: 51
23:23:47.497: [AMF Encoder]  [TL0.QL0.MinQP] TL0.QL0 Min QP (Type: Int64, Index 96)
23:23:47.497: [AMF Encoder]   Content Type: 0
23:23:47.498: [AMF Encoder]   Access: 
23:23:47.498: [AMF Encoder]   Values:
23:23:47.498: [AMF Encoder]     Current: Empty
23:23:47.498: [AMF Encoder]     Default: 22
23:23:47.498: [AMF Encoder]     Minimum: 0
23:23:47.498: [AMF Encoder]     Maximum: 51
23:23:47.498: [AMF Encoder]  [TL1.QL0.MinQP] TL1.QL0 Min QP (Type: Int64, Index 97)
23:23:47.498: [AMF Encoder]   Content Type: 0
23:23:47.498: [AMF Encoder]   Access: 
23:23:47.498: [AMF Encoder]   Values:
23:23:47.498: [AMF Encoder]     Current: Empty
23:23:47.498: [AMF Encoder]     Default: 22
23:23:47.499: [AMF Encoder]     Minimum: 0
23:23:47.499: [AMF Encoder]     Maximum: 51
23:23:47.499: [AMF Encoder]  [TL2.QL0.MinQP] TL2.QL0 Min QP (Type: Int64, Index 98)
23:23:47.499: [AMF Encoder]   Content Type: 0
23:23:47.499: [AMF Encoder]   Access: 
23:23:47.499: [AMF Encoder]   Values:
23:23:47.499: [AMF Encoder]     Current: Empty
23:23:47.499: [AMF Encoder]     Default: 22
23:23:47.499: [AMF Encoder]     Minimum: 0
23:23:47.499: [AMF Encoder]     Maximum: 51
23:23:47.499: [AMF Encoder]  [TL3.QL0.MinQP] TL3.QL0 Min QP (Type: Int64, Index 99)
23:23:47.500: [AMF Encoder]   Content Type: 0
23:23:47.500: [AMF Encoder]   Access: 
23:23:47.500: [AMF Encoder]   Values:
23:23:47.500: [AMF Encoder]     Current: Empty
23:23:47.500: [AMF Encoder]     Default: 22
23:23:47.500: [AMF Encoder]     Minimum: 0
23:23:47.500: [AMF Encoder]     Maximum: 51
23:23:47.500: [AMF Encoder]  [TL0.QL0.PeakBitrate] TL0.QL0 Peak Bitrate (Type: Int64, Index 100)
23:23:47.500: [AMF Encoder]   Content Type: 0
23:23:47.500: [AMF Encoder]   Access: 
23:23:47.501: [AMF Encoder]   Values:
23:23:47.501: [AMF Encoder]     Current: Empty
23:23:47.501: [AMF Encoder]     Default: 10000000
23:23:47.501: [AMF Encoder]     Minimum: 10000
23:23:47.501: [AMF Encoder]     Maximum: 100000000
23:23:47.501: [AMF Encoder]  [TL1.QL0.PeakBitrate] TL1.QL0 Peak Bitrate (Type: Int64, Index 101)
23:23:47.501: [AMF Encoder]   Content Type: 0
23:23:47.501: [AMF Encoder]   Access: 
23:23:47.501: [AMF Encoder]   Values:
23:23:47.501: [AMF Encoder]     Current: Empty
23:23:47.502: [AMF Encoder]     Default: 10000000
23:23:47.502: [AMF Encoder]     Minimum: 10000
23:23:47.502: [AMF Encoder]     Maximum: 100000000
23:23:47.502: [AMF Encoder]  [TL2.QL0.PeakBitrate] TL2.QL0 Peak Bitrate (Type: Int64, Index 102)
23:23:47.502: [AMF Encoder]   Content Type: 0
23:23:47.502: [AMF Encoder]   Access: 
23:23:47.502: [AMF Encoder]   Values:
23:23:47.502: [AMF Encoder]     Current: Empty
23:23:47.502: [AMF Encoder]     Default: 10000000
23:23:47.502: [AMF Encoder]     Minimum: 10000
23:23:47.502: [AMF Encoder]     Maximum: 100000000
23:23:47.502: [AMF Encoder]  [TL3.QL0.PeakBitrate] TL3.QL0 Peak Bitrate (Type: Int64, Index 103)
23:23:47.502: [AMF Encoder]   Content Type: 0
23:23:47.503: [AMF Encoder]   Access: 
23:23:47.503: [AMF Encoder]   Values:
23:23:47.503: [AMF Encoder]     Current: Empty
23:23:47.503: [AMF Encoder]     Default: 10000000
23:23:47.503: [AMF Encoder]     Minimum: 10000
23:23:47.503: [AMF Encoder]     Maximum: 100000000
23:23:47.503: [AMF Encoder]  [TL0.QL0.QPB] TL0.QL0 QP B (Type: Int64, Index 104)
23:23:47.503: [AMF Encoder]   Content Type: 0
23:23:47.503: [AMF Encoder]   Access: 
23:23:47.503: [AMF Encoder]   Values:
23:23:47.503: [AMF Encoder]     Current: Empty
23:23:47.503: [AMF Encoder]     Default: 22
23:23:47.504: [AMF Encoder]     Minimum: 0
23:23:47.504: [AMF Encoder]     Maximum: 51
23:23:47.504: [AMF Encoder]  [TL1.QL0.QPB] TL1.QL0 QP B (Type: Int64, Index 105)
23:23:47.504: [AMF Encoder]   Content Type: 0
23:23:47.504: [AMF Encoder]   Access: 
23:23:47.504: [AMF Encoder]   Values:
23:23:47.504: [AMF Encoder]     Current: Empty
23:23:47.504: [AMF Encoder]     Default: 22
23:23:47.504: [AMF Encoder]     Minimum: 0
23:23:47.504: [AMF Encoder]     Maximum: 51
23:23:47.504: [AMF Encoder]  [TL2.QL0.QPB] TL2.QL0 QP B (Type: Int64, Index 106)
23:23:47.504: [AMF Encoder]   Content Type: 0
23:23:47.504: [AMF Encoder]   Access: 
23:23:47.504: [AMF Encoder]   Values:
23:23:47.505: [AMF Encoder]     Current: Empty
23:23:47.505: [AMF Encoder]     Default: 22
23:23:47.505: [AMF Encoder]     Minimum: 0
23:23:47.505: [AMF Encoder]     Maximum: 51
23:23:47.505: [AMF Encoder]  [TL3.QL0.QPB] TL3.QL0 QP B (Type: Int64, Index 107)
23:23:47.505: [AMF Encoder]   Content Type: 0
23:23:47.505: [AMF Encoder]   Access: 
23:23:47.505: [AMF Encoder]   Values:
23:23:47.505: [AMF Encoder]     Current: Empty
23:23:47.505: [AMF Encoder]     Default: 22
23:23:47.505: [AMF Encoder]     Minimum: 0
23:23:47.505: [AMF Encoder]     Maximum: 51
23:23:47.505: [AMF Encoder]  [TL0.QL0.QPI] TL0.QL0 QP I (Type: Int64, Index 108)
23:23:47.506: [AMF Encoder]   Content Type: 0
23:23:47.506: [AMF Encoder]   Access: 
23:23:47.506: [AMF Encoder]   Values:
23:23:47.506: [AMF Encoder]     Current: Empty
23:23:47.506: [AMF Encoder]     Default: 22
23:23:47.506: [AMF Encoder]     Minimum: 0
23:23:47.506: [AMF Encoder]     Maximum: 51
23:23:47.506: [AMF Encoder]  [TL1.QL0.QPI] TL1.QL0 QP I (Type: Int64, Index 109)
23:23:47.506: [AMF Encoder]   Content Type: 0
23:23:47.506: [AMF Encoder]   Access: 
23:23:47.506: [AMF Encoder]   Values:
23:23:47.506: [AMF Encoder]     Current: Empty
23:23:47.506: [AMF Encoder]     Default: 22
23:23:47.507: [AMF Encoder]     Minimum: 0
23:23:47.507: [AMF Encoder]     Maximum: 51
23:23:47.507: [AMF Encoder]  [TL2.QL0.QPI] TL2.QL0 QP I (Type: Int64, Index 110)
23:23:47.507: [AMF Encoder]   Content Type: 0
23:23:47.507: [AMF Encoder]   Access: 
23:23:47.507: [AMF Encoder]   Values:
23:23:47.507: [AMF Encoder]     Current: Empty
23:23:47.507: [AMF Encoder]     Default: 22
23:23:47.507: [AMF Encoder]     Minimum: 0
23:23:47.507: [AMF Encoder]     Maximum: 51
23:23:47.507: [AMF Encoder]  [TL3.QL0.QPI] TL3.QL0 QP I (Type: Int64, Index 111)
23:23:47.507: [AMF Encoder]   Content Type: 0
23:23:47.507: [AMF Encoder]   Access: 
23:23:47.507: [AMF Encoder]   Values:
23:23:47.508: [AMF Encoder]     Current: Empty
23:23:47.508: [AMF Encoder]     Default: 22
23:23:47.508: [AMF Encoder]     Minimum: 0
23:23:47.508: [AMF Encoder]     Maximum: 51
23:23:47.508: [AMF Encoder]  [TL0.QL0.QPP] TL0.QL0 QP P (Type: Int64, Index 112)
23:23:47.508: [AMF Encoder]   Content Type: 0
23:23:47.508: [AMF Encoder]   Access: 
23:23:47.508: [AMF Encoder]   Values:
23:23:47.508: [AMF Encoder]     Current: Empty
23:23:47.508: [AMF Encoder]     Default: 22
23:23:47.508: [AMF Encoder]     Minimum: 0
23:23:47.508: [AMF Encoder]     Maximum: 51
23:23:47.508: [AMF Encoder]  [TL1.QL0.QPP] TL1.QL0 QP P (Type: Int64, Index 113)
23:23:47.509: [AMF Encoder]   Content Type: 0
23:23:47.509: [AMF Encoder]   Access: 
23:23:47.509: [AMF Encoder]   Values:
23:23:47.509: [AMF Encoder]     Current: Empty
23:23:47.509: [AMF Encoder]     Default: 22
23:23:47.509: [AMF Encoder]     Minimum: 0
23:23:47.509: [AMF Encoder]     Maximum: 51
23:23:47.509: [AMF Encoder]  [TL2.QL0.QPP] TL2.QL0 QP P (Type: Int64, Index 114)
23:23:47.509: [AMF Encoder]   Content Type: 0
23:23:47.509: [AMF Encoder]   Access: 
23:23:47.509: [AMF Encoder]   Values:
23:23:47.509: [AMF Encoder]     Current: Empty
23:23:47.509: [AMF Encoder]     Default: 22
23:23:47.509: [AMF Encoder]     Minimum: 0
23:23:47.509: [AMF Encoder]     Maximum: 51
23:23:47.510: [AMF Encoder]  [TL3.QL0.QPP] TL3.QL0 QP P (Type: Int64, Index 115)
23:23:47.510: [AMF Encoder]   Content Type: 0
23:23:47.510: [AMF Encoder]   Access: 
23:23:47.510: [AMF Encoder]   Values:
23:23:47.510: [AMF Encoder]     Current: Empty
23:23:47.510: [AMF Encoder]     Default: 22
23:23:47.510: [AMF Encoder]     Minimum: 0
23:23:47.510: [AMF Encoder]     Maximum: 51
23:23:47.510: [AMF Encoder]  [TL0.QL0.RateControlMethod] TL0.QL0 Rate Control Method (Type: Int64, Index 116)
23:23:47.510: [AMF Encoder]   Content Type: 0
23:23:47.510: [AMF Encoder]   Access: 
23:23:47.510: [AMF Encoder]   Values:
23:23:47.510: [AMF Encoder]     Current: Empty
23:23:47.510: [AMF Encoder]     Default: 2
23:23:47.511: [AMF Encoder]     Minimum: Empty
23:23:47.511: [AMF Encoder]     Maximum: Empty
23:23:47.511: [AMF Encoder]   Enumeration: 
23:23:47.511: [AMF Encoder]     Constant QP (0)
23:23:47.511: [AMF Encoder]     CBR (1)
23:23:47.511: [AMF Encoder]     Peak constrained VBR (2)
23:23:47.511: [AMF Encoder]     Latency constrained VBR (3)
23:23:47.511: [AMF Encoder]  [TL1.QL0.RateControlMethod] TL1.QL0 Rate Control Method (Type: Int64, Index 117)
23:23:47.511: [AMF Encoder]   Content Type: 0
23:23:47.511: [AMF Encoder]   Access: 
23:23:47.511: [AMF Encoder]   Values:
23:23:47.511: [AMF Encoder]     Current: Empty
23:23:47.511: [AMF Encoder]     Default: 2
23:23:47.511: [AMF Encoder]     Minimum: Empty
23:23:47.512: [AMF Encoder]     Maximum: Empty
23:23:47.512: [AMF Encoder]   Enumeration: 
23:23:47.512: [AMF Encoder]     Constant QP (0)
23:23:47.512: [AMF Encoder]     CBR (1)
23:23:47.512: [AMF Encoder]     Peak constrained VBR (2)
23:23:47.512: [AMF Encoder]     Latency constrained VBR (3)
23:23:47.512: [AMF Encoder]  [TL2.QL0.RateControlMethod] TL2.QL0 Rate Control Method (Type: Int64, Index 118)
23:23:47.512: [AMF Encoder]   Content Type: 0
23:23:47.512: [AMF Encoder]   Access: 
23:23:47.512: [AMF Encoder]   Values:
23:23:47.512: [AMF Encoder]     Current: Empty
23:23:47.512: [AMF Encoder]     Default: 2
23:23:47.512: [AMF Encoder]     Minimum: Empty
23:23:47.513: [AMF Encoder]     Maximum: Empty
23:23:47.513: [AMF Encoder]   Enumeration: 
23:23:47.513: [AMF Encoder]     Constant QP (0)
23:23:47.513: [AMF Encoder]     CBR (1)
23:23:47.513: [AMF Encoder]     Peak constrained VBR (2)
23:23:47.513: [AMF Encoder]     Latency constrained VBR (3)
23:23:47.513: [AMF Encoder]  [TL3.QL0.RateControlMethod] TL3.QL0 Rate Control Method (Type: Int64, Index 119)
23:23:47.513: [AMF Encoder]   Content Type: 0
23:23:47.513: [AMF Encoder]   Access: 
23:23:47.513: [AMF Encoder]   Values:
23:23:47.513: [AMF Encoder]     Current: Empty
23:23:47.513: [AMF Encoder]     Default: 2
23:23:47.513: [AMF Encoder]     Minimum: Empty
23:23:47.513: [AMF Encoder]     Maximum: Empty
23:23:47.514: [AMF Encoder]   Enumeration: 
23:23:47.514: [AMF Encoder]     Constant QP (0)
23:23:47.514: [AMF Encoder]     CBR (1)
23:23:47.514: [AMF Encoder]     Peak constrained VBR (2)
23:23:47.514: [AMF Encoder]     Latency constrained VBR (3)
23:23:47.514: [AMF Encoder]  [TL0.QL0.RateControlPreanalysisEnable] TL0.QL0 Rate Control Preanalysis Enable (Type: Boolean, Index 120)
23:23:47.514: [AMF Encoder]   Content Type: 0
23:23:47.514: [AMF Encoder]   Access: 
23:23:47.514: [AMF Encoder]   Values:
23:23:47.514: [AMF Encoder]     Current: Empty
23:23:47.514: [AMF Encoder]     Default: false
23:23:47.514: [AMF Encoder]     Minimum: Empty
23:23:47.514: [AMF Encoder]     Maximum: Empty
23:23:47.514: [AMF Encoder]  [TL1.QL0.RateControlPreanalysisEnable] TL1.QL0 Rate Control Preanalysis Enable (Type: Boolean, Index 121)
23:23:47.515: [AMF Encoder]   Content Type: 0
23:23:47.515: [AMF Encoder]   Access: 
23:23:47.515: [AMF Encoder]   Values:
23:23:47.515: [AMF Encoder]     Current: Empty
23:23:47.515: [AMF Encoder]     Default: false
23:23:47.515: [AMF Encoder]     Minimum: Empty
23:23:47.515: [AMF Encoder]     Maximum: Empty
23:23:47.515: [AMF Encoder]  [TL2.QL0.RateControlPreanalysisEnable] TL2.QL0 Rate Control Preanalysis Enable (Type: Boolean, Index 122)
23:23:47.515: [AMF Encoder]   Content Type: 0
23:23:47.515: [AMF Encoder]   Access: 
23:23:47.516: [AMF Encoder]   Values:
23:23:47.516: [AMF Encoder]     Current: Empty
23:23:47.516: [AMF Encoder]     Default: false
23:23:47.516: [AMF Encoder]     Minimum: Empty
23:23:47.516: [AMF Encoder]     Maximum: Empty
23:23:47.516: [AMF Encoder]  [TL3.QL0.RateControlPreanalysisEnable] TL3.QL0 Rate Control Preanalysis Enable (Type: Boolean, Index 123)
23:23:47.516: [AMF Encoder]   Content Type: 0
23:23:47.516: [AMF Encoder]   Access: 
23:23:47.516: [AMF Encoder]   Values:
23:23:47.517: [AMF Encoder]     Current: Empty
23:23:47.517: [AMF Encoder]     Default: false
23:23:47.517: [AMF Encoder]     Minimum: Empty
23:23:47.517: [AMF Encoder]     Maximum: Empty
23:23:47.517: [AMF Encoder]  [TL0.QL0.RateControlSkipFrameEnable] TL0.QL0 Rate Control Based Frame Skip (Type: Boolean, Index 124)
23:23:47.517: [AMF Encoder]   Content Type: 0
23:23:47.517: [AMF Encoder]   Access: 
23:23:47.517: [AMF Encoder]   Values:
23:23:47.517: [AMF Encoder]     Current: Empty
23:23:47.517: [AMF Encoder]     Default: true
23:23:47.517: [AMF Encoder]     Minimum: Empty
23:23:47.517: [AMF Encoder]     Maximum: Empty
23:23:47.517: [AMF Encoder]  [TL1.QL0.RateControlSkipFrameEnable] TL1.QL0 Rate Control Based Frame Skip (Type: Boolean, Index 125)
23:23:47.518: [AMF Encoder]   Content Type: 0
23:23:47.518: [AMF Encoder]   Access: 
23:23:47.518: [AMF Encoder]   Values:
23:23:47.518: [AMF Encoder]     Current: Empty
23:23:47.518: [AMF Encoder]     Default: true
23:23:47.518: [AMF Encoder]     Minimum: Empty
23:23:47.518: [AMF Encoder]     Maximum: Empty
23:23:47.518: [AMF Encoder]  [TL2.QL0.RateControlSkipFrameEnable] TL2.QL0 Rate Control Based Frame Skip (Type: Boolean, Index 126)
23:23:47.518: [AMF Encoder]   Content Type: 0
23:23:47.518: [AMF Encoder]   Access: 
23:23:47.518: [AMF Encoder]   Values:
23:23:47.518: [AMF Encoder]     Current: Empty
23:23:47.518: [AMF Encoder]     Default: true
23:23:47.518: [AMF Encoder]     Minimum: Empty
23:23:47.519: [AMF Encoder]     Maximum: Empty
23:23:47.519: [AMF Encoder]  [TL3.QL0.RateControlSkipFrameEnable] TL3.QL0 Rate Control Based Frame Skip (Type: Boolean, Index 127)
23:23:47.519: [AMF Encoder]   Content Type: 0
23:23:47.519: [AMF Encoder]   Access: 
23:23:47.519: [AMF Encoder]   Values:
23:23:47.519: [AMF Encoder]     Current: Empty
23:23:47.519: [AMF Encoder]     Default: true
23:23:47.519: [AMF Encoder]     Minimum: Empty
23:23:47.519: [AMF Encoder]     Maximum: Empty
23:23:47.519: [AMF Encoder]  [TL0.QL0.ReferenceBPicturesDeltaQP] TL0.QL0 Reference B-picture Delta (Type: Int64, Index 128)
23:23:47.519: [AMF Encoder]   Content Type: 0
23:23:47.519: [AMF Encoder]   Access: 
23:23:47.520: [AMF Encoder]   Values:
23:23:47.520: [AMF Encoder]     Current: Empty
23:23:47.520: [AMF Encoder]     Default: 2
23:23:47.520: [AMF Encoder]     Minimum: -10
23:23:47.520: [AMF Encoder]     Maximum: 10
23:23:47.520: [AMF Encoder]  [TL1.QL0.ReferenceBPicturesDeltaQP] TL1.QL0 Reference B-picture Delta (Type: Int64, Index 129)
23:23:47.520: [AMF Encoder]   Content Type: 0
23:23:47.520: [AMF Encoder]   Access: 
23:23:47.520: [AMF Encoder]   Values:
23:23:47.520: [AMF Encoder]     Current: Empty
23:23:47.520: [AMF Encoder]     Default: 2
23:23:47.520: [AMF Encoder]     Minimum: -10
23:23:47.521: [AMF Encoder]     Maximum: 10
23:23:47.521: [AMF Encoder]  [TL2.QL0.ReferenceBPicturesDeltaQP] TL2.QL0 Reference B-picture Delta (Type: Int64, Index 130)
23:23:47.521: [AMF Encoder]   Content Type: 0
23:23:47.521: [AMF Encoder]   Access: 
23:23:47.521: [AMF Encoder]   Values:
23:23:47.521: [AMF Encoder]     Current: Empty
23:23:47.521: [AMF Encoder]     Default: 2
23:23:47.521: [AMF Encoder]     Minimum: -10
23:23:47.521: [AMF Encoder]     Maximum: 10
23:23:47.521: [AMF Encoder]  [TL3.QL0.ReferenceBPicturesDeltaQP] TL3.QL0 Reference B-picture Delta (Type: Int64, Index 131)
23:23:47.521: [AMF Encoder]   Content Type: 0
23:23:47.521: [AMF Encoder]   Access: 
23:23:47.522: [AMF Encoder]   Values:
23:23:47.522: [AMF Encoder]     Current: Empty
23:23:47.522: [AMF Encoder]     Default: 2
23:23:47.522: [AMF Encoder]     Minimum: -10
23:23:47.522: [AMF Encoder]     Maximum: 10
23:23:47.522: [AMF Encoder]  [TL0.QL0.TargetBitrate] TL0.QL0 Bitrate (Type: Int64, Index 132)
23:23:47.522: [AMF Encoder]   Content Type: 0
23:23:47.522: [AMF Encoder]   Access: 
23:23:47.522: [AMF Encoder]   Values:
23:23:47.522: [AMF Encoder]     Current: Empty
23:23:47.522: [AMF Encoder]     Default: 10000000
23:23:47.522: [AMF Encoder]     Minimum: 10000
23:23:47.523: [AMF Encoder]     Maximum: 100000000
23:23:47.523: [AMF Encoder]  [TL1.QL0.TargetBitrate] TL1.QL0 Bitrate (Type: Int64, Index 133)
23:23:47.523: [AMF Encoder]   Content Type: 0
23:23:47.523: [AMF Encoder]   Access: 
23:23:47.523: [AMF Encoder]   Values:
23:23:47.523: [AMF Encoder]     Current: Empty
23:23:47.523: [AMF Encoder]     Default: 10000000
23:23:47.523: [AMF Encoder]     Minimum: 10000
23:23:47.523: [AMF Encoder]     Maximum: 100000000
23:23:47.523: [AMF Encoder]  [TL2.QL0.TargetBitrate] TL2.QL0 Bitrate (Type: Int64, Index 134)
23:23:47.523: [AMF Encoder]   Content Type: 0
23:23:47.523: [AMF Encoder]   Access: 
23:23:47.523: [AMF Encoder]   Values:
23:23:47.524: [AMF Encoder]     Current: Empty
23:23:47.524: [AMF Encoder]     Default: 10000000
23:23:47.524: [AMF Encoder]     Minimum: 10000
23:23:47.524: [AMF Encoder]     Maximum: 100000000
23:23:47.524: [AMF Encoder]  [TL3.QL0.TargetBitrate] TL3.QL0 Bitrate (Type: Int64, Index 135)
23:23:47.524: [AMF Encoder]   Content Type: 0
23:23:47.524: [AMF Encoder]   Access: 
23:23:47.524: [AMF Encoder]   Values:
23:23:47.524: [AMF Encoder]     Current: Empty
23:23:47.524: [AMF Encoder]     Default: 10000000
23:23:47.524: [AMF Encoder]     Minimum: 10000
23:23:47.525: [AMF Encoder]     Maximum: 100000000
23:23:47.525: [AMF Encoder]  [TL0.QL0.VBVBufferSize] TL0.QL0 VBV Buffer Size (Type: Int64, Index 136)
23:23:47.525: [AMF Encoder]   Content Type: 0
23:23:47.525: [AMF Encoder]   Access: 
23:23:47.525: [AMF Encoder]   Values:
23:23:47.525: [AMF Encoder]     Current: Empty
23:23:47.525: [AMF Encoder]     Default: 1000000
23:23:47.525: [AMF Encoder]     Minimum: 1000
23:23:47.525: [AMF Encoder]     Maximum: 100000000
23:23:47.525: [AMF Encoder]  [TL1.QL0.VBVBufferSize] TL1.QL0 VBV Buffer Size (Type: Int64, Index 137)
23:23:47.525: [AMF Encoder]   Content Type: 0
23:23:47.525: [AMF Encoder]   Access: 
23:23:47.526: [AMF Encoder]   Values:
23:23:47.526: [AMF Encoder]     Current: Empty
23:23:47.526: [AMF Encoder]     Default: 1000000
23:23:47.526: [AMF Encoder]     Minimum: 1000
23:23:47.526: [AMF Encoder]     Maximum: 100000000
23:23:47.526: [AMF Encoder]  [TL2.QL0.VBVBufferSize] TL2.QL0 VBV Buffer Size (Type: Int64, Index 138)
23:23:47.526: [AMF Encoder]   Content Type: 0
23:23:47.528: [AMF Encoder]   Access: 
23:23:47.529: [AMF Encoder]   Values:
23:23:47.529: [AMF Encoder]     Current: Empty
23:23:47.529: [AMF Encoder]     Default: 1000000
23:23:47.530: [AMF Encoder]     Minimum: 1000
23:23:47.530: [AMF Encoder]     Maximum: 100000000
23:23:47.530: [AMF Encoder]  [TL3.QL0.VBVBufferSize] TL3.QL0 VBV Buffer Size (Type: Int64, Index 139)
23:23:47.530: [AMF Encoder]   Content Type: 0
23:23:47.530: [AMF Encoder]   Access: 
23:23:47.530: [AMF Encoder]   Values:
23:23:47.530: [AMF Encoder]     Current: Empty
23:23:47.530: [AMF Encoder]     Default: 1000000
23:23:47.530: [AMF Encoder]     Minimum: 1000
23:23:47.531: [AMF Encoder]     Maximum: 100000000
23:23:47.531: [AMF Encoder]  [NumOfTemporalEnhancmentLayers] Num Of Temporal Enhancment Layers (Type: Int64, Index 140)
23:23:47.531: [AMF Encoder]   Content Type: 0
23:23:47.531: [AMF Encoder]   Access: 
23:23:47.531: [AMF Encoder]   Values:
23:23:47.531: [AMF Encoder]     Current: Empty
23:23:47.531: [AMF Encoder]     Default: 0
23:23:47.531: [AMF Encoder]     Minimum: 0
23:23:47.531: [AMF Encoder]     Maximum: 3
23:23:47.531: [AMF Encoder]  [NumOfQualityLayers] Num Of Quality Layers (Type: Int64, Index 141)
23:23:47.531: [AMF Encoder]   Content Type: 0
23:23:47.532: [AMF Encoder]   Access: 
23:23:47.533: [AMF Encoder]   Values:
23:23:47.533: [AMF Encoder]     Current: Empty
23:23:47.533: [AMF Encoder]     Default: 0
23:23:47.534: [AMF Encoder]     Minimum: 0
23:23:47.534: [AMF Encoder]     Maximum: 0
23:23:47.534: [AMF Encoder]  [MGSVectorMode] MGS Vector Mode (Type: Boolean, Index 142)
23:23:47.534: [AMF Encoder]   Content Type: 0
23:23:47.534: [AMF Encoder]   Access: 
23:23:47.534: [AMF Encoder]   Values:
23:23:47.534: [AMF Encoder]     Current: Empty
23:23:47.534: [AMF Encoder]     Default: false
23:23:47.534: [AMF Encoder]     Minimum: Empty
23:23:47.534: [AMF Encoder]     Maximum: Empty
23:23:47.535: [AMF Encoder]  [MGSVector0] MGS Vector 0 (Type: Int64, Index 143)
23:23:47.535: [AMF Encoder]   Content Type: 0
23:23:47.535: [AMF Encoder]   Access: 
23:23:47.535: [AMF Encoder]   Values:
23:23:47.535: [AMF Encoder]     Current: Empty
23:23:47.535: [AMF Encoder]     Default: 0
23:23:47.535: [AMF Encoder]     Minimum: 0
23:23:47.535: [AMF Encoder]     Maximum: 16
23:23:47.535: [AMF Encoder]  [MGSVector1] MGS Vector 1 (Type: Int64, Index 144)
23:23:47.535: [AMF Encoder]   Content Type: 0
23:23:47.535: [AMF Encoder]   Access: 
23:23:47.536: [AMF Encoder]   Values:
23:23:47.536: [AMF Encoder]     Current: Empty
23:23:47.536: [AMF Encoder]     Default: 0
23:23:47.536: [AMF Encoder]     Minimum: 0
23:23:47.536: [AMF Encoder]     Maximum: 16
23:23:47.536: [AMF Encoder]  [MGSVector2] MGS Vector 2 (Type: Int64, Index 145)
23:23:47.536: [AMF Encoder]   Content Type: 0
23:23:47.536: [AMF Encoder]   Access: 
23:23:47.536: [AMF Encoder]   Values:
23:23:47.536: [AMF Encoder]     Current: Empty
23:23:47.537: [AMF Encoder]     Default: 0
23:23:47.537: [AMF Encoder]     Minimum: 0
23:23:47.537: [AMF Encoder]     Maximum: 16
23:23:47.537: [AMF Encoder]  [MGSVector3] MGS Vector 3 (Type: Int64, Index 146)
23:23:47.537: [AMF Encoder]   Content Type: 0
23:23:47.537: [AMF Encoder]   Access: 
23:23:47.537: [AMF Encoder]   Values:
23:23:47.537: [AMF Encoder]     Current: Empty
23:23:47.537: [AMF Encoder]     Default: 0
23:23:47.537: [AMF Encoder]     Minimum: 0
23:23:47.537: [AMF Encoder]     Maximum: 16
23:23:47.538: [AMF Encoder]  [MGSKeyPicturePeriod] MGS Key Picture Period (Type: Int64, Index 147)
23:23:47.538: [AMF Encoder]   Content Type: 0
23:23:47.538: [AMF Encoder]   Access: 
23:23:47.538: [AMF Encoder]   Values:
23:23:47.538: [AMF Encoder]     Current: Empty
23:23:47.538: [AMF Encoder]     Default: 0
23:23:47.538: [AMF Encoder]     Minimum: 0
23:23:47.538: [AMF Encoder]     Maximum: 1000

