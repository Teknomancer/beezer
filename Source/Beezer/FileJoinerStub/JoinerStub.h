/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#ifndef _JOINER_STUB_H
#define _JOINER_STUB_H

#include <Application.h>

class JoinerWindow;

class JoinerStub : public BApplication
{
	public:
		JoinerStub ();

		// Inherited hooks
		void				ReadyToRun ();
		
	private:
		// Private members
		JoinerWindow		*m_joinWnd;
};

#endif /* _JOINER_STUB_H */
