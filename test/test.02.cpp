// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

// IDispatch dispatch tables (Dispatch / Function).
// Regression test for:
//  - the base class handoff, a call whose id equals (derivedStartId +
//    derivedFunctionCount) used to be swallowed by the derived table instead
//    of reaching the base,
//  - case insensitive name lookup (OLE Automation convention),
//  - Function::invoke argument passing and unknown name reporting.

#include <XYO/Win.hpp>

#include <stdio.h>
#include <string.h>

using namespace XYO::Win;

static int failed = 0;

static void check(bool condition, const char *name) {
	printf("%s %s\r\n", condition ? "[ok]  " : "[FAIL]", name);
	if (!condition) {
		++failed;
	};
};

// Base exposes baseMethod, its ids start right after the derived table.
class Base : public virtual Dispatch {
	public:
		XYO_WIN_DISPATCH__PROTOTYPE;

		void baseMethod(Variant &returnValue) {
			returnValue = (long int)100;
		};
};

XYO_WIN_DISPATCH__TABLE_BEGIN(Base, 3)
XYO_WIN_DISPATCH__TABLE_FUNCTION_0(baseMethod);
XYO_WIN_DISPATCH__TABLE_END(Dispatch)

// Derived owns ids 1 (derivedMethod) and 2 (addOne), so after its table the
// running id is 3, exactly baseMethod's id.
class Derived : public virtual Base {
	public:
		XYO_WIN_DISPATCH__PROTOTYPE;

		void derivedMethod(Variant &returnValue) {
			returnValue = (long int)200;
		};

		void addOne(Variant &returnValue, VARIANTARG *arg0) {
			DispatchVariant value(arg0);
			returnValue = (long int)((long int)value + 1);
		};
};

XYO_WIN_DISPATCH__TABLE_BEGIN(Derived, 1)
XYO_WIN_DISPATCH__TABLE_FUNCTION_0(derivedMethod);
XYO_WIN_DISPATCH__TABLE_FUNCTION_1(addOne);
XYO_WIN_DISPATCH__TABLE_END(Base)

// Call a named method with no arguments and return its VT_I4 result, or a
// sentinel on failure.
static long int callNoArgs(IDispatch *object, const wchar_t *name, HRESULT *hrOut) {
	Function function;
	BSTR functionName = SysAllocString(name);
	function.setObject(object);
	function.functionName(functionName);
	SysFreeString(functionName);

	HRESULT hr = function.invoke();
	if (hrOut != NULL) {
		*hrOut = hr;
	};
	if ((hr == S_OK) && (function.result()->vt == VT_I4)) {
		return function.result()->lVal;
	};
	return -1;
};

int main(int cmdN, char *cmdS[]) {
	try {
		Derived object;
		IDispatch *dispatch = object.getIDispatchValue();
		HRESULT hr;

		// Derived's own method
		check(callNoArgs(dispatch, L"derivedMethod", &hr) == 200, "derivedMethod");

		// Base method reached through the derived table, this is the id that
		// the off by one handoff used to swallow.
		check(callNoArgs(dispatch, L"baseMethod", &hr) == 100, "baseMethod through base handoff");

		// Case insensitive lookup
		check(callNoArgs(dispatch, L"BASEMETHOD", &hr) == 100, "baseMethod case insensitive");

		// One argument method
		{
			Function function;
			BSTR name = SysAllocString(L"addOne");
			function.setObject(dispatch);
			function.functionName(name);
			SysFreeString(name);
			hr = function.invoke(Variant((long int)41));
			check((hr == S_OK) && (function.result()->vt == VT_I4) && (function.result()->lVal == 42), "addOne argument");
		};

		// Unknown name is reported, not silently accepted
		callNoArgs(dispatch, L"doesNotExist", &hr);
		check(FAILED(hr), "unknown name fails");

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
