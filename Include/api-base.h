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

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
namespace Plugin {
	namespace API {
		struct Device {
			std::string Name;
			std::string UniqueId;

			Device();
			Device(std::string Name, std::string UniqueId);
			~Device();


			friend bool operator<(const Plugin::API::Device& left, const Plugin::API::Device& right);
			friend bool operator>(const Plugin::API::Device& left, const Plugin::API::Device& right);
			friend bool operator<=(const Plugin::API::Device& left, const Plugin::API::Device& right);
			friend bool operator>=(const Plugin::API::Device& left, const Plugin::API::Device& right);

			friend bool operator==(const Plugin::API::Device& left, const Plugin::API::Device& right);
			friend bool operator!=(const Plugin::API::Device& left, const Plugin::API::Device& right);
		};

		enum APIType {
			APIType_Base,
			APIType_Direct3D9,
			APIType_Direct3D11,
			APIType_OpenGL,
		};

		class APIBase {
			public:
			static std::vector<Plugin::API::Device> EnumerateDevices();
			static Plugin::API::Device GetDeviceForUniqueId(std::string uniqueId);
			static Plugin::API::Device GetDeviceForContext(void* context);

			static Plugin::API::APIType GetBestAvailableAPI();
			static std::unique_ptr<Plugin::API::APIBase> CreateBestAvailableAPI(Plugin::API::Device device);

			APIBase();
			APIBase(Device device);
			virtual ~APIBase();

			Plugin::API::Device GetDevice();

			virtual Plugin::API::APIType GetType();
			virtual void* GetContext();
			
			protected:
			Plugin::API::APIType myType;

			private:
			Plugin::API::Device myDevice;
		};
	}
}