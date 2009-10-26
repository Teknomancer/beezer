/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#ifndef _PROGRESS_WINDOW_H
#define _PROGRESS_WINDOW_H

#include <Window.h>
#include <Messenger.h>

class BButton;

class BevelView;
class BarberPole;

class ProgressWindow : public BWindow
{
	public:
		ProgressWindow (BWindow *callerWindow, BMessage *actionMessage,
				BMessenger *&messenger, volatile bool *&cancel);
		virtual ~ProgressWindow ();
		
		// Inherited hooks
		virtual void		MessageReceived (BMessage *message);
		
	protected:
		// Protected members
		BevelView			*m_backView;
		BarberPole			*m_barberPole;
		BStatusBar			*m_statusBar;
		BButton				*m_cancelButton;
		int32				m_fileCount,
							m_progressCount;
		char				m_updateText[B_PATH_NAME_LENGTH + 1];
		volatile bool		m_cancel;
		BMessenger			*m_messenger;
};

#endif /* _PROGRESS_WINDOW_H */
