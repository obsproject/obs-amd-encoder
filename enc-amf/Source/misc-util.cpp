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

// Plugin
#include "plugin.h"
#include "amd-amf.h"
#include "amd-amf-vce.h"
#include "amd-amf-vce-capabilities.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin::AMD;

namespace Plugin {
	namespace Utility {
		VCEProfileLevel inline GetMinimumProfileLevel(std::pair<uint32_t, uint32_t> frameSize, std::pair<uint32_t, uint32_t> frameRate) {
			typedef std::pair<uint32_t, uint32_t> levelRestriction;
			typedef std::pair<VCEProfileLevel, levelRestriction> level;

			static const level profileLevelLimit[] = { // [Level, [Samples, Samples_Per_Sec]]
				level(VCEProfileLevel_10, levelRestriction(25344, 380160)),
				level(VCEProfileLevel_11, levelRestriction(101376, 768000)),
				level(VCEProfileLevel_12, levelRestriction(101376, 1536000)),
				level(VCEProfileLevel_13, levelRestriction(101376, 3041280)),
				level(VCEProfileLevel_20, levelRestriction(101376, 3041280)),
				level(VCEProfileLevel_21, levelRestriction(202752, 5068800)),
				level(VCEProfileLevel_22, levelRestriction(414720, 5184000)),
				level(VCEProfileLevel_30, levelRestriction(414720, 10368000)),
				level(VCEProfileLevel_31, levelRestriction(921600, 27648000)),
				level(VCEProfileLevel_32, levelRestriction(1310720, 55296000)),
				//level(VCEProfileLevel_40, levelRestriction(2097152, 62914560)), // Technically identical to 4.1, but backwards compatible.
				level(VCEProfileLevel_41, levelRestriction(2097152, 62914560)),
				level(VCEProfileLevel_42, levelRestriction(2228224, 133693440)),
				level(VCEProfileLevel_50, levelRestriction(5652480, 150994944)),
				level(VCEProfileLevel_51, levelRestriction(9437184, 251658240)),
				level(VCEProfileLevel_52, levelRestriction(9437184, 530841600)),
				level((VCEProfileLevel)-1, levelRestriction(0, 0))
			};

			uint32_t samples = frameSize.first * frameSize.second;
			uint32_t samples_sec = (uint32_t)ceil((double_t)samples * ((double_t)frameRate.first / (double_t)frameRate.second));

			level curLevel = profileLevelLimit[0];
			for (uint32_t index = 0; curLevel.first != -1; index++) {
				curLevel = profileLevelLimit[index];

				if (samples > curLevel.second.first)
					continue;

				if (samples_sec > curLevel.second.second)
					continue;

				return curLevel.first;
			}
			return VCEProfileLevel_52;
		}
	}
}