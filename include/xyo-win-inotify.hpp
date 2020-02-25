//
// XYO Win
//
// Copyright (c) 2020 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef XYO_WIN_INOTIFY_HPP
#define XYO_WIN_INOTIFY_HPP

#ifndef XYO_WIN__DEPENDENCY_HPP
#include "xyo-win--dependency.hpp"
#endif

namespace XYO {
	namespace Win {

		class INotify :
			public virtual XYO::Object {
				XYO_INTERFACE(INotify);

			public:
				virtual void notify() = 0;
		};

	};
};

#endif
