// Win
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_UTIL_HPP
#define XYO_WIN_UTIL_HPP

#ifndef XYO_WIN_DEPENDENCY_HPP
#	include <XYO/Win/Dependency.hpp>
#endif

namespace XYO::Win::Util {

	XYO_WIN_EXPORT void sendMessageToAllChildWindowsIE(HWND hParendWnd, UINT MessageId, WPARAM wParam, LPARAM lParam);
	XYO_WIN_EXPORT void postMessageToProcessWindows(UINT MessageId, WPARAM wParam, LPARAM lParam);

};

#endif
