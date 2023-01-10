// Win
// Copyright (c) 2014-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_SHELL_HPP
#define XYO_WIN_SHELL_HPP

#ifndef XYO_WIN_DEPENDENCY_HPP
#	include <XYO/Win/Dependency.hpp>
#endif

namespace XYO::Win::Shell {

	XYO_WIN_EXPORT bool createLink(const char *outputFile, const char *workingDirectory, const char *path, const char *arguments, const char *iconPath, int iconIndex, bool runAsAdministrator);
	XYO_WIN_EXPORT bool createLinkW(const wchar_t *outputFile, const wchar_t *workingDirectory, const wchar_t *path, const wchar_t *arguments, const wchar_t *iconPath, int iconIndex, bool runAsAdministrator);

};

#endif
