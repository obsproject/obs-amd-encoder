/*
 * A Plugin that integrates the AMD AMF encoder into OBS Studio
 * Copyright (C) 2016 - 2017 Michael Fabian Dirks
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

#pragma once
#include "api-base.h"
#include "api-d3d9.h"
#include "api-d3d11.h"
#include "api-host.h"
#include "api-opengl.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <VersionHelpers.h>
#endif

using namespace Plugin::API;


// An Adapter on an API
bool Plugin::API::operator<(const Plugin::API::Adapter & left, const Plugin::API::Adapter & right) {
	if (left == right)
		return left.Name < right.Name;
	else
		return (((uint64_t)left.idLow + ((uint64_t)left.idHigh << 32)) < ((uint64_t)right.idLow + ((uint64_t)right.idHigh << 32)));
}

bool Plugin::API::operator>(const Plugin::API::Adapter & left, const Plugin::API::Adapter & right) {
	return right < left;
}

bool Plugin::API::operator<=(const Plugin::API::Adapter & left, const Plugin::API::Adapter & right) {
	return !(right < left);
}

bool Plugin::API::operator>=(const Plugin::API::Adapter & left, const Plugin::API::Adapter & right) {
	return !(left < right);
}

bool Plugin::API::operator==(const Plugin::API::Adapter & left, const Plugin::API::Adapter & right) {
	return ((left.idLow == right.idLow) && (right.idHigh == right.idHigh));
}

bool Plugin::API::operator!=(const Plugin::API::Adapter & left, const Plugin::API::Adapter & right) {
	return !(left == right);
}

// Instance of an API Adapter
Plugin::API::Instance::Instance() {}

Plugin::API::Instance::~Instance() {}

// API Interface
Plugin::API::IAPI::IAPI() {}

Plugin::API::IAPI::~IAPI() {}

Plugin::API::Adapter Plugin::API::IAPI::GetAdapterById(int32_t idLow, int32_t idHigh) {
	for (auto adapter : EnumerateAdapters()) {
		if ((adapter.idLow == idLow) && (adapter.idHigh == idHigh))
			return adapter;
	}
	return *(EnumerateAdapters().begin());
}

Plugin::API::Adapter Plugin::API::IAPI::GetAdapterByName(std::string name) {
	for (auto adapter : EnumerateAdapters()) {
		if (adapter.Name == name)
			return adapter;
	}
	return *(EnumerateAdapters().begin());
}

// Static API Stuff
static std::vector<std::shared_ptr<IAPI>> s_APIInstances;
void Plugin::API::InitializeAPIs() {
	// DirectX 11
	#ifdef _WIN32
	if (IsWindows8OrGreater()) {
		s_APIInstances.insert(s_APIInstances.end(), std::make_shared<Direct3D11>());
	}
	#endif

	// DirectX 9
	#ifdef _WIN32
	if (IsWindowsXPOrGreater()) {
		s_APIInstances.insert(s_APIInstances.end(), std::make_shared<Direct3D9>());
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

void Plugin::API::FinalizeAPIs() {
	s_APIInstances.clear();
}

size_t Plugin::API::CountAPIs() {
	return s_APIInstances.size();
}

std::string Plugin::API::GetAPIName(size_t index) {
	auto indAPI = s_APIInstances.begin();
	indAPI + index; // Advanced by x elements.

	if (indAPI == s_APIInstances.end())
		throw std::exception("Invalid API Index");

	return indAPI->get()->GetName();
}

std::shared_ptr<IAPI> Plugin::API::GetAPI(size_t index) {
	auto indAPI = s_APIInstances.begin();
	indAPI + index; // Advanced by x elements.

	if (indAPI == s_APIInstances.end())
		throw std::exception("Invalid API Index");

	return *indAPI;
}

std::shared_ptr<IAPI> Plugin::API::GetAPI(std::string name) {
	for (auto api : s_APIInstances) {
		if (name == api->GetName()) {
			return api;
		}
	}
	// If none was found, return the first one.
	return *s_APIInstances.begin();
}

std::shared_ptr<IAPI> Plugin::API::GetAPI(Type type) {
	for (auto api : s_APIInstances) {
		if (type == api->GetType()) {
			return api;
		}
	}
	// If none was found, return the first one.
	return *s_APIInstances.begin();
}

std::vector<std::shared_ptr<IAPI>> Plugin::API::EnumerateAPIs() {
	return std::vector<std::shared_ptr<IAPI>>(s_APIInstances);
}

std::vector<std::string> Plugin::API::EnumerateAPINames() {
	std::vector<std::string> names;
	for (auto api : s_APIInstances) {
		names.push_back(api->GetName());
	}
	return names;
}
