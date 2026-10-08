# XYO Win — Documentation

`xyo-win` is the Windows layer of the XYO C++ library stack: a small set of
classes and functions over the Win32 API for the XYO tools that need a
window, a tray application, the registry, shell links, COM automation,
local user accounts or a screenshot. It sits on top of `xyo-system`
(strings, files, `IApplication`) and `xyo-pixel32` (the `Bitmap` / PNG
code used by the screen capture), and is **Windows only**.

It keeps to plain Win32: the types you pass are `HWND`, `HKEY`, `MSG`,
`WNDCLASSEX`, `VARIANT`, `IDispatch *`, and nothing hides the API — the
library removes the boilerplate around it and fixes the details that are
easy to get wrong:

- **Windows as objects.** `Window` binds an `HWND` to a C++ object; you
  override `windowProcedure(uMsg, wParam, lParam)` instead of writing a
  `WNDPROC`. `Application` is a top-level window that is also the
  `IApplication` of the program (`XYO_APPLICATION_WINMAIN`), registers its
  window class, creates the window and runs the message loop.
  `SimpleApplication` adds a class name, a title, the icon from resource `1`
  and an optional "single instance" check.
- **One message loop.** `MessageManager` runs the loop for a list of
  windows: it ends when the last window is destroyed (or on `WM_QUIT`,
  then it closes the remaining windows), returns the `PostQuitMessage`
  exit code, does not spin while idle, and lets each window filter
  accelerator keys first.
- **COM automation, both directions.** `Function` calls a method of any
  `IDispatch` object by name; `Variant` owns a `VARIANT`;
  `DispatchVariant` reads arguments with type conversion and by reference
  values; `Dispatch` plus the `XYO_WIN_DISPATCH__*` macros expose your own
  C++ methods as an `IDispatch` (for scripts, `window.external`, ...).
- **System helpers.** `Registry::` (read / write strings and DWORDs, keys,
  ANSI and wide), `Shell::` (`.lnk` shortcuts with "run as administrator",
  start a process as another user), `User::` (local or AD accounts through
  ADSI, hide an account from the welcome screen), `Capture::` (desktop or
  window to `Bitmap` / PNG), `Util::` (post / send a message to groups of
  windows), `Ole::isValid()` (per thread OLE initialization).

```
proxy-forward, xyo-win-service, xyo-win-inject, dll-inject, ...
xyo-win               <-- this library (+ user32, gdi32, ole32, oleaut32,
                          shell32, secur32, activeds, adsiid, netapi32)
xyo-pixel32           (Bitmap, PNG save / load)
xyo-cryptography
xyo-system            (IApplication, String, Shell, File)
xyo-encoding          (String, UTF conversion)
xyo-multithreading
xyo-data-structures   (TDoubleEndedQueue)
xyo-managed-memory    (Object, TPointer)
xyo-platform
```

## Why it exists

Every small Windows tool needs the same pieces: register a class, create a
window, run a loop, keep a pointer to "this" in the window, read a setting
from the registry, put a shortcut on the desktop. Large frameworks (MFC,
ATL, WTL) bring much more than that and do not fit the XYO memory model.
`xyo-win` is the minimum, written once, with the edge cases handled:

| Problem | What `xyo-win` does |
|---------|---------------------|
| Mapping an `HWND` back to its C++ object | `Window` stores itself in the window extra bytes at `WM_NCCREATE` and dispatches every message to the virtual `windowProcedure` |
| A C++ window object deleted while its `HWND` is alive | `Window` holds a reference to itself from `WM_NCCREATE` to `WM_NCDESTROY` |
| `DestroyWindow` fails from a thread that does not own the window | `~Window` asks the owner thread (registered message, `SendMessage`) |
| Busy waiting message loops, lost exit codes | `MessageManager` waits with `MsgWaitForMultipleObjects` and returns the `WM_QUIT` code |
| A second copy of a tray tool | `SimpleApplication` with `singleInstance_` activates the running one and exits |
| `RegQueryValueEx` strings without a terminator, wrong byte sizes for wide strings | `Registry::readString*` use `RegGetValue` (always terminated), sizes are in bytes, defaults on failure |
| `IDispatch::Invoke` arguments arrive reversed, by reference or with another type | `Function::invoke(a, b, c)` takes them in call order; `DispatchVariant` follows `VT_BYREF` and converts with `VariantChangeType` |
| Saving a screenshot as PNG gives a transparent image | `Capture::*ToPNGFile` set the alpha to opaque before saving |
| OLE initialized once per process, but it is per thread | `Ole::isValid()` initializes the calling thread once and uninitializes it at thread exit |

