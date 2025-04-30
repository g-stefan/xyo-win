// Win
// Copyright (c) 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/Win/Copyright.hpp>
#include <XYO/Win/Copyright.rh>

namespace XYO::Win::Copyright {

	static const char *copyright_ = XYO_WIN_COPYRIGHT;
	static const char *publisher_ = XYO_WIN_PUBLISHER;
	static const char *company_ = XYO_WIN_COMPANY;
	static const char *contact_ = XYO_WIN_CONTACT;

	const char *copyright() {
		return copyright_;
	};

	const char *publisher() {
		return publisher_;
	};

	const char *company() {
		return company_;
	};

	const char *contact() {
		return contact_;
	};

};
