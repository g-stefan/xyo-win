---
name: xyo-win
description: >-
  How to use the xyo-win C++ library (namespace XYO::Win), the Windows
  layer of the XYO C++ stack on top of xyo-system and xyo-pixel32: Window
  (HWND bound to an Object, virtual windowProcedure, registerClass, create,
  translateAccelerator, setNotifyOnCreate / setNotifyOnDestroy),
  Application (window + IApplication, setWindowClassEx / setCreateStruct /
  setShowCmd / parseCommandLine / newWindow, messageManager_) and
  SimpleApplication (className_, windowName_, singleInstance_,
  isTrayIconic_, icon resource 1) with XYO_APPLICATION_WINMAIN;
  MessageManager (add, remove, processAllMessages exit code, WM_QUIT,
  postMessageToAll / sendMessageToAll); INotify / TNotify; COM automation:
  Ole::isValid, Variant, DispatchVariant, Function (call IDispatch methods
  by name), Dispatch with XYO_WIN_DISPATCH__PROTOTYPE / TABLE_BEGIN /
  TABLE_FUNCTION_0..8 / TABLE_END; Registry:: (createKey, writeString,
  readString, writeDWord, readDWord, deleteKey, getStringLength, ...W);
  Shell::createLink / createLinkW (.lnk, run as administrator), runAs /
  runAsW (CreateProcessWithLogonW); User:: ADSI local account functions
  (create / delete / hide from welcome screen); Capture::captureDesktop /
  captureWindow / ...ToPNGFile; Util:: message broadcast; the discontinued
  WebBrowser. Use when writing or reviewing code that includes
  <XYO/Win.hpp>, depends on "xyo-win" in fabricare.json, uses any of these
  names, or when working inside the xyo-win repository or programs built on
  it (proxy-forward, xyo-win-service, xyo-win-inject, dll-inject).
---

# xyo-win

Windows layer of the XYO C++ libraries, on top of `xyo-system` and
`xyo-pixel32` (see the `xyo-system`, `xyo-managed-memory`, `xyo-pixel32`
skills; their rules apply — `TPointer`, `Object`, `IApplication`,
`Bitmap`). Purpose: **thin, correct wrappers over Win32 / COM** for small
XYO tools — a window as an object, an application with a message loop, a
single instance tray tool, registry settings, `.lnk` shortcuts, late bound
COM calls, exposing C++ methods as `IDispatch`, local accounts,
screenshots. Windows only, plain Win32 types (`HWND`, `HKEY`, `VARIANT`).

Full documentation: `docs/` in the xyo-win repository
(`X:\Storage\XYO\Gitea\CPP\xyo-win\docs` on this machine): README,
getting-started, **windows**, **automation**, **system**, reference. Read
the matching page when you need more than this summary. When in doubt read
the sources in `source/XYO/Win/` (small, one class per file); the tests
`test/test.01.cpp` ... `test.04.cpp` show real usage of every part, and
`proxy-forward/source/XYO/ProxyForward/Application.cpp` is a complete
program built on `SimpleApplication`.

## Pick a tool

