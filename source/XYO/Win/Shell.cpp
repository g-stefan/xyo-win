// Win
// Copyright (c) 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

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
#include <shobjidl.h>
#include <objidl.h>
#include <shlguid.h>
#include <shlobj_core.h>
#include <lm.h>
#include <lmuse.h>
#include <userenv.h>

#include <XYO/Win/Shell.hpp>
#include <XYO/Win/OLE.hpp>

namespace XYO::Win::Shell {

	bool createLink(const char *outputFile, const char *workingDirectory, const char *path, const char *arguments, const char *iconPath, int iconIndex, bool runAsAdministrator) {
		StringUTF16 _outputFile = TUTFConvert<utf16, utf8>::from(outputFile);
		StringUTF16 _workingDirectory = TUTFConvert<utf16, utf8>::from(workingDirectory);
		StringUTF16 _path = TUTFConvert<utf16, utf8>::from(path);
		StringUTF16 _arguments = TUTFConvert<utf16, utf8>::from(arguments);
		StringUTF16 _iconPath = TUTFConvert<utf16, utf8>::from(iconPath);

		return createLinkW((const wchar_t *)_outputFile.value(), (const wchar_t *)_workingDirectory.value(), (const wchar_t *)_path.value(), (const wchar_t *)_arguments.value(), (const wchar_t *)_iconPath.value(), iconIndex, runAsAdministrator);
	};

	bool createLinkW(const wchar_t *outputFile, const wchar_t *workingDirectory, const wchar_t *path, const wchar_t *arguments, const wchar_t *iconPath, int iconIndex, bool runAsAdministrator) {
		IShellLinkW *pShellLink = nullptr;
		IPersistFile *pPersistFile = nullptr;
		IShellLinkDataList *pShellLinkDataList = nullptr;
		DWORD dwFlags = 0;

		if (!Ole::isValid()) {
			return false;
		};

		if (CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void **)&pShellLink) == S_OK) {

			if (pShellLink->SetWorkingDirectory(workingDirectory) != S_OK) {
				pShellLink->Release();
				return false;
			};

			if (pShellLink->SetPath(path) != S_OK) {
				pShellLink->Release();
				return false;
			};

			if (pShellLink->SetArguments(arguments) != S_OK) {
				pShellLink->Release();
				return false;
			};

			if (pShellLink->SetIconLocation(iconPath, iconIndex) != S_OK) {
				pShellLink->Release();
				return false;
			};

			if (runAsAdministrator) {

				if (pShellLink->QueryInterface(IID_IShellLinkDataList, (void **)&pShellLinkDataList) != S_OK) {
					pShellLink->Release();
					return false;
				};

				if (pShellLinkDataList->GetFlags(&dwFlags) != S_OK) {
					pShellLinkDataList->Release();
					pShellLink->Release();
					return false;
				};

				dwFlags |= SLDF_RUNAS_USER;

				if (pShellLinkDataList->SetFlags(dwFlags) != S_OK) {
					pShellLinkDataList->Release();
					pShellLink->Release();
					return false;
				};

				pShellLinkDataList->Release();
			};

			if (pShellLink->QueryInterface(IID_IPersistFile, (void **)&pPersistFile) != S_OK) {
				pShellLink->Release();
				return false;
			};

			wchar_t *outputFileS;
			outputFileS = (wchar_t *)SysAllocString((BSTR)outputFile);

			if (pPersistFile->Save(outputFileS, TRUE) != S_OK) {
				SysFreeString((BSTR)outputFileS);
				pPersistFile->Release();
				pShellLink->Release();
				return false;
			};

			SysFreeString((BSTR)outputFileS);

			pPersistFile->Release();
			pShellLink->Release();

			return true;
		};

		return false;
	};

	bool runAs(const char *username, const char *password, const char *command) {
		StringUTF16 _username = TUTFConvert<utf16, utf8>::from(username);
		StringUTF16 _password = TUTFConvert<utf16, utf8>::from(password);
		StringUTF16 _command = TUTFConvert<utf16, utf8>::from(command);

		return runAsW((const wchar_t *)_username.value(), (const wchar_t *)_password.value(), (const wchar_t *)_command.value());
	};

	bool runAsW(const wchar_t *username, const wchar_t *password, const wchar_t *command) {

		STARTUPINFOW startupInfo;
		PROCESS_INFORMATION processInfo;
		STARTUPINFOEXW startupInfoEx;
		PROFILEINFOW profileInfo;

		HANDLE logonToken;
		wchar_t computerName[1024];
		DWORD computerNameLn;

		memset(&startupInfo, 0, sizeof(startupInfo));
		memset(&processInfo, 0, sizeof(processInfo));

		startupInfo.cb = sizeof(startupInfo);
		startupInfo.dwFlags = STARTF_USESHOWWINDOW;
		startupInfo.wShowWindow = SW_SHOW;

		computerNameLn = 1024;
		computerName[0] = 0;
		GetComputerNameExW(ComputerNameNetBIOS, computerName, &computerNameLn);

		if (!LogonUserW((BSTR)username,
		                computerName,
		                (BSTR)password,
		                LOGON32_LOGON_INTERACTIVE,
		                LOGON32_PROVIDER_DEFAULT,
		                &logonToken)) {
			return false;
		};

		return CreateProcessWithLogonW(
		    (BSTR)username,
		    computerName,
		    (BSTR)password,
		    LOGON_WITH_PROFILE,
		    NULL,
		    (BSTR)command,
		    0,
		    NULL,
		    NULL,
		    &startupInfo,
		    &processInfo);
	};

};
