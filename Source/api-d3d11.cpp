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

#include <dxgi.h>
#include <d3d11.h>
#include <atlutil.h>

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin::API;

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

	HRESULT CreateDXGIFactory(REFIID riid, _Out_ void **ppFactory) {
		if (hModule == 0)
			return S_FALSE;

		typedef HRESULT(__stdcall *t_CreateDXGIFactory)(REFIID, void**);
		t_CreateDXGIFactory pCreateDXGIFactory = (t_CreateDXGIFactory)GetProcAddress(hModule, "CreateDXGIFactory");

		if (pCreateDXGIFactory) {
			return pCreateDXGIFactory(riid, ppFactory);
		}
		return S_FALSE;
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

std::string Plugin::API::Direct3D11::GetName() {
	return std::string("Direct3D 11");
}

std::vector<Adapter> Plugin::API::Direct3D11::EnumerateAdapters() {
	auto dxgiInst = SingletonDXGI::GetInstance();

	ATL::CComPtr<IDXGIFactory1> dxgiFactory;
	HRESULT hr = dxgiInst->CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgiFactory);
	if (FAILED(hr))
		throw std::exception("<" __FUNCTION_NAME__ "> Failed to enumerate adapters, error code %X.", hr);

	std::vector<Adapter> adapters;
	IDXGIAdapter1* dxgiAdapter = nullptr;
	for (size_t adapterIndex = 0;
		!FAILED(dxgiFactory->EnumAdapters1((UINT)adapterIndex, &dxgiAdapter));
		adapterIndex++) {
		DXGI_ADAPTER_DESC1 desc = DXGI_ADAPTER_DESC1();
		dxgiAdapter->GetDesc1(&desc);

		if (desc.VendorId != 0x1002 /* AMD */)
			continue;

		std::vector<char> descBuf(256);
		wcstombs(descBuf.data(), desc.Description, descBuf.size());
		adapters.push_back(Adapter(
			desc.AdapterLuid.LowPart,
			desc.AdapterLuid.HighPart,
			std::string(descBuf.data())
		));
	}

	return adapters;
}

Plugin::API::Adapter Plugin::API::Direct3D11::GetAdapterById(uint32_t idLow, uint32_t idHigh) {
	for (auto adapter : EnumerateAdapters()) {
		if ((adapter.idLow == idLow) && (adapter.idHigh == idHigh))
			return adapter;
	}
	return *(EnumerateAdapters().begin());
}

Plugin::API::Adapter Plugin::API::Direct3D11::GetAdapterByName(std::string name) {
	for (auto adapter : EnumerateAdapters()) {
		if (adapter.Name == name)
			return adapter;
	}
	return *(EnumerateAdapters().begin());
}

struct Direct3D11Instance {
	ATL::CComPtr<IDXGIFactory1> factory;
	ATL::CComPtr<ID3D11Device> device;
	ATL::CComPtr<ID3D11DeviceContext> context;
};

