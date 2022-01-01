//
// XYO Win
//
// Copyright (c) 2020-2022 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#include "xyo-win-version.hpp"

namespace XYO {
	namespace Win {
		namespace Version {

			static const char *version_ = "2.5.0";
			static const char *build_ = "20";
			static const char *versionWithBuild_ = "2.5.0.20";
			static const char *datetime_ = "2021-11-22 13:35:05";

			const char *version() {
				return version_;
			};
			const char *build() {
				return build_;
			};
			const char *versionWithBuild() {
				return versionWithBuild_;
			};
			const char *datetime() {
				return datetime_;
			};

		};
	};
};

