// Win
// Copyright (c) 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_CAPTUREDESKTOP_HPP
#define XYO_WIN_CAPTUREDESKTOP_HPP

#ifndef XYO_WIN_DEPENDENCY_HPP
#	include <XYO/Win/Dependency.hpp>
#endif

namespace XYO::Win::CaptureDesktop {

	XYO_WIN_EXPORT TPointer<Bitmap> captureDesktop();
	XYO_WIN_EXPORT bool captureDesktopToPNGFile(char *fileName);

};

#endif
