/*
 * A Plugin that integrates the AMD AMF encoder into OBS Studio
 * Copyright (C) 2016 - 2018 Michael Fabian Dirks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "api-d3d9.hpp"
#include <cinttypes>
#include <list>
#include <mutex>

using namespace Plugin::API;

Plugin::API::Direct3D9::Direct3D9()
{
	HRESULT hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &m_Direct3D9Ex);
	if (FAILED(hr)) {
		std::vector<char> buf(1024);
		snprintf(buf.data(), buf.size(), "<%s> Failed to create D3D9Ex, error code %X.", __FUNCTION_NAME__, hr);
		throw std::exception(buf.data());
	}

	std::list<LUID>        enumeratedLUIDs;
	D3DADAPTER_IDENTIFIER9 adapterIdentifier;
	for (size_t adapterIndex = 0;
		 !FAILED(m_Direct3D9Ex->GetAdapterIdentifier((UINT)adapterIndex, 0, &adapterIdentifier)); adapterIndex++) {
		if (adapterIdentifier.VendorId != 0x1002 /* AMD */)
			continue;

		LUID adapterLUID;
		if (FAILED(m_Direct3D9Ex->GetAdapterLUID((UINT)adapterIndex, &adapterLUID)))
			continue;

		bool enumerated = false;
		for (LUID enumeratedLUID : enumeratedLUIDs) {
			if ((enumeratedLUID.LowPart == adapterLUID.LowPart) && (enumeratedLUID.HighPart == adapterLUID.HighPart)) {
				enumerated = true;
				break;
			}
		}
		if (enumerated)
			continue;
		else
			enumeratedLUIDs.push_back(adapterLUID);

		std::vector<char> buf(1024);
		snprintf(buf.data(), buf.size(), "%s [%s] (VEN_%04x/DEV_%04x/SUB_%04x/REV_%04x)", adapterIdentifier.Description,
				 adapterIdentifier.DeviceName,

				 adapterIdentifier.VendorId, adapterIdentifier.DeviceId, adapterIdentifier.SubSysId,
				 adapterIdentifier.Revision);

		m_Adapters.emplace_back(Adapter(adapterLUID.LowPart, adapterLUID.HighPart, std::string(buf.data())));
	}
}

Plugin::API::Direct3D9::~Direct3D9()
{
	//m_InstanceMap.clear(); // Need to destroy IDirect3D9Device9Ex before IDirect3D9Ex.
	m_Direct3D9Ex->Release();
}

std::string Plugin::API::Direct3D9::GetName()
{
	return std::string("Direct3D 9");
}

Plugin::API::Type Plugin::API::Direct3D9::GetType()
{
	return Type::Direct3D9;
}

std::vector<Adapter> Plugin::API::Direct3D9::EnumerateAdapters()
{
	return m_Adapters;
}

std::shared_ptr<Instance> Plugin::API::Direct3D9::CreateInstance(Adapter adapter)
{
	//std::pair<int32_t, int32_t> key = std::make_pair(adapter.idLow, adapter.idHigh);
	//auto inst = m_InstanceMap.find(key);
	//if (inst != m_InstanceMap.end())
	//	return inst->second;

	auto inst2 = std::make_shared<Direct3D9Instance>(this, adapter);
	//m_InstanceMap.insert_or_assign(key, inst2);
	return inst2;
}

