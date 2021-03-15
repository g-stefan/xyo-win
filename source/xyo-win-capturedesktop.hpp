//
// XYO Win
//
// Copyright (c) 2020-2021 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef XYO_WIN_CAPTUREDESKTOP_HPP
#define XYO_WIN_CAPTUREDESKTOP_HPP

#ifndef XYO_WIN__DEPENDENCY_HPP
#include "xyo-win--dependency.hpp"
#endif

namespace XYO {
	namespace Win {
		namespace CaptureDesktop {

			XYO_WIN_EXPORT bool captureDesktopToPNGFile(char *fileName);

		};
	};
};

#endif

