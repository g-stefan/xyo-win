// Win
// Copyright (c) 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_WINDOW_HPP
#define XYO_WIN_WINDOW_HPP

#ifndef XYO_WIN_DEPENDENCY_HPP
#	include <XYO/Win/Dependency.hpp>
#endif

#ifndef XYO_WIN_INOTIFY_HPP
#	include <XYO/Win/INotify.hpp>
#endif

namespace XYO::Win {

	class Window : public virtual Object {
		private:
			HWND hWnd_;
			XYO_WIN_EXPORT static LRESULT CALLBACK windowProcedure_(HWND hWnd,
			                                                        UINT uMsg,
			                                                        WPARAM wParam,
			                                                        LPARAM lParam);
			XYO_WIN_EXPORT bool destroyAndWait_();

			TPointer<INotify> notifyOnCreate_;
			TPointer<INotify> notifyOnDestroy_;

			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(Window);

		public:
			XYO_WIN_EXPORT Window();
			XYO_WIN_EXPORT ~Window();

			inline operator HWND() {
				return hWnd_;
			};

			XYO_WIN_EXPORT operator HINSTANCE();

			XYO_WIN_EXPORT virtual LRESULT windowProcedure(UINT uMsg, WPARAM wParam, LPARAM lParam);
			XYO_WIN_EXPORT virtual bool translateAccelerator(MSG &msg);

			XYO_WIN_EXPORT bool create(DWORD dwExStyle,
			                           LPCTSTR lpClassName,
			                           LPCTSTR lpWindowName,
			                           DWORD dwStyle,
			                           int x, int y,
			                           int nWidth, int nHeight,
			                           HWND hWndParent,
			                           HMENU hMenu,
			                           HINSTANCE hInstance);

			XYO_WIN_EXPORT bool create(LPCTSTR lpClassName,
			                           LPCTSTR lpWindowName,
			                           DWORD dwStyle,
			                           int x, int y,
			                           int nWidth, int nHeight,
			                           HWND hWndParent,
			                           HMENU hMenu,
			                           HINSTANCE hInstance);

			XYO_WIN_EXPORT void setNotifyOnCreate(INotify *);
			XYO_WIN_EXPORT void setNotifyOnDestroy(INotify *);

			XYO_WIN_EXPORT static bool registerClass(WNDCLASS &);
			XYO_WIN_EXPORT static bool registerClass(WNDCLASSEX &);

			XYO_WIN_EXPORT static void initMemory();
	};

};

#endif
