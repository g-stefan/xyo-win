// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

// Window, Application, MessageManager and Capture.
// Regression test for:
//  - the exit code of MessageManager::processAllMessages, it used to be the
//    wParam of the last processed message instead of the PostQuitMessage code,
//  - thread messages (hwnd NULL, SetTimer(NULL, ...) callbacks), they used to
//    be dropped by the message loop,
//  - closing the remaining windows after WM_QUIT, the loop used to wait for a
//    message that never came once a window was destroyed,
//  - TNotify / setNotifyOnCreate / setNotifyOnDestroy.
// A broken message loop hangs, a watchdog thread ends the test with an error.

#include <XYO/Win.hpp>

#include <stdio.h>
#include <string.h>

using namespace XYO::Win;

static int failed = 0;

static void check(bool condition, const char *name) {
	printf("%s %s\r\n", condition ? "[ok]  " : "[FAIL]", name);
	fflush(stdout);
	if (!condition) {
		++failed;
	};
};

static DWORD WINAPI watchdog(LPVOID) {
	Sleep(30000);
	printf("[FAIL] timeout, the message loop did not end\r\n");
	printf("FAILED\r\n");
	fflush(stdout);
	ExitProcess(2);
	return 0;
};

#define WUM_TEST_CAPTURE (WM_USER + 1)
#define WUM_TEST_CLOSE (WM_USER + 2)

static const char *pngFile = "test.03.capture.png";

// --- Application: exit code, thread timer, capture

class TestApplication : public virtual Application {
		XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(TestApplication);

	public:
		static TestApplication *instance;
		bool timerFired;
		bool captureDone;

		inline TestApplication() {
			timerFired = false;
			captureDone = false;
		};

		void setWindowClassEx(WNDCLASSEX &wndClassEx) {
			wndClassEx.lpszClassName = "XYO.Win.Test03.Application";
		};

		void setCreateStruct(CREATESTRUCT &createStruct) {
			createStruct.lpszName = "xyo-win test.03";
			createStruct.x = 32;
			createStruct.y = 32;
			createStruct.cx = 320;
			createStruct.cy = 200;
		};

		int setShowCmd(int) {
			return SW_SHOWNOACTIVATE;
		};

		static void CALLBACK timerProcedure(HWND, UINT, UINT_PTR idEvent, DWORD) {
			KillTimer(NULL, idEvent);
			instance->timerFired = true;
			PostMessage(*instance, WUM_TEST_CAPTURE, 0, 0);
		};

		void testCapture() {
			RECT rect;
			GetClientRect(*this, &rect);

			TPointer<Bitmap> bitmap = Capture::captureWindow(*this);
			check(bitmap.value() != nullptr, "captureWindow");
			if (bitmap.value() != nullptr) {
				check(((long)bitmap->getWidth() == (rect.right - rect.left)) && ((long)bitmap->getHeight() == (rect.bottom - rect.top)), "captureWindow size is the client area");
				check(bitmap->getPixelWidth() == 32, "captureWindow 32 bits");
			};

			check(Capture::captureWindowToPNGFile(*this, pngFile), "captureWindowToPNGFile");
			TPointer<Image> image = Process::pngLoad(pngFile);
			check(image.value() != nullptr, "captureWindowToPNGFile readable");
			if (image.value() != nullptr) {
				check((image->width == (rect.right - rect.left)) && (image->height == (rect.bottom - rect.top)), "captureWindowToPNGFile size");
				check(XYO_PIXEL32_A(image->pixel[image->height / 2][image->width / 2]) == 255, "captureWindowToPNGFile opaque");
			};
			DeleteFileA(pngFile);

			check(Capture::captureWindow(NULL).value() == nullptr, "captureWindow invalid window");

			TPointer<Bitmap> desktop = Capture::captureDesktop();
			check(desktop.value() != nullptr, "captureDesktop");
			if (desktop.value() != nullptr) {
				check(((int)desktop->getWidth() == GetSystemMetrics(SM_CXVIRTUALSCREEN)) && ((int)desktop->getHeight() == GetSystemMetrics(SM_CYVIRTUALSCREEN)), "captureDesktop size is the virtual screen");
			};
		};

		LRESULT windowProcedure(UINT uMsg, WPARAM wParam, LPARAM lParam) {
			switch (uMsg) {
			case WM_CREATE:
				// A thread timer, its WM_TIMER has no window
				if (SetTimer(NULL, 0, 50, timerProcedure) == 0) {
					PostMessage(*this, WUM_TEST_CAPTURE, 0, 0);
				};
				break;
			case WUM_TEST_CAPTURE:
				testCapture();
				captureDone = true;
				// The exit code must not be this wParam
				PostMessage(*this, WUM_TEST_CLOSE, 12345, 0);
				return 0;
			case WUM_TEST_CLOSE:
				DestroyWindow(*this);
				return 0;
			case WM_DESTROY:
				PostQuitMessage(7);
				break;
			default:
				break;
			};
			return Application::windowProcedure(uMsg, wParam, lParam);
		};
};

TestApplication *TestApplication::instance = nullptr;

