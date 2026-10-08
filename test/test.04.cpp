// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

// Shell links, OLE, Registry, Variant and Dispatch edge cases.
// Regression test for:
//  - XYO::Win::Shell::createLink / runAs with NULL arguments, the UTF-8 to UTF-16
//    conversion used to dereference the null pointer,
//  - XYO::Win::Registry::writeString / writeStringW with a NULL string,
//  - Variant(int), it used to be ambiguous,
//  - Dispatch::Invoke with NULL DISPPARAMS and QueryInterface with NULL.

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

// Read back a .lnk file
static bool readLink(const wchar_t *file, wchar_t *path, wchar_t *arguments, wchar_t *workingDirectory, DWORD *flags) {
	IShellLinkW *shellLink = nullptr;
	IPersistFile *persistFile = nullptr;
	IShellLinkDataList *dataList = nullptr;
	bool retV = false;

	if (CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void **)&shellLink) != S_OK) {
		return false;
	};
	if (shellLink->QueryInterface(IID_IPersistFile, (void **)&persistFile) == S_OK) {
		if (persistFile->Load(file, STGM_READ) == S_OK) {
			path[0] = 0;
			arguments[0] = 0;
			workingDirectory[0] = 0;
			shellLink->GetPath(path, MAX_PATH, NULL, SLGP_RAWPATH);
			shellLink->GetArguments(arguments, MAX_PATH);
			shellLink->GetWorkingDirectory(workingDirectory, MAX_PATH);
			*flags = 0;
			if (shellLink->QueryInterface(IID_IShellLinkDataList, (void **)&dataList) == S_OK) {
				dataList->GetFlags(flags);
				dataList->Release();
			};
			retV = true;
		};
		persistFile->Release();
	};
	shellLink->Release();
	return retV;
};

static void testShell() {
	wchar_t directory[MAX_PATH];
	wchar_t linkW[MAX_PATH];
	char link[MAX_PATH];
	wchar_t path[MAX_PATH];
	wchar_t arguments[MAX_PATH];
	wchar_t workingDirectory[MAX_PATH];
	DWORD flags;

	check(XYO::Win::Ole::isValid(), "XYO::Win::Ole::isValid");

	GetCurrentDirectoryW(MAX_PATH, directory);
	swprintf_s(linkW, MAX_PATH, L"%s\\test.04.lnk", directory);
	WideCharToMultiByte(CP_UTF8, 0, linkW, -1, link, MAX_PATH, NULL, NULL);

	// All fields
	DeleteFileW(linkW);
	check(XYO::Win::Shell::createLink(link, "C:\\Windows", "C:\\Windows\\notepad.exe", "readme.txt", "C:\\Windows\\notepad.exe", 0, true), "createLink");
	check(readLink(linkW, path, arguments, workingDirectory, &flags), "createLink readable");
	check(_wcsicmp(path, L"C:\\Windows\\notepad.exe") == 0, "createLink path");
	check(wcscmp(arguments, L"readme.txt") == 0, "createLink arguments");
	check(_wcsicmp(workingDirectory, L"C:\\Windows") == 0, "createLink working directory");
	check((flags & SLDF_RUNAS_USER) != 0, "createLink run as administrator");

	// Optional fields left out
	DeleteFileW(linkW);
	check(XYO::Win::Shell::createLink(link, NULL, "C:\\Windows\\notepad.exe", NULL, NULL, 0, false), "createLink with NULL optional fields");
	check(readLink(linkW, path, arguments, workingDirectory, &flags), "createLink NULL fields readable");
	check(_wcsicmp(path, L"C:\\Windows\\notepad.exe") == 0, "createLink NULL fields path");
	check(arguments[0] == 0, "createLink NULL fields no arguments");
	check((flags & SLDF_RUNAS_USER) == 0, "createLink not run as administrator");
	DeleteFileW(linkW);

	// Required fields
	check(!XYO::Win::Shell::createLink(NULL, NULL, "C:\\Windows\\notepad.exe", NULL, NULL, 0, false), "createLink without file fails");
	check(!XYO::Win::Shell::createLink(link, NULL, NULL, NULL, NULL, 0, false), "createLink without path fails");
	check(!XYO::Win::Shell::createLinkW(NULL, NULL, NULL, NULL, NULL, 0, false), "createLinkW without file fails");
	check(GetFileAttributesW(linkW) == INVALID_FILE_ATTRIBUTES, "no link written on failure");

	// A thread in the multithreaded apartment: OLE can not be initialized
	// there, but the shell link can be created
	struct MTA {
			const char *link;
			bool oleValid;
			bool created;

			static void procedure(MTA *this_) {
				CoInitializeEx(NULL, COINIT_MULTITHREADED);
				this_->oleValid = XYO::Win::Ole::isValid();
				this_->created = XYO::Win::Shell::createLink(this_->link, NULL, "C:\\Windows\\notepad.exe", NULL, NULL, 0, false);
				CoUninitialize();
			};
	} mta = {link, true, false};
	Thread thread;
	check(thread.start((ThreadProcedure)MTA::procedure, &mta), "start MTA thread");
	thread.join();
	check(!mta.oleValid, "Ole::isValid false in the multithreaded apartment");
	check(mta.created, "createLink in the multithreaded apartment");
	check(readLink(linkW, path, arguments, workingDirectory, &flags) && (_wcsicmp(path, L"C:\\Windows\\notepad.exe") == 0), "createLink in the multithreaded apartment readable");
	DeleteFileW(linkW);

	// runAs only checks its arguments here, no logon is attempted
	check(!XYO::Win::Shell::runAs(NULL, NULL, "cmd.exe"), "runAs without user fails");
	check(!XYO::Win::Shell::runAs("user", "password", NULL), "runAs without command fails");
	check(!XYO::Win::Shell::runAsW(NULL, NULL, NULL), "runAsW without arguments fails");
};

