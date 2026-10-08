// Win
// Copyright (c) 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <windows.h>

#include <XYO/Win/Registry.hpp>

namespace XYO::Win::Registry {

	// RegGetValue always null-terminates string data and fails with
	// ERROR_MORE_DATA when the buffer is too small; REG_EXPAND_SZ is returned
	// unexpanded, like RegQueryValueEx did.
	static const DWORD readStringFlags_ = RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND;

	// Copy the default value truncated to the buffer, def can be NULL.
	static void copyDefault_(char *str, size_t length, const char *def) {
		if ((str == NULL) || (length == 0)) {
			return;
		};
		if (def == NULL) {
			def = "";
		};
		strncpy_s(str, length, def, _TRUNCATE);
	};

	static void copyDefaultW_(wchar_t *str, size_t length, const wchar_t *def) {
		if ((str == NULL) || (length == 0)) {
			return;
		};
		if (def == NULL) {
			def = L"";
		};
		wcsncpy_s(str, length, def, _TRUNCATE);
	};

	BOOL createKey(HKEY masterkey, const char *key) {
		HKEY mykey;
		if (RegCreateKeyExA(masterkey, key, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &mykey, NULL) != ERROR_SUCCESS) {
			return FALSE;
		}
		RegCloseKey(mykey);
		return TRUE;
	};

	BOOL createKeyVolatile(HKEY masterkey, const char *key) {
		HKEY mykey;
		if (RegCreateKeyExA(masterkey, key, 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &mykey, NULL) != ERROR_SUCCESS) {
			return FALSE;
		}
		RegCloseKey(mykey);
		return TRUE;
	};

	BOOL writeString(HKEY masterkey, const char *key, const char *reg, const char *_str) {
		HKEY mykey;
		BOOL retval;
		if (RegOpenKeyExA(masterkey, key, 0, KEY_SET_VALUE, &mykey) != ERROR_SUCCESS) {
			return FALSE;
		}
		// NULL is written as the empty string
		if (_str == NULL) {
			_str = "";
		};
		if (RegSetValueExA(mykey, reg, 0, REG_SZ, (const BYTE *)_str, (DWORD)strlen(_str) + 1) == ERROR_SUCCESS) {
			retval = TRUE;
		} else {
			retval = FALSE;
		}
		RegCloseKey(mykey);
		return retval;
	};

	BOOL writeDWord(HKEY masterkey, const char *key, const char *reg, unsigned long int val) {
		HKEY mykey;
		BOOL retval;
		DWORD value = (DWORD)val;
		if (RegOpenKeyExA(masterkey, key, 0, KEY_SET_VALUE, &mykey) != ERROR_SUCCESS) {
			return FALSE;
		}
		if (RegSetValueExA(mykey, reg, 0, REG_DWORD, (BYTE *)&value, sizeof(DWORD)) == ERROR_SUCCESS) {
			retval = TRUE;
		} else {
			retval = FALSE;
		}
		RegCloseKey(mykey);
		return retval;
	};

	// sz is the size of _str in bytes (= characters)
	BOOL readString(HKEY masterkey, const char *key, const char *reg, char *_str, unsigned long int sz, const char *def) {
		DWORD size = (DWORD)sz;
		if (RegGetValueA(masterkey, key, reg, readStringFlags_, NULL, _str, &size) == ERROR_SUCCESS) {
			return TRUE;
		};
		copyDefault_(_str, sz, def);
		return FALSE;
	};

	BOOL readDWord(HKEY masterkey, const char *key, const char *reg, unsigned long int *_str, unsigned long int def) {
		DWORD value;
		DWORD size = sizeof(DWORD);
		if (RegGetValueA(masterkey, key, reg, RRF_RT_REG_DWORD, NULL, &value, &size) == ERROR_SUCCESS) {
			*_str = value;
			return TRUE;
		};
		*_str = def;
		return FALSE;
	};

	BOOL deleteKey(HKEY masterkey, const char *key, const char *reg, BOOL value) {
		HKEY mykey;
		BOOL retval;
		// The access rights of the parent key do not matter for RegDeleteKey
		if (RegOpenKeyExA(masterkey, key, 0, KEY_SET_VALUE, &mykey) != ERROR_SUCCESS) {
			return FALSE;
		}
		if (value) {
			retval = (RegDeleteValueA(mykey, reg) == ERROR_SUCCESS);
		} else {
			retval = (RegDeleteKeyA(mykey, reg) == ERROR_SUCCESS);
		};
		RegCloseKey(mykey);
		return retval;
	};

