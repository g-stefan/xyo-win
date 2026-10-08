// Win
// Copyright (c) 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <string.h>
#include <ole2.h>
#include <shlobj.h>
#include <docobj.h>
#include <mshtml.h>
#include <exdisp.h>
#include <exdispid.h>
#include <stdio.h>

#include <XYO/Win/Function.hpp>

namespace XYO::Win {

	Function::Function() {
		functionName_ = NULL;
		// DISPID 0 is valid (DISPID_VALUE), mark as not resolved
		dispIdMember_ = DISPID_UNKNOWN;
		memset(&exceptInfo_, 0, sizeof(exceptInfo_));
		VariantInit(&varResult_);
		refObject_ = NULL;
		nArgErr_ = 0;
	};

	Function::~Function() {
		if (functionName_ != NULL) {
			SysFreeString(functionName_);
		}
		VariantClear(&varResult_);
		releaseObject();
	};

	void Function::setObject(IDispatch *x) {
		if (x != NULL) {
			x->AddRef();
		};
		if (refObject_ != NULL) {
			refObject_->Release();
		};
		refObject_ = x;
		dispIdMember_ = DISPID_UNKNOWN;
	};

	void Function::releaseObject() {
		if (refObject_ != NULL) {
			refObject_->Release();
		};
		refObject_ = NULL;
		dispIdMember_ = DISPID_UNKNOWN;
	};

	void Function::functionName(BSTR Name) {
		if (functionName_ != NULL) {
			SysFreeString(functionName_);
		}
		functionName_ = SysAllocString(Name);
		dispIdMember_ = DISPID_UNKNOWN;
	};

	HRESULT Function::invokeWithArguments_(const Variant *const *arguments, UINT count) {
		HRESULT retVal;
		DISPPARAMS dispParams;
		VARIANTARG rgvarg[8];
		UINT k;

		if ((refObject_ == NULL) || (count > 8) || (functionName_ == NULL)) {
			return E_INVALIDARG;
		}
		if (dispIdMember_ == DISPID_UNKNOWN) {
			retVal = getDispatchId();
			if (retVal != S_OK) {
				dispIdMember_ = DISPID_UNKNOWN;
				return retVal;
			}
		};
		memset(&exceptInfo_, 0, sizeof(exceptInfo_));
		nArgErr_ = (UINT)-1;

		// Arguments are [in] for IDispatch::Invoke, the callee does not own or
		// free them, a shallow copy is enough. rgvarg is in reverse order.
		for (k = 0; k < count; ++k) {
			rgvarg[count - 1 - k] = *(arguments[k]->value());
		};

		memset(&dispParams, 0, sizeof(dispParams));
		dispParams.cArgs = count;
		dispParams.rgvarg = (count > 0) ? rgvarg : NULL;
		dispParams.cNamedArgs = 0;
		VariantClear(&varResult_);
		return refObject_->Invoke(dispIdMember_, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &dispParams, &varResult_, &exceptInfo_, &nArgErr_);
	};

	HRESULT Function::invoke() {
		return invokeWithArguments_(NULL, 0);
	};

	HRESULT Function::invoke(const Variant &v0) {
		const Variant *arguments[] = {&v0};
		return invokeWithArguments_(arguments, 1);
	};

	HRESULT Function::invoke(const Variant &v1, const Variant &v0) {
		const Variant *arguments[] = {&v1, &v0};
		return invokeWithArguments_(arguments, 2);
	};

	HRESULT Function::invoke(const Variant &v2, const Variant &v1, const Variant &v0) {
		const Variant *arguments[] = {&v2, &v1, &v0};
		return invokeWithArguments_(arguments, 3);
	};

	HRESULT Function::invoke(const Variant &v3, const Variant &v2, const Variant &v1, const Variant &v0) {
		const Variant *arguments[] = {&v3, &v2, &v1, &v0};
		return invokeWithArguments_(arguments, 4);
	};

	HRESULT Function::invoke(const Variant &v4, const Variant &v3, const Variant &v2, const Variant &v1, const Variant &v0) {
		const Variant *arguments[] = {&v4, &v3, &v2, &v1, &v0};
		return invokeWithArguments_(arguments, 5);
	};

	HRESULT Function::invoke(const Variant &v5, const Variant &v4, const Variant &v3, const Variant &v2, const Variant &v1, const Variant &v0) {
		const Variant *arguments[] = {&v5, &v4, &v3, &v2, &v1, &v0};
		return invokeWithArguments_(arguments, 6);
	};

	HRESULT Function::invoke(const Variant &v6, const Variant &v5, const Variant &v4, const Variant &v3, const Variant &v2, const Variant &v1, const Variant &v0) {
		const Variant *arguments[] = {&v6, &v5, &v4, &v3, &v2, &v1, &v0};
		return invokeWithArguments_(arguments, 7);
	};

	HRESULT Function::invoke(const Variant &v7, const Variant &v6, const Variant &v5, const Variant &v4, const Variant &v3, const Variant &v2, const Variant &v1, const Variant &v0) {
		const Variant *arguments[] = {&v7, &v6, &v5, &v4, &v3, &v2, &v1, &v0};
		return invokeWithArguments_(arguments, 8);
	};

};
