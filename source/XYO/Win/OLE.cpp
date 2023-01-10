// Win
// Copyright (c) 2014-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

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
		isValid = false;
		HRESULT initialized;
		initialized = OleInitialize(NULL);
		if (initialized != OLE_E_WRONGCOMPOBJ) {
			isValid = true;
		};
	};

	XOle::~XOle() {
		OleUninitialize();
	};

	bool isValid() {
		return (TSingletonProcess<XOle>::getValue())->isValid;
	};

};
