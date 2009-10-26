/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#ifndef _AUTHOR_WINDOW_H
#define _AUTHOR_WINDOW_H

#include <Window.h>

#include "AppConstants.h"
#include "LangStrings.h"
#include "BevelView.h"

#define M_CLOSE_AUTHOR		'clat'

class AuthorWindow : public BWindow
{
	public:
		AuthorWindow ();
	
		// Inherited hooks
		virtual void		Quit ();
		
	private:
		BevelView			*m_backView,
							*m_contentView;
};

#endif /* _AUTHOR_WINDOW_H */
