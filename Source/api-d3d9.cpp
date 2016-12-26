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
#include "api-d3d9.h"

#include <mutex>
#include <list>

#ifdef _DEBUG
#define D3D_DEBUG_INFO
#endif
#pragma comment(lib, "d3d9.lib")
#include <d3d9.h>
#include <atlutil.h>

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin::API;

std::string Plugin::API::Direct3D9::GetName() {
	return std::string("Direct3D 9");
}

std::vector<Adapter> Plugin::API::Direct3D9::EnumerateAdapters() {
	ATL::CComPtr<IDirect3D9Ex> pD3DEx;
	HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &pD3DEx);
	if (FAILED(hr))
		throw std::exception("<" __FUNCTION_NAME__ "> Failed to enumerate adapters, error code %X.", hr);

	std::vector<Adapter> adapters;
	std::list<LUID> enumeratedLUIDs;
	D3DADAPTER_IDENTIFIER9 adapterIdentifier;
	for (size_t adapterIndex = 0;
		!FAILED(pD3DEx->GetAdapterIdentifier((UINT)adapterIndex, 0, &adapterIdentifier));
		adapterIndex++) {

		if (adapterIdentifier.VendorId != 0x1002 /* AMD */)
			continue;

		LUID adapterLUID;
		if (FAILED(pD3DEx->GetAdapterLUID((UINT)adapterIndex, &adapterLUID)))
			continue;

		bool enumerated = false;
		for (LUID enumeratedLUID : enumeratedLUIDs) {
			if ((enumeratedLUID.LowPart == adapterLUID.LowPart)
				&& (enumeratedLUID.HighPart == adapterLUID.HighPart)) {
				enumerated = true;
				break;
			}
		}
		if (enumerated)
			continue;
		else
			enumeratedLUIDs.push_back(adapterLUID);

		adapters.emplace_back(
			Adapter(adapterLUID.LowPart, adapterLUID.HighPart,
				std::string(adapterIdentifier.Description)));
	}

	return adapters;
}

Plugin::API::Adapter Plugin::API::Direct3D9::GetAdapterById(uint32_t idLow, uint32_t idHigh) {
	for (auto adapter : EnumerateAdapters()) {
		if ((adapter.idLow == idLow) && (adapter.idHigh == idHigh))
			return adapter;
	}
	return *(EnumerateAdapters().begin());
}

Plugin::API::Adapter Plugin::API::Direct3D9::GetAdapterByName(std::string name) {
	for (auto adapter : EnumerateAdapters()) {
		if (adapter.Name == name)
			return adapter;
	}
	return *(EnumerateAdapters().begin());
}

struct Direct3D9Instance {
	ATL::CComPtr<IDirect3D9Ex> d3d;
	ATL::CComPtr<IDirect3DDevice9Ex> device;
};

