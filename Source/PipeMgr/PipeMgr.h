/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *	See "License.txt" for licensing info.
*/

#include <OS.h>
#include <List.h>
#include <SupportDefs.h>

class PipeMgr
{
	public:
		PipeMgr ();
		virtual ~PipeMgr ();
		
		// Additional hooks
		void				FlushArgs ();
		status_t			AddArg (const char *argv);
		void				Pipe () const;
		thread_id			Pipe (int *outdes) const;
		thread_id			Pipe (int *outdes, int *errdes) const;
		void				PrintToStream () const;
		
		// Custom operators
		PipeMgr& operator	<< (const char *arg);
		PipeMgr& operator	<< (BString arg);
		
	protected:
		// Protected members
		BList				m_argList;
};
