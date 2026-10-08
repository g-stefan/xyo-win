// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

// Registry round-trip, Variant and DispatchVariant conversions.
// Uses HKEY_CURRENT_USER so it needs no elevation.

#include <XYO/Win.hpp>

#include <stdio.h>
#include <string.h>

using XYO::Win::Variant;
using XYO::Win::DispatchVariant;

static int failed = 0;

static void check(bool condition, const char *name) {
	printf("%s %s\r\n", condition ? "[ok]  " : "[FAIL]", name);
	if (!condition) {
		++failed;
	};
};

static void testRegistryAnsi() {
	char *key = (char *)"Software\\XYO\\Win\\Test";
	char buffer[256];
	unsigned long int dword = 0;

	check(XYO::Win::Registry::createKey(HKEY_CURRENT_USER, key) == TRUE, "createKey");

	check(XYO::Win::Registry::writeString(HKEY_CURRENT_USER, key, (char *)"str", (char *)"hello") == TRUE, "writeString");
	memset(buffer, 0x7F, sizeof(buffer));
	check(XYO::Win::Registry::readString(HKEY_CURRENT_USER, key, (char *)"str", buffer, sizeof(buffer), (char *)"def") == TRUE, "readString found");
	check(strcmp(buffer, "hello") == 0, "readString value");

	// Missing value returns the default and FALSE
	check(XYO::Win::Registry::readString(HKEY_CURRENT_USER, key, (char *)"missing", buffer, sizeof(buffer), (char *)"def") == FALSE, "readString missing");
	check(strcmp(buffer, "def") == 0, "readString default");

	check(XYO::Win::Registry::writeDWord(HKEY_CURRENT_USER, key, (char *)"dw", 123456) == TRUE, "writeDWord");
	check(XYO::Win::Registry::readDWord(HKEY_CURRENT_USER, key, (char *)"dw", &dword, 0) == TRUE, "readDWord found");
	check(dword == 123456, "readDWord value");
	check(XYO::Win::Registry::readDWord(HKEY_CURRENT_USER, key, (char *)"missing", &dword, 42) == FALSE, "readDWord missing");
	check(dword == 42, "readDWord default");

	// Cleanup values then the key itself
	XYO::Win::Registry::deleteKey(HKEY_CURRENT_USER, key, (char *)"str", TRUE);
	XYO::Win::Registry::deleteKey(HKEY_CURRENT_USER, key, (char *)"dw", TRUE);
	check(XYO::Win::Registry::deleteKey(HKEY_CURRENT_USER, (char *)"Software\\XYO\\Win", (char *)"Test", FALSE) == TRUE, "deleteKey");
};

static void testRegistryWide() {
	wchar_t *key = (wchar_t *)L"Software\\XYO\\Win\\TestW";
	wchar_t buffer[256];
	DWORD length = 0;

	check(XYO::Win::Registry::createKeyW(HKEY_CURRENT_USER, key) == TRUE, "createKeyW");

	// A three character string: verify the full string is stored, the byte
	// size bug used to truncate it to half.
	check(XYO::Win::Registry::writeStringW(HKEY_CURRENT_USER, key, (wchar_t *)L"str", (wchar_t *)L"world") == TRUE, "writeStringW");
	check(XYO::Win::Registry::getStringLengthW(HKEY_CURRENT_USER, key, (wchar_t *)L"str", &length) == TRUE, "getStringLengthW");
	check(length == (wcslen(L"world") + 1) * sizeof(wchar_t), "getStringLengthW size in bytes");

	check(XYO::Win::Registry::readStringW(HKEY_CURRENT_USER, key, (wchar_t *)L"str", buffer, sizeof(buffer), (wchar_t *)L"def") == TRUE, "readStringW found");
	check(wcscmp(buffer, L"world") == 0, "readStringW value");

	check(XYO::Win::Registry::readStringW(HKEY_CURRENT_USER, key, (wchar_t *)L"missing", buffer, sizeof(buffer), (wchar_t *)L"def") == FALSE, "readStringW missing");
	check(wcscmp(buffer, L"def") == 0, "readStringW default");

	XYO::Win::Registry::deleteKeyW(HKEY_CURRENT_USER, key, (wchar_t *)L"str", TRUE);
	check(XYO::Win::Registry::deleteKeyW(HKEY_CURRENT_USER, (wchar_t *)L"Software\\XYO\\Win", (wchar_t *)L"TestW", FALSE) == TRUE, "deleteKeyW");
};

static void testVariant() {
	// Integer conversions
	Variant a((long int)7);
	check((long int)a == 7, "Variant long");

	// LPCSTR to BSTR, and a copy that must not double free
	Variant s((LPCSTR) "text");
	Variant copy;
	copy = s;
	check(copy.value()->vt == VT_BSTR, "Variant copy is BSTR");
	check(wcscmp(copy.value()->bstrVal, L"text") == 0, "Variant copy value");

	// NULL string must not crash and yields an empty (NULL) BSTR
	Variant empty((LPCSTR)NULL);
	check(empty.value()->vt == VT_BSTR, "Variant NULL string type");
	check(empty.value()->bstrVal == NULL, "Variant NULL string value");
};

static void testDispatchVariant() {
	// A by reference variant must be followed to its value
	VARIANTARG inner;
	VariantInit(&inner);
	inner.vt = VT_I4;
	inner.lVal = 55;

	VARIANTARG byref;
	VariantInit(&byref);
	byref.vt = VT_BYREF | VT_VARIANT;
	byref.pvarVal = &inner;

	DispatchVariant dv(&byref);
	check((long int)dv == 55, "DispatchVariant by reference");

	// A type mismatch is converted, not reinterpreted
	VARIANTARG asString;
	VariantInit(&asString);
	asString.vt = VT_BSTR;
	asString.bstrVal = SysAllocString(L"123");
	DispatchVariant dvs(&asString);
	check((long int)dvs == 123, "DispatchVariant string to long");
	VariantClear(&asString);
};

int main(int cmdN, char *cmdS[]) {
	try {
		testRegistryAnsi();
		testRegistryWide();
		testVariant();
		testDispatchVariant();
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
