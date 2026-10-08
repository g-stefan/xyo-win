// Win
// Copyright (c) 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <ole2.h>
#include <stdio.h>

#include <XYO/Win/OLE.hpp>

namespace XYO::Win::Ole {

	class XOle {
		public:
			bool isValid;

			XOle();
			~XOle();
	};

	XOle::XOle() {
		// S_OK or S_FALSE (already initialized on this thread) must be balanced
		// by OleUninitialize; any failure (RPC_E_CHANGED_MODE when the thread
		// is in the multithreaded apartment, OLE_E_WRONGCOMPOBJ, ...) must not.
		isValid = SUCCEEDED(OleInitialize(NULL));
	};

	XOle::~XOle() {
		if (isValid) {
			OleUninitialize();
		};
	};

	bool isValid() {
		// OLE is initialized per thread (apartment)
		return (TSingletonThread<XOle>::getValue())->isValid;
	};

};
