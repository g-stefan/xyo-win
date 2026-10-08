// Win
// Copyright (c) 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#ifndef SECURITY_WIN32
#	define SECURITY_WIN32
#endif

#include <windows.h>
#include <objbase.h>
#include <iads.h>
#include <adshlp.h>
#include <wininet.h>
#include <Iptypes.h>
#include <Iphlpapi.h>
#include <Security.h>
#include <stdio.h>
#include <strsafe.h>

#include <XYO/Win/User.hpp>
#include <XYO/Win/Registry.hpp>
#include <XYO/Win/OLE.hpp>

namespace XYO::Win::User {

	HRESULT addToUserGroupsUser(IADsUser *pUser, BSTR UserName) {

		VARIANT var;
		IADsGroup *pADs;
		ULONG lFetch;
		IDispatch *pDisp;
		IUnknown *pUnk;
		IEnumVARIANT *pEnum;

		IADsMembers *pGroups;
		HRESULT hr = S_OK;
		hr = pUser->Groups(&pGroups);

		if (FAILED(hr)) {
			return hr;
		};

		hr = pGroups->get__NewEnum(&pUnk);
		pGroups->Release();

		if (FAILED(hr)) {
			return hr;
		};

		hr = pUnk->QueryInterface(IID_IEnumVARIANT, (void **)&pEnum);
		pUnk->Release();

		if (FAILED(hr)) {
			return hr;
		};

		// The enumeration reuses hr and ends at S_FALSE, so keep the operation
		// result separately and report the first Add failure to the caller.
		HRESULT hrResult = S_OK;
		VariantInit(&var);
		hr = pEnum->Next(1, &var, &lFetch);
		while (hr == S_OK) {
			if (lFetch == 1) {
				pDisp = V_DISPATCH(&var);
				if (SUCCEEDED(pDisp->QueryInterface(IID_IADsGroup, (void **)&pADs))) {
					HRESULT hrAdd = pADs->Add(UserName);
					if (FAILED(hrAdd) && SUCCEEDED(hrResult)) {
						hrResult = hrAdd;
					};
					pADs->Release();
				};
			};
			VariantClear(&var);
			pDisp = NULL;
			hr = pEnum->Next(1, &var, &lFetch);
		};

		pEnum->Release();
		return hrResult;
	};

