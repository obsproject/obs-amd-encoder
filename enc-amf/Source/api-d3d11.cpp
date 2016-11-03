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
#include "api-d3d11.h"

#include <vector>
#include <string>
#include <sstream>

#include <dxgi.h>
#include <d3d11.h>

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin::API;

Plugin::API::Device BuildDeviceFromAdapter(DXGI_ADAPTER_DESC1* pAdapter) {
	if (pAdapter == nullptr)
		return Device("INVALID DEVICE", "");

	std::vector<char> uidBuf(1024);
	sprintf(uidBuf.data(), "%ld:%ld:%ld:%ld",
		pAdapter->VendorId,
		pAdapter->DeviceId,
		pAdapter->SubSysId,
		pAdapter->Revision);

	std::vector<char> nameBuf(1024);
	wcstombs(nameBuf.data(), pAdapter->Description, 1024);

	return Device(std::string(nameBuf.data()), std::string(uidBuf.data()));
}

std::vector<Plugin::API::Device> Plugin::API::Direct3D11::EnumerateDevices() {
	std::vector<Plugin::API::Device> devices = std::vector<Plugin::API::Device>();

	IDXGIFactory1* pFactory = NULL;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
	if (FAILED(hr)) {
		return devices;
	}

	IDXGIAdapter1* pAdapter = NULL;
	for (uint32_t iAdapterIndex = 0; pFactory->EnumAdapters1(iAdapterIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND; iAdapterIndex++) {
		DXGI_ADAPTER_DESC1 adapterDesc = DXGI_ADAPTER_DESC1();
		std::memset(&adapterDesc, 0, sizeof(DXGI_ADAPTER_DESC1));

		if (pAdapter->GetDesc1(&adapterDesc) == S_OK) {
			// Only allow AMD devices to be listed here.
			if (adapterDesc.VendorId != 0x1002)
				continue;

			devices.push_back(BuildDeviceFromAdapter(&adapterDesc));
		}
	}

	pFactory->Release();

	return devices;
}

Plugin::API::Device Plugin::API::Direct3D11::GetDeviceForUniqueId(std::string uniqueId) {
	Plugin::API::Device device = Device("", "");

	IDXGIFactory1* pFactory = NULL;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
	if (FAILED(hr)) {
		return device;
	}

	IDXGIAdapter1* pAdapter = NULL;
	for (uint32_t iAdapterIndex = 0; pFactory->EnumAdapters1(iAdapterIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND; iAdapterIndex++) {
		DXGI_ADAPTER_DESC1 adapterDesc = DXGI_ADAPTER_DESC1();
		std::memset(&adapterDesc, 0, sizeof(DXGI_ADAPTER_DESC1));

		if (pAdapter->GetDesc1(&adapterDesc) == S_OK) {
			// Only allow AMD devices to be listed here.
			if (adapterDesc.VendorId != 0x1002)
				continue;

			Plugin::API::Device device2 = BuildDeviceFromAdapter(&adapterDesc);

			if (uniqueId == device2.UniqueId) {
				device = device2;
				break;
			}
		}
	}

	pFactory->Release();
	return device;
}

Plugin::API::Direct3D11::Direct3D11(Device device) : BaseAPI(device) {
	IDXGIFactory1 *pFactory;

	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory))))
		throw new std::exception("Unable to create D3D11 driver.");

	try {
		IDXGIAdapter1 *pAdapter = NULL,
			*pAdapter2 = NULL;
		for (uint32_t iAdapterIndex = 0; pFactory->EnumAdapters1(iAdapterIndex, &pAdapter2) != DXGI_ERROR_NOT_FOUND; iAdapterIndex++) {
			DXGI_ADAPTER_DESC1 adapterDesc = DXGI_ADAPTER_DESC1();
			std::memset(&adapterDesc, 0, sizeof(DXGI_ADAPTER_DESC1));

			if (pAdapter2->GetDesc1(&adapterDesc) == S_OK) {
				// Only allow AMD devices to be listed here.
				if (adapterDesc.VendorId != 0x1002)
					continue;

				Plugin::API::Device device2 = BuildDeviceFromAdapter(&adapterDesc);

				if (device.UniqueId == device2.UniqueId) {
					pAdapter = pAdapter2;
					break;
				}
			}
		}

		try {
			D3D_FEATURE_LEVEL featureLevels[] = {
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0,
			};
			uint32_t flags =
				D3D11_CREATE_DEVICE_BGRA_SUPPORT |
				D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT |
				D3D11_CREATE_DEVICE_VIDEO_SUPPORT;

			HRESULT hr = D3D11CreateDevice(pAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL,
				flags, featureLevels, 2, D3D11_SDK_VERSION,
				&pDevice, NULL, &pDeviceContext);

			if (FAILED(hr)) {
				throw new std::exception("Unable to create D3D11 device.");
			}
		} catch (...) {
			if (pAdapter)
				pAdapter->Release();

			throw;
		}
	} catch (...) {
		if (pFactory)
			pFactory->Release();

		throw;
	}
}

Plugin::API::Direct3D11::~Direct3D11() {
	if (pDeviceContext)
		pDeviceContext->Release();
	if (pDevice)
		pDevice->Release();
}

void* Plugin::API::Direct3D11::GetContext() {
	return pDevice;
}
#endif