## Concepts at a glance

| Need | Use | Notes |
|------|-----|-------|
| GUI program with one main window | derive from `SimpleApplication`, `XYO_APPLICATION_WINMAIN(MyApplication)` | set `className_`, `windowName_` in the constructor |
| Handle messages | override `LRESULT windowProcedure(UINT, WPARAM, LPARAM)` | call the base for the rest |
| More windows in the same loop | `Window` + `Window::registerClass` + `create(...)`, `messageManager_->add(window)` | the loop ends when all are gone |
| Exit code | `PostQuitMessage(code)` in `WM_DESTROY` | returned by `Application::main` |
| Call a COM method by name | `Function f; f.setObject(dispatch); f.functionName(name); f.invoke(Variant(...), ...)` | result in `f.result()` |
| Expose C++ methods to COM | derive from `Dispatch`, `XYO_WIN_DISPATCH__TABLE_BEGIN / FUNCTION_n / END` | ids start at `StartId > 0` |
| Registry value | `XYO::Win::Registry::readString(HKEY_CURRENT_USER, key, name, buffer, size, "default")` | qualify `Registry::` (clashes with `XYO::ManagedMemory::Registry`) |
| Desktop shortcut | `XYO::Win::Shell::createLink("x.lnk", dir, exe, args, icon, 0, false)` | UTF-8; qualify `Shell::` (clashes with `XYO::System::Shell`) |
| Screenshot | `Capture::captureDesktopToPNGFile("screen.png")` | all monitors |

## Contents

| Document | What it covers |
|----------|----------------|
| [Getting started](getting-started.md) | Build it, depend on it, include and namespaces, first program, conventions |
| [Windows and the message loop](windows.md) | `Window`, `Application`, `SimpleApplication`, `MessageManager`, `INotify` / `TNotify`, threads |
| [COM automation](automation.md) | `Ole`, `Variant`, `DispatchVariant`, `Function`, `Dispatch` and the dispatch table macros |
| [System helpers](system.md) | `Registry`, `Shell` (links, run as), `User` (accounts), `Capture`, `Util` |
| [API reference](reference.md) | Every public symbol on one page, including the discontinued `WebBrowser` |

The types used here come from the layers below: `Object` / `TPointer` from
`xyo-managed-memory`, `TDoubleEndedQueue` from `xyo-data-structures`,
`String` / `StringUTF16` / `TUTFConvert` from `xyo-encoding`,
`IApplication` from `xyo-system`, `Bitmap` / `Image` / `Process::` from
`xyo-pixel32`. Each repository has its own `docs/`.

## Source map

```
source/XYO/Win.hpp                       umbrella header, include this
source/XYO/Win/
    Dependency.hpp                       windows.h, ole2.h, shlobj.h, iads.h, ...; export macro, namespaces
    Window[.cpp]                         Window: HWND <-> object, window procedure, registerClass, create
    INotify.hpp, TNotify.hpp             callback interface, member function callback
    MessageManager[.cpp]                 window list + message loop
    Application[.cpp]                    top-level window + IApplication + message loop
    SimpleApplication[.cpp]              class / title / icon / single instance
    OLE[.cpp]                            Ole::isValid, per thread OleInitialize
    Variant.hpp                          owning VARIANT wrapper
    DispatchVariant.hpp                  non owning argument reader with conversion
    Function[.cpp]                       call an IDispatch method by name
    Dispatch[.cpp]                       IDispatch implementation + XYO_WIN_DISPATCH__* table macros
    Registry[.cpp]                       Registry:: functions
    Shell[.cpp]                          Shell::createLink, runAs
    User[.cpp]                           User:: ADSI account functions
    Capture[.cpp]                        Capture:: desktop / window screenshots
    Util[.cpp]                           Util:: message broadcast helpers
    Discontinued/WebBrowser[.cpp]        Internet Explorer (MSHTML) host window, kept for old code
    Copyright / License / Version         library metadata
    Library.rc / *.rh                    Windows DLL version resource
test/test.01.cpp                         Registry round trip, Variant, DispatchVariant
test/test.02.cpp                         dispatch tables: base class handoff, case insensitive names
test/test.03.cpp                         Application, MessageManager, notify, Capture
test/test.04.cpp                         Shell links, Registry edge cases, Variant, Dispatch, metadata
```

## AI assistant skill

A Claude Code skill describing how to use this library lives in
[`.claude/skills/xyo-win/`](../.claude/skills/xyo-win/SKILL.md). It is
picked up automatically inside this repository; copy the folder to
`~/.claude/skills/` to have it available in the projects that depend on
`xyo-win`.
