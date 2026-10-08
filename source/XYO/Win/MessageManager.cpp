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

#include <vector>

#include <XYO/Win/MessageManager.hpp>
#include <XYO/Win/TNotify.hpp>

namespace XYO::Win {

	MessageManager::MessageManager() {
		InitializeCriticalSection(&cs_);
	};

	MessageManager::~MessageManager() {
		WindowList::Node *x;
		for (x = windowList_.head; x != NULL; x = x->next) {
			x->value->setNotifyOnDestroy(NULL);
		};
		windowList_.empty();
		DeleteCriticalSection(&cs_);
	};

	typedef void (MessageManager::*eventDestroy_)(MessageManager::WindowList::Node *);

	MessageManager::WindowList::Node *MessageManager::add(Window *p) {
		WindowList::Node *retV;
		TPointer<TNotify<MessageManager, eventDestroy_, WindowList::Node *>> n;

		EnterCriticalSection(&cs_);
		retV = windowList_.pushToTailX(p);
		LeaveCriticalSection(&cs_);

		n.newMemory();
		n->set(this, &MessageManager::eventOnDestroy_, retV);
		p->setNotifyOnDestroy(n.value());
		return retV;
	};

	void MessageManager::remove(WindowList::Node *window) {
		EnterCriticalSection(&cs_);
		windowList_.extractNode(window);
		LeaveCriticalSection(&cs_);
		// The destroy notification points at this node, a window removed
		// while still open must not call it after the node is deleted
		window->value->setNotifyOnDestroy(nullptr);
		window->value.deleteMemory();
		windowList_.deleteNode(window);
	};

	int MessageManager::processAllMessages() {
		MSG msg;
		WindowList::Node *scan;
		// The exit code is the one of WM_QUIT (PostQuitMessage), not the
		// wParam of whatever message happened to be processed last.
		int exitCode = 0;
		bool hasQuit = false;
		while (!windowList_.isEmpty()) {
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) {
					exitCode = (int)msg.wParam;
					hasQuit = true;
					break;
				};
				for (scan = windowList_.head; scan != NULL; scan = scan->next) {
					if ((scan->value.value())->translateAccelerator(msg)) {
						break;
					};
				};
				if (scan != nullptr) {
					continue;
				};
				// Thread messages (hwnd NULL, SetTimer(NULL, ...) callbacks)
				// are dispatched too
				if ((msg.hwnd == NULL) || IsWindow(msg.hwnd)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				};
			} else {
				// Block until a message arrives instead of spinning, no idle CPU
				MsgWaitForMultipleObjects(0, NULL, FALSE, INFINITE, QS_ALLINPUT);
			};
		};
		while (!windowList_.isEmpty()) {
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) {
					continue;
				};
				if ((msg.hwnd == NULL) || IsWindow(msg.hwnd)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				};
			} else {
				// No pending message: force the remaining windows to close.
				// DestroyWindow removes the window from the list (WM_NCDESTROY)
				// before it returns, so there is nothing to wait for; a window
				// that cannot be destroyed here (owned by another thread,
				// already gone) is no longer tracked, the loop always ends.
				scan = windowList_.head;
				if (!DestroyWindow(*(scan->value.value()))) {
					if (windowList_.head == scan) {
						eventOnDestroy_(scan);
					};
				};
			};
		};
		if (!hasQuit) {
			// The last window was destroyed before WM_QUIT was retrieved,
			// take the exit code from the pending WM_QUIT if there is one.
			if (PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE)) {
				exitCode = (int)msg.wParam;
			};
		};
		return exitCode;
	};

	void MessageManager::eventOnDestroy_(WindowList::Node *window) {
		remove(window);
	};

	void MessageManager::postMessageToAll(UINT m) {
		WindowList::Node *x;
		for (x = windowList_.head; x != NULL; x = x->next) {
			PostMessage(*(x->value.value()), m, 0, 0);
		};
	};

	void MessageManager::sendMessageToAll(UINT m) {
		// The message can destroy windows (WM_CLOSE, ...), which removes and
		// deletes their nodes, so do not walk the list while sending.
		std::vector<HWND> windows;
		WindowList::Node *x;
		for (x = windowList_.head; x != NULL; x = x->next) {
			windows.push_back(*(x->value.value()));
		};
		for (HWND hWnd : windows) {
			if (IsWindow(hWnd)) {
				SendMessage(hWnd, m, 0, 0);
			};
		};
	};

	void MessageManager::initMemory() {
		WindowList::initMemory();
	};

};
