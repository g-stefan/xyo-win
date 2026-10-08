// Win
// Copyright (c) 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <stdio.h>

#include <XYO/Win/Capture.hpp>
#include <XYO/Pixel32.hpp>

namespace XYO::Win::Capture {

	static_assert(sizeof(BITMAPFILEHEADER) == sizeof(BitmapFileHeader), "BITMAPFILEHEADER layout");
	static_assert(sizeof(BITMAPINFOHEADER) == sizeof(BitmapInfoHeader), "BITMAPINFOHEADER layout");

	// Copy a rectangle of hdcSource to a new 32 bits bitmap (BI_RGB, bottom-up)
	static TPointer<Bitmap> captureDC(HDC hdcSource, int x, int y, int width, int height, DWORD rop) {
		HDC hdcCapture;
		HBITMAP bmpCapture;
		HBITMAP bmpCaptureOld;
		bool isOk;

		if ((width <= 0) || (height <= 0)) {
			return nullptr;
		};

		hdcCapture = CreateCompatibleDC(hdcSource);
		if (hdcCapture == NULL) {
			return nullptr;
		};

		bmpCapture = CreateCompatibleBitmap(hdcSource, width, height);
		if (bmpCapture == NULL) {
			DeleteDC(hdcCapture);
			return nullptr;
		};

		bmpCaptureOld = (HBITMAP)SelectObject(hdcCapture, bmpCapture);
		if (bmpCaptureOld == NULL) {
			DeleteObject(bmpCapture);
			DeleteDC(hdcCapture);
			return nullptr;
		};

		isOk = BitBlt(hdcCapture, 0, 0, width, height, hdcSource, x, y, rop);

		// GetDIBits requires the bitmap not selected into a device context
		SelectObject(hdcCapture, bmpCaptureOld);

		if (!isOk) {
			DeleteObject(bmpCapture);
			DeleteDC(hdcCapture);
			return nullptr;
		};

		// 32 bits rows are always DWORD aligned, no palette
		size_t imageSize = (size_t)width * 4 * (size_t)height;
		size_t offBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		size_t fileSize = offBits + imageSize;
		if (fileSize > 0xFFFFFFFFu) {
			DeleteObject(bmpCapture);
			DeleteDC(hdcCapture);
			return nullptr;
		};

		uint8_t *imageFile = new uint8_t[fileSize];
		BITMAPFILEHEADER *fileHeader = (BITMAPFILEHEADER *)imageFile;
		BITMAPINFO *info = (BITMAPINFO *)(imageFile + sizeof(BITMAPFILEHEADER));

		memset(imageFile, 0, offBits);
		fileHeader->bfType = XYO_PIXEL32_BITMAP_FILE_ID;
		fileHeader->bfSize = (DWORD)fileSize;
		fileHeader->bfOffBits = (DWORD)offBits;
		info->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		info->bmiHeader.biWidth = width;
		info->bmiHeader.biHeight = height;
		info->bmiHeader.biPlanes = 1;
		info->bmiHeader.biBitCount = 32;
		info->bmiHeader.biCompression = BI_RGB;
		info->bmiHeader.biSizeImage = (DWORD)imageSize;

		// For 32 bits BI_RGB GetDIBits does not write a color table,
		// the header in the image is the BITMAPINFO it needs
		isOk = (GetDIBits(hdcCapture, bmpCapture, 0, height, imageFile + offBits, info, DIB_RGB_COLORS) == height);

		DeleteObject(bmpCapture);
		DeleteDC(hdcCapture);

		if (!isOk) {
			delete[] imageFile;
			return nullptr;
		};

		return Bitmap::newImageOwner((BitmapImage *)imageFile);
	};

	TPointer<Bitmap> captureDesktop() {
		TPointer<Bitmap> retV;
		HDC hdcDesktop;

		hdcDesktop = GetDC(NULL);
		if (hdcDesktop == NULL) {
			return nullptr;
		};

		// The screen DC covers the whole virtual screen (all monitors),
		// coordinates are relative to the primary monitor
		retV = captureDC(hdcDesktop,
		                 GetSystemMetrics(SM_XVIRTUALSCREEN),
		                 GetSystemMetrics(SM_YVIRTUALSCREEN),
		                 GetSystemMetrics(SM_CXVIRTUALSCREEN),
		                 GetSystemMetrics(SM_CYVIRTUALSCREEN),
		                 SRCCOPY | CAPTUREBLT);

		ReleaseDC(NULL, hdcDesktop);
		return retV;
	};

	bool captureDesktopToPNGFile(const char *fileName) {
		TPointer<Bitmap> image = captureDesktop();
		if (image.value() == nullptr) {
			return false;
		};
		TPointer<Bitmap> image2 = image->convertTo32Bits();
		if (image2.value() == nullptr) {
			return false;
		};
		image2->setAlpha32(255);
		return Process::bitmap32SavePNG(image2, fileName);
	};

	// Client area of the window
	TPointer<Bitmap> captureWindow(HWND hwnd) {
		TPointer<Bitmap> retV;
		HDC hdcWindow;
		RECT rect;

		if (!GetClientRect(hwnd, &rect)) {
			return nullptr;
		};

		hdcWindow = GetDC(hwnd);
		if (hdcWindow == NULL) {
			return nullptr;
		};

		retV = captureDC(hdcWindow, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SRCCOPY);

		ReleaseDC(hwnd, hdcWindow);
		return retV;
	};

	bool captureWindowToPNGFile(HWND hwnd, const char *fileName) {
		TPointer<Bitmap> image = captureWindow(hwnd);
		if (image.value() == nullptr) {
			return false;
		};
		TPointer<Bitmap> image2 = image->convertTo32Bits();
		if (image2.value() == nullptr) {
			return false;
		};
		image2->setAlpha32(255);
		return Process::bitmap32SavePNG(image2, fileName);
	};

};
