// Win
// Copyright (c) 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_DISPATCHVARIANT_HPP
#define XYO_WIN_DISPATCHVARIANT_HPP

#ifndef XYO_WIN_DEPENDENCY_HPP
#	include <XYO/Win/Dependency.hpp>
#endif

namespace XYO::Win {

	class DispatchVariant {
		protected:
			VARIANTARG *variantArg_;

			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(DispatchVariant);

			// Arguments received by IDispatch::Invoke can be passed by
			// reference (VT_BYREF | VT_VARIANT), follow them to the value.
			static inline const VARIANTARG *dereference(const VARIANTARG *x) {
				while ((x != nullptr) && (x->vt == (VT_BYREF | VT_VARIANT)) && (x->pvarVal != nullptr)) {
					x = x->pvarVal;
				};
				return x;
			};

			// Convert to a scalar type (no resources to clear),
			// handles VT_BYREF and other numeric types
			inline bool toScalar(VARTYPE vt, VARIANT &out) {
				const VARIANTARG *x = dereference(variantArg_);
				VariantInit(&out);
				if (x == nullptr) {
					return false;
				};
				if (x->vt == vt) {
					out = *x;
					return true;
				};
				if (SUCCEEDED(VariantChangeType(&out, const_cast<VARIANTARG *>(x), 0, vt))) {
					return true;
				};
				VariantInit(&out);
				return false;
			};

		public:
			inline DispatchVariant() {
				variantArg_ = nullptr;
			};

			inline ~DispatchVariant(){};

			inline DispatchVariant(VARIANTARG *x) {
				variantArg_ = x;
			};

			inline VARIANTARG *value() {
				return variantArg_;
			};

			inline operator VARIANT_BOOL() {
				VARIANT x;
				return toScalar(VT_BOOL, x) ? x.boolVal : VARIANT_FALSE;
			};

			inline operator VARIANT_BOOL *() {
				return (variantArg_ != nullptr) ? variantArg_->pboolVal : nullptr;
			};

			inline operator VARIANTARG *() {
				return (variantArg_ != nullptr) ? variantArg_->pvarVal : nullptr;
			};

			inline operator unsigned long int() {
				VARIANT x;
				return toScalar(VT_UI4, x) ? x.ulVal : 0;
			};

			inline operator long int() {
				VARIANT x;
				return toScalar(VT_I4, x) ? x.lVal : 0;
			};

			inline operator int() {
				VARIANT x;
				return toScalar(VT_INT, x) ? x.intVal : 0;
			};

			inline operator unsigned int() {
				VARIANT x;
				return toScalar(VT_UINT, x) ? x.uintVal : 0;
			};

			// Not owned by the caller, no conversion
			inline operator BSTR() {
				const VARIANTARG *x = dereference(variantArg_);
				if (x == nullptr) {
					return nullptr;
				};
				if (x->vt == VT_BSTR) {
					return x->bstrVal;
				};
				if ((x->vt == (VT_BYREF | VT_BSTR)) && (x->pbstrVal != nullptr)) {
					return *(x->pbstrVal);
				};
				return nullptr;
			};

			// Not owned by the caller, no conversion
			inline operator IUnknown *() {
				const VARIANTARG *x = dereference(variantArg_);
				if (x == nullptr) {
					return nullptr;
				};
				if ((x->vt == VT_UNKNOWN) || (x->vt == VT_DISPATCH)) {
					return x->punkVal;
				};
				if (((x->vt == (VT_BYREF | VT_UNKNOWN)) || (x->vt == (VT_BYREF | VT_DISPATCH))) && (x->ppunkVal != nullptr)) {
					return *(x->ppunkVal);
				};
				return nullptr;
			};

			// Not owned by the caller, no conversion
			inline operator IDispatch *() {
				const VARIANTARG *x = dereference(variantArg_);
				if (x == nullptr) {
					return nullptr;
				};
				if (x->vt == VT_DISPATCH) {
					return x->pdispVal;
				};
				if ((x->vt == (VT_BYREF | VT_DISPATCH)) && (x->ppdispVal != nullptr)) {
					return *(x->ppdispVal);
				};
				return nullptr;
			};

			inline operator IUnknown **() {
				return (variantArg_ != nullptr) ? variantArg_->ppunkVal : nullptr;
			};

			inline operator IDispatch **() {
				return (variantArg_ != nullptr) ? variantArg_->ppdispVal : nullptr;
			};
	};

};

#endif
