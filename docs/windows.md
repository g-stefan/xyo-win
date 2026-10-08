# Windows and the message loop

```
Object
 └─ Window                    HWND <-> object, virtual windowProcedure
     └─ Application           + IApplication: class, window, MessageManager loop
         └─ SimpleApplication + class name, title, icon 1, single instance
MessageManager                list of TPointer<Window> + the message loop
INotify / TNotify             "call this member function" callbacks
```

## Window

`Window` connects one `HWND` to one C++ object. You register a window class
through `Window::registerClass`, create the window with `Window::create`,
and every message for it arrives in the virtual

```cpp
virtual LRESULT windowProcedure(UINT uMsg, WPARAM wParam, LPARAM lParam);
```

The base implementation is `DefWindowProc`. Override it, handle what you
need, and return the base call for everything else:

```cpp
class Panel : public virtual Window {
	public:
		LRESULT windowProcedure(UINT uMsg, WPARAM wParam, LPARAM lParam) {
			switch (uMsg) {
			case WM_PAINT: {
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(*this, &ps);
				FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));
				EndPaint(*this, &ps);
			};
				return 0;
			default:
				break;
			};
			return Window::windowProcedure(uMsg, wParam, lParam);
		};
};

WNDCLASSEX wc;
memset(&wc, 0, sizeof(wc));
wc.hInstance = GetModuleHandle(NULL);
wc.hCursor = LoadCursor(NULL, IDC_ARROW);
wc.lpszClassName = "XYO.MyTool.Panel";
Window::registerClass(wc);            // sets cbSize, lpfnWndProc, cbWndExtra

TPointer<Panel> panel;
panel.newMemory();
panel->create(0, "XYO.MyTool.Panel", "Panel", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
              CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, NULL, NULL, GetModuleHandle(NULL));
```

How it works and what it implies:

- `registerClass(WNDCLASS &)` / `registerClass(WNDCLASSEX &)` set
  `lpfnWndProc` to the shared static procedure and `cbWndExtra` to
  `sizeof(void *)`: the extra bytes at offset `0` hold the `Window *`.
  Do not use `SetWindowLongPtr(hwnd, 0, ...)` yourself, and do not ask for
  more extra bytes through this class. Registering the same class name
  twice fails (`false`).
- `create(...)` (with or without `dwExStyle`) is `CreateWindow[Ex]` with
  `lpParam = this`. At `WM_NCCREATE` the object stores its `HWND`, puts
  itself in the extra bytes and **takes a reference to itself**; at
  `WM_NCDESTROY` it clears the `HWND` and drops that reference. So a
  `TPointer`-held window is never deleted while its `HWND` exists, even if
  you release all your pointers; the object can be deleted right after
  `WM_NCDESTROY`. A window on the stack is fine too (the reference count
  does not delete stack objects), as long as it outlives its `HWND`.
- A window of the class created with plain `CreateWindow` (no `Window`
  object, `lpParam` NULL) just gets `DefWindowProc`.
- `operator HWND()` gives the handle, `NULL` before `create` and after
  destruction; `operator HINSTANCE()` the instance of the window.
- `~Window()` destroys a still open window. `DestroyWindow` works only on
  the thread that owns the window, so from any other thread the destructor
  sends a registered message (`"XYO.Win.Window.DestroyWindow"`) to the
  owner thread and waits until it is done.
- `translateAccelerator(MSG &msg)` is called by `MessageManager` for every
  message before it is dispatched. Return `true` when you handled it (e.g.
  `TranslateAccelerator(*this, accel, &msg) != 0`, `IsDialogMessage`), the
  message is then not dispatched. The default returns `false`.
- `setNotifyOnCreate(INotify *)` is called at `WM_NCCREATE` (before your
  `windowProcedure` sees it), `setNotifyOnDestroy(INotify *)` at
  `WM_NCDESTROY` (after it). `MessageManager::add` uses the destroy
  notification itself — don't replace it on a window you added to a
  manager.
- `Window` is not copyable or movable. Call `Window::initMemory()` once
  (the application macros do this for you through `T::initMemory()`).

## Application

`Application` is a `Window` that is also the program (`IApplication`).
`Application::main(cmdN, cmdS)` does:

1. create its `MessageManager` (`messageManager_`),
2. `parseCommandLine(cmdN, cmdS)` — return non-zero to stop with that
   exit code (default `0`),
3. `newWindow(SW_SHOWDEFAULT, true)` — register the class and create the
   window, return `-1` on failure,
4. `messageManager_->processAllMessages()` and return its exit code.

`newWindow` builds the class and the window from overridable steps, each
"init" fills defaults and each "set" lets you change them:

| Step | Default | Override to |
|------|---------|-------------|
| `initWindowClassEx(WNDCLASSEX &)` | `CS_HREDRAW \| CS_VREDRAW \| CS_DBLCLKS \| ...`, `IDI_APPLICATION`, arrow cursor, `COLOR_APPWORKSPACE` brush, class `"Class.Unknown"` | replace the defaults entirely |
| `setWindowClassEx(WNDCLASSEX &)` | nothing | set `lpszClassName`, `hIcon`, `hbrBackground`, menu ... |
| `initCreateStruct(CREATESTRUCT &, WNDCLASSEX &)` | `CW_USEDEFAULT` position / size, overlapped resizable style, `WS_EX_APPWINDOW \| WS_EX_CLIENTEDGE`, title `"Unknown"` | replace the defaults entirely |
| `setCreateStruct(CREATESTRUCT &)` | nothing | set `lpszName`, `x`, `y`, `cx`, `cy`, `style`, `dwExStyle`, `hwndParent` ... |
| `setShowCmd(int)` | returns its argument | return `SW_HIDE` for a background tool, `SW_SHOWNOACTIVATE`, ... |

