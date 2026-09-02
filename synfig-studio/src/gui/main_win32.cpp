/*
For general Scribus (>=1.3.2) copyright and licensing information please refer
to the COPYING file provided with the program. Following this notice may exist
a copyright and/or license notice that predates the release of Scribus 1.3.2
for which a new license (GPL+exception) is in place.
*/
/***************************************************************************
						main.cpp  -  description
							-------------------
	begin                : Fre Apr  6 21:47:55 CEST 2001
	copyright            : (C) 2001 by Franz Schmid
	email                : Franz.Schmid@altmuehlnet.de
	copyright            : (C) 2004 by Alessandro Rimoldi
	email                : http://ideale.ch/contact
	copyright            : (C) 2005 by Craig Bradney
	email                : cbradney@zip.com.au
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifdef _WIN32

#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#define MAX_LINES 500

#include <windows.h>
#include <wincon.h>

#include "exchndl.h"
#include <ctime>
#include <vector>
#include <synfig/filesystem_path.h>
#include <synfig/general.h>
#include <synfig/version.h>
#include "localization.h"

void redirectIOToConsole()
{
	int hConHandle;
	HANDLE lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate console
	if( GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE )
		AllocConsole();
	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = MAX_LINES;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
	//redirect unbuffered STDOUT to the console
	lStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle((intptr_t) lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, nullptr, _IONBF, 0 );
	// redirect unbuffered STDIN to the console
	lStdHandle = GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle((intptr_t) lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, nullptr, _IONBF, 0 );
	// redirect unbuffered STDERR to the console
	lStdHandle = GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle((intptr_t) lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, nullptr, _IONBF, 0 );
	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog 
	// point to console as well
	std::ios::sync_with_stdio();
}

static std::string
get_current_timestamp()
{
	std::time_t current_time = std::time(nullptr);
	std::string time_str(30, 0);
	std::strftime(&time_str[0], time_str.size(), "%Y-%m-%d.%H-%M-%S", std::localtime(&current_time));
	return time_str;
}

static bool
is_directory_exists(LPCWSTR path)
{
	DWORD dwAttrib = GetFileAttributesW(path);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
			(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

static bool
create_directory_recursive(const synfig::filesystem::Path& filename)
{
	wchar_t folder[MAX_PATH];
	ZeroMemory(folder, MAX_PATH * sizeof(wchar_t));

	wchar_t* end = wcschr(filename.c_str(), L'/');

	while (end != NULL) {
		wcsncpy(folder, filename.c_str(), end - filename.c_str() + 1);
		if (!is_directory_exists(folder)) {
			if (!CreateDirectoryW(folder, NULL)) {
				if (GetLastError() != ERROR_ALREADY_EXISTS) {
					synfig::error(_("Couldn't create directory %s (for %s)"), synfig::filesystem::Path::from_native(folder).u8_str(), filename.u8_str());
					return false;
				}
			}
		}
		end = wcschr(++end, L'/');
	}
	return true;
}

void
register_crash_report()
{
	std::vector<synfig::filesystem::Path> possible_lib_paths = {{"."}};
	HMODULE hModule = nullptr;

	for (const auto& path : possible_lib_paths) {
		hModule = LoadLibraryW((path.native() + L"/exchndl.dll").c_str());
		if (hModule)
			break;
	}

	if (!hModule) {
		synfig::warning(_("Couldn't find library exchndl.dll to crash report, if any"));
		return;
	}

	typedef void (*PFNEXCHNDLINIT)();
	PFNEXCHNDLINIT pfnExcHndlInit = (PFNEXCHNDLINIT)GetProcAddress(hModule, "ExcHndlInit");

	if (!pfnExcHndlInit) {
		synfig::warning(_("Couldn't find entrypoint ExcHndlInit in library exchndl.dll"));
		return;
	}

	pfnExcHndlInit();

	std::wstring localappdata = _wgetenv(L"LOCALAPPDATA");
	if (!localappdata.empty()) {
		auto filename = synfig::filesystem::Path::from_native(localappdata);
		filename /= synfig::filesystem::Path(synfig::strprintf("synfig/crashes/synfig-%s-%s.RPT", synfig::get_version(), get_current_timestamp().c_str()));

		bool has_specific_filepath = create_directory_recursive(filename);

		if (has_specific_filepath) {
			typedef BOOL (*PFEXCHNDLSETLOGFILENAMEW)(const WCHAR*);
			PFEXCHNDLSETLOGFILENAMEW pfExcHndlSetLogFileNameW = (PFEXCHNDLSETLOGFILENAMEW)GetProcAddress(hModule, "ExcHndlSetLogFileNameW");
			if (pfExcHndlSetLogFileNameW) {
				if (pfExcHndlSetLogFileNameW(filename.c_str()))
					synfig::info(_("If a crash occurs, refer to file %s"), filename.u8_str());
			}
		}
	}
}

#endif /* WIN32 */
