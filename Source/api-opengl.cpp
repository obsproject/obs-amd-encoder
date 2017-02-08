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

#include "api-opengl.h"
#include <vector>

using namespace Plugin::API;

Plugin::API::OpenGL::OpenGL() {
	// ToDo: Adapter enumeration needs to go by Display/Desktop.
	// - Nvidia is the only one that has GPU Affinity extension.
	// - Intel perhaps too since they used Nvidia technology. (Until recently at least)
}

Plugin::API::OpenGL::~OpenGL() {

}

std::string Plugin::API::OpenGL::GetName() {
	return std::string("OpenGL");
}

Plugin::API::Type Plugin::API::OpenGL::GetType() {
	return Type::OpenGL;
}

std::vector<Adapter> Plugin::API::OpenGL::EnumerateAdapters() {
	std::vector<Adapter> adapters;
	adapters.push_back(Adapter(0, 0, "Default"));
	return adapters;
}

std::shared_ptr<Instance> Plugin::API::OpenGL::CreateInstance(Adapter adapter) {
	// ToDo: Actually create a hidden window and OpenGL context. Not that it is going to be useful.
	return std::make_unique<OpenGLInstance>();
}

Plugin::API::OpenGLInstance::OpenGLInstance() {

}

Plugin::API::OpenGLInstance::~OpenGLInstance() {

}

Plugin::API::Adapter Plugin::API::OpenGLInstance::GetAdapter() {
	return Adapter(0, 0, "Default");
}

void* Plugin::API::OpenGLInstance::GetContext() {
	return nullptr;
}
