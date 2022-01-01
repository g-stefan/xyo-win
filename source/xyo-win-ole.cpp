//
// XYO Win
//
// Copyright (c) 2020-2022 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#include <windows.h>
#include <ole2.h>
#include <stdio.h>

#include "xyo-win-ole.hpp"

namespace XYO {
	namespace Win {
		namespace Ole {

			class XOle {
				public:
					bool isValid;

					XOle();
					~XOle();
			};

			XOle::XOle() {
				isValid = false;
				HRESULT initialized;				
				initialized=OleInitialize(NULL);
				if(initialized != OLE_E_WRONGCOMPOBJ){
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
	};
};

