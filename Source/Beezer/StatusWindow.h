/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#ifndef _STATUS_WINDOW_H
#define _STATUS_WINDOW_H

#include <Window.h>
#include <Messenger.h>

class BevelView;
class BarberPole;

class StatusWindow : public BWindow
{
	public:
		StatusWindow (const char *title, BWindow *callerWindow, const char *text, volatile bool *cancel,
					bool showWindow = true);
	
		// Inherited hooks
		virtual void		MessageReceived (BMessage *message);
		
	private:
		BevelView			*m_backView;
		BarberPole			*m_barberPole;
		volatile bool		*m_cancel;
};

#endif /* _STATUS_WINDOW_H */
