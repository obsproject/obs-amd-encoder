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

#pragma once
#include <map>
#include <memory>
#include <string.h>
#include <vector>
#include "plugin.hpp"

namespace Plugin {
	namespace API {
		enum class Type : uint8_t {
			Host,
			Direct3D9,
			Direct3D11,
			OpenGL,
		};

		// An Adapter on an API
		struct Adapter {
			int32_t     idLow, idHigh;
			std::string Name;

			Adapter() : idLow(0), idHigh(0), Name("Invalid Device") {}
			Adapter(const int32_t p_idLow, const int32_t p_idHigh, const std::string& p_Name)
				: idLow(p_idLow), idHigh(p_idHigh), Name(p_Name)
			{}
			Adapter(Adapter const& o) : Name(o.Name), idLow(o.idLow), idHigh(o.idHigh) {}
			void operator=(Adapter const& o)
			{
				idLow  = o.idLow;
				idHigh = o.idHigh;
				Name   = o.Name;
			}

			friend bool operator<(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
			friend bool operator>(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
			friend bool operator<=(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
			friend bool operator>=(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);

			friend bool operator==(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
			friend bool operator!=(const Plugin::API::Adapter& left, const Plugin::API::Adapter& right);
		};

		// Instance of an API Adapter
		struct Instance {
			public:
			Instance();
			virtual ~Instance();

			virtual Adapter GetAdapter() = 0;
			virtual void*   GetContext() = 0;
		};

		// API Interface
		class IAPI {
			public:
			IAPI();
			virtual ~IAPI();

			virtual std::string GetName() = 0;
			virtual Type        GetType() = 0;

			virtual std::vector<Adapter> EnumerateAdapters() = 0;
			Adapter                      GetAdapterById(const int32_t idLow, const int32_t idHigh);
			Adapter                      GetAdapterByName(const std::string& name);

			virtual std::shared_ptr<Instance> CreateInstance(Adapter adapter) = 0;
		};

		// Static API Stuff
		void                               InitializeAPIs();
		void                               FinalizeAPIs();
		size_t                             CountAPIs();
		std::string                        GetAPIName(size_t index);
		std::shared_ptr<IAPI>              GetAPI(size_t index);
		std::shared_ptr<IAPI>              GetAPI(const std::string& name);
		std::shared_ptr<IAPI>              GetAPI(Type type);
		std::vector<std::shared_ptr<IAPI>> EnumerateAPIs();
		std::vector<std::string>           EnumerateAPINames();
	} // namespace API
} // namespace Plugin
