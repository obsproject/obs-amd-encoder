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
#include "api-base.h"

#include "api-d3d9.h"
#include "api-d3d11.h"
#include "api-host.h"
#include "api-opengl.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <VersionHelpers.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin::API;

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

//////////////////////////////////////////////////////////////////////////
// API Index
//////////////////////////////////////////////////////////////////////////
static std::vector<std::shared_ptr<Base>> s_APIInstances;

void Plugin::API::Base::Initialize() {
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

	// OpenGL
	{
		s_APIInstances.insert(s_APIInstances.end(), std::make_shared<OpenGL>());
	}

	// Host
	{
		s_APIInstances.insert(s_APIInstances.end(), std::make_shared<Host>());
	}
}

size_t Plugin::API::Base::GetAPICount() {
	return s_APIInstances.size();
}

std::shared_ptr<Base> Plugin::API::Base::GetAPIInstance(size_t index) {
	auto indAPI = s_APIInstances.begin();
	for (size_t n = 0; n < index; n++)
		indAPI++;

	if (indAPI == s_APIInstances.end())
		throw std::exception("Invalid API Index");

	return *indAPI;
}

std::string Plugin::API::Base::GetAPIName(size_t index) {
	auto indAPI = s_APIInstances.begin();
	indAPI + index; // Advanced by x elements.

	if (indAPI == s_APIInstances.end())
		throw std::exception("Invalid API Index");

	return indAPI->get()->GetName();
}


std::shared_ptr<Base> Plugin::API::Base::GetAPIByName(std::string name) {
	for (auto api : s_APIInstances) {
		if (name == api->GetName()) {
			return api;
		}
	}
	// If none was found, return the first one.
	return *s_APIInstances.begin();
}

std::vector<std::shared_ptr<Base>> Plugin::API::Base::EnumerateAPIs() {
	return std::vector<std::shared_ptr<Base>>(s_APIInstances);
}

std::vector<std::string> Plugin::API::Base::EnumerateAPINames() {
	std::vector<std::string> names;
	for (auto api : s_APIInstances) {
		names.push_back(api->GetName());
	}
	return names;
}
