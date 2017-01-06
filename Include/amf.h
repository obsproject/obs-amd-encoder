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
#include <memory>
#include <windows.h>

// Plugin
#include "plugin.h"

// AMD AMF SDK 
#pragma warning( push )
#pragma warning( disable: 4458 )
#include "core\Factory.h"
#include "components\Component.h"
#include "components\ComponentCaps.h"
#include "components\VideoEncoderVCE.h"
#pragma warning( pop )

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////

namespace Plugin {
	namespace AMD {
		class AMF {
			#pragma region Singleton
			public:
			static std::shared_ptr<Plugin::AMD::AMF> GetInstance();
			#pragma endregion Singleton

			public:
			AMF();
			~AMF();

			amf::AMFFactory* GetFactory();
			amf::AMFTrace* GetTrace();
			amf::AMFDebug* GetDebug();

			void EnableDebugTrace(bool enable);

			private:
			uint32_t m_TimerPeriod; /// High-Precision Timer Accuracy (nanoseconds)
			
			/// AMF Values
			HMODULE m_AMFModule;
			uint64_t m_AMFVersion_Compiler;
			uint64_t m_AMFVersion_Runtime;

			/// AMF Functions
			AMFQueryVersion_Fn AMFQueryVersion;
			AMFInit_Fn AMFInit;

			/// AMF Objects
			amf::AMFFactory* m_AMFFactory;
			amf::AMFTrace* m_AMFTrace;
			amf::AMFDebug* m_AMFDebug;
			amf::AMFTraceWriter* m_TraceWriter;
		};
	}
}