	HRESULT createUserAccountOnAD(wchar_t *UserName, wchar_t *FullName, wchar_t *Description, wchar_t *Password, wchar_t *ComputerName, wchar_t *ComputerNameAndUser, wchar_t *ThisComputerNameAndUser) {

		HRESULT hr;
		BSTR _ComputerName;
		BSTR _User;
		BSTR _UserName;
		BSTR _Password;
		BSTR _FullName;
		BSTR _Description;
		BSTR _userAccountControl;
		BSTR _sAMAccountName;
		BSTR _userFlags;
		BSTR _ComputerNameAndUser;
		IADsContainer *pUsers = NULL;
		IDispatch *pDisp = NULL;
		IADsUser *padsUser = NULL;

		if ((ComputerName == NULL) ||
		    (UserName == NULL) ||
		    (Password == NULL) ||
		    (FullName == NULL) ||
		    (Description == NULL) ||
		    (ComputerNameAndUser == NULL) ||
		    (ThisComputerNameAndUser == NULL)) {
			return E_INVALIDARG;
		};

		_ComputerName = SysAllocString(ComputerName);
		_User = SysAllocString(L"user");
		_UserName = SysAllocString(UserName);
		_Password = SysAllocString(Password);
		_FullName = SysAllocString(FullName);
		_Description = SysAllocString(Description);
		_userAccountControl = SysAllocString(L"UserAccountControl");
		_sAMAccountName = SysAllocString(L"SAMAccountName");
		_userFlags = SysAllocString(L"UserFlags");
		_ComputerNameAndUser = SysAllocString(ComputerNameAndUser);

		if ((_ComputerName == NULL) ||
		    (_User == NULL) ||
		    (_UserName == NULL) ||
		    (_Password == NULL) ||
		    (_FullName == NULL) ||
		    (_userAccountControl == NULL) ||
		    (_sAMAccountName == NULL) ||
		    (_userFlags == NULL) ||
		    (_ComputerNameAndUser == NULL) ||
		    (_Description == NULL)) {
			if (_ComputerName != NULL) {
				SysFreeString(_ComputerName);
			}
			if (_User != NULL) {
				SysFreeString(_User);
			}
			if (_UserName != NULL) {
				SysFreeString(_UserName);
			}
			if (_Password != NULL) {
				SysFreeString(_Password);
			}
			if (_FullName != NULL) {
				SysFreeString(_FullName);
			}
			if (_Description != NULL) {
				SysFreeString(_Description);
			}
			if (_userAccountControl != NULL) {
				SysFreeString(_userAccountControl);
			}
			if (_userFlags != NULL) {
				SysFreeString(_userFlags);
			}
			if (_sAMAccountName != NULL) {
				SysFreeString(_sAMAccountName);
			}
			if (_ComputerNameAndUser != NULL) {
				SysFreeString(_ComputerNameAndUser);
			}
			return E_OUTOFMEMORY;
		};

		// ADSI is COM: initialize OLE on this thread when not done yet (a
		// multithreaded apartment thread is fine as it is)
		Ole::isValid();

		hr = ADsGetObject(_ComputerName, IID_IADsContainer, (LPVOID *)&pUsers);
		if (SUCCEEDED(hr)) {
			hr = pUsers->Create(_User, _UserName, &pDisp);
			if (SUCCEEDED(hr)) {
				hr = pDisp->QueryInterface(IID_IADsUser, (void **)&padsUser);
				if (SUCCEEDED(hr)) {
					VARIANT value;
					// sAMAccountName is the logon name, not the full name
					VariantInit(&value);
					value.vt = VT_BSTR;
					value.bstrVal = SysAllocString(_UserName);
					hr = padsUser->Put(_sAMAccountName, value);
					VariantClear(&value);
					if (SUCCEEDED(hr)) {
						hr = padsUser->put_FullName(_FullName);
					};
					if (SUCCEEDED(hr)) {
						hr = padsUser->put_Description(_Description);
					};
					if (SUCCEEDED(hr)) {
						hr = padsUser->SetInfo();
					};
					if (SUCCEEDED(hr)) {
						hr = padsUser->SetPassword(_Password);
					};
					if (SUCCEEDED(hr)) {
						hr = padsUser->put_AccountDisabled(FALSE);
					};
					if (SUCCEEDED(hr)) {
						hr = padsUser->put_PasswordRequired(TRUE);
					};
					if (SUCCEEDED(hr)) {
						VariantInit(&value);
						// UserAccountControl is only meaningful for the LDAP
						// provider; the WinNT (local SAM) provider usually fails
						// this Get, so it is best effort. Only touch lVal when the
						// value really is an integer.
						if (SUCCEEDED(padsUser->Get(_userAccountControl, &value)) && (value.vt == VT_I4)) {
							value.lVal |= ADS_UF_DONT_EXPIRE_PASSWD;
							hr = padsUser->Put(_userAccountControl, value);
						};
						VariantClear(&value);
					};
					if (SUCCEEDED(hr)) {
						VariantInit(&value);
						// UserFlags is the WinNT provider equivalent of
						// UserAccountControl; guard the union access the same way.
						if (SUCCEEDED(padsUser->Get(_userFlags, &value)) && (value.vt == VT_I4)) {
							value.lVal |= ADS_UF_DONT_EXPIRE_PASSWD;
							hr = padsUser->Put(_userFlags, value);
						};
						VariantClear(&value);
					};
					if (SUCCEEDED(hr)) {
						hr = padsUser->SetInfo();
					};
					padsUser->Release();

					if (SUCCEEDED(hr)) {
						IADsUser *thisUser = NULL;
						hr = ADsGetObject(ThisComputerNameAndUser, IID_IADsUser, (LPVOID *)&thisUser);
						if (SUCCEEDED(hr)) {
							hr = addToUserGroupsUser(thisUser, _ComputerNameAndUser);
							thisUser->Release();
						};
					};
				};
				pDisp->Release();
			};
			pUsers->Release();
		};
		SysFreeString(_ComputerName);
		SysFreeString(_User);
		SysFreeString(_UserName);
		SysFreeString(_Password);
		SysFreeString(_FullName);
		SysFreeString(_Description);
		SysFreeString(_userAccountControl);
		SysFreeString(_sAMAccountName);
		SysFreeString(_userFlags);
		SysFreeString(_ComputerNameAndUser);
		return hr;
	};