Then the window is shown with `ShowWindow(*this, setShowCmd(cmdShow))` and
added to the message manager. `newWindow(cmdShow, false)` creates another
window of the already registered class (useful after a "new window"
command).

```cpp
class Viewer : public virtual Application {
	public:
		void setWindowClassEx(WNDCLASSEX &wc) {
			wc.lpszClassName = "XYO.Viewer";
			wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		};

		void setCreateStruct(CREATESTRUCT &cs) {
			cs.lpszName = "Viewer";
			cs.cx = 800;
			cs.cy = 600;
		};

		LRESULT windowProcedure(UINT uMsg, WPARAM wParam, LPARAM lParam) {
			if (uMsg == WM_DESTROY) {
				PostQuitMessage(0);
			};
			return Application::windowProcedure(uMsg, wParam, lParam);
		};
};

XYO_APPLICATION_WINMAIN(Viewer);
```

`PostQuitMessage` is optional: the loop also ends when the last window is
destroyed. Call it to choose the exit code.

## SimpleApplication

`SimpleApplication` fills the common parts from four protected members,
set them in your constructor:

| Member | Default | Used for |
|--------|---------|----------|
| `LPCSTR className_` | `"SimpleApplication"` | `lpszClassName`, single instance lookup |
| `LPCSTR windowName_` | `"Simple application"` | window title, single instance lookup |
| `bool singleInstance_` | `false` | when `true`, `main` looks for a top-level window with the same class **and** title; if found it restores / shows / activates it and returns `0` without creating a window |
| `bool isTrayIconic_` | `false` | with `singleInstance_`, `true` = don't activate the running window (a hidden tray tool) |

`setWindowClassEx` also loads the icon with resource id `1` from the
executable (`LoadIcon(hInstance, MAKEINTRESOURCE(1))`), when there is one.
`getSingleInstanceWindow()` returns the running instance's window (or
`NULL`) — use it in your own `main` to implement "--stop" style commands.
The strings are pointers, keep them alive (literals, or `String` members
whose `value()` you assign).

## MessageManager

```cpp
TPointer<MessageManager> manager;
manager.newMemory();
manager->add(window1);   // TPointer<Window> kept until the window is destroyed
manager->add(window2);
int exitCode = manager->processAllMessages();
```

- `add(Window *)` appends the window to the list (the list holds a
  reference) and sets the window's destroy notification, so a destroyed
  window leaves the list by itself. Add a window after `create`.
- `processAllMessages()` runs the loop on the calling thread **while the
  list is not empty**:
  - each message is first offered to every window's `translateAccelerator`,
    then `TranslateMessage` + `DispatchMessage` (thread messages with no
    window, like `SetTimer(NULL, ...)` callbacks, are dispatched too);
  - with no message it blocks in `MsgWaitForMultipleObjects` (no CPU use);
  - on `WM_QUIT` it processes what is still queued, then destroys the
    windows left in the list one by one (a window it can not destroy, e.g.
    owned by another thread, is dropped from the list) and returns;
  - the result is the `WM_QUIT` exit code (`PostQuitMessage(code)`), also
    when the last window was destroyed before `WM_QUIT` was read; `0` when
    there was no `WM_QUIT`.
- `remove(node)` takes a window out of the list without destroying it
  (`add` returns the node).
- `postMessageToAll(UINT)` / `sendMessageToAll(UINT)` send a message
  (`wParam = lParam = 0`) to every window in the list; `sendMessageToAll`
  is safe when the message closes windows.

One manager per GUI thread: only the windows created by the thread that
calls `processAllMessages` get their messages there.

## INotify and TNotify

`INotify` is a one method interface (`virtual void notify() = 0`, an
`Object`). `TNotify<Class, Function, Data>` implements it by calling a
member function with a stored argument:

```cpp
class Counter : public virtual Object {
	public:
		int closed = 0;
		void onClosed(int) {
			++closed;
		};
};

typedef TNotify<Counter, void (Counter::*)(int), int> CounterNotify;

TPointer<Counter> counter;
counter.newMemory();
TPointer<CounterNotify> notify;
notify.newMemory();
notify->set(counter, &Counter::onClosed, 0);
window->setNotifyOnDestroy(notify);
```

`TNotify` holds a `TPointer<Class>` to the target; `clear()` releases it.
The window keeps the notification alive while it runs, so a notification
may replace or clear itself (`setNotifyOnDestroy(nullptr)`) from inside
`notify()`.

## Threads and windows

- A window's messages are processed by the thread that created it. Create
  and run a window (and its `MessageManager`) on the same thread.
- From a worker thread use `PostMessage(hwnd, WM_APP + n, ...)` to hand
  work to the window thread; `SendMessage` blocks until the window thread
  processes it.
- Destroying a `Window` object from another thread works (see `~Window`),
  but blocks until the owner thread runs its loop.
- `Util::postMessageToProcessWindows(id, w, l)` posts to every top-level
  window of the process, `Util::sendMessageToAllChildWindowsIE(parent, id,
  w, l)` sends to the `Internet Explorer_Server` children of a window
  (see [System helpers](system.md)).
