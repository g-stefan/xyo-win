// Win
// Copyright (c) 2014-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2024 Grigore Stefan <g_stefan@yahoo.com>
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
#include <docobj.h>
#include <mshtml.h>
#include <MsHtmHst.h>
#include <exdisp.h>
#include <exdispid.h>
#include <servprov.h>
#include <iads.h>
#include <adshlp.h>
#include <wininet.h>
#include <Iptypes.h>
#include <Iphlpapi.h>
#include <Security.h>
#include <shellapi.h>

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
