# COM automation

Four small pieces cover late bound COM (`IDispatch`) in both directions:

| Class | Owns | Use it to |
|-------|------|-----------|
| `Variant` | its `VARIANT` (`VariantClear` in the destructor) | build arguments and return values |
| `DispatchVariant` | nothing (points at a `VARIANTARG`) | read an argument you received, with conversion |
| `Function` | a `BSTR` name, a reference to the object, the result | call a method of an `IDispatch` by name |
| `Dispatch` + `XYO_WIN_DISPATCH__*` | nothing (no COM reference counting) | expose your C++ methods as an `IDispatch` |

## OLE initialization

```cpp
if (!Ole::isValid()) {
	return false;   // OleInitialize failed on this thread
};
```

`Ole::isValid()` calls `OleInitialize(NULL)` the first time it runs on a
thread (single threaded apartment) and `OleUninitialize` when the thread
ends; it returns whether the initialization succeeded (`S_OK` or
`S_FALSE`). It fails with `RPC_E_CHANGED_MODE` on a thread already in the
multithreaded apartment (`CoInitializeEx(NULL, COINIT_MULTITHREADED)`).
`Shell::createLink` and the discontinued `WebBrowser` call it themselves;
call it before `CoCreateInstance` in your own code. Use it on the main
thread or on threads started by `xyo-multithreading` (`Thread`), which
have the per thread registry it relies on.

## Variant

`Variant` is an `Object` holding one `VARIANTARG`. Constructors and
assignments pick the variant type from the C++ type:

| C++ value | `vt` | Notes |
|-----------|------|-------|
| `long int` | `VT_I4` | the usual automation integer: write `Variant((long int)5)` |
| `int` | `VT_INT` | `Variant(5)`, `v = 5` |
| `unsigned long int` | `VT_UI4` | |
| `VARIANT_BOOL` | `VT_BOOL` | `VARIANT_TRUE` / `VARIANT_FALSE`, cast: `Variant((VARIANT_BOOL)VARIANT_TRUE)` |
| `LPCSTR` | `VT_BSTR` | converted from the ANSI code page; `NULL` gives a `NULL` BSTR (empty) |
| `BSTR` | `VT_BSTR` | copied (`SysAllocString`), you keep yours |
| `IUnknown *` / `IDispatch *` | `VT_UNKNOWN` / `VT_DISPATCH` | `AddRef`, released with the variant |
| `VARIANT_BOOL *`, `VARIANTARG *`, `IUnknown **`, `IDispatch **` | `VT_BYREF \| ...` | output arguments, the pointer is not owned |
| `const VARIANTARG &`, `const Variant &` | same as source | deep copy (`VariantCopy`) |
| `Variant &&` | same as source | moved, the source becomes `VT_EMPTY` |

`value()` returns the `VARIANTARG *` (to pass to Win32 / COM functions).
The conversion operators (`(long int)v`, `(BSTR)v`, `(IDispatch *)v`, ...)
read the matching union member **without checking `vt`** — check
`v.value()->vt` first, or wrap the value in a `DispatchVariant` to convert.

## DispatchVariant

`DispatchVariant` wraps a `VARIANTARG *` you do not own (a dispatch
argument, a `Function::result()`), follows `VT_BYREF | VT_VARIANT`
chains, and converts:

| Read as | Behaviour |
|---------|-----------|
| `(long int)`, `(int)`, `(unsigned long int)`, `(unsigned int)`, `(VARIANT_BOOL)` | same type: the value; other types: `VariantChangeType` (so `"123"` gives `123`, `VT_I2` widens); failure or `NULL`: `0` / `VARIANT_FALSE` |
| `(BSTR)` | the string of a `VT_BSTR` or `VT_BYREF \| VT_BSTR`, else `nullptr`; not a copy, don't free it |
| `(IUnknown *)`, `(IDispatch *)` | the interface (also by reference), else `nullptr`; no `AddRef` |
| `(VARIANT_BOOL *)`, `(VARIANTARG *)`, `(IUnknown **)`, `(IDispatch **)` | the by reference pointer, to write output arguments |