static void testApplication() {
	TestApplication application;
	TestApplication::instance = &application;
	char *cmdS[] = {(char *)"test.03", nullptr};

	int exitCode = application.main(1, cmdS);

	check(application.timerFired, "thread timer dispatched");
	check(application.captureDone, "capture step reached");
	check(exitCode == 7, "exit code is the PostQuitMessage code");
	check((HWND)application == NULL, "application window destroyed");
	TestApplication::instance = nullptr;
};

// --- MessageManager with several windows, closed after WM_QUIT

class Counter : public virtual Object {
	public:
		int created;
		int destroyed;

		inline Counter() {
			created = 0;
			destroyed = 0;
		};

		void onCreate(int) {
			++created;
		};

		void onDestroy(int) {
			++destroyed;
		};
};

typedef void (Counter::*CounterFunction)(int);
typedef TNotify<Counter, CounterFunction, int> CounterNotify;

static const char *windowClassName = "XYO.Win.Test03.Window";

static bool registerWindowClass() {
	WNDCLASSEX wndClassEx;
	memset(&wndClassEx, 0, sizeof(wndClassEx));
	wndClassEx.hInstance = GetModuleHandle(NULL);
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassEx.lpszClassName = windowClassName;
	return Window::registerClass(wndClassEx);
};

static bool createWindow(Window *window) {
	return window->create(0, windowClassName, "xyo-win test.03 window", WS_OVERLAPPEDWINDOW,
	                      0, 0, 100, 100, NULL, NULL, GetModuleHandle(NULL));
};

static void testMessageManager() {
	check(registerWindowClass(), "registerClass");

	TPointer<Counter> counter;
	counter.newMemory();

	TPointer<CounterNotify> notifyOnCreate;
	notifyOnCreate.newMemory();
	notifyOnCreate->set(counter, &Counter::onCreate, 0);

	TPointer<MessageManager> messageManager;
	messageManager.newMemory();

	TPointer<Window> window1;
	TPointer<Window> window2;
	TPointer<Window> window3;
	window1.newMemory();
	window2.newMemory();
	window3.newMemory();

	window1->setNotifyOnCreate(notifyOnCreate);
	window2->setNotifyOnCreate(notifyOnCreate);
	window3->setNotifyOnCreate(notifyOnCreate);

	check(createWindow(window1) && createWindow(window2) && createWindow(window3), "create windows");
	check(counter->created == 3, "notify on create");

	messageManager->add(window1);
	messageManager->add(window2);
	messageManager->add(window3);

	// A window destroyed by its owner is removed from the list
	DestroyWindow(*window2);
	check((HWND)(*window2) == NULL, "window destroyed");

	// WM_QUIT with windows still open: the loop destroys them and ends
	PostQuitMessage(3);
	int exitCode = messageManager->processAllMessages();
	check(exitCode == 3, "processAllMessages exit code after WM_QUIT");
	check(((HWND)(*window1) == NULL) && ((HWND)(*window3) == NULL), "remaining windows destroyed");

	// A window without a message manager, notify on destroy
	TPointer<CounterNotify> notifyOnDestroy;
	notifyOnDestroy.newMemory();
	notifyOnDestroy->set(counter, &Counter::onDestroy, 0);

	TPointer<Window> window4;
	window4.newMemory();
	window4->setNotifyOnDestroy(notifyOnDestroy);
	check(createWindow(window4), "create window");
	DestroyWindow(*window4);
	check(counter->destroyed == 1, "notify on destroy");

	// A window removed from the manager while open, then destroyed: the
	// manager's destroy notification must be gone with the node
	TPointer<MessageManager> removeManager;
	removeManager.newMemory();
	TPointer<Window> window5;
	window5.newMemory();
	check(createWindow(window5), "create window to remove");
	MessageManager::WindowList::Node *node = removeManager->add(window5);
	removeManager->remove(node);
	DestroyWindow(*window5);
	check((HWND)(*window5) == NULL, "removed window destroyed");
	check(removeManager->processAllMessages() == 0, "processAllMessages after remove");

	// No windows, no wait
	TPointer<MessageManager> emptyManager;
	emptyManager.newMemory();
	check(emptyManager->processAllMessages() == 0, "processAllMessages without windows");

	// Plain CreateWindow of a registered class, no Window object behind it
	HWND plain = CreateWindowExA(0, windowClassName, "plain", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, GetModuleHandle(NULL), NULL);
	check(plain != NULL, "CreateWindow without a Window object");
	if (plain != NULL) {
		DestroyWindow(plain);
	};
};

int main(int cmdN, char *cmdS[]) {
	XYO::ManagedMemory::Registry::registryInit();
	Application::initMemory();
	MessageManager::initMemory();
	Window::initMemory();

	HANDLE watchdogThread = CreateThread(NULL, 0, watchdog, NULL, 0, NULL);
	if (watchdogThread != NULL) {
		CloseHandle(watchdogThread);
	};

	try {
		testApplication();
		testMessageManager();
	} catch (const std::exception &e) {
		printf("* Error: %s\r\n", e.what());
		return 1;
	} catch (...) {
		printf("* Error: Unknown\r\n");
		return 1;
	};

	printf("%s\r\n", failed == 0 ? "PASSED" : "FAILED");
	return (failed == 0) ? 0 : 1;
};
