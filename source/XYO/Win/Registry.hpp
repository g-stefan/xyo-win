// Win
// Copyright (c) 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_REGISTRY_H
#define XYO_WIN_REGISTRY_H

#ifndef XYO_WIN_DEPENDENCY_HPP
#	include <XYO/Win/Dependency.hpp>
#endif

namespace XYO::Win {
	namespace Registry {

		XYO_WIN_EXPORT BOOL createKey(HKEY, const char *);
		XYO_WIN_EXPORT BOOL createKeyVolatile(HKEY, const char *);
		XYO_WIN_EXPORT BOOL writeString(HKEY, const char *, const char *, const char *);
		XYO_WIN_EXPORT BOOL writeDWord(HKEY, const char *, const char *, unsigned long int);
		XYO_WIN_EXPORT BOOL readString(HKEY, const char *, const char *, char *, unsigned long int, const char *);
		XYO_WIN_EXPORT BOOL readDWord(HKEY, const char *, const char *, unsigned long int *, unsigned long int);
		XYO_WIN_EXPORT BOOL deleteKey(HKEY, const char *, const char *, BOOL);
		XYO_WIN_EXPORT BOOL getStringLength(HKEY masterkey, const char *key, const char *reg, LPDWORD out);

		XYO_WIN_EXPORT BOOL createKeyW(HKEY, const wchar_t *);
		XYO_WIN_EXPORT BOOL createKeyVolatileW(HKEY, const wchar_t *);
		XYO_WIN_EXPORT BOOL writeStringW(HKEY, const wchar_t *, const wchar_t *, const wchar_t *);
		XYO_WIN_EXPORT BOOL writeDWordW(HKEY, const wchar_t *, const wchar_t *, unsigned long int);
		XYO_WIN_EXPORT BOOL readStringW(HKEY, const wchar_t *, const wchar_t *, wchar_t *, unsigned long int, const wchar_t *);
		XYO_WIN_EXPORT BOOL readDWordW(HKEY, const wchar_t *, const wchar_t *, unsigned long int *, unsigned long int);
		XYO_WIN_EXPORT BOOL deleteKeyW(HKEY, const wchar_t *, const wchar_t *, BOOL);
		XYO_WIN_EXPORT BOOL getStringLengthW(HKEY masterkey, const wchar_t *key, const wchar_t *reg, LPDWORD out);

	};
};

#endif
