// Win
// Copyright (c) 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <windows.h>
#include <stdio.h>

#include <XYO/Win/Capture.hpp>
#include <XYO/Pixel32.hpp>

namespace XYO::Win::Capture {

	class MonitorInfo : public Object {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(MonitorInfo);

		public:
			HDC hdc;
			RECT rect;

			inline MonitorInfo() {
				hdc = NULL;
				memset(&rect, 0, sizeof(RECT));
			};

			inline ~MonitorInfo() {
				if (hdc != NULL) {
					DeleteDC(hdc);
				};
			};
	};

	static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
		TDynamicArray<MonitorInfo, 4, TMemorySystem> *monitorInfo = reinterpret_cast<TDynamicArray<MonitorInfo, 4, TMemorySystem> *>(dwData);

		HDC hdcMonitorCopy = CreateCompatibleDC(hdcMonitor);
		if (hdcMonitorCopy) {
			int monitorWidth = GetDeviceCaps(hdcMonitor, HORZRES);
			int monitorHeight = GetDeviceCaps(hdcMonitor, VERTRES);
			HBITMAP bmpMonitor = CreateCompatibleBitmap(hdcMonitor, monitorWidth, monitorHeight);
			if (bmpMonitor) {
				HBITMAP bmpMonitorOld = (HBITMAP)SelectObject(hdcMonitorCopy, bmpMonitor);
				if (bmpMonitorOld) {
					if (BitBlt(hdcMonitorCopy, 0, 0, monitorWidth, monitorHeight, hdcMonitor, lprcMonitor->left, lprcMonitor->top, SRCCOPY | CAPTUREBLT)) {

						int index_ = monitorInfo->length();
						(monitorInfo->index(index_)).hdc = hdcMonitorCopy;
						(monitorInfo->index(index_)).rect = *lprcMonitor;

						DeleteDC(hdcMonitor);
						DeleteObject(bmpMonitor);

						return TRUE;
					};
					DeleteObject(bmpMonitorOld);
				};
				DeleteObject(bmpMonitor);
			};
			DeleteDC(hdcMonitorCopy);
		};

