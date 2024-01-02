// Win
// Copyright (c) 2014-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_FUNCTION_HPP
#define XYO_WIN_FUNCTION_HPP

#ifndef XYO_WIN_VARIANT_HPP
#	include <XYO/Win/Variant.hpp>
#endif

namespace XYO::Win {

	class Function : public virtual Object {
		protected:
			BSTR functionName_;
			DISPID dispIdMember_;
			EXCEPINFO exceptInfo_;
			VARIANTARG varResult_;
			IDispatch *refObject_;
			UINT nArgErr_;

			XYO_DISALLOW_COPY_ASSIGN_MOVE(Function);

		public:
			XYO_WIN_EXPORT Function();
			XYO_WIN_EXPORT ~Function();

			XYO_WIN_EXPORT void setObject(IDispatch *);
			XYO_WIN_EXPORT void releaseObject();

			inline IDispatch *getObject() {
				return refObject_;
			};

			XYO_WIN_EXPORT void functionName(BSTR Name);

			inline BSTR functionName() {
				return functionName_;
			};

			inline VARIANTARG *result() {
				return &varResult_;
			};

			inline EXCEPINFO *exceptInfo() {
				return &exceptInfo_;
			};

			inline DISPID dispatchId() {
				return dispIdMember_;
			};

			inline UINT argErr() {
				return nArgErr_;
			};

			inline HRESULT getDispatchId() {
				return refObject_->GetIDsOfNames(IID_NULL, &functionName_, 1, LOCALE_SYSTEM_DEFAULT, &dispIdMember_);
			};

			XYO_WIN_EXPORT HRESULT invoke();
			XYO_WIN_EXPORT HRESULT invoke(Variant);
			XYO_WIN_EXPORT HRESULT invoke(Variant, Variant);
			XYO_WIN_EXPORT HRESULT invoke(Variant, Variant, Variant);
			XYO_WIN_EXPORT HRESULT invoke(Variant, Variant, Variant, Variant);
			XYO_WIN_EXPORT HRESULT invoke(Variant, Variant, Variant, Variant, Variant);
			XYO_WIN_EXPORT HRESULT invoke(Variant, Variant, Variant, Variant, Variant, Variant);
			XYO_WIN_EXPORT HRESULT invoke(Variant, Variant, Variant, Variant, Variant, Variant, Variant);
			XYO_WIN_EXPORT HRESULT invoke(Variant, Variant, Variant, Variant, Variant, Variant, Variant, Variant);
	};

};

#endif
