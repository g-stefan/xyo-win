//
// XYO Win
//
// Copyright (c) 2020-2022 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef SECURITY_WIN32
#define SECURITY_WIN32
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

#include "xyo-win-shell.hpp"
#include "xyo-win-ole.hpp"

namespace XYO {
	namespace Win {
		namespace Shell {

			bool createLink(const char *outputFile, const char *workingDirectory, const char *path, const char *arguments, const char *iconPath, int iconIndex, bool runAsAdministrator) {
				IShellLink *pShellLink = nullptr;
				IPersistFile *pPersistFile = nullptr;
				IShellLinkDataList *pShellLinkDataList = nullptr;
				DWORD dwFlags = 0;

				if (!Ole::isValid()) {
					return false;
				};

				if (CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **) &pShellLink) == S_OK) {

					if(pShellLink->SetWorkingDirectory(workingDirectory) != S_OK) {
						pShellLink->Release();
						return false;
					};

					if(pShellLink->SetPath(path) != S_OK) {
						pShellLink->Release();
						return false;
					};

					if(pShellLink->SetArguments(arguments) != S_OK) {
						pShellLink->Release();
						return false;
					};

					if(pShellLink->SetIconLocation(iconPath, iconIndex) != S_OK) {
						pShellLink->Release();
						return false;
					};

					if(runAsAdministrator) {

						if(pShellLink->QueryInterface(IID_IShellLinkDataList, (void **) &pShellLinkDataList) != S_OK) {
							pShellLink->Release();
							return false;
						};

						if(pShellLinkDataList->GetFlags(&dwFlags) != S_OK) {
							pShellLinkDataList->Release();
							pShellLink->Release();
							return false;
						};


						dwFlags |= SLDF_RUNAS_USER;

						if(pShellLinkDataList->SetFlags(dwFlags) != S_OK) {
							pShellLinkDataList->Release();
							pShellLink->Release();
							return false;
						};


						pShellLinkDataList->Release();

					};

					if(pShellLink->QueryInterface(IID_IPersistFile, (void **) &pPersistFile) != S_OK) {
						pShellLink->Release();
						return false;
					};

					wchar_t *outputFileS;
					StringUtf16 outputFileU16(TUtfConvert<utf16, utf8>::from(outputFile));
					outputFileS=(wchar_t *)SysAllocString((BSTR)outputFileU16.value());

					if(pPersistFile->Save(outputFileS, TRUE) != S_OK) {
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

			bool createLinkW(const wchar_t *outputFile, const wchar_t *workingDirectory, const wchar_t *path, const wchar_t *arguments, const wchar_t *iconPath, int iconIndex, bool runAsAdministrator) {
				IShellLinkW *pShellLink = nullptr;
				IPersistFile *pPersistFile = nullptr;
				IShellLinkDataList *pShellLinkDataList = nullptr;
				DWORD dwFlags = 0;

				if (!Ole::isValid()) {
					return false;
				};

				if (CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (void **) &pShellLink) == S_OK) {

					if(pShellLink->SetWorkingDirectory(workingDirectory) != S_OK) {
						pShellLink->Release();
						return false;
					};

					if(pShellLink->SetPath(path) != S_OK) {
						pShellLink->Release();
						return false;
					};

					if(pShellLink->SetArguments(arguments) != S_OK) {
						pShellLink->Release();
						return false;
					};

					if(pShellLink->SetIconLocation(iconPath, iconIndex) != S_OK) {
						pShellLink->Release();
						return false;
					};

					if(runAsAdministrator) {

						if(pShellLink->QueryInterface(IID_IShellLinkDataList, (void **) &pShellLinkDataList) != S_OK) {
							pShellLink->Release();
							return false;
						};

						if(pShellLinkDataList->GetFlags(&dwFlags) != S_OK) {
							pShellLinkDataList->Release();
							pShellLink->Release();
							return false;
						};


						dwFlags |= SLDF_RUNAS_USER;

						if(pShellLinkDataList->SetFlags(dwFlags) != S_OK) {
							pShellLinkDataList->Release();
							pShellLink->Release();
							return false;
						};


						pShellLinkDataList->Release();

					};

					if(pShellLink->QueryInterface(IID_IPersistFile, (void **) &pPersistFile) != S_OK) {
						pShellLink->Release();
						return false;
					};

					wchar_t *outputFileS;
					outputFileS=(wchar_t *)SysAllocString((BSTR)outputFile);

					if(pPersistFile->Save(outputFileS, TRUE) != S_OK) {
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

		};
	};
};

