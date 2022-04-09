//
// XYO Win
//
// Copyright (c) 2020-2022 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef XYO_WIN_VERSION_HPP
#define XYO_WIN_VERSION_HPP

#define XYO_WIN_VERSION_ABCD $VERSION_ABCD
#define XYO_WIN_VERSION_STR "$VERSION_VERSION"
#define XYO_WIN_VERSION_STR_BUILD "$VERSION_BUILD"
#define XYO_WIN_VERSION_STR_DATETIME "$VERSION_DATETIME"

#ifndef XYO_RC

#	ifndef XYO_WIN__EXPORT_HPP
#		include "xyo-win--export.hpp"
#	endif

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
