// Win
// Copyright (c) 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <windows.h>
#include <string.h>
#include <stdio.h>

#include <XYO/Win/SimpleApplication.hpp>

namespace XYO::Win {

	SimpleApplication::SimpleApplication() {
		className_ = "SimpleApplication";
		windowName_ = "Simple application";
		singleInstance_ = false;
		isTrayIconic_ = false;
	};

	void SimpleApplication::setWindowClassEx(WNDCLASSEX &wndclassEx) {
		HICON hIcon;
		wndclassEx.lpszClassName = className_;

		hIcon = LoadIcon(wndclassEx.hInstance, MAKEINTRESOURCE(1));
		if (hIcon != NULL) {
			wndclassEx.hIcon = hIcon;
		};
	};

	void SimpleApplication::setCreateStruct(CREATESTRUCT &createStruct) {
		createStruct.lpszName = windowName_;
	};

	int SimpleApplication::main(int cmdN, char *cmdS[]) {
		if (singleInstance_) {
			HWND wndApp;
			wndApp = FindWindowExA(NULL, NULL, className_, windowName_);
			if (wndApp) {
				if (!isTrayIconic_) {
					SetForegroundWindow(wndApp);
					ShowWindow(wndApp, SW_SHOW);
					SetFocus(wndApp);
				};
				return 0;
			};
		};
		return Application::main(cmdN, cmdS);
	};

	HWND SimpleApplication::getSingleInstanceWindow() {
		return FindWindowExA(NULL, NULL, className_, windowName_);
	};

	void SimpleApplication::initMemory() {
		Application::initMemory();
	};

};