static void testRegistry() {
	const char *key = "Software\\XYO\\Win\\Test04";
	const wchar_t *keyW = L"Software\\XYO\\Win\\Test04W";
	char buffer[16];
	wchar_t bufferW[16];
	DWORD length;

	check(XYO::Win::Registry::createKey(HKEY_CURRENT_USER, key) == TRUE, "createKey");
	check(XYO::Win::Registry::writeString(HKEY_CURRENT_USER, key, "null", NULL) == TRUE, "writeString NULL");
	strcpy_s(buffer, sizeof(buffer), "x");
	check(XYO::Win::Registry::readString(HKEY_CURRENT_USER, key, "null", buffer, sizeof(buffer), "def") == TRUE, "readString of NULL");
	check(buffer[0] == 0, "NULL is stored as the empty string");

	// A value longer than the buffer: FALSE and the default
	check(XYO::Win::Registry::writeString(HKEY_CURRENT_USER, key, "long", "0123456789abcdefghijklmnopqrstuvwxyz") == TRUE, "writeString long");
	check(XYO::Win::Registry::readString(HKEY_CURRENT_USER, key, "long", buffer, sizeof(buffer), "def") == FALSE, "readString too small buffer");
	check(strcmp(buffer, "def") == 0, "readString too small buffer default");
	check(XYO::Win::Registry::getStringLength(HKEY_CURRENT_USER, key, "long", &length) == TRUE, "getStringLength");
	check(length == 37, "getStringLength includes the terminator");

	// A NULL default is the empty string
	check(XYO::Win::Registry::readString(HKEY_CURRENT_USER, key, "missing", buffer, sizeof(buffer), NULL) == FALSE, "readString missing NULL default");
	check(buffer[0] == 0, "readString NULL default is empty");

	// Wrong type
	check(XYO::Win::Registry::writeDWord(HKEY_CURRENT_USER, key, "dw", 1) == TRUE, "writeDWord");
	check(XYO::Win::Registry::readString(HKEY_CURRENT_USER, key, "dw", buffer, sizeof(buffer), "def") == FALSE, "readString of a DWORD fails");
	unsigned long int dword;
	check(XYO::Win::Registry::readDWord(HKEY_CURRENT_USER, key, "long", &dword, 9) == FALSE, "readDWord of a string fails");
	check(dword == 9, "readDWord of a string default");

	// Missing key
	check(XYO::Win::Registry::writeString(HKEY_CURRENT_USER, "Software\\XYO\\Win\\Test04Missing", "x", "y") == FALSE, "writeString missing key fails");

	check(XYO::Win::Registry::deleteKey(HKEY_CURRENT_USER, "Software\\XYO\\Win", "Test04", FALSE) == TRUE, "deleteKey with values");

	check(XYO::Win::Registry::createKeyW(HKEY_CURRENT_USER, keyW) == TRUE, "createKeyW");
	check(XYO::Win::Registry::writeStringW(HKEY_CURRENT_USER, keyW, L"null", NULL) == TRUE, "writeStringW NULL");
	bufferW[0] = L'x';
	bufferW[1] = 0;
	check(XYO::Win::Registry::readStringW(HKEY_CURRENT_USER, keyW, L"null", bufferW, sizeof(bufferW), L"def") == TRUE, "readStringW of NULL");
	check(bufferW[0] == 0, "NULL is stored as the empty wide string");
	check(XYO::Win::Registry::deleteKeyW(HKEY_CURRENT_USER, L"Software\\XYO\\Win", L"Test04W", FALSE) == TRUE, "deleteKeyW");

	// Volatile key, gone after a reboot
	check(XYO::Win::Registry::createKeyVolatile(HKEY_CURRENT_USER, "Software\\XYO\\Win\\Test04Volatile") == TRUE, "createKeyVolatile");
	check(XYO::Win::Registry::deleteKey(HKEY_CURRENT_USER, "Software\\XYO\\Win", "Test04Volatile", FALSE) == TRUE, "deleteKey volatile");
};

