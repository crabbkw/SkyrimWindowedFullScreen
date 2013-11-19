// Copyright 2013 Casey Crabb
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.If not, see <http://www.gnu.org/licenses/>.

// A simple utility to (optionally) launch Skyrim and set it to Windowed mode (without window decorations, taking the full screen)
// It will look for skse_loader.exe first, then TESV.exe in the current working directory
// If one is found, it is launched first before searching for a window to fix.


#include "stdafx.h"

using namespace std;

unique_ptr<wstring> GetWindowProcessExecutable(HWND hWnd);
void goFullscreenWindowed(HWND windowHandle);

BOOL CALLBACK enumWindowsProc(HWND windowHandle, LPARAM lParam) {
	int windowTitleLength = GetWindowTextLengthW(windowHandle);

	if (windowTitleLength == 0) {
		return TRUE;
	}

	wchar_t* windowTitle = new wchar_t[windowTitleLength + 1];
	GetWindowText(windowHandle, windowTitle, windowTitleLength + 1);

	wstring wTitle = (windowTitle);
	delete windowTitle;
	windowTitle = NULL;

	int compareResult = wTitle.compare(L"Skyrim");
	if (compareResult != 0) {
		//wcout << "Skipping non Skyrim window: " << wTitle << "\n";
		return TRUE;
	}

	unique_ptr<wstring> exeName = GetWindowProcessExecutable(windowHandle);

	basic_string <wchar_t>::size_type index;

	index = exeName->find(L"TESV.exe", 0);
	if (index != string::npos) {
		wcout << "Going full screen for app " << *exeName << "\n";
		goFullscreenWindowed(windowHandle);
		return FALSE;
	}
	else {
		wcout << "Skipping window because the app name [" << *exeName << "] doesn't contain TESV.exe\n";
	}

	return TRUE;
}

void goFullscreenWindowed(HWND windowHandle) {
	LONG lStyle = GetWindowLong(windowHandle, GWL_STYLE);
	lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
	SetWindowLong(windowHandle, GWL_STYLE, lStyle);

	LONG lExStyle = GetWindowLong(windowHandle, GWL_EXSTYLE);
	lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	SetWindowLong(windowHandle, GWL_EXSTYLE, lExStyle);

	SetWindowPos(windowHandle, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}

unique_ptr<wstring> GetWindowProcessExecutable(HWND hWnd) {
	DWORD dwProcess = 0;
	unique_ptr<wstring> emptyName (new wstring(L""));

	GetWindowThreadProcessId(hWnd, &dwProcess);
	if (0 == dwProcess) { return emptyName; }

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, dwProcess);
	if (NULL == hProcess) { return emptyName; }
	wchar_t exeName[32768];

	BOOL result = GetModuleFileNameExW(hProcess, NULL, exeName, 32768);
	CloseHandle(hProcess);
	hProcess = NULL;

	if (!result) {
		return emptyName;
	}

	//wcout << "name is: " << exeName << "\n";
	return unique_ptr<wstring> (new wstring(exeName));
}

bool exists(const wchar_t *filename) {
	ifstream infile (filename);
	return infile.good();
}

int _tmain(int argc, _TCHAR* argv[]) {
	_PROCESS_INFORMATION procInfo;
	STARTUPINFO startupInfo;
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);

	wchar_t *path = NULL;
	wchar_t *exe = NULL;
	
	if (exists(L"skse_loader.exe")) {
		wcout << "Found skse_loader.exe; using that to launch Skyrim\n";
		exe = L"skse_loader.exe";
	}
	else if (exists(L"TESV.exe")) {
		wcout << "Found TESV.exe; using that to launch Skyrim\n";
		exe = L"TESV.exe";
	}

	wcout.flush();

	// If we have an executable, launch it and wait for a bit.
	if (exe != NULL) {
		BOOL result = CreateProcessW(exe, NULL, NULL, NULL, FALSE, (CREATE_NEW_PROCESS_GROUP | CREATE_UNICODE_ENVIRONMENT), NULL, path, &startupInfo, &procInfo);

		if (result == TRUE) {
			CloseHandle(procInfo.hProcess);
			CloseHandle(procInfo.hThread);
			Sleep(5000);
		}
		else {
			wcout << "Could not start skse_loader.exe or TESV.exe\n";
			wcout << "Still looking for a Skyrim window and process...\n";
		}
	}
	else {
		wcout << "Neither skse_loader.exe nor TESV.exe found in current working directory.\n";
		wcout << "Not launching Skyrim, but still searching to see if it is already running.\n";
	}


	EnumWindows(enumWindowsProc, 0);
	/*
	wstring winTitle = (L"Skyrim");

	HWND win_handle = FindWindow(0, winTitle.c_str());
	*/

	return 0;
}

