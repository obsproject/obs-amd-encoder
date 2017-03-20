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
#include "plugin.h"
#include <vector>
#include <map>
#include <string.h>
#include <memory>

namespace Plugin {
	namespace API {
		/**
		 *
		 */
		enum class Type : uint8_t {
			Host,
			Direct3D9,
			Direct3D11,
			OpenGL,
		};

		// An Adapter on an API
		struct Adapter {
			int32_t idLow, idHigh;
			std::string Name;

			Adapter()
				: idLow(0), idHigh(0), Name("Invalid Device") {}
			Adapter(int32_t p_idLow, int32_t p_idHigh, std::string p_Name)
				: idLow(p_idLow), idHigh(p_idHigh), Name(p_Name) {}
			Adapter(Adapter const& o) {
				idLow = o.idLow;
				idHigh = o.idHigh;
				Name = o.Name;
			}
			void operator=(Adapter const& o) {
				idLow = o.idLow;
				idHigh = o.idHigh;
				Name = o.Name;
			}

			friend bool operator<(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
			friend bool operator>(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
			friend bool operator<=(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
			friend bool operator>=(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);

			friend bool operator==(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
			friend bool operator!=(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
		};

		// Instance of an API Adapter
		struct Instance {
			public:
			Instance();
			virtual ~Instance();

			virtual Adapter GetAdapter() = 0;
			virtual void* GetContext() = 0;
		};

		// API Interface
		class IAPI {
			public:
			IAPI();
			virtual ~IAPI();

			virtual std::string GetName() = 0;
			virtual Type GetType() = 0;

			virtual std::vector<Adapter> EnumerateAdapters() = 0;
			Adapter GetAdapterById(int32_t idLow, int32_t idHigh);
			Adapter GetAdapterByName(std::string name);

			virtual std::shared_ptr<Instance> CreateInstance(Adapter adapter) = 0;
		};

		// Static API Stuff
		void InitializeAPIs();
		void FinalizeAPIs();
		size_t CountAPIs();
		std::string GetAPIName(size_t index);
		std::shared_ptr<IAPI> GetAPI(size_t index);
		std::shared_ptr<IAPI> GetAPI(std::string name);
		std::shared_ptr<IAPI> GetAPI(Type type);
		std::vector<std::shared_ptr<IAPI>> EnumerateAPIs();
		std::vector<std::string> EnumerateAPINames();
	}
}