Plugin::API::Direct3D9Instance::Direct3D9Instance(Direct3D9* api, Adapter adapter)
	: m_API(api), m_Adapter(adapter), m_Device(nullptr)
{
	size_t                 adapterNum = (size_t)-1;
	D3DADAPTER_IDENTIFIER9 adapterIdentifier;
	for (size_t adapterIndex = 0;
		 !FAILED(api->m_Direct3D9Ex->GetAdapterIdentifier((UINT)adapterIndex, 0, &adapterIdentifier)); adapterIndex++) {
		if (adapterIdentifier.VendorId != 0x1002 /* AMD */)
			continue;

		LUID adapterLUID;
		if (FAILED(api->m_Direct3D9Ex->GetAdapterLUID((UINT)adapterIndex, &adapterLUID)))
			continue;

		if ((static_cast<int32_t>(adapterLUID.LowPart) == adapter.idLow)
			&& (static_cast<int32_t>(adapterLUID.HighPart) == adapter.idHigh)) {
			adapterNum = adapterIndex;
			break;
		}
	}
	if (adapterNum == -1)
		throw std::invalid_argument("adapter");

	D3DPRESENT_PARAMETERS presentParameters;
	std::memset(&presentParameters, 0, sizeof(D3DPRESENT_PARAMETERS));
	presentParameters.BackBufferWidth      = 0;
	presentParameters.BackBufferHeight     = 0;
	presentParameters.BackBufferFormat     = D3DFMT_UNKNOWN;
	presentParameters.Windowed             = TRUE;
	presentParameters.SwapEffect           = D3DSWAPEFFECT_COPY;
	presentParameters.hDeviceWindow        = GetDesktopWindow();
	presentParameters.Flags                = D3DPRESENTFLAG_VIDEO;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	D3DCAPS9 ddCaps;
	std::memset(&ddCaps, 0, sizeof(ddCaps));
	HRESULT hr = api->m_Direct3D9Ex->GetDeviceCaps((UINT)adapterNum, D3DDEVTYPE_HAL, &ddCaps);
	if (FAILED(hr)) {
		std::vector<char> buf(1024);
		snprintf(buf.data(), buf.size(), "<%s> Unable to query capabilities for D3D9 adapter, error code %X.",
				 __FUNCTION_NAME__, hr);
		throw std::exception(buf.data());
	}

	DWORD vp = 0;
	if (ddCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	} else {
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	hr = api->m_Direct3D9Ex->CreateDeviceEx((UINT)adapterNum, D3DDEVTYPE_HAL, presentParameters.hDeviceWindow,
											vp | D3DCREATE_NOWINDOWCHANGES | D3DCREATE_MULTITHREADED,
											&presentParameters, NULL, &m_Device);
	if (FAILED(hr)) {
		std::vector<char> buf(1024);
		snprintf(buf.data(), buf.size(), "<%s> Unable to create D3D9 device, error code %X.", __FUNCTION_NAME__, hr);
		throw std::exception(buf.data());
	}
}

Plugin::API::Direct3D9Instance::~Direct3D9Instance()
{
	//std::pair<int32_t, int32_t> key = std::make_pair(m_Adapter.idLow, m_Adapter.idHigh);
	//m_API->m_InstanceMap.erase(key);

	//m_Device->Release(); // Can't release/free on AMD hardware?
}

Plugin::API::Adapter Plugin::API::Direct3D9Instance::GetAdapter()
{
	/*if (pInstance == nullptr)
		throw std::invalid_argument("instance");

	Direct3D9Instance* instance = static_cast<Direct3D9Instance*>(pInstance);
	if (instance == nullptr)
		throw std::invalid_argument("instance");

	D3DDEVICE_CREATION_PARAMETERS par;
	HRESULT hr = instance->device->GetCreationParameters(&par);
	if (FAILED(hr)) {
		std::vector<char> buf(1024);
		snprintf(buf.data(), "<%s> Unable to get adapter from D3D9 device, error code %X.", __FUNCTION_NAME__, hr);
		throw std::exception(buf.data());
	}

	auto adapters = Direct3D9::EnumerateAdapters();
	if (par.AdapterOrdinal > adapters.size())
		return *adapters.begin();

	auto adapter = adapters.begin();
	for (size_t n = 0; n < par.AdapterOrdinal; n++)
		adapter++;
	return *adapter;*/
	return m_Adapter;
}

void* Plugin::API::Direct3D9Instance::GetContext()
{
	return m_Device;
}
