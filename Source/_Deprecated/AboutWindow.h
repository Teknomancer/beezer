/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Somebody)
 *	See "License.txt" for licensing info.
*/

#ifndef _ABOUT_WINDOW_H
#define _ABOUT_WINDOW_H

#include <Window.h>
#include <TextView.h>

#include "AppConstants.h"
#include "LangStrings.h"

#define K_SCROLL_DELAY		33000
#define K_EXPAND_DELAY		6000
#define M_CLOSE_ABOUT		'clab'

class MarqueeView : public BTextView
{
	public:
		MarqueeView (BRect frame, const char *name, BRect textRect, uint32 resizeMask,
				uint32 flags = B_WILL_DRAW);
		MarqueeView (BRect frame, const char *name, BRect textRect, const BFont *initialFont,
				const rgb_color *initialColor, uint32 resizeMask, uint32 flags);
	
		// Inherited hooks
		void				ScrollBy (float dh, float dv);
		void				ScrollTo (float x, float y);
		
		// Public members
		float				curPos,
							rightEdge;
};

class AboutWindow : public BWindow
{
	public:
		AboutWindow (const char *compileTimeStr);
		
		// Inherited hooks
		virtual void		DispatchMessage (BMessage *message, BHandler *handler);
		virtual void		Quit ();
				
	private:
		// Thread functions
		static int32		_scroller (void *data);

		// Private members
		BView				*m_backView,
							*m_titleView,
							*m_separatorView;
		MarqueeView			*m_textView;
		BBitmap				*m_separatorBmp;
		BString				m_lineFeeds;
		thread_id			m_scrollThreadID;
		const char			*m_creditsText;
		float				m_heightOfExpandedWindow;
		static const uint32	M_ANIMATE_SLIDE = 'anim';
};

#endif /* _ABOUT_WINDOW_H */
