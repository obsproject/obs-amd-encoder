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

struct EnumWindowsData {
	DWORD processId;
	HWND bestWindowId;
};

BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam) {
	EnumWindowsData* data = (EnumWindowsData*)lParam;

	DWORD processId;
	GetWindowThreadProcessId(handle, &processId);
	if ((processId == data->processId)
		&& (GetWindow(handle, GW_OWNER) == (HWND)0)
		&& (IsWindowVisible(handle))) {
		return TRUE;
	}
	data->bestWindowId = handle;
	return FALSE;
}

Plugin::API::Direct3D9::Direct3D9(Device device) : APIBase(device) {
	this->myType = APIType_Direct3D9;

	pDirect3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pDirect3D)
		throw std::exception("Unable to create D3D9 driver.");

	// Find Adapter Index
	uint32_t usedAdapter = 0;
	uint32_t adapterCount = pDirect3D->GetAdapterCount();
	for (uint32_t adapterIndex = 0; adapterIndex <= adapterCount; adapterIndex++) {
		D3DADAPTER_IDENTIFIER9 adapterDesc = D3DADAPTER_IDENTIFIER9();
		pDirect3D->GetAdapterIdentifier(adapterIndex, 0, &adapterDesc);

		if (adapterDesc.VendorId != 0x1002)
			continue;

		Device device2 = BuildDeviceFromAdapter(&adapterDesc);
		if (device2.UniqueId == device.UniqueId) {
			usedAdapter = adapterIndex++;
			break;
		}
	}

	EnumWindowsData data = EnumWindowsData();
	data.processId = GetCurrentProcessId();
	EnumWindows(EnumWindowsCallback, (LPARAM)&data);

	D3DPRESENT_PARAMETERS pPresentParameter = D3DPRESENT_PARAMETERS();
	pPresentParameter.BackBufferWidth = 1280;
	pPresentParameter.BackBufferHeight = 720;
	pPresentParameter.BackBufferFormat = D3DFORMAT::D3DFMT_X8R8G8B8;
	pPresentParameter.BackBufferCount = 2;
	pPresentParameter.MultiSampleType = D3DMULTISAMPLE_TYPE::D3DMULTISAMPLE_NONE;
	pPresentParameter.MultiSampleQuality = 0;
	pPresentParameter.SwapEffect = D3DSWAPEFFECT::D3DSWAPEFFECT_DISCARD;
	pPresentParameter.hDeviceWindow = data.bestWindowId;
	pPresentParameter.Windowed = TRUE;
	pPresentParameter.EnableAutoDepthStencil = FALSE;
	pPresentParameter.AutoDepthStencilFormat = D3DFORMAT::D3DFMT_A1;
	pPresentParameter.Flags = D3DPRESENTFLAG_VIDEO;
	pPresentParameter.FullScreen_RefreshRateInHz = 0;
	pPresentParameter.PresentationInterval = 60;
		
	HRESULT hr = pDirect3D->CreateDevice(usedAdapter,
		D3DDEVTYPE_HAL,
		data.bestWindowId,
		0, //D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX | D3DCREATE_MULTITHREADED | D3DCREATE_PUREDEVICE | D3DCREATE_MIXED_VERTEXPROCESSING | D3DCREATE_NOWINDOWCHANGES,
		&pPresentParameter,
		&pDirect3DDevice);
	switch (hr) {
		case D3DERR_DEVICELOST:
			throw std::exception("D3DERR_DEVICELOST");
			break;
		case D3DERR_INVALIDCALL:
			throw std::exception("D3DERR_INVALIDCALL");
			break;
		case D3DERR_NOTAVAILABLE:
			throw std::exception("D3DERR_NOTAVAILABLE");
			break;
		case D3DERR_OUTOFVIDEOMEMORY:
			throw std::exception("D3DERR_OUTOFVIDEOMEMORY");
			break;
	}
	if (FAILED(hr))
		throw std::exception("Unable to create D3D9 device.");
}

Plugin::API::Direct3D9::~Direct3D9() {
	if (pDirect3DDevice)
		pDirect3DDevice->Release();

	if (pDirect3D)
		pDirect3D->Release();
}

void* Plugin::API::Direct3D9::GetContext() {
	return pDirect3DDevice;
}

#endif
