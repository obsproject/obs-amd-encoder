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

//////////////////////////////////////////////////////////////////////////
// Includes
//////////////////////////////////////////////////////////////////////////
#include "api-d3d11.h"

#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <mutex>

#ifdef _WIN32
#include <dxgi.h>
#include <d3d11.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin::API;

#ifdef _WIN32
class SingletonDXGI {
	public:

	#pragma region Singleton
	static std::shared_ptr<SingletonDXGI> GetInstance() {
		static std::shared_ptr<SingletonDXGI> __instance = std::make_shared<SingletonDXGI>();
		static std::mutex __mutex;

		const std::lock_guard<std::mutex> lock(__mutex);
		return __instance;
	}
	#pragma endregion Singleton

	SingletonDXGI() {
		hModule = LoadLibrary(TEXT("dxgi.dll"));
		if (hModule == 0)
			throw std::exception("Unable to load 'dxgi.dll'.");

	}
	~SingletonDXGI() {
		if (hModule)
			FreeLibrary(hModule);
	}

	HRESULT CreateDXGIFactory1(REFIID riid, _Out_ void **ppFactory) {
		if (hModule == 0)
			return S_FALSE;

		typedef HRESULT(__stdcall *t_CreateDXGIFactory1)(REFIID, void**);
		t_CreateDXGIFactory1 pCreateDXGIFactory1 = (t_CreateDXGIFactory1)GetProcAddress(hModule, "CreateDXGIFactory1");

		if (pCreateDXGIFactory1) {
			return pCreateDXGIFactory1(riid, ppFactory);
		}
		return S_FALSE;
	}

	private:
	HMODULE hModule;
};
class SingletonD3D11 {
	public:

	#pragma region Singleton
	static std::shared_ptr<SingletonD3D11> GetInstance() {
		static std::shared_ptr<SingletonD3D11> __instance = std::make_shared<SingletonD3D11>();
		static std::mutex __mutex;

		const std::lock_guard<std::mutex> lock(__mutex);
		return __instance;
	}
	#pragma endregion Singleton

	SingletonD3D11() {
		hModule = LoadLibrary(TEXT("d3d11.dll"));
		if (hModule == 0)
			throw std::exception("Unable to load 'd3d11.dll'.");

	}
	~SingletonD3D11() {
		if (hModule)
			FreeLibrary(hModule);
	}

	HRESULT WINAPI D3D11CreateDevice(
		_In_opt_ IDXGIAdapter* pAdapter,
		D3D_DRIVER_TYPE DriverType,
		HMODULE Software,
		UINT Flags,
		_In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
		UINT FeatureLevels,
		UINT SDKVersion,
		_Out_opt_ ID3D11Device** ppDevice,
		_Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel,
		_Out_opt_ ID3D11DeviceContext** ppImmediateContext) {

		if (hModule == 0)
			return S_FALSE;

		typedef HRESULT(__stdcall *t_D3D11CreateDevice)(_In_opt_ IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT,
			CONST D3D_FEATURE_LEVEL*, UINT, UINT, _Out_opt_ ID3D11Device**,
			_Out_opt_ D3D_FEATURE_LEVEL*, _Out_opt_ ID3D11DeviceContext**);
		t_D3D11CreateDevice pD3D11CreateDevice = (t_D3D11CreateDevice)GetProcAddress(hModule, "D3D11CreateDevice");

		if (pD3D11CreateDevice) {
			return pD3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels,
				SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);
		}
		return S_FALSE;
	}

	private:
	HMODULE hModule;
};

Plugin::API::Device BuildDeviceFromAdapter(DXGI_ADAPTER_DESC* pAdapter) {
	if (pAdapter == nullptr)
		return Device("INVALID DEVICE", "");

	std::vector<char> uidBuf(1024);
	sprintf(uidBuf.data(), "%ld:%ld",
		pAdapter->AdapterLuid.LowPart,
		pAdapter->AdapterLuid.HighPart);

	std::vector<char> nameBuf(1024);
	wcstombs(nameBuf.data(), pAdapter->Description, 1024);

	return Device(std::string(nameBuf.data()), std::string(uidBuf.data()));
}
#endif

