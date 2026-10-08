// Win
// Copyright (c) 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_DEPENDENCY_HPP
#define XYO_WIN_DEPENDENCY_HPP

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#ifndef SECURITY_WIN32
#	define SECURITY_WIN32
#endif

#include <windows.h>
#include <ole2.h>
#include <shlobj.h>
#include <Shobjidl.h>
// iads.h is needed by User.hpp (IADsUser in its public interface).
#include <iads.h>
#include <adshlp.h>
#include <Security.h>
#include <shellapi.h>

// The Internet Explorer / MSHTML document-hosting headers (docobj.h, mshtml.h,
// MsHtmHst.h, exdisp.h, exdispid.h, servprov.h, wininet.h) are large and only
// the discontinued WebBrowser needs them, so they are included by
// WebBrowser.hpp and its .cpp instead of by every consumer of this library.

#ifndef XYO_SYSTEM_HPP
#	include <XYO/System.hpp>
#endif

#ifndef XYO_PIXEL32_HPP
#include <XYO/Pixel32.hpp>
#endif

// -- Export

#ifdef XYO_WIN_INTERNAL
#	define XYO_WIN_EXPORT XYO_PLATFORM_LIBRARY_EXPORT
#else
#	define XYO_WIN_EXPORT XYO_PLATFORM_LIBRARY_IMPORT
#endif
#ifdef XYO_WIN_LIBRARY
#	undef XYO_WIN_EXPORT
#	define XYO_WIN_EXPORT
#endif

// --

namespace XYO::Win {
	using namespace XYO::ManagedMemory;
	using namespace XYO::DataStructures;
	using namespace XYO::Encoding;
	using namespace XYO::Multithreading;
	using namespace XYO::System;
	using namespace XYO::Pixel32;
};

#endif
