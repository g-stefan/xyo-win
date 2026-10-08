# Getting started

## 1. Build and install

The library is built with [fabricare](https://github.com/g-stefan/fabricare),
the build tool used by all XYO C++ projects. The XYO layers up to
`xyo-system`, `xyo-cryptography` and `xyo-pixel32` must be installed to
the SDK first. It builds on Windows only (MSVC). From the repository root:

```bash
fabricare make       # build into output/
fabricare test       # build and run test/test.*.cpp (run make first)
fabricare install    # copy output/{bin,include,lib} to ~/.fabricare/<platform>
fabricare clean      # remove output/ and temp/
```

| Project   | Kind                                | Notes |
|-----------|-------------------------------------|-------|
| `xyo-win` | DLL (`dll-or-lib`)                  | a static library when the platform is built static; links `secur32`, `adsiid`, `activeds`, `netapi32` |
| `test.01` | test executable                     | Registry round trip in `HKEY_CURRENT_USER`, `Variant`, `DispatchVariant` |
| `test.02` | test executable                     | `Dispatch` tables: base class handoff, case insensitive names, `Function` |
| `test.03` | test executable                     | `Application`, `MessageManager`, `TNotify`, `Capture` (opens a small window for a moment) |
| `test.04` | test executable                     | `Shell::createLink`, `Registry` edge cases, `Variant`, `Dispatch`, metadata |

The tests need no elevation: the registry tests use
`HKEY_CURRENT_USER\Software\XYO\Win\Test*` and remove their keys, the shell
link test writes `test.04.lnk` in the current folder and deletes it. The
`User::` account functions are not tested (they change the machine).

## 2. Depend on it from another fabricare project

In the consumer's `fabricare.json`:

```json
{
	"name": "my-tool",
	"make": "exe",
	"sourcePath": "XYO/MyTool",
	"dependency": [
		"xyo-win"
	]
}
```

`xyo-system`, `xyo-cryptography`, `xyo-pixel32` and the layers below come
in as transitive dependencies, together with the Windows import libraries
listed above.

## 3. Include and namespaces

```cpp
#include <XYO/Win.hpp>
```

The umbrella header includes `<windows.h>` (with `WIN32_LEAN_AND_MEAN`
and `SECURITY_WIN32`), `ole2.h`, `shlobj.h`, `Shobjidl.h`, `iads.h`,
`adshlp.h`, `Security.h`, `shellapi.h`, then `<XYO/System.hpp>`,
`<XYO/Pixel32.hpp>` and every public header of this library except the
discontinued `WebBrowser` (include
`<XYO/Win/Discontinued/WebBrowser.hpp>` for it).

The window functions use the `TCHAR` Win32 names (`LPCTSTR`,
`CreateWindowEx`, ...) and the library is built **without** `UNICODE`, so
they are the ANSI (`char`) versions. `Registry`, `Shell` and `User` have
explicit wide (`...W`) functions; `Shell::createLink` / `runAs` take UTF-8.

Namespace `XYO::Win` contains

```cpp
using namespace XYO::ManagedMemory;
using namespace XYO::DataStructures;
using namespace XYO::Encoding;
using namespace XYO::Multithreading;
using namespace XYO::System;
using namespace XYO::Pixel32;
```

so `using namespace XYO::Win;` is enough to write `TPointer`, `String`,
`Bitmap`, `Window`, `Variant` unqualified. The free functions are in nested
namespaces: `Registry::`, `Shell::`, `User::`, `Capture::`, `Util::`,
`Ole::`.

**Two of them clash with namespaces of the lower layers** once you write
`using namespace XYO::Win;`:

| Write | Not | Because of |
|-------|-----|------------|
| `XYO::Win::Registry::readString(...)` | `Registry::readString(...)` | `XYO::ManagedMemory::Registry` |
| `XYO::Win::Shell::createLink(...)` | `Shell::createLink(...)` | `XYO::System::Shell` |

Inside `namespace XYO::Win { ... }` (or a namespace of your own that
declares `namespace Win = XYO::Win;`) the short names work.

The same applies to your own names: a global `class Application` (or
`Window`, `Function`, `Variant`, ...) next to `using namespace XYO::Win;`
is ambiguous with `XYO::Win::Application`. Give your classes other names
(`MyTool` below) or put them in a namespace of your own, as the XYO tools
do (`namespace XYO::ProxyForward { class Application : public virtual
SimpleApplication ... }`). `Capture::`,
`User::`, `Util::`, `Ole::` do not clash. The metadata namespaces exist in
every XYO layer: write `XYO::Win::Version::version()` in full.

## 4. First program

A tray-style tool: one hidden window, a single running instance, a value
stored in the registry, a shortcut created on request, a screenshot
command, exit code from `PostQuitMessage`.

```cpp
#include <XYO/Win.hpp>
#include <string.h>

using namespace XYO::Win;

namespace Win = XYO::Win;

class MyTool : public virtual SimpleApplication {
		XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(MyTool);

	public:
		MyTool();

		int main(int cmdN, char *cmdS[]);
		int setShowCmd(int);
		LRESULT windowProcedure(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

MyTool::MyTool() {
	className_ = "XYO.MyTool";        // window class, also used to find the running instance
	windowName_ = "My Tool";
	singleInstance_ = true;           // a second start activates the first one and exits
};

int MyTool::main(int cmdN, char *cmdS[]) {
	if ((cmdN > 1) && (strcmp(cmdS[1], "--shortcut") == 0)) {
		char exe[MAX_PATH];
		GetModuleFileNameA(NULL, exe, MAX_PATH);
		// UTF-8 paths; NULL leaves a field unset
		return Win::Shell::createLink("My Tool.lnk", NULL, exe, NULL, exe, 0, false) ? 0 : 1;
	};
	if ((cmdN > 1) && (strcmp(cmdS[1], "--screenshot") == 0)) {
		return Capture::captureDesktopToPNGFile("screen.png") ? 0 : 1;
	};
	return SimpleApplication::main(cmdN, cmdS); // single instance check, window, message loop
};

int MyTool::setShowCmd(int) {
	return SW_HIDE; // runs in the background
};

LRESULT MyTool::windowProcedure(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE: {
		unsigned long int runs;
		Win::Registry::createKey(HKEY_CURRENT_USER, "Software\\XYO\\MyTool");
		Win::Registry::readDWord(HKEY_CURRENT_USER, "Software\\XYO\\MyTool", "runs", &runs, 0);
		Win::Registry::writeDWord(HKEY_CURRENT_USER, "Software\\XYO\\MyTool", "runs", runs + 1);
	};
		break;
	case WM_DESTROY:
		PostQuitMessage(0); // the exit code of the program
		break;
	default:
		break;
	};
	return SimpleApplication::windowProcedure(uMsg, wParam, lParam);
};

XYO_APPLICATION_WINMAIN(MyTool);
```

`XYO_APPLICATION_WINMAIN(T)` (from `xyo-system`) defines `WinMain`,
initializes the XYO memory registry, calls `T::initMemory()`, splits the
command line into `cmdN` / `cmdS` and calls `T::main`. Use
`XYO_APPLICATION_MAIN(T)` for a console program that opens windows.

To close the running instance from outside, find it the same way
`SimpleApplication` does and post `WM_CLOSE`:

```cpp
HWND running = FindWindowExA(NULL, NULL, "XYO.MyTool", "My Tool");
if (running) {
	PostMessage(running, WM_CLOSE, 0, 0);
};
```

A complete real program built this way is `proxy-forward`
(`source/XYO/ProxyForward/Application.cpp`): command line in `main`, a
hidden `SimpleApplication`, a server thread started in `WM_CREATE` and
stopped in `WM_DESTROY`.

## 5. Conventions

- **Results.** `bool` / `BOOL` functions return `false` / `FALSE` on any
  failure, `HRESULT` functions the COM error, `TPointer` results
  `nullptr`. Nothing throws. Use `GetLastError()` right after a failed
  Win32 based call for details.
- **Strings.** `Registry::*`, `User::*` take ANSI (`char`) or wide
  (`wchar_t`) strings as in the Win32 API; `Shell::createLink` / `runAs`
  take UTF-8 and convert to UTF-16; `Variant(LPCSTR)` converts from the
  ANSI code page; `Function::functionName` takes a `BSTR` (copied).
- **Ownership.** Windows, notifications and message managers are
  `Object`s held by `TPointer` (or on the stack for the application).
  COM interfaces are raw pointers with the usual `AddRef` / `Release`
  rules; `Variant` owns its value, `DispatchVariant` never does.
- **Threads.** A window belongs to the thread that created it, and its
  messages are processed by that thread's loop: run one `MessageManager`
  per GUI thread. OLE / COM is initialized per thread by `Ole::isValid()`
  (single threaded apartment). The `Registry`, `Shell`, `User`, `Capture`
  functions are thread safe in the Win32 sense (no shared state).
- **Windows only.** There is no Linux build of this library.
