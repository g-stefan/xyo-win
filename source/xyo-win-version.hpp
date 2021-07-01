//
// XYO Win
//
// Copyright (c) 2020-2021 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef XYO_WIN_VERSION_HPP
#define XYO_WIN_VERSION_HPP

#define XYO_WIN_VERSION_ABCD                 2,0,0,6
#define XYO_WIN_VERSION_STR                 "2.0.0"
#define XYO_WIN_VERSION_STR_BUILD           "6"
#define XYO_WIN_VERSION_STR_DATETIME        "2021-07-01 17:02:13"

#ifndef XYO_RC

#ifndef XYO_WIN__EXPORT_HPP
#include "xyo-win--export.hpp"
#endif

namespace XYO {
	namespace Win {
		namespace Version {
			XYO_WIN_EXPORT const char *version();
			XYO_WIN_EXPORT const char *build();
			XYO_WIN_EXPORT const char *versionWithBuild();
			XYO_WIN_EXPORT const char *datetime();
		};
	};
};

#endif
#endif

