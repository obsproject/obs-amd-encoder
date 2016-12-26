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
#include "plugin.h"

#include <vector>
#include <map>

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
namespace Plugin {
	namespace API {
		enum class Type {
			Host,
			Direct3D9,
			Direct3D11,
			OpenGL,
		};

		struct Adapter {
			int32_t idLow, idHigh;
			std::string Name;

			Adapter() : idLow(0), idHigh(0), Name("Invalid Device") {}
			Adapter(int32_t idLow, int32_t idHigh, std::string Name) : idLow(idLow), idHigh(idHigh), Name(Name) {}

			friend bool operator<(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
			friend bool operator>(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
			friend bool operator<=(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
			friend bool operator>=(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);

			friend bool operator==(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
			friend bool operator!=(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
		};

		class Base {
			//////////////////////////////////////////////////////////////////////////
			// API Index
			//////////////////////////////////////////////////////////////////////////
			public:
			static void Initialize();

			static size_t GetAPICount();
			static std::shared_ptr<Base> GetAPIInstance(size_t index);
			static std::string GetAPIName(size_t index);
			static std::shared_ptr<Base> GetAPIByName(std::string name);
			static std::vector<std::shared_ptr<Base>> EnumerateAPIs();
			static std::vector<std::string> EnumerateAPINames();

			//////////////////////////////////////////////////////////////////////////
			// API
			//////////////////////////////////////////////////////////////////////////
			public:
			virtual std::string GetName() = 0;
			virtual Type GetType() = 0;

			virtual std::vector<Adapter> EnumerateAdapters() = 0;
			virtual Adapter GetAdapterById(uint32_t idLow, uint32_t idHigh) = 0;
			virtual Adapter GetAdapterByName(std::string name) = 0;

			virtual void* CreateInstanceOnAdapter(Adapter adapter) = 0;
			virtual Adapter GetAdapterForInstance(void* instance) = 0;
			virtual void* GetContextFromInstance(void* instance) = 0;
			virtual void DestroyInstance(void* instance) = 0;
		};
	}
}