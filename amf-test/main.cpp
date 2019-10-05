/*
 * A Plugin that integrates the AMD AMF encoder into OBS Studio
 * Copyright (C) 2016 - 2017 Michael Fabian Dirks
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

#include <iostream>
#include "amf-capabilities.hpp"
#include "amf.hpp"
#include "api-base.hpp"

#if defined(_WIN32) || defined(_WIN64)
extern "C" {
#include <windows.h>
}
#endif

using namespace Plugin;
using namespace Plugin::AMD;

int main(int argc, char* argv[])
{
	argc;
	argv;

#if defined(_WIN32) || defined(_WIN64)
	SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS);
#endif

	try {
		AMF::Initialize();
		API::InitializeAPIs();
		CapabilityManager::Initialize();
		CapabilityManager::Finalize();
		API::FinalizeAPIs();
		AMF::Finalize();
		return 0;
	} catch (std::exception& ex) {
		printf("[AMF] %s", ex.what());
		fflush(NULL);
		return 1;
	} catch (...) {
		printf("[AMF] Unknown Error");
		fflush(NULL);
		return 2;
	}
}
