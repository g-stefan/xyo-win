# API reference

Everything is declared in `<XYO/Win.hpp>`, namespace `XYO::Win`, unless
noted. Windows only. `false` / `FALSE` / `nullptr` / a failed `HRESULT`
mean failure; nothing throws.

## Window (`Window.hpp`)

```cpp
class Window : public virtual Object {                // not copyable / movable
	Window();
	~Window();                                         // destroys a still open window (asks the owner thread)

	operator HWND();                                   // NULL before create / after WM_NCDESTROY
	operator HINSTANCE();                              // GetWindowLongPtr(GWLP_HINSTANCE)

	virtual LRESULT windowProcedure(UINT uMsg, WPARAM wParam, LPARAM lParam);  // default DefWindowProc
	virtual bool translateAccelerator(MSG &msg);       // true = handled, not dispatched; default false

	bool create(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle,
	            int x, int y, int nWidth, int nHeight,
	            HWND hWndParent, HMENU hMenu, HINSTANCE hInstance);  // CreateWindowEx, lpParam = this
	bool create(LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle,
	            int x, int y, int nWidth, int nHeight,
	            HWND hWndParent, HMENU hMenu, HINSTANCE hInstance);  // CreateWindow, lpParam = this

	void setNotifyOnCreate(INotify *);                 // called at WM_NCCREATE
	void setNotifyOnDestroy(INotify *);                // called at WM_NCDESTROY (used by MessageManager)

	static bool registerClass(WNDCLASS &);             // sets lpfnWndProc, cbWndExtra = sizeof(void *)
	static bool registerClass(WNDCLASSEX &);           // also cbSize
	static void initMemory();
};
```

## INotify / TNotify (`INotify.hpp`, `TNotify.hpp`)

```cpp
class INotify : public virtual Object {
	virtual void notify() = 0;
};

template <typename Class, typename Function, typename Data>
class TNotify : public virtual Object, public virtual INotify {   // not copyable
	void set(TPointer<Class> object, Function function, Data data);   // function: void (Class::*)(Data)
	void notify();                                     // (object->*function)(data)
	void clear();                                      // release the object
};
```

## MessageManager (`MessageManager.hpp`)

```cpp
class MessageManager : public virtual Object {        // not copyable
	typedef TDoubleEndedQueue<TPointer<Window>> WindowList;

	MessageManager();
	~MessageManager();

	WindowList::Node *add(Window *window);             // keep a reference, remove at WM_NCDESTROY
	void remove(WindowList::Node *window);             // stop tracking (does not destroy the window)

	int processAllMessages();                          // loop until no window is left; WM_QUIT code or 0
	void postMessageToAll(UINT);                       // PostMessage(hwnd, m, 0, 0) to each window
	void sendMessageToAll(UINT);                       // SendMessage(hwnd, m, 0, 0), safe if windows close

	static void initMemory();
};
```

## Application (`Application.hpp`)

```cpp
class Application : public virtual Window, public virtual IApplication {   // not copyable
	TPointer<MessageManager> messageManager_;          // protected, created by main

	int main(int cmdN, char *cmdS[]);                  // parseCommandLine, newWindow, processAllMessages

	virtual void setWindowClassEx(WNDCLASSEX &);       // customize the class, default nothing
	virtual void setCreateStruct(CREATESTRUCT &);      // customize the window, default nothing
	virtual int setShowCmd(int);                       // ShowWindow command, default unchanged
	virtual int parseCommandLine(int cmdN, char *cmdS[]);   // non-zero = exit with it, default 0
	virtual void initWindowClassEx(WNDCLASSEX &);      // defaults, class "Class.Unknown"
	virtual void initCreateStruct(CREATESTRUCT &, WNDCLASSEX &);   // defaults, title "Unknown"
	virtual bool newWindow(int cmdShow, bool regWndClass = false);  // build, create, show, add

	static void initMemory();
};
```

## SimpleApplication (`SimpleApplication.hpp`)

```cpp
class SimpleApplication : public virtual Application {   // not copyable
	// protected
	LPCSTR className_;                                 // "SimpleApplication"
	LPCSTR windowName_;                                // "Simple application"
	bool singleInstance_;                              // false
	bool isTrayIconic_;                                // false

	SimpleApplication();
	void setWindowClassEx(WNDCLASSEX &);               // className_, icon resource 1
	void setCreateStruct(CREATESTRUCT &);              // windowName_
	int main(int cmdN, char *cmdS[]);                  // single instance check, then Application::main
	HWND getSingleInstanceWindow();                    // FindWindowEx(className_, windowName_)

	static void initMemory();
};
```

## Ole (`OLE.hpp`)

```cpp
namespace Ole {
	bool isValid();                                    // OleInitialize once per thread; false if it failed
};
```

## Variant (`Variant.hpp`)