| Need | Use |
|------|-----|
| GUI program, one main window | `class App : public virtual SimpleApplication`, set `className_` / `windowName_` in the constructor, `XYO_APPLICATION_WINMAIN(App);` |
| Only one running copy | `singleInstance_ = true;` (`isTrayIconic_ = true` to not activate the running one) |
| Background / tray tool | override `int setShowCmd(int) { return SW_HIDE; }` |
| Handle messages | override `LRESULT windowProcedure(UINT, WPARAM, LPARAM)`, return the base call for the rest |
| Command line | override `main` (then call `SimpleApplication::main`) or `parseCommandLine` (non-zero = exit code) |
| Window size / title / style | override `setCreateStruct(CREATESTRUCT &)` (call the base in a `SimpleApplication`) |
| Class icon / brush / menu | override `setWindowClassEx(WNDCLASSEX &)` |
| Exit code | `PostQuitMessage(code)` in `WM_DESTROY` |
| Extra window, same loop | `Window::registerClass(wc)`, `TPointer<MyWindow> w; w.newMemory(); w->create(...)`, `messageManager_->add(w)` |
| Accelerators / dialog keys | override `bool translateAccelerator(MSG &)` |
| Callback on create / destroy | `TNotify<C, void (C::*)(D), D>` + `setNotifyOnCreate` / `setNotifyOnDestroy` |
| Call a COM method by name | `Function f; f.setObject(disp); f.functionName(bstr); f.invoke(Variant(...), ...)`, `f.result()` |
| Read a COM argument safely | `DispatchVariant v(arg); long int n = (long int)v;` |
| Expose C++ methods as IDispatch | derive `Dispatch`, `XYO_WIN_DISPATCH__PROTOTYPE;`, table macros in the `.cpp` |
| Registry setting | `XYO::Win::Registry::readString / writeString / readDWord / writeDWord` |
| Shortcut | `XYO::Win::Shell::createLink(lnk, workDir, exe, args, icon, 0, admin)` (UTF-8, NULL = not set) |
| Start as another user | `XYO::Win::Shell::runAs(user, password, commandLine)` |
| Screenshot | `Capture::captureDesktopToPNGFile(file)`, `Capture::captureWindow(hwnd)` |
| Local account | `User::createUserAccountAsCurrentUserPrivilegeOnLocalComputer`, `deleteUserAccountOnLocalComputer` (elevated) |

## Hard rules

1. **Include and namespaces**: `#include <XYO/Win.hpp>`,
   `using namespace XYO::Win;` (re-exports ManagedMemory, DataStructures,
   Encoding, Multithreading, System, Pixel32). Then **`Registry::` and
   `Shell::` are ambiguous** (`XYO::ManagedMemory::Registry`,
   `XYO::System::Shell`): write `XYO::Win::Registry::...` /
   `XYO::Win::Shell::...` or `namespace Win = XYO::Win; Win::Registry::...`.
   `Capture::`, `User::`, `Util::`, `Ole::` are fine. Metadata in full:
   `XYO::Win::Version::version()`. `WebBrowser` needs
   `<XYO/Win/Discontinued/WebBrowser.hpp>`, namespace
   `XYO::Win::Discontinued` (Internet Explorer, legacy only). Your own
   global names clash too: a global `class Application` (or `Window`,
   `Function`, `Variant`) next to `using namespace XYO::Win;` is ambiguous —
   name it otherwise or put it in your own namespace
   (`namespace XYO::MyTool { class Application ... }`, like proxy-forward).
2. **ANSI build.** The window API uses `LPCTSTR` and the library is built
   without `UNICODE`: class names, titles are `char`. `Registry` / `User`
   have `...W` wide versions; `Shell::createLink` / `runAs` take UTF-8.
3. **Window lifetime.** A `Window` holds a reference to itself from
   `WM_NCCREATE` to `WM_NCDESTROY`, so a `TPointer` window is never deleted
   while its `HWND` exists. Stack windows (the application) are fine, they
   must outlive the `HWND`. `(HWND)window` is `NULL` after destruction.
   `~Window` destroys an open window (other thread: sends a registered
   message to the owner and waits). Not copyable.
4. **Window classes**: register with `Window::registerClass` (it sets
   `lpfnWndProc`, `cbWndExtra = sizeof(void *)` — the extra bytes hold the
   object pointer; never `SetWindowLongPtr(hwnd, 0, ...)`), create with
   `window->create(...)` (passes `this`). A class name can be registered
   once per process. `Application::main` registers its class itself.
5. **windowProcedure**: always end with the base class call
   (`SimpleApplication::windowProcedure(...)` / `Window::windowProcedure`)
   for unhandled messages; return `-1` from `WM_CREATE` to fail creation.
