/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#ifndef _BARBER_POLE_H
#define _BARBER_POLE_H

#include "BevelView.h"

class BarberPole : public BevelView
{
	public:
		BarberPole (BRect frame, const char *name);

		// Additional hooks
		void				SetValue (bool animate, bool adjustPulse = true);
		void				Animate ();
		bool				IsAnimating () const;
		void				GetPreferredSize (float *width, float *height);
		void				AttachedToWindow ();
		float				Width () const;
		
	private:
		// Private inherited hooks
		void				Draw (BRect updateRect);
		void				Pulse ();
		
		// Data members
		BBitmap				*m_poleImage;
		float				m_imageHeight,
							m_edgeThickness,
							m_y;
		bool				m_animate;
		
		typedef BevelView	_inherited;
};

#endif /* _BARBER_POLE_H */
