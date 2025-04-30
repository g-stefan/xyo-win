// Win
// Copyright (c) 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_APPLICATION_HPP
#define XYO_WIN_APPLICATION_HPP

#ifndef XYO_WIN_WINDOW_HPP
#	include <XYO/Win/Window.hpp>
#endif

#ifndef XYO_WIN_MESSAGEMANAGER_HPP
#	include <XYO/Win/MessageManager.hpp>
#endif

namespace XYO::Win {

	class Application : public virtual Window,
	                    public virtual IApplication {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(Application);

		protected:
			TPointer<MessageManager> messageManager_;

		public:
			inline Application(){};
			inline ~Application(){};

			XYO_WIN_EXPORT int main(int cmdN, char *cmdS[]);

			XYO_WIN_EXPORT virtual void setWndclassEx(WNDCLASSEX &);
			XYO_WIN_EXPORT virtual void setCreateStruct(CREATESTRUCT &);
			XYO_WIN_EXPORT virtual int setShowCmd(int);
			XYO_WIN_EXPORT virtual int parseCommandLine(int cmdN, char *cmdS[]);
			XYO_WIN_EXPORT virtual void initWndclassEx(WNDCLASSEX &wndclassEx);
			XYO_WIN_EXPORT virtual void initCreateStruct(CREATESTRUCT &createStruct, WNDCLASSEX &wndclassEx);
			XYO_WIN_EXPORT virtual bool newWindow(int cmdShow, bool regWndClass = false);

			XYO_WIN_EXPORT static void initMemory();
	};

};

#endif
