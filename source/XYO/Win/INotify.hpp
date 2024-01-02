// Win
// Copyright (c) 2014-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_INOTIFY_HPP
#define XYO_WIN_INOTIFY_HPP

#ifndef XYO_WIN_DEPENDENCY_HPP
#	include <XYO/Win/Dependency.hpp>
#endif

namespace XYO::Win {

	class INotify : public virtual Object {
			XYO_INTERFACE(INotify);

		public:
			virtual void notify() = 0;
	};

};

#endif
