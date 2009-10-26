/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#ifndef _STARTUP_WINDOW_H
#define _STARTUP_WINDOW_H

#include <Window.h>

class BBitmap;
class BPopUpMenu;

class BevelView;
class RecentMgr;
class ImageButton;
class BeezerStringView;

class BubbleHelper;

#define M_CLOSE_STARTUP 'stcl'

class StartupWindow : public BWindow
{
	public:
		StartupWindow (RecentMgr *recentMgr, BubbleHelper *helper, bool startup);
		
		// Inherited hooks
		virtual void		MessageReceived (BMessage *message);
		virtual void		Quit ();
		virtual bool		QuitRequested ();
		
	protected:
		// Protected members
		BevelView			*m_backView;
		BeezerStringView	*m_headingView;
		ImageButton			*m_createBtn,
							*m_openBtn,
							*m_openRecentBtn,
							*m_toolsBtn,
							*m_prefsBtn;
		RecentMgr			*m_recentMgr;
		BubbleHelper		*m_bubbleHelper;
		BPopUpMenu			*m_recentMenu,
							*m_toolsMenu;
};

#endif /* _STARTUP_WINDOW_H */