	HRESULT removeUserAccountOnAD(wchar_t *ComputerNameAndUser) {
		HRESULT hr;
		IADsUser *padsUser = NULL;
		IADsContainer *padsContainer = NULL;
		BSTR ADsClass = NULL;
		BSTR parentPath = NULL;
		BSTR userName = NULL;
		BSTR userClass = NULL;

		if (ComputerNameAndUser == NULL) {
			return E_INVALIDARG;
		};

		Ole::isValid();

		hr = ADsGetObject(ComputerNameAndUser, IID_IADsUser, (LPVOID *)&padsUser);
		if (FAILED(hr)) {
			return hr;
		};

		hr = padsUser->get_Class(&ADsClass);
		if (SUCCEEDED(hr)) {
			if (lstrcmpiW(ADsClass, L"user") != 0) {
				// Not a user object, nothing to remove
				hr = E_INVALIDARG;
			};
			SysFreeString(ADsClass);
		};

		if (SUCCEEDED(hr)) {
			hr = padsUser->get_Parent(&parentPath);
		};
		if (SUCCEEDED(hr)) {
			hr = ADsGetObject(parentPath, IID_IADsContainer, (LPVOID *)&padsContainer);
			SysFreeString(parentPath);
		};
		if (SUCCEEDED(hr)) {
			hr = padsUser->get_Name(&userName);
		};
		if (SUCCEEDED(hr)) {
			// IADsContainer::Delete takes BSTRs, a wide string literal has no
			// length prefix
			userClass = SysAllocString(L"user");
			if (userClass == NULL) {
				hr = E_OUTOFMEMORY;
			};
		};
		if (SUCCEEDED(hr)) {
			removeUserFromItsGroups(padsUser, userName);
			hr = padsContainer->Delete(userClass, userName);
		};
		if (userClass != NULL) {
			SysFreeString(userClass);
		};
		if (userName != NULL) {
			SysFreeString(userName);
		};

		if (padsContainer != NULL) {
			padsContainer->Release();
		};
		padsUser->Release();
		return hr;
	};

	HRESULT removeUserFromItsGroups(IADsUser *pUser, BSTR UserName) {

		VARIANT var;
		IADsGroup *pADs;
		ULONG lFetch;
		IDispatch *pDisp;
		IUnknown *pUnk;
		IEnumVARIANT *pEnum;

		IADsMembers *pGroups;
		HRESULT hr = S_OK;
		hr = pUser->Groups(&pGroups);

		if (FAILED(hr)) {
			return hr;
		}

		hr = pGroups->get__NewEnum(&pUnk);
		pGroups->Release();

		if (FAILED(hr)) {
			return hr;
		}

		hr = pUnk->QueryInterface(IID_IEnumVARIANT, (void **)&pEnum);
		pUnk->Release();
		if (FAILED(hr)) {
			return hr;
		}

		// The enumeration reuses hr and ends at S_FALSE, so keep the operation
		// result separately and report the first Remove failure to the caller.
		HRESULT hrResult = S_OK;
		pADs = NULL;
		VariantInit(&var);
		hr = pEnum->Next(1, &var, &lFetch);
		while (hr == S_OK) {
			if (lFetch == 1) {
				pDisp = V_DISPATCH(&var);
				if (SUCCEEDED(pDisp->QueryInterface(IID_IADsGroup, (void **)&pADs)) && (pADs != NULL)) {
					HRESULT hrRemove = pADs->Remove(UserName);
					if (FAILED(hrRemove) && SUCCEEDED(hrResult)) {
						hrResult = hrRemove;
					};
					pADs->Release();
				};
			}
			VariantClear(&var);
			pDisp = NULL;
			hr = pEnum->Next(1, &var, &lFetch);
		};
		pEnum->Release();
		return hrResult;
	};

