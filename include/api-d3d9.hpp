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
#include <atlutil.h>
#include <d3d9.h>
#include "api-base.hpp"

#ifdef _DEBUG
#define D3D_DEBUG_INFO
#endif
#pragma comment(lib, "d3d9.lib")

namespace Plugin {
	namespace API {
		class Direct3D9 : public IAPI {
			friend class Direct3D9Instance;

			public:
			Direct3D9();
			~Direct3D9();

			virtual std::string               GetName() override;
			virtual Type                      GetType() override;
			virtual std::vector<Adapter>      EnumerateAdapters() override;
			virtual std::shared_ptr<Instance> CreateInstance(Adapter adapter) override;

			protected:
			IDirect3D9Ex* m_Direct3D9Ex;
			//std::map<std::pair<int32_t, int32_t>, std::shared_ptr<Instance>> m_InstanceMap;

			private:
			std::vector<Adapter> m_Adapters;
		};

		class Direct3D9Instance : public Instance {
			public:
			Direct3D9Instance(Direct3D9* api, Adapter adapter);
			~Direct3D9Instance();

			virtual Adapter GetAdapter() override;
			virtual void*   GetContext() override;

			private:
			Direct3D9*          m_API;
			Adapter             m_Adapter;
			IDirect3DDevice9Ex* m_Device;
		};
	} // namespace API
} // namespace Plugin