static void testVariant() {
	Variant i(5);
	check((i.value()->vt == VT_INT) && ((int)i == 5), "Variant(int)");

	Variant b((VARIANT_BOOL)VARIANT_TRUE);
	check((b.value()->vt == VT_BOOL) && ((VARIANT_BOOL)b == VARIANT_TRUE), "Variant(VARIANT_BOOL)");

	Variant u((unsigned long int)7);
	check((u.value()->vt == VT_UI4) && ((unsigned long int)u == 7), "Variant(unsigned long int)");

	// Move steals, the source is left empty
	Variant s("text");
	Variant moved(std::move(s));
	check((moved.value()->vt == VT_BSTR) && (wcscmp(moved.value()->bstrVal, L"text") == 0), "Variant move");
	check(s.value()->vt == VT_EMPTY, "Variant moved from is empty");

	// Self assignment keeps the value
	Variant &self = moved;
	moved = self;
	check((moved.value()->vt == VT_BSTR) && (wcscmp(moved.value()->bstrVal, L"text") == 0), "Variant self assignment");

	// Changing type releases the old value
	moved = (long int)3;
	check((moved.value()->vt == VT_I4) && ((long int)moved == 3), "Variant change type");

	// Wide BSTR
	BSTR text = SysAllocString(L"wide");
	Variant w(text);
	SysFreeString(text);
	check((w.value()->vt == VT_BSTR) && (wcscmp((BSTR)w, L"wide") == 0), "Variant(BSTR) copies");

	// DispatchVariant: missing value and conversions
	DispatchVariant none(nullptr);
	check((long int)none == 0, "DispatchVariant NULL");
	check((BSTR)none == nullptr, "DispatchVariant NULL BSTR");

	VARIANTARG number;
	VariantInit(&number);
	number.vt = VT_I2;
	number.iVal = -12;
	DispatchVariant dn(&number);
	check((long int)dn == -12, "DispatchVariant VT_I2 to long");
	check((int)dn == -12, "DispatchVariant VT_I2 to int");

	VARIANTARG notANumber;
	VariantInit(&notANumber);
	notANumber.vt = VT_BSTR;
	notANumber.bstrVal = SysAllocString(L"abc");
	DispatchVariant dnan(&notANumber);
	check((long int)dnan == 0, "DispatchVariant failed conversion is 0");
	check(wcscmp((BSTR)dnan, L"abc") == 0, "DispatchVariant BSTR");
	VariantClear(&notANumber);
};

class Calculator : public virtual Dispatch {
	public:
		XYO_WIN_DISPATCH__PROTOTYPE;

		void answer(Variant &returnValue) {
			returnValue = (long int)42;
		};

		void add(Variant &returnValue, VARIANTARG *a, VARIANTARG *b) {
			returnValue = (long int)((long int)DispatchVariant(a) + (long int)DispatchVariant(b));
		};

		void concat(Variant &returnValue, VARIANTARG *a, VARIANTARG *b) {
			BSTR x = DispatchVariant(a);
			BSTR y = DispatchVariant(b);
			size_t xLn = (x != nullptr) ? wcslen(x) : 0;
			size_t yLn = (y != nullptr) ? wcslen(y) : 0;
			BSTR result = SysAllocStringLen(NULL, (UINT)(xLn + yLn));
			memcpy(result, x, xLn * sizeof(wchar_t));
			memcpy(result + xLn, y, yLn * sizeof(wchar_t));
			returnValue = result;
			SysFreeString(result);
		};
};

