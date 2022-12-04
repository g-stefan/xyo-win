// Win
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_SIMPLEAPPLICATION_HPP
#define XYO_WIN_SIMPLEAPPLICATION_HPP

#ifndef XYO_WIN_APPLICATION_HPP
#	include <XYO/Win/Application.hpp>
#endif

namespace XYO::Win {

		class SimpleApplication : public virtual Application {
			protected:
				LPSTR className_;
				LPSTR windowName_;
				bool singleInstance_;
				bool isTrayIconic_;

				XYO_DISALLOW_COPY_ASSIGN_MOVE(SimpleApplication);

			public:
				XYO_WIN_EXPORT SimpleApplication();
				inline ~SimpleApplication(){};

				XYO_WIN_EXPORT void setWndclassEx(WNDCLASSEX &);
				XYO_WIN_EXPORT void setCreateStruct(CREATESTRUCT &);
				XYO_WIN_EXPORT int main(int cmdN, char *cmdS[]);
				XYO_WIN_EXPORT HWND getSingleInstanceWindow();

				XYO_WIN_EXPORT static void initMemory();
		};

	};

#endif
