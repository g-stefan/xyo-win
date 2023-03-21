// Win
// Copyright (c) 2014-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_COPYRIGHT_HPP
#define XYO_WIN_COPYRIGHT_HPP

#ifndef XYO_WIN_DEPENDENCY_HPP
#	include <XYO/Win/Dependency.hpp>
#endif

namespace XYO::Win::Copyright {
	XYO_WIN_EXPORT std::string copyright();
	XYO_WIN_EXPORT std::string publisher();
	XYO_WIN_EXPORT std::string company();
	XYO_WIN_EXPORT std::string contact();
};

#endif
