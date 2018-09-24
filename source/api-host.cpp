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

#include "api-host.hpp"

using namespace Plugin::API;

std::string Plugin::API::Host::GetName()
{
	return "Host";
}

Plugin::API::Type Plugin::API::Host::GetType()
{
	return Type::Host;
}

std::vector<Adapter> Plugin::API::Host::EnumerateAdapters()
{
	std::vector<Adapter> list;
	list.push_back(Adapter(0, 0, "Default"));
	return list;
}

std::shared_ptr<Instance> Plugin::API::Host::CreateInstance(Adapter adapter)
{
	return std::make_unique<HostInstance>();
}

Plugin::API::Adapter Plugin::API::HostInstance::GetAdapter()
{
	return Adapter(0, 0, "Default");
}

void* Plugin::API::HostInstance::GetContext()
{
	return nullptr;
}
