//
// XYO Win
//
// Copyright (c) 2020 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef XYO_WIN_SHELL_HPP
#define XYO_WIN_SHELL_HPP

#ifndef XYO_WIN__DEPENDENCY_HPP
#include "xyo-win--dependency.hpp"
#endif

namespace XYO {
	namespace Win {
		namespace Shell {

			XYO_WIN_EXPORT bool createLink(const char *outputFile, const char *workingDirectory, const char *path, const char *arguments, const char *iconPath, int iconIndex, bool runAsAdministrator);
			XYO_WIN_EXPORT bool createLinkW(const wchar_t *outputFile, const wchar_t *workingDirectory, const wchar_t *path, const wchar_t *arguments, const wchar_t *iconPath, int iconIndex, bool runAsAdministrator);

		};
	};
};

#endif

