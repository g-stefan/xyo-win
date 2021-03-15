//
// XYO Win
//
// Copyright (c) 2020-2021 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef XYO_WIN_COPYRIGHT_HPP
#define XYO_WIN_COPYRIGHT_HPP

#define XYO_WIN_COPYRIGHT            "Copyright (c) Grigore Stefan"
#define XYO_WIN_PUBLISHER            "Grigore Stefan"
#define XYO_WIN_COMPANY              XYO_WIN_PUBLISHER
#define XYO_WIN_CONTACT              "g_stefan@yahoo.com"
#define XYO_WIN_FULL_COPYRIGHT       XYO_WIN_COPYRIGHT " <" XYO_WIN_CONTACT ">"

#ifndef XYO_RC

#ifndef XYO_WIN__EXPORT_HPP
#include "xyo-win--export.hpp"
#endif

namespace XYO {
	namespace Win {
		namespace Copyright {
			XYO_WIN_EXPORT const char *copyright();
			XYO_WIN_EXPORT const char *publisher();
			XYO_WIN_EXPORT const char *company();
			XYO_WIN_EXPORT const char *contact();
			XYO_WIN_EXPORT const char *fullCopyright();
		};
	};
};

#endif
#endif