6. **Message loop**: `MessageManager::processAllMessages` runs while its
   window list is not empty, dispatches thread messages too, blocks when
   idle, on `WM_QUIT` destroys the remaining windows, returns the
   `PostQuitMessage` code (0 without one). One manager per GUI thread; a
   window's messages go to the thread that created it. Do not replace
   `setNotifyOnDestroy` on a window added to a manager (the manager uses
   it); use `remove(node)` to stop tracking without destroying.
7. **Threads**: from workers, `PostMessage(hwnd, WM_APP + n, ...)` to the
   window thread; never call window functions of another thread's window
   expecting immediate effect. `Ole::isValid()` initializes OLE (STA) once
   per thread and is false on an MTA thread; call it before
   `CoCreateInstance` in your own code. Use it on the main thread or on
   `XYO::Multithreading::Thread` threads (per thread registry).
8. **SimpleApplication strings** (`className_`, `windowName_`) are
   `LPCSTR` pointers: assign literals or the `value()` of a `String`
   member that lives as long as the application. The single instance check
   matches class **and** title.
9. **Variant** owns its value. Integer literals: `(long int)5` →
   `VT_I4` (the usual automation integer), `5` → `VT_INT`. `Variant(LPCSTR)`
   converts from the ANSI code page. The conversion operators read the union
   member **without checking `vt`** — check `value()->vt` or use
   `DispatchVariant` to convert. `DispatchVariant` never owns, follows
   `VT_BYREF | VT_VARIANT`, converts with `VariantChangeType`, `0` /
   `nullptr` on failure; its `BSTR` / interface results are borrowed.
10. **Function**: arguments in call order (0..8), `DISPATCH_METHOD` only
    (no property get / put, no named arguments). The dispatch id is cached
    until `setObject` / `functionName` / `releaseObject`. `result()` is
    overwritten by the next call. `functionName` copies the `BSTR`.
11. **Dispatch tables**: `TABLE_BEGIN(Class, StartId)` with `StartId > 0`
    (id 0 = `DISPID_VALUE` is never dispatched); methods
    `void m(Variant &returnValue, VARIANTARG *a0, ...)` with arguments in
    call order; names match case insensitively; wrong argument count →
    `DISP_E_BADPARAMCOUNT`. `TABLE_END(Base)` forwards unknown names / ids
    to `Base`: a derived table must use ids **below** the base's range
    (derived `1..k`, base from `k + 1`). `AddRef` / `Release` are no-ops —
    the C++ object must outlive every user; hand out
    `getIDispatchValue()`.
12. **Registry**: sizes are **bytes** (also for `readStringW` and
    `getStringLength*`). `readString*` always terminate, return `FALSE` and
    copy the default (`NULL` = `""`) on a missing value, wrong type or too
    small buffer; `REG_EXPAND_SZ` is not expanded. `write*` need an
    existing key (`createKey` first). `deleteKey(m, key, reg, TRUE)`
    deletes a value, `FALSE` deletes the subkey `reg` (no subkeys of its
    own). `createKey` asks `KEY_ALL_ACCESS`. `HKEY_LOCAL_MACHINE` writes
    need elevation.
13. **Shell::createLink**: `outputFile` and `path` required, the other
    strings may be `NULL`; overwrites; `runAsAdministrator` sets
    `SLDF_RUNAS_USER`. `runAs` returns once the process started (no wait),
    `username` may be `user`, `DOMAIN\user`, `user@domain`.
14. **Capture**: `Bitmap` results are 32-bit bottom-up BGRA with
    **undefined alpha** — `bitmap->setAlpha32(255)` before
    `Process::imageFromBitmap`; the `...ToPNGFile` functions do that.
    `captureDesktop` = all monitors (virtual screen); `captureWindow` = the
    client area as on screen (occluded parts are not reliable, minimized =
    `nullptr`). Sizes depend on the process DPI awareness.
15. **User::** functions change the machine (accounts, HKLM) and need an
    elevated process; `createUserAccountAsCurrentUserPrivilege...` copies
    **all** the current user's group memberships to the new account. Never
    run them in tests or without the user asking.

## Recipes

Single instance tray tool with a command to stop the running copy:

