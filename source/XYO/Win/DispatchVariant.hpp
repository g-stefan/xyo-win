// Win
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
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

				XYO_DISALLOW_COPY_ASSIGN_MOVE(DispatchVariant);

			public:
				inline DispatchVariant(){};

				inline ~DispatchVariant(){};

				inline DispatchVariant(VARIANTARG *x) {
					variantArg_ = x;
				};

				inline VARIANTARG *value() {
					return variantArg_;
				};

				inline operator VARIANT_BOOL() {
					return variantArg_->boolVal;
				};

				inline operator VARIANT_BOOL *() {
					return variantArg_->pboolVal;
				};

				inline operator VARIANTARG *() {
					return variantArg_->pvarVal;
				};

				inline operator unsigned long int() {
					return variantArg_->ulVal;
				};

				inline operator long int() {
					return variantArg_->lVal;
				};

				inline operator int() {
					return variantArg_->intVal;
				};

				inline operator unsigned int() {
					return variantArg_->uintVal;
				};

				inline operator BSTR() {
					return variantArg_->bstrVal;
				};

				inline operator IUnknown *() {
					return variantArg_->punkVal;
				};

				inline operator IDispatch *() {
					return variantArg_->pdispVal;
				};

				inline operator IUnknown **() {
					return variantArg_->ppunkVal;
				};

				inline operator IDispatch **() {
					return variantArg_->ppdispVal;
				};
		};

	};

#endif
