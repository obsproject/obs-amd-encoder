/*
MIT License

Copyright (c) 2016-2017

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
#include "api-base.h"
#include <d3d9.h>
#include <atlutil.h>

#ifdef _DEBUG
#define D3D_DEBUG_INFO
#endif
#pragma comment(lib, "d3d9.lib")

namespace Plugin {
	namespace API {
		class Direct3D9 : public IAPI {
			friend class Direct3D9Instance;
			public:

			Direct3D9();
			~Direct3D9();

			virtual std::string GetName() override;
			virtual Type GetType() override;
			virtual std::vector<Adapter> EnumerateAdapters() override;
			virtual std::shared_ptr<Instance> CreateInstance(Adapter adapter) override;

			protected:
			IDirect3D9Ex* m_Direct3D9Ex;
			//std::map<std::pair<int32_t, int32_t>, std::shared_ptr<Instance>> m_InstanceMap;

			private:
			std::vector<Adapter> m_Adapters;
		};

		class Direct3D9Instance : public Instance {
			public:
			Direct3D9Instance(Direct3D9* api, Adapter adapter);
			~Direct3D9Instance();

			virtual Adapter GetAdapter() override;
			virtual void* GetContext() override;

			private:
			Direct3D9* m_API;
			Adapter m_Adapter;
			IDirect3DDevice9Ex* m_Device;
		};
	}
}
