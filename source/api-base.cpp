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

#include "api-base.hpp"
#include <cinttypes>
#include "api-d3d11.hpp"
#include "api-d3d9.hpp"
#include "api-host.hpp"
#include "api-opengl.hpp"

#if defined(_WIN32) || defined(_WIN64)
extern "C" {
#include <VersionHelpers.h>
#include <windows.h>
}
#endif

using namespace Plugin::API;

// An Adapter on an API
bool Plugin::API::operator<(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right)
{
	if (left == right)
		return left.Name < right.Name;
	else
		return (((uint64_t)left.idLow + ((uint64_t)left.idHigh << 32))
				< ((uint64_t)right.idLow + ((uint64_t)right.idHigh << 32)));
}

bool Plugin::API::operator>(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right)
{
	return right < left;
}

bool Plugin::API::operator<=(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right)
{
	return !(right < left);
}

bool Plugin::API::operator>=(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right)
{
	return !(left < right);
}

bool Plugin::API::operator==(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right)
{
	return ((left.idLow == right.idLow) && (right.idHigh == right.idHigh));
}

bool Plugin::API::operator!=(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right)
{
	return !(left == right);
}

// Instance of an API Adapter
Plugin::API::Instance::Instance() {}

Plugin::API::Instance::~Instance() {}

// API Interface
Plugin::API::IAPI::IAPI() {}

Plugin::API::IAPI::~IAPI() {}

Plugin::API::Adapter Plugin::API::IAPI::GetAdapterById(const int32_t idLow, const int32_t idHigh)
{
	for (auto adapter : EnumerateAdapters()) {
		if ((adapter.idLow == idLow) && (adapter.idHigh == idHigh))
			return adapter;
	}
	return *(EnumerateAdapters().begin());
}

Plugin::API::Adapter Plugin::API::IAPI::GetAdapterByName(const std::string& name)
{
	for (auto adapter : EnumerateAdapters()) {
		if (adapter.Name == name)
			return adapter;
	}
	return *(EnumerateAdapters().begin());
}

// Static API Stuff
static std::vector<std::shared_ptr<IAPI>> s_APIInstances;
void                                      Plugin::API::InitializeAPIs()
{
#ifdef _WIN32
	if (IsWindows8OrGreater()) {
		// DirectX 11
		try {
			s_APIInstances.insert(s_APIInstances.end(), std::make_shared<Direct3D11>());
		} catch (const std::exception& PLOG_VAR(ex)) {
			PLOG_WARNING("Direct3D 11 is not supported due to error: %s", ex.what());
		} catch (...) {
			PLOG_WARNING("Direct3D 11 not supported.");
		}
	} else if (IsWindowsXPOrGreater()) {
		// Direct3D 9
		try {
			s_APIInstances.insert(s_APIInstances.end(), std::make_shared<Direct3D9>());
		} catch (const std::exception& PLOG_VAR(ex)) {
			PLOG_WARNING("Direct3D 9 is not supported due to error: %s", ex.what());
		} catch (...) {
			PLOG_WARNING("Direct3D 9 not supported.");
		}
	}
#endif

	// Mikhail says these are for compatibility only, not actually backends.
	//// OpenGL
	//{
	//	s_APIInstances.insert(s_APIInstances.end(), std::make_shared<OpenGL>());
	//}

	//// Host
	//{
	//	s_APIInstances.insert(s_APIInstances.end(), std::make_shared<Host>());
	//}
}

void Plugin::API::FinalizeAPIs()
{
	s_APIInstances.clear();
}

size_t Plugin::API::CountAPIs()
{
	return s_APIInstances.size();
}

std::string Plugin::API::GetAPIName(size_t index)
{
	if (index >= s_APIInstances.size())
		throw std::exception("Invalid API Index");

	return s_APIInstances[index].get()->GetName();
}

std::shared_ptr<IAPI> Plugin::API::GetAPI(size_t index)
{
	if (index >= s_APIInstances.size())
		throw std::exception("Invalid API Index");

	return s_APIInstances[index];
}

std::shared_ptr<IAPI> Plugin::API::GetAPI(const std::string& name)
{
	for (auto api : s_APIInstances) {
		if (name == api->GetName()) {
			return api;
		}
	}
	// If none was found, return the first one.
	return *s_APIInstances.begin();
}

std::shared_ptr<IAPI> Plugin::API::GetAPI(Type type)
{
	for (auto api : s_APIInstances) {
		if (type == api->GetType()) {
			return api;
		}
	}
	// If none was found, return the first one.
	return *s_APIInstances.begin();
}

std::vector<std::shared_ptr<IAPI>> Plugin::API::EnumerateAPIs()
{
	return std::vector<std::shared_ptr<IAPI>>(s_APIInstances);
}

std::vector<std::string> Plugin::API::EnumerateAPINames()
{
	std::vector<std::string> names;
	for (auto api : s_APIInstances) {
		names.push_back(api->GetName());
	}
	return names;
}
