// Win
// Copyright (c) 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <stdio.h>

#include <XYO/Win/Window.hpp>

namespace XYO::Win {

	Window::Window() {
		hWnd_ = NULL;
	};

	Window::~Window() {
		destroyAndWait_();
	};

	// GetWindowLongPtr / GWLP_* map to GetWindowLong / GWL_* on 32 bit, so a
	// single call works on both, no per-platform branch that can fall through.
	Window::operator HINSTANCE() {
		return (HINSTANCE)GetWindowLongPtr(hWnd_, GWLP_HINSTANCE);
	};

	// DestroyWindow works only on the thread that owns the window,
	// other threads ask the owner thread to destroy it.
	static UINT destroyWindowMessage_() {
		static UINT message = RegisterWindowMessageA("XYO.Win.Window.DestroyWindow");
		return message;
	};

	bool Window::destroyAndWait_() {
		if (hWnd_ == NULL) {
			return false;
		}
		if (GetWindowThreadProcessId(hWnd_, NULL) == GetCurrentThreadId()) {
			// WM_NCDESTROY is processed before DestroyWindow returns
			DestroyWindow(hWnd_);
		} else {
			// Synchronous, returns after the owner thread destroyed the window,
			// or at once if the owner thread is gone
			SendMessage(hWnd_, destroyWindowMessage_(), 0, 0);
		};
		return true;
	};

	LRESULT Window::windowProcedure(UINT uMsg, WPARAM wParam, LPARAM lParam) {
		return DefWindowProc(hWnd_, uMsg, wParam, lParam);
	};

	LRESULT CALLBACK Window::windowProcedure_(HWND hWnd,
	                                          UINT uMsg,
	                                          WPARAM wParam,
	                                          LPARAM lParam) {
		Window *vWindow;
		if (uMsg == WM_NCCREATE) {
			vWindow = (Window *)(((LPCREATESTRUCT)lParam)->lpCreateParams);
			// A window of a registered class created by plain CreateWindow
			// (not Window::create) has no Window object
			if (vWindow == NULL) {
				return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
			};
			vWindow->incReferenceCount();
			vWindow->hWnd_ = hWnd;

			SetWindowLongPtr(hWnd, 0, (LONG_PTR)vWindow);

			if (vWindow->notifyOnCreate_) {
				// The notify can replace or clear notifyOnCreate_,
				// keep it alive until it returns
				TPointer<INotify> notify(vWindow->notifyOnCreate_);
				notify->notify();
			};
		};

		vWindow = (Window *)GetWindowLongPtr(hWnd, 0);
		if (vWindow != NULL) {
			if ((uMsg == destroyWindowMessage_()) && (uMsg != 0)) {
				DestroyWindow(hWnd);
				return 0;
			};
			if (uMsg == WM_NCDESTROY) {

				SetWindowLongPtr(hWnd, 0, (LONG_PTR)NULL);

				LRESULT retVal = vWindow->windowProcedure(uMsg, wParam, lParam);
				if (vWindow->notifyOnDestroy_) {
					// MessageManager's notify clears notifyOnDestroy_ (the
					// last reference) from inside notify(), keep it alive
					TPointer<INotify> notify(vWindow->notifyOnDestroy_);
					notify->notify();
				};
				vWindow->hWnd_ = NULL;
				vWindow->decReferenceCount();
				return retVal;
			} else {
				return vWindow->windowProcedure(uMsg, wParam, lParam);
			};
		};
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	};

	bool Window::create(DWORD dwExStyle,
	                    LPCTSTR lpClassName,
	                    LPCTSTR lpWindowName,
	                    DWORD dwStyle,
	                    int x, int y,
	                    int nWidth, int nHeight,
	                    HWND hWndParent,
	                    HMENU hMenu,
	                    HINSTANCE hInstance) {
		return (::CreateWindowEx(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, this) != NULL);
	};

	bool Window::create(LPCTSTR lpClassName,
	                    LPCTSTR lpWindowName,
	                    DWORD dwStyle,
	                    int x, int y,
	                    int nWidth, int nHeight,
	                    HWND hWndParent,
	                    HMENU hMenu,
	                    HINSTANCE hInstance) {
		return (::CreateWindow(lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, this) != NULL);
	};

	bool Window::registerClass(WNDCLASS &wc) {
		wc.lpfnWndProc = Window::windowProcedure_;
		wc.cbWndExtra = sizeof(void *);
		return ::RegisterClass(&wc) != 0;
	};

	bool Window::registerClass(WNDCLASSEX &wc) {
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.lpfnWndProc = Window::windowProcedure_;
		wc.cbWndExtra = sizeof(void *);
		return ::RegisterClassEx(&wc) != 0;
	};

	void Window::setNotifyOnCreate(INotify *value) {
		notifyOnCreate_ = value;
	};

	void Window::setNotifyOnDestroy(INotify *value) {
		notifyOnDestroy_ = value;
	};

	bool Window::translateAccelerator(MSG &msg) {
		msg;
		return false;
	};

	void Window::initMemory() {
		TPointer<INotify>::initMemory();
	};

};
