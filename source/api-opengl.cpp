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

#include "api-opengl.hpp"
#include <vector>

using namespace Plugin::API;

Plugin::API::OpenGL::OpenGL()
{
	// ToDo: Adapter enumeration needs to go by Display/Desktop.
	// - Nvidia is the only one that has GPU Affinity extension.
	// - Intel perhaps too since they used Nvidia technology. (Until recently at least)
}

Plugin::API::OpenGL::~OpenGL() {}

std::string Plugin::API::OpenGL::GetName()
{
	return std::string("OpenGL");
}

Plugin::API::Type Plugin::API::OpenGL::GetType()
{
	return Type::OpenGL;
}

std::vector<Adapter> Plugin::API::OpenGL::EnumerateAdapters()
{
	std::vector<Adapter> adapters;
	adapters.push_back(Adapter(0, 0, "Default"));
	return adapters;
}

std::shared_ptr<Instance> Plugin::API::OpenGL::CreateInstance(Adapter adapter)
{
	// ToDo: Actually create a hidden window and OpenGL context. Not that it is going to be useful.
	return std::make_unique<OpenGLInstance>();
}

Plugin::API::OpenGLInstance::OpenGLInstance() {}

Plugin::API::OpenGLInstance::~OpenGLInstance() {}

Plugin::API::Adapter Plugin::API::OpenGLInstance::GetAdapter()
{
	return Adapter(0, 0, "Default");
}

void* Plugin::API::OpenGLInstance::GetContext()
{
	return nullptr;
}
