// Win
// Copyright (c) 2014-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2014-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_WIN_MESSAGEMANAGER_HPP
#define XYO_WIN_MESSAGEMANAGER_HPP

#ifndef XYO_WIN_WINDOW_HPP
#	include <XYO/Win/Window.hpp>
#endif

namespace XYO::Win {

	class MessageManager : public virtual Object {
			XYO_DISALLOW_COPY_ASSIGN_MOVE(MessageManager);

		public:
			typedef TDoubleEndedQueue<TPointer<Window>> WindowList;

		protected:
			CRITICAL_SECTION cs_;
			WindowList windowList_;
			void eventOnDestroy_(WindowList::Node *window);

		public:
			XYO_WIN_EXPORT MessageManager();
			XYO_WIN_EXPORT ~MessageManager();

			XYO_WIN_EXPORT WindowList::Node *add(Window *window);
			XYO_WIN_EXPORT void remove(WindowList::Node *window);

			XYO_WIN_EXPORT int processAllMessages();

			XYO_WIN_EXPORT void postMessageToAll(UINT);
			XYO_WIN_EXPORT void sendMessageToAll(UINT);

			XYO_WIN_EXPORT static void initMemory();
	};

};

#endif
