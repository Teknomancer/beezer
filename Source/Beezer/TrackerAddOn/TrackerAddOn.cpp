/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * -> Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * -> Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * -> Neither the name of the author nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Roster.h>
#include <Entry.h>
#include <Alert.h>
#include <TrackerAddOn.h>
#include <String.h>

#include <stdlib.h>

#include "Shared.h"

//=============================================================================================================//

void process_refs (entry_ref dirRef, BMessage *message, void *reserved)
{
    message->what = M_LAUNCH_TRACKER_ADDON;
    be_roster->Launch (K_APP_SIGNATURE, message);
}

//=============================================================================================================//

int main ()
{
    BString str = "Beezer Tracker add-on";
    str << "\n" << "© 2005 Ramshankar.\n\n";
    str << "This must be placed in:\n";
    str << "/boot/home/config/add-ons/Tracker\n\n";
    str << "To use this add-on choose the files you with to archive, right-click them and choose this add-on…";
    
    str.Prepend ("alert \"");
    str.Append ("\"");
    system (str.String());
    return 0;
}

//=============================================================================================================//