```cpp
class Variant : public virtual Object {               // owns var_, VariantClear in the destructor
	Variant();                                         // VT_EMPTY
	Variant(const Variant &);  Variant(Variant &&);    // VariantCopy / move
	Variant(const VARIANTARG &);                       // VariantCopy
	Variant(VARIANT_BOOL);                             // VT_BOOL
	Variant(const VARIANT_BOOL *);                     // VT_BYREF | VT_BOOL
	Variant(const VARIANTARG *);                       // VT_BYREF | VT_VARIANT
	Variant(unsigned long int);                        // VT_UI4
	Variant(long int);                                 // VT_I4
	Variant(int);                                      // VT_INT
	Variant(LPCSTR);                                   // VT_BSTR from the ANSI code page, NULL -> NULL BSTR
	Variant(const BSTR);                               // VT_BSTR, copy
	Variant(const IUnknown *);  Variant(const IDispatch *);     // VT_UNKNOWN / VT_DISPATCH, AddRef
	Variant(const IUnknown **); Variant(const IDispatch **);    // VT_BYREF | ...

	Variant &operator=(...);                           // the same types (and int -> VT_INT)

	VARIANTARG *value();  const VARIANTARG *value() const;

	operator VARIANT_BOOL();  operator VARIANT_BOOL *();  operator VARIANTARG *();
	operator unsigned long int();  operator long int();  operator int();
	operator BSTR();  operator IUnknown *();  operator IDispatch *();
	operator IUnknown **();  operator IDispatch **();  // the union member, vt not checked

	static void variantCopy(VARIANTARG *dest, const VARIANTARG *src);   // VariantCopy, dest empty on failure
	static BSTR fromString(LPCSTR);                    // ANSI -> new BSTR (caller frees), NULL -> NULL
};
```

## DispatchVariant (`DispatchVariant.hpp`)

```cpp
class DispatchVariant {                               // not copyable, does not own the variant
	DispatchVariant();
	DispatchVariant(VARIANTARG *);
	VARIANTARG *value();

	operator VARIANT_BOOL();  operator unsigned long int();  operator long int();
	operator int();  operator unsigned int();          // follow VT_BYREF | VT_VARIANT, VariantChangeType, 0 on failure
	operator BSTR();  operator IUnknown *();  operator IDispatch *();   // also by reference, no copy / AddRef
	operator VARIANT_BOOL *();  operator VARIANTARG *();
	operator IUnknown **();  operator IDispatch **();  // the by reference pointer
};
```

## Function (`Function.hpp`)

```cpp
class Function : public virtual Object {              // not copyable
	Function();
	~Function();                                       // free the name, clear the result, release the object

	void setObject(IDispatch *);                       // AddRef, resets the dispatch id
	void releaseObject();
	IDispatch *getObject();

	void functionName(BSTR name);                      // copied, resets the dispatch id
	BSTR functionName();

	HRESULT getDispatchId();                           // GetIDsOfNames (the object must be set)
	DISPID dispatchId();                               // DISPID_UNKNOWN until resolved

	HRESULT invoke();                                  // DISPATCH_METHOD
	HRESULT invoke(const Variant &a0, ..., const Variant &a7);   // 1 to 8 arguments, call order

	VARIANTARG *result();                              // valid until the next invoke
	EXCEPINFO *exceptInfo();
	UINT argErr();
};
```

## Dispatch (`Dispatch.hpp`)

```cpp
class Dispatch : public virtual Object, public virtual ::IDispatch {   // not copyable
	// IUnknown: QueryInterface (IUnknown, IDispatch), AddRef / Release no-ops returning 1
	// IDispatch: GetTypeInfoCount / GetTypeInfo E_NOTIMPL, GetIDsOfNames, Invoke -> invokeAndId
	virtual HRESULT invokeAndId(UINT mode, DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult,
	                            LPOLESTR *names, UINT count, DISPID *outID);   // mode 0: names -> ids, 1: call
	virtual void setDispatchFunctionsIdAndNames();     // unused hook
	virtual ::IDispatch *getIDispatchValue();          // this as IDispatch *
};

XYO_WIN_DISPATCH__PROTOTYPE                            // in the class: declare invokeAndId
XYO_WIN_DISPATCH__TABLE_BEGIN(Class, StartId)          // in a .cpp: first id (> 0)
XYO_WIN_DISPATCH__TABLE_FUNCTION_0(method) ... XYO_WIN_DISPATCH__TABLE_FUNCTION_8(method)
                                                       // void method(Variant &returnValue, VARIANTARG *a0, ...)
XYO_WIN_DISPATCH__TABLE_END(BaseClass)                 // unknown names / ids go to BaseClass
```

## Registry (`Registry.hpp`, namespace `XYO::Win::Registry`)

```cpp
BOOL createKey(HKEY masterkey, const char *key);
BOOL createKeyVolatile(HKEY masterkey, const char *key);
BOOL writeString(HKEY masterkey, const char *key, const char *reg, const char *str);   // NULL -> ""
BOOL writeDWord(HKEY masterkey, const char *key, const char *reg, unsigned long int value);
BOOL readString(HKEY masterkey, const char *key, const char *reg, char *str, unsigned long int sizeInBytes, const char *def);
BOOL readDWord(HKEY masterkey, const char *key, const char *reg, unsigned long int *value, unsigned long int def);
BOOL deleteKey(HKEY masterkey, const char *key, const char *reg, BOOL isValue);   // TRUE: value, FALSE: subkey
BOOL getStringLength(HKEY masterkey, const char *key, const char *reg, LPDWORD sizeInBytes);
// createKeyW, createKeyVolatileW, writeStringW, writeDWordW, readStringW, readDWordW, deleteKeyW,
// getStringLengthW: the same with const wchar_t * (sizes in bytes)
```