	BOOL getStringLength(HKEY masterkey, const char *key, const char *reg, LPDWORD out) {
		HKEY mykey;
		BOOL retval;
		if (out == NULL) {
			return FALSE;
		}

		if (RegOpenKeyExA(masterkey, key, 0, KEY_QUERY_VALUE, &mykey) != ERROR_SUCCESS) {
			*out = 0;
			return FALSE;
		};
		if (RegQueryValueExA(mykey, reg, NULL, NULL, NULL, out) == ERROR_SUCCESS) {
			retval = TRUE;
		} else {
			retval = FALSE;
			*out = 0;
		};
		RegCloseKey(mykey);
		return retval;
	};

	BOOL createKeyW(HKEY masterkey, const wchar_t *key) {
		HKEY mykey;
		if (RegCreateKeyExW(masterkey, key, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &mykey, NULL) != ERROR_SUCCESS) {
			return FALSE;
		}
		RegCloseKey(mykey);
		return TRUE;
	};

	BOOL createKeyVolatileW(HKEY masterkey, const wchar_t *key) {
		HKEY mykey;
		if (RegCreateKeyExW(masterkey, key, 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &mykey, NULL) != ERROR_SUCCESS) {
			return FALSE;
		}
		RegCloseKey(mykey);
		return TRUE;
	};

	BOOL writeStringW(HKEY masterkey, const wchar_t *key, const wchar_t *reg, const wchar_t *_str) {
		HKEY mykey;
		BOOL retval;
		if (RegOpenKeyExW(masterkey, key, 0, KEY_SET_VALUE, &mykey) != ERROR_SUCCESS) {
			return FALSE;
		}
		// NULL is written as the empty string
		if (_str == NULL) {
			_str = L"";
		};
		// Size is in bytes, including the terminating null character
		if (RegSetValueExW(mykey, reg, 0, REG_SZ, (const BYTE *)_str, (DWORD)((wcslen(_str) + 1) * sizeof(wchar_t))) == ERROR_SUCCESS) {
			retval = TRUE;
		} else {
			retval = FALSE;
		}
		RegCloseKey(mykey);
		return retval;
	};

	BOOL writeDWordW(HKEY masterkey, const wchar_t *key, const wchar_t *reg, unsigned long int val) {
		HKEY mykey;
		BOOL retval;
		DWORD value = (DWORD)val;
		if (RegOpenKeyExW(masterkey, key, 0, KEY_SET_VALUE, &mykey) != ERROR_SUCCESS) {
			return FALSE;
		}
		if (RegSetValueExW(mykey, reg, 0, REG_DWORD, (BYTE *)&value, sizeof(DWORD)) == ERROR_SUCCESS) {
			retval = TRUE;
		} else {
			retval = FALSE;
		}
		RegCloseKey(mykey);
		return retval;
	};

	// sz is the size of _str in bytes
	BOOL readStringW(HKEY masterkey, const wchar_t *key, const wchar_t *reg, wchar_t *_str, unsigned long int sz, const wchar_t *def) {
		DWORD size = (DWORD)sz;
		if (RegGetValueW(masterkey, key, reg, readStringFlags_, NULL, _str, &size) == ERROR_SUCCESS) {
			return TRUE;
		};
		copyDefaultW_(_str, sz / sizeof(wchar_t), def);
		return FALSE;
	};

	BOOL readDWordW(HKEY masterkey, const wchar_t *key, const wchar_t *reg, unsigned long int *_str, unsigned long int def) {
		DWORD value;
		DWORD size = sizeof(DWORD);
		if (RegGetValueW(masterkey, key, reg, RRF_RT_REG_DWORD, NULL, &value, &size) == ERROR_SUCCESS) {
			*_str = value;
			return TRUE;
		};
		*_str = def;
		return FALSE;
	};

	BOOL deleteKeyW(HKEY masterkey, const wchar_t *key, const wchar_t *reg, BOOL value) {
		HKEY mykey;
		BOOL retval;
		// The access rights of the parent key do not matter for RegDeleteKey
		if (RegOpenKeyExW(masterkey, key, 0, KEY_SET_VALUE, &mykey) != ERROR_SUCCESS) {
			return FALSE;
		}
		if (value) {
			retval = (RegDeleteValueW(mykey, reg) == ERROR_SUCCESS);
		} else {
			retval = (RegDeleteKeyW(mykey, reg) == ERROR_SUCCESS);
		};
		RegCloseKey(mykey);
		return retval;
	};

	BOOL getStringLengthW(HKEY masterkey, const wchar_t *key, const wchar_t *reg, LPDWORD out) {
		HKEY mykey;
		BOOL retval;
		if (out == NULL) {
			return FALSE;
		}

		if (RegOpenKeyExW(masterkey, key, 0, KEY_QUERY_VALUE, &mykey) != ERROR_SUCCESS) {
			*out = 0;
			return FALSE;
		};
		if (RegQueryValueExW(mykey, reg, NULL, NULL, NULL, out) == ERROR_SUCCESS) {
			retval = TRUE;
		} else {
			retval = FALSE;
			*out = 0;
		};
		RegCloseKey(mykey);
		return retval;
	};

};