std::vector<Plugin::API::Device> Plugin::API::Direct3D11::EnumerateDevices() {
	std::vector<Plugin::API::Device> devices = std::vector<Plugin::API::Device>();

	#ifdef _WIN32
	IDXGIFactory1* pFactory = NULL;
	auto singletonDXGI = SingletonDXGI::GetInstance();
	HRESULT hr = singletonDXGI->CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
	if (FAILED(hr)) {
		return devices;
	}

	AMF_LOG_DEBUG("<" __FUNCTION_NAME__ "> Enumerating devices: ");

	IDXGIAdapter* pAdapter = NULL;
	for (uint32_t iAdapterIndex = 0; pFactory->EnumAdapters(iAdapterIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND; iAdapterIndex++) {
		DXGI_ADAPTER_DESC adapterDesc = DXGI_ADAPTER_DESC();
		std::memset(&adapterDesc, 0, sizeof(DXGI_ADAPTER_DESC));

		if (pAdapter->GetDesc(&adapterDesc) == S_OK) {
			AMF_LOG_DEBUG("<" __FUNCTION_NAME__ ">   [%d] %ls",
				iAdapterIndex,
				adapterDesc.Description);

			// Only allow AMD devices to be listed here.
			if (adapterDesc.VendorId != 0x1002)
				continue;

			devices.push_back(BuildDeviceFromAdapter(&adapterDesc));
		}
	}

	pFactory->Release();
	#endif

	return devices;
}

Plugin::API::Device Plugin::API::Direct3D11::GetDeviceForUniqueId(std::string uniqueId) {
	Plugin::API::Device device = Device();

	#ifdef _WIN32
	IDXGIFactory1* pFactory = NULL;
	auto singletonDXGI = SingletonDXGI::GetInstance();
	HRESULT hr = singletonDXGI->CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
	if (FAILED(hr)) {
		return device;
	}

	IDXGIAdapter* pAdapter = NULL;
	for (uint32_t iAdapterIndex = 0; pFactory->EnumAdapters(iAdapterIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND; iAdapterIndex++) {
		DXGI_ADAPTER_DESC adapterDesc = DXGI_ADAPTER_DESC();
		std::memset(&adapterDesc, 0, sizeof(DXGI_ADAPTER_DESC));

		if (pAdapter->GetDesc(&adapterDesc) == S_OK) {
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
	#endif

	return device;
}

Plugin::API::Device Plugin::API::Direct3D11::GetDeviceForContext(void* context) {
	Plugin::API::Device device = Device();

	#ifdef _WIN32
	if (!context)
		return Device();

	ID3D11Device* d3dDevice = static_cast<ID3D11Device*>(context);
	if (!d3dDevice)
		return Device();

	IDXGIDevice* dxgiDevice = nullptr;
	d3dDevice->QueryInterface(&dxgiDevice);
	if (!dxgiDevice)
		return Device();

	IDXGIAdapter* dxgiAdapter = nullptr;
	dxgiDevice->GetAdapter(&dxgiAdapter);

	DXGI_ADAPTER_DESC dxgiAdapterDesc = DXGI_ADAPTER_DESC();
	dxgiAdapter->GetDesc(&dxgiAdapterDesc);

	dxgiAdapter->Release();
	dxgiDevice->Release();

	device = BuildDeviceFromAdapter(&dxgiAdapterDesc);
	#endif

	return device;
}

Plugin::API::Direct3D11::Direct3D11(Device device) : APIBase(device) {
	#ifdef _WIN32
	IDXGIFactory1 *pFactory;

	auto singletonDXGI = SingletonDXGI::GetInstance();
	if (FAILED(singletonDXGI->CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory))))
		throw std::exception("Unable to create D3D11 driver.");

	try {
		IDXGIAdapter *pAdapter = NULL;
		if (device.UniqueId != "") {
			IDXGIAdapter *pAdapter2 = NULL;
			for (uint32_t iAdapterIndex = 0; pFactory->EnumAdapters(iAdapterIndex, &pAdapter2) != DXGI_ERROR_NOT_FOUND; iAdapterIndex++) {
				DXGI_ADAPTER_DESC adapterDesc = DXGI_ADAPTER_DESC();
				std::memset(&adapterDesc, 0, sizeof(DXGI_ADAPTER_DESC));

				if (pAdapter2->GetDesc(&adapterDesc) == S_OK) {
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
		}

		try {
			D3D_FEATURE_LEVEL featureLevels[] = {
				D3D_FEATURE_LEVEL_11_1,
				D3D_FEATURE_LEVEL_11_0
			};
			D3D_FEATURE_LEVEL featureLevel;
			uint32_t flags =
				D3D11_CREATE_DEVICE_BGRA_SUPPORT |
				D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT |
				D3D11_CREATE_DEVICE_VIDEO_SUPPORT;

			DXGI_ADAPTER_DESC desc;
			if (pAdapter != NULL) {
				pAdapter->GetDesc(&desc);
			}

			auto singletonD3D11 = SingletonD3D11::GetInstance();
			HRESULT hr = singletonD3D11->D3D11CreateDevice(
				pAdapter, pAdapter == NULL ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN,
				NULL, flags,
				featureLevels, _countof(featureLevels),
				D3D11_SDK_VERSION,
				reinterpret_cast<ID3D11Device**>(&pDevice),
				&featureLevel,
				reinterpret_cast<ID3D11DeviceContext**>(&pDeviceContext));
			if (FAILED(hr)) {
				AMF_LOG_ERROR("Unable to create D3D11.1 device.");
				hr = singletonD3D11->D3D11CreateDevice(
					pAdapter, pAdapter == NULL ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN,
					NULL, flags,
					featureLevels + 1, _countof(featureLevels) - 1,
					D3D11_SDK_VERSION,
					reinterpret_cast<ID3D11Device**>(&pDevice),
					&featureLevel,
					reinterpret_cast<ID3D11DeviceContext**>(&pDeviceContext));
				if (FAILED(hr)) {
					throw std::exception("Unable to create D3D11 device.");
				}
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
	#else
	throw std::exception("Platform not supported!");
	#endif
}

Plugin::API::Direct3D11::~Direct3D11() {
	#ifdef _WIN32
	if (pDeviceContext)
		static_cast<ID3D11DeviceContext*>(pDeviceContext)->Release();
	pDeviceContext = nullptr;
	if (pDevice)
		static_cast<ID3D11Device*>(pDevice)->Release();
	pDevice = nullptr;
	#else
	throw std::exception("Platform not supported!");
	#endif
}

Plugin::API::APIType Plugin::API::Direct3D11::GetType() {
	return APIType_Direct3D11;
}

void* Plugin::API::Direct3D11::GetContext() {
	return pDevice;
}
