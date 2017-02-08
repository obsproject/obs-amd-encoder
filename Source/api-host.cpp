/*
MIT License

Copyright (c) 2016-2017

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

#include "api-host.h"

using namespace Plugin::API;

std::string Plugin::API::Host::GetName() {
	return "Host";
}

Plugin::API::Type Plugin::API::Host::GetType() {
	return Type::Host;
}

std::vector<Adapter> Plugin::API::Host::EnumerateAdapters() {
	std::vector<Adapter> list;
	list.push_back(Adapter(0, 0, "Default"));
	return list;
}

std::shared_ptr<Instance> Plugin::API::Host::CreateInstance(Adapter adapter) {
	return std::make_unique<HostInstance>();
}

Plugin::API::Adapter Plugin::API::HostInstance::GetAdapter() {
	return Adapter(0, 0, "Default");
}

void* Plugin::API::HostInstance::GetContext() {
	return nullptr;
}
