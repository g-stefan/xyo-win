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
#include <oleauto.h>
#include <stdio.h>

#pragma warning(disable : 4100)

#include <XYO/Win/Dispatch.hpp>

namespace XYO::Win {

	void Dispatch::setDispatchFunctionsIdAndNames(){};

	// IUnknown

	HRESULT STDMETHODCALLTYPE Dispatch::QueryInterface(REFIID riid, LPVOID *ppvObj) {
		if (ppvObj == NULL) {
			return (E_POINTER);
		};
		if (IsEqualGUID(riid, IID_IUnknown)) {
			AddRef();
			*ppvObj = static_cast<IUnknown *>(this);
		} else if (IsEqualGUID(riid, IID_IDispatch)) {
			AddRef();
			*ppvObj = static_cast<IDispatch *>(this);
		} else {
			*ppvObj = NULL;
			return (E_NOINTERFACE);
		};
		return (S_OK);
	};

	ULONG STDMETHODCALLTYPE Dispatch::AddRef() {
		return (ULONG)1;
	};

	ULONG STDMETHODCALLTYPE Dispatch::Release() {
		return (ULONG)1;
	};

	// IDispatch

	HRESULT STDMETHODCALLTYPE Dispatch::GetTypeInfoCount(UINT *) {
		return (E_NOTIMPL);
	};

	HRESULT STDMETHODCALLTYPE Dispatch::GetTypeInfo(UINT, LCID, LPTYPEINFO *) {
		return (E_NOTIMPL);
	};

	HRESULT STDMETHODCALLTYPE Dispatch::GetIDsOfNames(REFIID, LPOLESTR *names, UINT count, LCID, DISPID *outID) {
		UINT k;
		HRESULT retV;
		if ((names == NULL) || (outID == NULL)) {
			return (E_INVALIDARG);
		};
		for (k = 0; k < count; ++k) {
			outID[k] = DISPID_UNKNOWN;
		};
		retV = invokeAndId(0, 0, NULL, NULL, names, count, outID);
		if (retV == S_FALSE) {
			// No table knows the name
			return DISP_E_UNKNOWNNAME;
		};
		return retV;
	};

	HRESULT STDMETHODCALLTYPE Dispatch::Invoke(DISPID dispIdMember, REFIID, LCID, WORD, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *, UINT *) {
		// The dispatch tables read pDispParams->cArgs / rgvarg, a caller
		// passing NULL for a call without arguments must not crash them
		DISPPARAMS noParams = {NULL, NULL, 0, 0};
		if (pDispParams == NULL) {
			pDispParams = &noParams;
		};
		return invokeAndId(1, dispIdMember, pDispParams, pVarResult, NULL, 0, NULL);
	};

	::IDispatch *Dispatch::getIDispatchValue() {
		return static_cast<::IDispatch *>(this);
	};

	HRESULT Dispatch::invokeAndId(UINT mode, DISPID dispIdMember, DISPPARAMS *pDispParams, VARIANT *pVarResult, LPOLESTR *names, UINT count, DISPID *outID) {
		if (mode == 0) {
			return (S_FALSE);
		};
		if (mode == 1) {
			return (DISP_E_MEMBERNOTFOUND);
		};
		return (S_FALSE);
	};

};