```cpp
#include <XYO/Win.hpp>

using namespace XYO::Win;

class MyTool : public virtual SimpleApplication {
		XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(MyTool);

	public:
		inline MyTool() {
			className_ = "XYO.MyTool";
			windowName_ = "My Tool";
			singleInstance_ = true;
			isTrayIconic_ = true;
		};

		int main(int cmdN, char *cmdS[]) {
			HWND running = getSingleInstanceWindow();
			if ((cmdN > 1) && (strcmp(cmdS[1], "--stop") == 0)) {
				if (running) {
					PostMessage(running, WM_CLOSE, 0, 0);
				};
				return 0;
			};
			return SimpleApplication::main(cmdN, cmdS);
		};

		int setShowCmd(int) {
			return SW_HIDE;
		};

		LRESULT windowProcedure(UINT uMsg, WPARAM wParam, LPARAM lParam) {
			switch (uMsg) {
			case WM_CREATE:
				// start worker threads, timers ...
				break;
			case WM_DESTROY:
				// stop them
				PostQuitMessage(0);
				break;
			default:
				break;
			};
			return SimpleApplication::windowProcedure(uMsg, wParam, lParam);
		};
};

XYO_APPLICATION_WINMAIN(MyTool);
```

Settings in the registry:

```cpp
namespace Win = XYO::Win;
const char *key = "Software\\XYO\\MyTool";
char folder[MAX_PATH];
unsigned long int port;
Win::Registry::createKey(HKEY_CURRENT_USER, key);
Win::Registry::readString(HKEY_CURRENT_USER, key, "folder", folder, sizeof(folder), "C:\\");
Win::Registry::readDWord(HKEY_CURRENT_USER, key, "port", &port, 8080);
Win::Registry::writeDWord(HKEY_CURRENT_USER, key, "port", port);
```

Call a method of a COM object:

```cpp
if (!Ole::isValid()) {
	return false;
};
IDispatch *object = ...;                       // CoCreateInstance, a property, an argument
Function function;
BSTR name = SysAllocString(L"Add");
function.setObject(object);
function.functionName(name);
SysFreeString(name);
HRESULT hr = function.invoke(Variant((long int)40), Variant((long int)2));
if (SUCCEEDED(hr)) {
	long int sum = (long int)DispatchVariant(function.result());
};
```

Expose methods to a script host (`window.external`, a scripting engine):

```cpp
class External : public virtual Dispatch {
	public:
		XYO_WIN_DISPATCH__PROTOTYPE;
		void version(Variant &returnValue) {
			returnValue = XYO::Win::Version::version();   // LPCSTR -> BSTR
		};
		void log(Variant &returnValue, VARIANTARG *text) {
			BSTR value = DispatchVariant(text);         // borrowed, may be nullptr
			OutputDebugStringW((value != nullptr) ? value : L"");
		};
};

XYO_WIN_DISPATCH__TABLE_BEGIN(External, 1)
XYO_WIN_DISPATCH__TABLE_FUNCTION_0(version);
XYO_WIN_DISPATCH__TABLE_FUNCTION_1(log);
XYO_WIN_DISPATCH__TABLE_END(Dispatch)

// External external;   // must outlive every use of external.getIDispatchValue()
```

Screenshot to a resized PNG:

```cpp
TPointer<Bitmap> screen = Capture::captureDesktop();
if (screen) {
	screen->setAlpha32(255);
	TPointer<Image> image = Process::imageFromBitmap(screen);
	if (image) {
		TPointer<Image> half = Process::resize(image, image->width / 2, image->height / 2);
		Process::pngSave(half, "screen-half.png");
	};
};
```

## Building

`fabricare make`, `fabricare test`, `fabricare install`, `fabricare clean`
in the repository (see the `fabricare` skill; clear
`NoDefaultCurrentDirectoryInExePath` first on this machine). Consumers add
`"xyo-win"` to `dependency` in `fabricare.json`. test.03 opens a small
window for a moment and needs an interactive desktop; it has a 30 second
watchdog. Source files use CRLF.
