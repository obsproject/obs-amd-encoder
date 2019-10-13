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
#include "plugin.hpp"
#include <sstream>
#include "amf-capabilities.hpp"
#include "amf.hpp"
#include "api-base.hpp"
#include "enc-h264.hpp"
#include "enc-h265.hpp"

#pragma warning(push)
#pragma warning(disable : 4201)
extern "C" {
#include <obs-module.h>
#include <util/pipe.h>
#include <util/platform.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
}
#pragma warning(pop)

using namespace Plugin;
using namespace Plugin::AMD;

#if defined(_WIN32) || defined(_WIN64)
BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID)
{
	return TRUE;
}

static inline bool create_process(const char* cmd_line, HANDLE stdout_handle, HANDLE* process)
{
	PROCESS_INFORMATION pi         = {0};
	wchar_t*            cmd_line_w = NULL;
	STARTUPINFOW        si         = {0};
	bool                success    = false;

	si.cb         = sizeof(si);
	si.dwFlags    = STARTF_USESTDHANDLES;
	si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
	si.hStdOutput = stdout_handle;
	si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);

	os_utf8_to_wcs_ptr(cmd_line, 0, &cmd_line_w);
	if (cmd_line_w) {
		success = !!CreateProcessW(NULL, cmd_line_w, NULL, NULL, true, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

		if (success) {
			*process = pi.hProcess;
			CloseHandle(pi.hThread);
		}

		bfree(cmd_line_w);
	}

	return success;
}

static bool create_pipe(HANDLE* input, HANDLE* output)
{
	SECURITY_ATTRIBUTES sa = {0};

	sa.nLength        = sizeof(sa);
	sa.bInheritHandle = true;

	if (!CreatePipe(input, output, &sa, 0)) {
		return false;
	}

	return true;
}
#endif

OBS_DECLARE_MODULE();
OBS_MODULE_AUTHOR("Michael Fabian Dirks");
OBS_MODULE_USE_DEFAULT_LOCALE("enc-amf", "en-US");

MODULE_EXPORT bool obs_module_load(void)
{
	try {
		PLOG_DEBUG("<%s> Loading...", __FUNCTION_NAME__);

#ifdef _WIN32
		// Out-of-process AMF Test
		{
			unsigned long returnCode = 0xFFFFFFFF;
			HANDLE        hProcess, hRead, hReadImp, hOut;
			char*         path = obs_module_file("enc-amf-test" BIT_STR ".exe");

			if (!create_pipe(&hReadImp, &hOut)) {
				PLOG_ERROR("Failed to create pipes for AMF test.");
				bfree(path);
				return false;
			}
			if (!DuplicateHandle(GetCurrentProcess(), hReadImp, GetCurrentProcess(), &hRead, 0, FALSE,
								 DUPLICATE_SAME_ACCESS)) {
				CloseHandle(hReadImp);
				CloseHandle(hOut);
				PLOG_ERROR("Failed to modify pipes for AMF test.");
				bfree(path);
				return false;
			};
			CloseHandle(hReadImp);
			if (!create_process(path, hOut, &hProcess)) {
				CloseHandle(hRead);
				CloseHandle(hOut);
				PLOG_ERROR("Failed to start AMF test subprocess.");
				bfree(path);
				return false;
			}

			CloseHandle(hOut);
			bfree(path);

			char  buf[1024];
			DWORD bufread = 0;
			while (ReadFile(hRead, buf, sizeof(buf), &bufread, NULL)) {
				blog(LOG_ERROR, "%.*s", bufread, buf);
			}

			if (WaitForSingleObject(hProcess, 2000) == WAIT_OBJECT_0)
				GetExitCodeProcess(hProcess, &returnCode);

			CloseHandle(hProcess);
			CloseHandle(hRead);

			switch (returnCode) {
			case STATUS_ACCESS_VIOLATION:
			case STATUS_ARRAY_BOUNDS_EXCEEDED:
			case STATUS_BREAKPOINT:
			case STATUS_DATATYPE_MISALIGNMENT:
			case STATUS_FLOAT_DENORMAL_OPERAND:
			case STATUS_FLOAT_DIVIDE_BY_ZERO:
			case STATUS_FLOAT_INEXACT_RESULT:
			case STATUS_FLOAT_INVALID_OPERATION:
			case STATUS_FLOAT_OVERFLOW:
			case STATUS_FLOAT_STACK_CHECK:
			case STATUS_FLOAT_UNDERFLOW:
			case STATUS_GUARD_PAGE_VIOLATION:
			case STATUS_ILLEGAL_INSTRUCTION:
			case STATUS_IN_PAGE_ERROR:
			case STATUS_INTEGER_DIVIDE_BY_ZERO:
			case STATUS_INTEGER_OVERFLOW:
			case STATUS_INVALID_DISPOSITION:
			case STATUS_INVALID_HANDLE:
			case STATUS_NONCONTINUABLE_EXCEPTION:
			case STATUS_PRIVILEGED_INSTRUCTION:
			case STATUS_SINGLE_STEP:
			case STATUS_STACK_OVERFLOW:
			case STATUS_UNWIND_CONSOLIDATE:
			default:
				PLOG_ERROR("A critical error occurred during AMF Testing.");
				return false;
			case 2:
			case 1:
				PLOG_ERROR("AMF Test failed due to one or more errors.");
				return false;
			case 0:
				break;
			}
		}
#endif

		// AMF
		Plugin::AMD::AMF::Initialize();

		// Initialize Graphics APIs
		Plugin::API::InitializeAPIs();

		// AMF Capabilities
		Plugin::AMD::CapabilityManager::Initialize();

		// Register Encoders
		Plugin::Interface::H264Interface::encoder_register();
		Plugin::Interface::H265Interface::encoder_register();

		PLOG_DEBUG("<%s> Loaded.", __FUNCTION_NAME__);
		return true;
	} catch (const std::exception& ex) {
		PLOG_ERROR("Failed to load due to error: %s", ex.what());
	} catch (...) {
		PLOG_ERROR("Failed to load with unknown error.");
	}
	return false;
}

/** Optional: Called when the module is unloaded.  */
MODULE_EXPORT void obs_module_unload(void)
{
	try {
		Plugin::AMD::CapabilityManager::Finalize();
		Plugin::API::FinalizeAPIs();
		Plugin::AMD::AMF::Finalize();
	} catch (const std::exception& ex) {
		PLOG_ERROR("Failed to unload due to error: %s", ex.what());
	} catch (...) {
		PLOG_ERROR("An unexpected unknown exception was thrown during unloading. Please report this as a bug.");
	}
}

/** Optional: Returns the full name of the module */
MODULE_EXPORT const char* obs_module_name()
{
	return "AMD Advanced Media Framework Plugin";
}

/** Optional: Returns a description of the module */
MODULE_EXPORT const char* obs_module_description()
{
	return "AMD Advanced Media Framework Plugin";
}
