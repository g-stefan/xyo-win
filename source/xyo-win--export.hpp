//
// XYO Win
//
// Copyright (c) 2020-2022 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef XYO_WIN__EXPORT_HPP
#define XYO_WIN__EXPORT_HPP

#ifndef XYO__EXPORT_HPP
#include "xyo--export.hpp"
#endif

#ifdef XYO_COMPILE_DYNAMIC_LIBRARY
#   ifdef  XYO_WIN_INTERNAL
#       define XYO_WIN_EXPORT XYO_LIBRARY_EXPORT
#   else
#       define XYO_WIN_EXPORT XYO_LIBRARY_IMPORT
#   endif
#else
#   define XYO_WIN_EXPORT
#endif

#endif

