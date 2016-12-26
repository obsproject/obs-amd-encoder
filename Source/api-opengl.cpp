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
#include "api-opengl.h"

#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif
#include <gl/GL.h>

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin::API;

std::string Plugin::API::OpenGL::GetName() {
	return std::string("OpenGL");
}

std::vector<Adapter> Plugin::API::OpenGL::EnumerateAdapters() {
	std::vector<Adapter> adapters;
	adapters.push_back(Adapter(0, 0, "Default"));
	return adapters;
}

Plugin::API::Adapter Plugin::API::OpenGL::GetAdapterById(uint32_t idLow, uint32_t idHigh) {
	for (auto adapter : EnumerateAdapters()) {
		if ((adapter.idLow == idLow) && (adapter.idHigh == idHigh))
			return adapter;
	}
	return *(EnumerateAdapters().begin());
}

Plugin::API::Adapter Plugin::API::OpenGL::GetAdapterByName(std::string name) {
	for (auto adapter : EnumerateAdapters()) {
		if (adapter.Name == name)
			return adapter;
	}
	return *(EnumerateAdapters().begin());
}

void* Plugin::API::OpenGL::CreateInstanceOnAdapter(Adapter adapter) {
	return nullptr;
}

void Plugin::API::OpenGL::DestroyInstance(void* instance) {
	return;
}

Plugin::API::Adapter Plugin::API::OpenGL::GetAdapterForInstance(void* instance) {
	return *(EnumerateAdapters().begin());
}

void* Plugin::API::OpenGL::GetContextFromInstance(void* instance) {
	return nullptr;
}

Plugin::API::Type Plugin::API::OpenGL::GetType() {
	return Type::OpenGL;
}