```cpp
DispatchVariant count(argument);
long int n = (long int)count;   // works for VT_I4, VT_I2, VT_BSTR "42", by reference ...
```

## Function: call a method by name

```cpp
Function function;
BSTR name = SysAllocString(L"Navigate");
function.setObject(dispatch);         // AddRef, released by releaseObject / destructor
function.functionName(name);          // copied
SysFreeString(name);

HRESULT hr = function.invoke(Variant("https://example.com"));
if (SUCCEEDED(hr)) {
	DispatchVariant result(function.result());
	...
} else if (hr == DISP_E_EXCEPTION) {
	EXCEPINFO *e = function.exceptInfo(); // e->bstrDescription ...
};
```

- `invoke()` with 0 to 8 `Variant` arguments, **in call order** (the
  reversal `IDispatch::Invoke` needs is done for you), as
  `DISPATCH_METHOD`, `LOCALE_SYSTEM_DEFAULT`, no named arguments.
- The dispatch id is looked up (`GetIDsOfNames`) on the first call and
  cached; `setObject`, `releaseObject` and `functionName` reset it.
  `dispatchId()` shows it (`DISPID_UNKNOWN` = not resolved).
- `result()` is the return value, valid until the next call; it is
  cleared before each call. `exceptInfo()` and `argErr()` report
  `DISP_E_EXCEPTION` / `DISP_E_TYPEMISMATCH` details.
- `E_INVALIDARG` without an object or a name. Property get / put are not
  supported (only `DISPATCH_METHOD`).

## Dispatch: expose C++ methods

Derive from `Dispatch`, declare the dispatch function with
`XYO_WIN_DISPATCH__PROTOTYPE`, and write the method table in a `.cpp`:

```cpp
class Calculator : public virtual Dispatch {
	public:
		XYO_WIN_DISPATCH__PROTOTYPE;

		void answer(Variant &returnValue) {
			returnValue = (long int)42;
		};

		void add(Variant &returnValue, VARIANTARG *a, VARIANTARG *b) {
			returnValue = (long int)((long int)DispatchVariant(a) + (long int)DispatchVariant(b));
		};
};

XYO_WIN_DISPATCH__TABLE_BEGIN(Calculator, 1)    // first id, must be > 0
XYO_WIN_DISPATCH__TABLE_FUNCTION_0(answer);     // id 1
XYO_WIN_DISPATCH__TABLE_FUNCTION_2(add);        // id 2
XYO_WIN_DISPATCH__TABLE_END(Dispatch)           // the base class gets unknown names / ids
```

- `XYO_WIN_DISPATCH__TABLE_FUNCTION_n(name)` (`n` = 0 ... 8) binds the
  method `name` (also the script visible name, matched **case
  insensitively**) taking `Variant &returnValue` and `n` `VARIANTARG *`
  arguments in call order. A call with another argument count gets
  `DISP_E_BADPARAMCOUNT`.
- Ids are consecutive from `StartId`. `0` (`DISPID_VALUE`) is never
  dispatched, so start at `1` or more.
- **Inheritance.** `TABLE_END(Base)` hands names and ids it does not know
  to `Base::invokeAndId`. A derived class starts its table **before** the
  base's range: if `Derived` has 2 methods from id `1`, `Base` starts at
  `3` (see `test/test.02.cpp`). Overlapping ranges make base methods
  unreachable.
- `getIDispatchValue()` returns the `IDispatch *` to hand out (e.g. from
  `IDocHostUIHandler::GetExternal`, as an argument of a `Function`).
- `AddRef` / `Release` are no-ops returning `1`: the object's life is the
  C++ object's (stack, member, `TPointer`). Only give it to code that does
  not outlive it; it is not a COM server and has no type information
  (`GetTypeInfoCount` / `GetTypeInfo` return `E_NOTIMPL`).
- `Invoke` ignores `wFlags`, `riid`, `lcid` and named arguments; a `NULL`
  `pDispParams` counts as no arguments. `GetIDsOfNames` returns
  `DISP_E_UNKNOWNNAME` for a name no table knows.
- Return a value by assigning `returnValue`; it is copied to the caller's
  `pVarResult` (`VariantCopyInd`) when the caller asked for one.