XYO_WIN_DISPATCH__TABLE_BEGIN(Calculator, 1)
XYO_WIN_DISPATCH__TABLE_FUNCTION_0(answer);
XYO_WIN_DISPATCH__TABLE_FUNCTION_2(add);
XYO_WIN_DISPATCH__TABLE_FUNCTION_2(concat);
XYO_WIN_DISPATCH__TABLE_END(Dispatch)

static void testDispatch() {
	Calculator calculator;
	IDispatch *dispatch = calculator.getIDispatchValue();
	HRESULT hr;

	// QueryInterface
	void *object = nullptr;
	check(dispatch->QueryInterface(IID_IDispatch, &object) == S_OK && (object == dispatch), "QueryInterface IDispatch");
	check(dispatch->QueryInterface(IID_IStream, &object) == E_NOINTERFACE && (object == nullptr), "QueryInterface unknown interface");
	check(dispatch->QueryInterface(IID_IDispatch, NULL) == E_POINTER, "QueryInterface NULL");

	// GetIDsOfNames
	LPOLESTR name = (LPOLESTR)L"answer";
	DISPID id = DISPID_UNKNOWN;
	check(dispatch->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_SYSTEM_DEFAULT, &id) == S_OK && (id == 1), "GetIDsOfNames");

	// Invoke with NULL DISPPARAMS for a method without arguments
	VARIANT result;
	VariantInit(&result);
	hr = dispatch->Invoke(id, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, NULL, &result, NULL, NULL);
	check((hr == S_OK) && (result.vt == VT_I4) && (result.lVal == 42), "Invoke NULL DISPPARAMS");
	VariantClear(&result);

	// Wrong argument count
	VARIANTARG argument;
	VariantInit(&argument);
	argument.vt = VT_I4;
	argument.lVal = 1;
	DISPPARAMS oneArgument = {&argument, NULL, 1, 0};
	hr = dispatch->Invoke(id, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &oneArgument, &result, NULL, NULL);
	check(hr == DISP_E_BADPARAMCOUNT, "Invoke wrong argument count");

	// Unknown id and DISPID_VALUE
	hr = dispatch->Invoke(100, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, NULL, &result, NULL, NULL);
	check(hr == DISP_E_MEMBERNOTFOUND, "Invoke unknown id");
	hr = dispatch->Invoke(DISPID_VALUE, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, NULL, &result, NULL, NULL);
	check(hr == DISP_E_MEMBERNOTFOUND, "Invoke DISPID_VALUE");

	// Through Function: argument order, conversions, strings
	Function function;
	BSTR functionName = SysAllocString(L"add");
	function.setObject(dispatch);
	function.functionName(functionName);
	SysFreeString(functionName);
	hr = function.invoke(Variant((long int)40), Variant("2"));
	check((hr == S_OK) && (function.result()->vt == VT_I4) && (function.result()->lVal == 42), "Function invoke add with conversion");
	check(function.dispatchId() == 2, "Function dispatch id cached");

	functionName = SysAllocString(L"concat");
	function.functionName(functionName);
	SysFreeString(functionName);
	check(function.dispatchId() == DISPID_UNKNOWN, "Function name change resets the id");
	hr = function.invoke(Variant("ab"), Variant("cd"));
	check((hr == S_OK) && (function.result()->vt == VT_BSTR) && (wcscmp(function.result()->bstrVal, L"abcd") == 0), "Function invoke argument order");

	// Not set up
	Function empty;
	check(empty.invoke() == E_INVALIDARG, "Function without object");
	function.releaseObject();
	check(function.invoke() == E_INVALIDARG, "Function after releaseObject");
};

static void testMetadata() {
	check(strlen(XYO::Win::Version::version()) > 0, "Version::version");
	check(strstr(XYO::Win::Version::versionWithBuild(), XYO::Win::Version::version()) != nullptr, "Version::versionWithBuild");
	check(strstr(XYO::Win::Copyright::copyright(), "Grigore Stefan") != nullptr, "Copyright::copyright");
	check(XYO::Win::License::license().find("MIT") != std::string::npos, "License::license");
};

int main(int cmdN, char *cmdS[]) {
	XYO::ManagedMemory::Registry::registryInit();

	try {
		testShell();
		testRegistry();
		testVariant();
		testDispatch();
		testMetadata();
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
