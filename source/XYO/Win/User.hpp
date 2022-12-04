// Win
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_USER_HPP
#define XYO_WIN_USER_HPP

#ifndef XYO_WIN_DEPENDENCY_HPP
#	include <XYO/Win/Dependency.hpp>
#endif

namespace XYO::Win::User {

	XYO_WIN_EXPORT HRESULT addToUserGroupsUser(IADsUser *pUser, BSTR UserName);
	XYO_WIN_EXPORT HRESULT createUserAccountOnAD(wchar_t *UserName, wchar_t *FullName, wchar_t *Description, wchar_t *Password, wchar_t *ComputerName, wchar_t *ComputerNameAndUser, wchar_t *ThisComputerNameAndUser);
	XYO_WIN_EXPORT HRESULT removeUserAccountOnAD(wchar_t *ComputerNameAndUser);
	XYO_WIN_EXPORT HRESULT removeUserFromItsGroups(IADsUser *pUser, BSTR UserName);
	XYO_WIN_EXPORT bool hideUserFromWelcomeScreen(wchar_t *_user_name);
	XYO_WIN_EXPORT bool showUserInWelcomeScreen(wchar_t *_user_name);
	XYO_WIN_EXPORT bool createUserAccountAsCurrentUserPrivilegeOnLocalComputer(wchar_t *_user_name, wchar_t *_password, wchar_t *_full_name, wchar_t *_description);
	XYO_WIN_EXPORT bool deleteUserAccountOnLocalComputer(wchar_t *_user_name);
	XYO_WIN_EXPORT bool createHiddenUserAccountAsCurrentUserPrivilegeOnLocalComputer(wchar_t *_user_name, wchar_t *_password, wchar_t *_full_name, wchar_t *_description);

};

#endif
