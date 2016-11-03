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

#ifdef _WIN32
//////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////
#include "api-d3d9.h"

#include "d3d9.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin::API;

Plugin::API::Device BuildDeviceFromAdapter(D3DADAPTER_IDENTIFIER9* pAdapter) {
	if (pAdapter == nullptr)
		return Device("INVALID DEVICE", "");

	std::vector<char> uidBuf(1024);
	sprintf(uidBuf.data(), "%ld:%ld:%ld:%ld",
		pAdapter->VendorId,
		pAdapter->DeviceId,
		pAdapter->SubSysId,
		pAdapter->Revision);

	std::vector<char> nameBuf(1024);
	sprintf(nameBuf.data(), "%s (%s)",
		pAdapter->DeviceName,
		pAdapter->Description);

	return Device(std::string(nameBuf.data()), std::string(uidBuf.data()));
}


std::vector<Plugin::API::Device> Plugin::API::Direct3D9::EnumerateDevices() {
	std::vector<Plugin::API::Device> devices = std::vector<Plugin::API::Device>();

	IDirect3D9* pDirect3D = Direct3DCreate9(D3D_SDK_VERSION);

	uint32_t adapterCount = pDirect3D->GetAdapterCount();
	for (uint32_t adapterIndex = 0; adapterIndex < adapterCount; adapterIndex++) {
		D3DADAPTER_IDENTIFIER9 adapterDesc = D3DADAPTER_IDENTIFIER9();
		pDirect3D->GetAdapterIdentifier(adapterIndex, 0, &adapterDesc);

		if (adapterDesc.VendorId != 0x1002)
			continue;

		Device device = BuildDeviceFromAdapter(&adapterDesc);
		devices.push_back(device);
	}

	pDirect3D->Release();

	return devices;
}

Plugin::API::Device Plugin::API::Direct3D9::GetDeviceForUniqueId(std::string uniqueId) {
	Plugin::API::Device device = Device("", "");

	IDirect3D9* pDirect3D = Direct3DCreate9(D3D_SDK_VERSION);

	uint32_t adapterCount = pDirect3D->GetAdapterCount();
	for (uint32_t adapterIndex = 0; adapterIndex <= adapterCount; adapterIndex++) {
		D3DADAPTER_IDENTIFIER9 adapterDesc = D3DADAPTER_IDENTIFIER9();
		pDirect3D->GetAdapterIdentifier(adapterIndex, 0, &adapterDesc);

		if (adapterDesc.VendorId != 0x1002)
			continue;

		Device device2 = BuildDeviceFromAdapter(&adapterDesc);
		
		if (device2.UniqueId == uniqueId)
			device = device2;
	}

	pDirect3D->Release();

	return device;
}

Plugin::API::Direct3D9::Direct3D9(Device device) {

}

Plugin::API::Direct3D9::~Direct3D9() {

}

void* Plugin::API::Direct3D9::GetContext() {

}


#endif