void* Plugin::API::Direct3D9::CreateInstanceOnAdapter(Adapter adapter) {
	ATL::CComPtr<IDirect3D9Ex> pD3DEx;
	HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &pD3DEx);
	if (FAILED(hr)) {
		std::vector<char> buf(1024);
		std::sprintf(buf.data(), "<" __FUNCTION_NAME__ "> Unable to create Direct3D 9 instance, error code %X.", hr);
		throw std::exception(buf.data());
	}

	size_t adapterNum = (size_t)-1;
	D3DADAPTER_IDENTIFIER9 adapterIdentifier;
	for (size_t adapterIndex = 0;
		!FAILED(pD3DEx->GetAdapterIdentifier((UINT)adapterIndex, 0, &adapterIdentifier));
		adapterIndex++) {

		if (adapterIdentifier.VendorId != 0x1002 /* AMD */)
			continue;

		LUID adapterLUID;
		if (FAILED(pD3DEx->GetAdapterLUID((UINT)adapterIndex, &adapterLUID)))
			continue;

		if ((adapterLUID.LowPart == adapter.idLow)
			&& (adapterLUID.HighPart == adapter.idHigh)) {
			adapterNum = adapterIndex;
			break;
		}
	}
	if (adapterNum == -1)
		throw std::invalid_argument("adapter");

	D3DPRESENT_PARAMETERS presentParameters;
	std::memset(&presentParameters, 0, sizeof(D3DPRESENT_PARAMETERS));
	presentParameters.BackBufferWidth = 0;
	presentParameters.BackBufferHeight = 0;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_COPY;
	presentParameters.hDeviceWindow = GetDesktopWindow();
	presentParameters.Flags = D3DPRESENTFLAG_VIDEO;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	D3DCAPS9    ddCaps;
	std::memset(&ddCaps, 0, sizeof(ddCaps));
	hr = pD3DEx->GetDeviceCaps((UINT)adapterNum, D3DDEVTYPE_HAL, &ddCaps);
	if (FAILED(hr)) {
		std::vector<char> buf(1024);
		std::sprintf(buf.data(), "<" __FUNCTION_NAME__ "> Unable to query capabilities for D3D9 adapter, error code %X.", hr);
		throw std::exception(buf.data());
	}

	DWORD       vp = 0;
	if (ddCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	} else {
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	ATL::CComPtr<IDirect3DDevice9Ex> pD3DDeviceEx;
	hr = pD3DEx->CreateDeviceEx(
		(UINT)adapterNum,
		D3DDEVTYPE_HAL,
		presentParameters.hDeviceWindow,
		vp | D3DCREATE_NOWINDOWCHANGES | D3DCREATE_MULTITHREADED,
		&presentParameters,
		NULL,
		&pD3DDeviceEx
	);
	if (FAILED(hr)) {
		std::vector<char> buf(1024);
		std::sprintf(buf.data(), "<" __FUNCTION_NAME__ "> Unable to create D3D9 device, error code %X.", hr);
		throw std::exception(buf.data());
	}

	Direct3D9Instance* instance = new Direct3D9Instance();
	instance->d3d = pD3DEx;
	instance->device = pD3DDeviceEx;
	return instance;
}

Plugin::API::Adapter Plugin::API::Direct3D9::GetAdapterForInstance(void* pInstance) {
	if (pInstance == nullptr)
		throw std::invalid_argument("instance");

	Direct3D9Instance* instance = static_cast<Direct3D9Instance*>(pInstance);
	if (instance == nullptr)
		throw std::invalid_argument("instance");

	D3DDEVICE_CREATION_PARAMETERS par;
	HRESULT hr = instance->device->GetCreationParameters(&par);
	if (FAILED(hr)) {
		std::vector<char> buf(1024);
		std::sprintf(buf.data(), "<" __FUNCTION_NAME__ "> Unable to get adapter from D3D9 device, error code %X.", hr);
		throw std::exception(buf.data());
	}

	auto adapters = Direct3D9::EnumerateAdapters();
	if (par.AdapterOrdinal > adapters.size())
		return *adapters.begin();
	
	auto adapter = adapters.begin();
	for (size_t n = 0; n < par.AdapterOrdinal; n++)
		adapter++;
	return *adapter;	
}

void* Plugin::API::Direct3D9::GetContextFromInstance(void* pInstance) {
	if (pInstance == nullptr)
		throw std::invalid_argument("instance");

	Direct3D9Instance* instance = static_cast<Direct3D9Instance*>(pInstance);
	if (instance == nullptr)
		throw std::invalid_argument("instance");

	return instance->device;
}

void Plugin::API::Direct3D9::DestroyInstance(void* pInstance) {
	if (pInstance == nullptr)
		throw std::invalid_argument("instance");

	Direct3D9Instance* instance = static_cast<Direct3D9Instance*>(pInstance);
	if (instance == nullptr)
		throw std::invalid_argument("instance");

	delete instance;
}

Plugin::API::Type Plugin::API::Direct3D9::GetType() {
	return Type::Direct3D9;
}