		DeleteDC(hdcMonitor);
		return TRUE;
	};

	TPointer<Bitmap> captureDesktop() {
		HDC hdcDesktop;
		HDC hdcCapture;
		HBITMAP bmpCapture;
		HBITMAP bmpCaptureOld;
		TDynamicArray<MonitorInfo, 4, TMemorySystem> monitorInfo;
		int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		int minLeft = 0;
		int minTop = 0;
		int k;

		hdcDesktop = GetDC(NULL);
		if (hdcDesktop == NULL) {
			return nullptr;
		};

		hdcCapture = CreateCompatibleDC(hdcDesktop);
		if (hdcCapture == NULL) {
			DeleteDC(hdcDesktop);
			return nullptr;
		};

		bmpCapture = CreateCompatibleBitmap(hdcDesktop, screenWidth, screenHeight);
		if (bmpCapture == NULL) {
			DeleteDC(hdcDesktop);
			DeleteDC(hdcCapture);
			return nullptr;
		};

		bmpCaptureOld = (HBITMAP)SelectObject(hdcCapture, bmpCapture);
		if (bmpCaptureOld == NULL) {
			DeleteDC(hdcDesktop);
			DeleteDC(hdcCapture);
			DeleteObject(bmpCapture);
			return nullptr;
		};

		if (!EnumDisplayMonitors(hdcDesktop, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitorInfo))) {
			SelectObject(hdcCapture, bmpCaptureOld);
			DeleteDC(hdcDesktop);
			DeleteDC(hdcCapture);
			DeleteObject(bmpCapture);
			return nullptr;
		};

		for (k = 0; k < monitorInfo.length(); ++k) {
			if (monitorInfo[k].rect.left < minLeft) {
				minLeft = monitorInfo[k].rect.left;
			};
			if (monitorInfo[k].rect.top < minTop) {
				minTop = monitorInfo[k].rect.top;
			};
		};

		for (k = 0; k < monitorInfo.length(); ++k) {
			if (!BitBlt(hdcCapture, monitorInfo[k].rect.left - minLeft, monitorInfo[k].rect.top - minTop, monitorInfo[k].rect.right - monitorInfo[k].rect.left, monitorInfo[k].rect.bottom - monitorInfo[k].rect.top, monitorInfo[k].hdc, 0, 0, SRCCOPY)) {
				SelectObject(hdcCapture, bmpCaptureOld);
				DeleteDC(hdcDesktop);
				DeleteDC(hdcCapture);
				DeleteObject(bmpCapture);
				return nullptr;
			};
		};

		SelectObject(hdcCapture, bmpCaptureOld);

		uint8_t *imageBI = new uint8_t[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)];
		memset(imageBI, 0, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
		((LPBITMAPINFO)imageBI)->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		((LPBITMAPINFO)imageBI)->bmiHeader.biHeight = screenHeight;
		((LPBITMAPINFO)imageBI)->bmiHeader.biWidth = screenWidth;
		((LPBITMAPINFO)imageBI)->bmiHeader.biPlanes = 1;
		((LPBITMAPINFO)imageBI)->bmiHeader.biBitCount = 32;
		((LPBITMAPINFO)imageBI)->bmiHeader.biCompression = BI_RGB;
		if (!GetDIBits(hdcCapture, bmpCapture, 0, screenHeight, NULL, (LPBITMAPINFO)imageBI, DIB_RGB_COLORS)) {
			DeleteDC(hdcDesktop);
			DeleteDC(hdcCapture);
			DeleteObject(bmpCapture);
			return nullptr;
		};

		if (((LPBITMAPINFO)imageBI)->bmiHeader.biCompression != BI_RGB) {
			DeleteDC(hdcDesktop);
			DeleteDC(hdcCapture);
			DeleteObject(bmpCapture);
			delete[] imageBI;
			return nullptr;
		};

		DWORD imageSize = ((LPBITMAPINFO)imageBI)->bmiHeader.biSizeImage;
		DWORD imageBits = (((LPBITMAPINFO)imageBI)->bmiHeader.biPlanes) * (((LPBITMAPINFO)imageBI)->bmiHeader.biBitCount);
		DWORD palSize = 0;
		switch (imageBits) {
		case 1:
			palSize = 2;
			break;
		case 4:
			palSize = 16;
			break;
		case 8:
			palSize = 256;
			break;
		};
		DWORD imageFileSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + palSize * sizeof(RGBQUAD) + imageSize;
		uint8_t *imageFile = new uint8_t[imageFileSize];
		memset(imageFile, 0, imageFileSize);
		((BITMAPFILEHEADER *)imageFile)->bfType = XYO_PIXEL32_BITMAP_FILE_ID;
		((BITMAPFILEHEADER *)imageFile)->bfReserved1 = 0;
		((BITMAPFILEHEADER *)imageFile)->bfReserved2 = 0;
		((BITMAPFILEHEADER *)imageFile)->bfSize = imageSize;
		((BITMAPFILEHEADER *)imageFile)->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + palSize * sizeof(RGBQUAD);
		memcpy((((BITMAPFILEHEADER *)imageFile) + 1), imageBI, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));

		if (!GetDIBits(hdcCapture, bmpCapture, 0, screenHeight, &imageFile[((BITMAPFILEHEADER *)imageFile)->bfOffBits], (LPBITMAPINFO)imageBI, DIB_RGB_COLORS)) {
			DeleteDC(hdcDesktop);
			DeleteDC(hdcCapture);
			DeleteObject(bmpCapture);
			delete[] imageBI;
			delete[] imageFile;
			return nullptr;
		};

		TPointer<Bitmap> image = Bitmap::newImageOwner((BitmapImage *)imageFile);

		DeleteDC(hdcDesktop);
		DeleteDC(hdcCapture);
		DeleteObject(bmpCapture);
		delete[] imageBI;

		return image;
	};

	bool captureDesktopToPNGFile(char *fileName) {
		TPointer<Bitmap> image = captureDesktop();
		TPointer<Bitmap> image2 = image->convertTo32Bits();
		image2->setAlpha32(0);
		return Process::bitmap32SavePNG(image2, fileName);
	};

	TPointer<Bitmap> captureWindow(HWND hwnd) {
		HDC hdcWindow;
		HDC hdcCapture;
		HBITMAP bmpCapture;
		HBITMAP bmpCaptureOld;
		int windowWidth;
		int windowHeight;

		hdcWindow = GetDC(hwnd);
		if (hdcWindow == NULL) {
			return nullptr;
		};

		windowWidth = GetDeviceCaps(hdcWindow, HORZRES);
		windowHeight = GetDeviceCaps(hdcWindow, VERTRES);

		hdcCapture = CreateCompatibleDC(hdcWindow);
		if (hdcCapture == NULL) {
			DeleteDC(hdcWindow);
			return nullptr;
		};

		bmpCapture = CreateCompatibleBitmap(hdcWindow, windowWidth, windowHeight);
		if (bmpCapture == NULL) {
			DeleteDC(hdcWindow);
			DeleteDC(hdcCapture);
			return nullptr;
		};

		bmpCaptureOld = (HBITMAP)SelectObject(hdcCapture, bmpCapture);
		if (bmpCaptureOld == NULL) {
			DeleteDC(hdcWindow);
			DeleteDC(hdcCapture);
			DeleteObject(bmpCapture);
			return nullptr;
		};

		if (!BitBlt(hdcCapture, 0, 0, windowWidth, windowHeight, hdcWindow, 0, 0, SRCCOPY)) {
			SelectObject(hdcCapture, bmpCaptureOld);
			DeleteDC(hdcWindow);
			DeleteDC(hdcCapture);
			DeleteObject(bmpCapture);
			return nullptr;
		};

		SelectObject(hdcCapture, bmpCaptureOld);

		uint8_t *imageBI = new uint8_t[sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)];
		memset(imageBI, 0, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
		((LPBITMAPINFO)imageBI)->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		((LPBITMAPINFO)imageBI)->bmiHeader.biHeight = windowHeight;
		((LPBITMAPINFO)imageBI)->bmiHeader.biWidth = windowWidth;
		((LPBITMAPINFO)imageBI)->bmiHeader.biPlanes = 1;
		((LPBITMAPINFO)imageBI)->bmiHeader.biBitCount = 32;
		((LPBITMAPINFO)imageBI)->bmiHeader.biCompression = BI_RGB;
		if (!GetDIBits(hdcCapture, bmpCapture, 0, windowHeight, NULL, (LPBITMAPINFO)imageBI, DIB_RGB_COLORS)) {
			DeleteDC(hdcWindow);
			DeleteDC(hdcCapture);
			DeleteObject(bmpCapture);
			return nullptr;
		};

		if (((LPBITMAPINFO)imageBI)->bmiHeader.biCompression != BI_RGB) {
			DeleteDC(hdcWindow);
			DeleteDC(hdcCapture);
			DeleteObject(bmpCapture);
			delete[] imageBI;
			return nullptr;
		};

		DWORD imageSize = ((LPBITMAPINFO)imageBI)->bmiHeader.biSizeImage;
		DWORD imageBits = (((LPBITMAPINFO)imageBI)->bmiHeader.biPlanes) * (((LPBITMAPINFO)imageBI)->bmiHeader.biBitCount);
		DWORD palSize = 0;
		switch (imageBits) {
		case 1:
			palSize = 2;
			break;
		case 4:
			palSize = 16;
			break;
		case 8:
			palSize = 256;
			break;
		};
		DWORD imageFileSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + palSize * sizeof(RGBQUAD) + imageSize;
		uint8_t *imageFile = new uint8_t[imageFileSize];
		memset(imageFile, 0, imageFileSize);
		((BITMAPFILEHEADER *)imageFile)->bfType = XYO_PIXEL32_BITMAP_FILE_ID;
		((BITMAPFILEHEADER *)imageFile)->bfReserved1 = 0;
		((BITMAPFILEHEADER *)imageFile)->bfReserved2 = 0;
		((BITMAPFILEHEADER *)imageFile)->bfSize = imageSize;
		((BITMAPFILEHEADER *)imageFile)->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + palSize * sizeof(RGBQUAD);
		memcpy((((BITMAPFILEHEADER *)imageFile) + 1), imageBI, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));

		if (!GetDIBits(hdcCapture, bmpCapture, 0, windowHeight, &imageFile[((BITMAPFILEHEADER *)imageFile)->bfOffBits], (LPBITMAPINFO)imageBI, DIB_RGB_COLORS)) {
			DeleteDC(hdcWindow);
			DeleteDC(hdcCapture);
			DeleteObject(bmpCapture);
			delete[] imageBI;
			delete[] imageFile;
			return nullptr;
		};

		TPointer<Bitmap> image = Bitmap::newImageOwner((BitmapImage *)imageFile);

		DeleteDC(hdcWindow);
		DeleteDC(hdcCapture);
		DeleteObject(bmpCapture);
		delete[] imageBI;

		return image;
	};

	bool captureWindowToPNGFile(HWND hwnd, char *fileName) {
		TPointer<Bitmap> image = captureWindow(hwnd);
		TPointer<Bitmap> image2 = image->convertTo32Bits();
		image2->setAlpha32(0);
		return Process::bitmap32SavePNG(image2, fileName);
	};

};
