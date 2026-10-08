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

	// NULL stays NULL (optional argument), the UTF-8 conversion does not
	// accept a null pointer.
	static StringUTF16 fromUTF8_(const char *value) {
		if (value == nullptr) {
			return StringUTF16();
		};
		return TUTFConvert<utf16, utf8>::from(value);
	};

	static const wchar_t *valueOrNull_(const StringUTF16 &converted, const char *value) {
		if (value == nullptr) {
			return nullptr;
		};
		return (const wchar_t *)converted.value();
	};

	bool createLink(const char *outputFile, const char *workingDirectory, const char *path, const char *arguments, const char *iconPath, int iconIndex, bool runAsAdministrator) {
		StringUTF16 _outputFile = fromUTF8_(outputFile);
		StringUTF16 _workingDirectory = fromUTF8_(workingDirectory);
		StringUTF16 _path = fromUTF8_(path);
		StringUTF16 _arguments = fromUTF8_(arguments);
		StringUTF16 _iconPath = fromUTF8_(iconPath);

		return createLinkW(valueOrNull_(_outputFile, outputFile), valueOrNull_(_workingDirectory, workingDirectory), valueOrNull_(_path, path), valueOrNull_(_arguments, arguments), valueOrNull_(_iconPath, iconPath), iconIndex, runAsAdministrator);
	};

	bool createLinkW(const wchar_t *outputFile, const wchar_t *workingDirectory, const wchar_t *path, const wchar_t *arguments, const wchar_t *iconPath, int iconIndex, bool runAsAdministrator) {
		IShellLinkW *pShellLink = nullptr;
		IPersistFile *pPersistFile = nullptr;
		IShellLinkDataList *pShellLinkDataList = nullptr;
		DWORD dwFlags = 0;

		// The link file and its target are required, the other fields are
		// optional (NULL = not set)
		if ((outputFile == nullptr) || (path == nullptr)) {
			return false;
		};

		// Initialize OLE on this thread when not done yet; a thread already
		// in the multithreaded apartment can not be switched (isValid is
		// false) but can create the shell link as well.
		Ole::isValid();

		if (CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void **)&pShellLink) == S_OK) {

			if ((workingDirectory != nullptr) && (pShellLink->SetWorkingDirectory(workingDirectory) != S_OK)) {
				pShellLink->Release();
				return false;
			};

			if (pShellLink->SetPath(path) != S_OK) {
				pShellLink->Release();
				return false;
			};

			if ((arguments != nullptr) && (pShellLink->SetArguments(arguments) != S_OK)) {
				pShellLink->Release();
				return false;
			};

			if ((iconPath != nullptr) && (pShellLink->SetIconLocation(iconPath, iconIndex) != S_OK)) {
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

			// IPersistFile::Save takes a plain wide string, not a BSTR
			if (pPersistFile->Save(outputFile, TRUE) != S_OK) {
				pPersistFile->Release();
				pShellLink->Release();
				return false;
			};

			pPersistFile->Release();
			pShellLink->Release();

			return true;
		};

		return false;
	};

	bool runAs(const char *username, const char *password, const char *command) {
		StringUTF16 _username = fromUTF8_(username);
		StringUTF16 _password = fromUTF8_(password);
		StringUTF16 _command = fromUTF8_(command);

		return runAsW(valueOrNull_(_username, username), valueOrNull_(_password, password), valueOrNull_(_command, command));
	};

	bool runAsW(const wchar_t *username, const wchar_t *password, const wchar_t *command) {

		STARTUPINFOW startupInfo;
		PROCESS_INFORMATION processInfo;

		wchar_t *commandLine;
		size_t commandLength;
		BOOL created;

		if ((username == NULL) || (command == NULL)) {
			return false;
		};

		memset(&startupInfo, 0, sizeof(startupInfo));
		memset(&processInfo, 0, sizeof(processInfo));

		startupInfo.cb = sizeof(startupInfo);
		startupInfo.dwFlags = STARTF_USESHOWWINDOW;
		startupInfo.wShowWindow = SW_SHOW;

		// CreateProcessWithLogonW may modify lpCommandLine in place, so it must
		// point at a writable buffer, never at a string literal.
		commandLength = wcslen(command) + 1;
		commandLine = new wchar_t[commandLength];
		wcscpy_s(commandLine, commandLength, command);

		// Passing NULL as the domain lets Windows resolve local, UPN and
		// DOMAIN\user names itself. CreateProcessWithLogonW validates the
		// credentials on its own, no separate LogonUser is needed (a second
		// logon would double the count against the account lockout policy).
		created = CreateProcessWithLogonW(
		    username,
		    NULL,
		    password,
		    LOGON_WITH_PROFILE,
		    NULL,
		    commandLine,
		    0,
		    NULL,
		    NULL,
		    &startupInfo,
		    &processInfo);

		delete[] commandLine;

		if (!created) {
			return false;
		};

		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
		return true;
	};

};