void* Plugin::API::Direct3D11::CreateInstanceOnAdapter(Adapter adapter) {
	HRESULT hr;

	auto dxgiInst = SingletonDXGI::GetInstance();

	ATL::CComPtr<IDXGIFactory1> dxgiFactory;
	hr = dxgiInst->CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgiFactory);
	if (FAILED(hr)) {
		std::vector<char> buf(1024);
		std::sprintf(buf.data(), "<" __FUNCTION_NAME__ "> Failed to enumerate adapters, error code %X.", hr);
		throw std::exception(buf.data());
	}

	LUID adapterLUID;
	adapterLUID.LowPart = adapter.idLow;
	adapterLUID.HighPart = adapter.idHigh;

	ATL::CComPtr<IDXGIAdapter> dxgiAdapter;
	for (size_t adapterIndex = 0;
		!FAILED(dxgiFactory->EnumAdapters((UINT)adapterIndex, &dxgiAdapter));
		adapterIndex++) {
		DXGI_ADAPTER_DESC desc = DXGI_ADAPTER_DESC();
		dxgiAdapter->GetDesc(&desc);

		if (desc.VendorId != 0x1002 /* AMD */)
			continue;

		if ((desc.AdapterLuid.HighPart == adapterLUID.HighPart)
			&& (desc.AdapterLuid.LowPart == adapterLUID.LowPart)) {
			hr = NOERROR;
			break;
		} else {
			hr = E_INVALIDARG;
		}
	}
	if (FAILED(hr))
		throw std::invalid_argument("adapter");

	// Create D3D Stuff
	auto d3dInst = SingletonD3D11::GetInstance();
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};
	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dContext;
	for (size_t c = 0; c < 3; c++) {
		uint32_t flags = 0;

		switch (c) {
			case 0:
				flags |= D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
			case 1:
				flags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;
			case 2:
				break;
		}

		hr = d3dInst->D3D11CreateDevice(
			dxgiAdapter,
			dxgiAdapter == NULL ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN,
			NULL,
			flags,
			featureLevels + 1, _countof(featureLevels) - 1,
			D3D11_SDK_VERSION,
			&d3dDevice,
			NULL,
			&d3dContext);
		if (SUCCEEDED(hr)) {
			break;
		} else {
			AMF_LOG_WARNING("<" __FUNCTION_NAME__ "> Unable to create D3D11 device, error code %X (mode %Iu).", hr, c);
		}
	}
	if (FAILED(hr)) {
		std::vector<char> buf(1024);
		std::sprintf(buf.data(), "<" __FUNCTION_NAME__ "> Unable to create D3D11 device, error code %X.", hr);
		throw std::exception(buf.data());
	}

	Direct3D11Instance* instance = new Direct3D11Instance();
	instance->factory = dxgiFactory;
	instance->device = d3dDevice;
	instance->context = d3dContext;
	return instance;
}

Plugin::API::Adapter Plugin::API::Direct3D11::GetAdapterForInstance(void* pInstance) {
	HRESULT hr;

	if (pInstance == nullptr)
		throw std::invalid_argument("instance");

	Direct3D11Instance* instance = static_cast<Direct3D11Instance*>(pInstance);
	if (instance == nullptr)
		throw std::invalid_argument("instance");

	ATL::CComPtr<IDXGIAdapter> dxgiAdapter;
	hr = instance->device->QueryInterface(&dxgiAdapter);
	if (FAILED(hr)) {
		std::vector<char> buf(1024);
		std::sprintf(buf.data(), "<" __FUNCTION_NAME__ "> Failed to query Adapter from D3D11 device, error code %X.", hr);
		throw std::exception(buf.data());
	}

	DXGI_ADAPTER_DESC adapterDesc;
	hr = dxgiAdapter->GetDesc(&adapterDesc);
	if (FAILED(hr)) {
		std::vector<char> buf(1024);
		std::sprintf(buf.data(), "<" __FUNCTION_NAME__ "> Failed to get description from DXGI adapter, error code %X.", hr);
		throw std::exception(buf.data());
	}

	std::vector<char> descBuf(256);
	wcstombs(descBuf.data(), adapterDesc.Description, descBuf.size());

	return Adapter(
		adapterDesc.AdapterLuid.LowPart,
		adapterDesc.AdapterLuid.HighPart,
		std::string(descBuf.data())
	);
}

void* Plugin::API::Direct3D11::GetContextFromInstance(void* pInstance) {
	if (pInstance == nullptr)
		throw std::invalid_argument("instance");

	Direct3D11Instance* instance = static_cast<Direct3D11Instance*>(pInstance);
	if (instance == nullptr)
		throw std::invalid_argument("instance");

	return instance->device;
}

void Plugin::API::Direct3D11::DestroyInstance(void* pInstance) {
	if (pInstance == nullptr)
		throw std::invalid_argument("instance");

	Direct3D11Instance* instance = static_cast<Direct3D11Instance*>(pInstance);
	if (instance == nullptr)
		throw std::invalid_argument("instance");

	delete instance;
}

Plugin::API::Type Plugin::API::Direct3D11::GetType() {
	return Type::Direct3D11;
}