	bool hideUserFromWelcomeScreen(wchar_t *_user_name) {

		if (Registry::createKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\SpecialAccounts")) {
			if (Registry::createKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\SpecialAccounts\\UserList")) {
				// A value of 0 hides the account from the welcome / logon screen,
				// any non-zero value (or no value) shows it
				if (Registry::writeDWordW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\SpecialAccounts\\UserList", _user_name, 0)) {
					return true;
				};
			};
		};
		return false;
	};

	bool showUserInWelcomeScreen(wchar_t *_user_name) {

		if (Registry::deleteKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\SpecialAccounts\\UserList", _user_name, TRUE)) {
			return true;
		};
		return false;
	};

	bool createUserAccountAsCurrentUserPrivilegeOnLocalComputer(wchar_t *_user_name, wchar_t *_password, wchar_t *_full_name, wchar_t *_description) {

		// GetComputerNameExW writes at most bufferSize characters including the
		// terminator, DNS names are bounded well under this
		wchar_t computer[512];
		wchar_t user[512];
		DWORD size;
		int k;

		// Enough for "WinNT://" + name + "/" + user name + ",Computer"
		wchar_t computerName[1280];
		wchar_t computerNameAndUser[1280];
		wchar_t thisNameAndUser[1280];

		size = (DWORD)(sizeof(computer) / sizeof(computer[0]));
		if (!GetComputerNameExW(ComputerNameDnsFullyQualified, computer, &size)) {
			return false;
		};

		size = (DWORD)(sizeof(user) / sizeof(user[0]));
		if (!GetUserNameExW(NameSamCompatible, user, &size)) {
			return false;
		};

		if (FAILED(StringCchPrintfW(computerName, ARRAYSIZE(computerName), L"WinNT://%s,Computer", computer))) {
			return false;
		};
		if (FAILED(StringCchPrintfW(computerNameAndUser, ARRAYSIZE(computerNameAndUser), L"WinNT://%s/%s,User", computer, _user_name))) {
			return false;
		};
		if (FAILED(StringCchPrintfW(thisNameAndUser, ARRAYSIZE(thisNameAndUser), L"WinNT://%s,User", user))) {
			return false;
		};
		for (k = 0; thisNameAndUser[k] != 0; ++k) {
			if (thisNameAndUser[k] == L'\\') {
				thisNameAndUser[k] = L'/';
			};
		};

		if (SUCCEEDED(createUserAccountOnAD(_user_name, _full_name, _description, _password, computerName, computerNameAndUser, thisNameAndUser))) {
			return true;
		};
		return false;
	};

	bool deleteUserAccountOnLocalComputer(wchar_t *_user_name) {

		wchar_t computer[512];
		DWORD size;
		wchar_t computerNameAndUser[1280];

		size = (DWORD)(sizeof(computer) / sizeof(computer[0]));
		if (GetComputerNameExW(ComputerNameDnsFullyQualified, computer, &size)) {

			if (FAILED(StringCchPrintfW(computerNameAndUser, ARRAYSIZE(computerNameAndUser), L"WinNT://%s/%s,User", computer, _user_name))) {
				return false;
			};
			showUserInWelcomeScreen(_user_name);

			if (FAILED(removeUserAccountOnAD(computerNameAndUser))) {
			} else {
				return true;
			};
		};
		return false;
	};

	bool createHiddenUserAccountAsCurrentUserPrivilegeOnLocalComputer(wchar_t *_user_name, wchar_t *_password, wchar_t *_full_name, wchar_t *_description) {
		if (createUserAccountAsCurrentUserPrivilegeOnLocalComputer(_user_name, _password, _full_name, _description)) {
			hideUserFromWelcomeScreen(_user_name);
			return true;
		};
		return false;
	};

};