## Shell (`Shell.hpp`, namespace `XYO::Win::Shell`)

```cpp
bool createLink(const char *outputFile, const char *workingDirectory, const char *path,
                const char *arguments, const char *iconPath, int iconIndex, bool runAsAdministrator);   // UTF-8
bool createLinkW(const wchar_t *outputFile, const wchar_t *workingDirectory, const wchar_t *path,
                 const wchar_t *arguments, const wchar_t *iconPath, int iconIndex, bool runAsAdministrator);
bool runAs(const char *username, const char *password, const char *command);                     // UTF-8
bool runAsW(const wchar_t *username, const wchar_t *password, const wchar_t *command);
```

## User (`User.hpp`, namespace `XYO::Win::User`)

```cpp
HRESULT addToUserGroupsUser(IADsUser *pUser, BSTR memberPath);
HRESULT createUserAccountOnAD(wchar_t *userName, wchar_t *fullName, wchar_t *description, wchar_t *password,
                              wchar_t *containerPath, wchar_t *newUserPath, wchar_t *templateUserPath);
HRESULT removeUserAccountOnAD(wchar_t *userPath);
HRESULT removeUserFromItsGroups(IADsUser *pUser, BSTR member);
bool hideUserFromWelcomeScreen(wchar_t *userName);
bool showUserInWelcomeScreen(wchar_t *userName);
bool createUserAccountAsCurrentUserPrivilegeOnLocalComputer(wchar_t *userName, wchar_t *password, wchar_t *fullName, wchar_t *description);
bool deleteUserAccountOnLocalComputer(wchar_t *userName);
bool createHiddenUserAccountAsCurrentUserPrivilegeOnLocalComputer(wchar_t *userName, wchar_t *password, wchar_t *fullName, wchar_t *description);
```

## Capture (`Capture.hpp`, namespace `XYO::Win::Capture`)

```cpp
TPointer<Bitmap> captureDesktop();                     // virtual screen, 32 bits, alpha undefined
bool captureDesktopToPNGFile(const char *fileName);    // opaque RGBA PNG
TPointer<Bitmap> captureWindow(HWND hwnd);             // client area
bool captureWindowToPNGFile(HWND hwnd, const char *fileName);
```

## Util (`Util.hpp`, namespace `XYO::Win::Util`)

```cpp
void sendMessageToAllChildWindowsIE(HWND parent, UINT messageId, WPARAM wParam, LPARAM lParam);
void postMessageToProcessWindows(UINT messageId, WPARAM wParam, LPARAM lParam);
```

## Metadata

```cpp
namespace XYO::Win::Copyright { const char *copyright(); const char *publisher(); const char *company(); const char *contact(); };
namespace XYO::Win::License { std::string license(); std::string shortLicense(); };   // MIT
namespace XYO::Win::Version { const char *version(); const char *build(); const char *versionWithBuild(); const char *datetime(); };
```

## Discontinued: WebBrowser (`Discontinued/WebBrowser.hpp`)

Namespace `XYO::Win::Discontinued`, not included by `<XYO/Win.hpp>`. A
`Window` that hosts the Internet Explorer WebBrowser control (MSHTML)
filling its client area. Kept so that old programs still build; Internet
Explorer is retired, do not use it in new code (use WebView2).

```cpp
class WebBrowser : public virtual Window, IStorage, IOleInPlaceFrame, IOleClientSite, IOleInPlaceSite,
                   IOleCommandTarget, IDocHostUIHandler, IDocHostShowUI, IServiceProvider,
                   DWebBrowserEvents2, IDropTarget, IHttpSecurity, IWindowForBindingUI,
                   INewWindowManager, IAuthenticate, IInternetSecurityManager, IProtectFocus,
                   IHTMLOMWindowServices {
	WebBrowser();
	LRESULT windowProcedure(UINT, WPARAM, LPARAM);     // WM_CREATE: create the control, go to about:blank,
	                                                   // then to the default address; WM_SIZE: resize it
	bool translateAccelerator(MSG &);                  // the control's accelerators
	void setBrowserDefaultAddress(const String &url);  // before create, default "about:blank"
	int Navigate(String url);                          // 1 = started
	// DWebBrowserEvents2 virtual handlers: BeforeNavigate2, NavigateComplete, DocumentComplete,
	// TitleChange, StatusTextChange, NewWindow2, WindowClosing (closes the window), OnQuit, ...
	// the COM interfaces are implemented with no-op reference counting, like Dispatch
};

#define WUM_BROWSER_DO_NAVIGATE1 (WM_USER + 2000)      // internal, avoid these message numbers
#define WUM_BROWSER_DO_NAVIGATE2 (WM_USER + 2001)
```
