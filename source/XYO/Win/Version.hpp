// Win
// Copyright (c) 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_VERSION_HPP
#define XYO_WIN_VERSION_HPP

#ifndef XYO_WIN_DEPENDENCY_HPP
#	include <XYO/Win/Dependency.hpp>
#endif

namespace XYO::Win::Version {

	XYO_WIN_EXPORT const char *version();
	XYO_WIN_EXPORT const char *build();
	XYO_WIN_EXPORT const char *versionWithBuild();
	XYO_WIN_EXPORT const char *datetime();

};

#endif
