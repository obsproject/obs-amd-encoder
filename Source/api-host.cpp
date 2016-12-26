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
#include "api-host.h"

//////////////////////////////////////////////////////////////////////////
// Code
//////////////////////////////////////////////////////////////////////////
using namespace Plugin::API;

std::string Plugin::API::Host::GetName() {
	return "Host";
}

Plugin::API::Type Plugin::API::Host::GetType() {
	return Type::Host;
}

std::vector<Adapter> Plugin::API::Host::EnumerateAdapters() {
	std::vector<Adapter> list;
	list.push_back(Adapter(0, 0, TEXT_T(AMF_UTIL_DEFAULT)));
	return list;
}

Plugin::API::Adapter Plugin::API::Host::GetAdapterById(uint32_t idLow, uint32_t idHigh) {
	return Adapter(0, 0, TEXT_T(AMF_UTIL_DEFAULT));
}

Plugin::API::Adapter Plugin::API::Host::GetAdapterByName(std::string name) {
	return Adapter(0, 0, TEXT_T(AMF_UTIL_DEFAULT));
}

void* Plugin::API::Host::CreateInstanceOnAdapter(Adapter adapter) {
	return nullptr;
}

Plugin::API::Adapter Plugin::API::Host::GetAdapterForInstance(void* pInstance) {
	return Adapter(0, 0, TEXT_T(AMF_UTIL_DEFAULT));
}

void* Plugin::API::Host::GetContextFromInstance(void* pInstance) {
	throw std::exception("Host API does not have a Context.");
}

void Plugin::API::Host::DestroyInstance(void* pInstance) {}
