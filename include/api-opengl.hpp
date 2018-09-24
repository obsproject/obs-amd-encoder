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
#include "api-base.hpp"

extern "C" {
#ifdef _WIN32
#include <windows.h>
#endif
#include <gl/GL.h>
}

namespace Plugin {
	namespace API {
		class OpenGL : public IAPI {
			public:
			OpenGL();
			~OpenGL();

			virtual std::string               GetName() override;
			virtual Type                      GetType() override;
			virtual std::vector<Adapter>      EnumerateAdapters() override;
			virtual std::shared_ptr<Instance> CreateInstance(Adapter adapter) override;
		};

		class OpenGLInstance : public Instance {
			public:
			OpenGLInstance();
			~OpenGLInstance();

			virtual Adapter GetAdapter() override;
			virtual void*   GetContext() override;

			private:
			Adapter adapter;
		};
	} // namespace API
} // namespace Plugin
