// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2005 Ramshankar (aka Teknomancer).
// All rights reserved.

#include <Roster.h>
#include <Entry.h>
#include <Alert.h>
#include <TrackerAddOn.h>
#include <String.h>

#include <cstdlib>

#include "Shared.h"


void process_refs(entry_ref, BMessage* message, void*)
{
    message->what = M_LAUNCH_TRACKER_ADDON;
    be_roster->Launch(K_APP_SIGNATURE, message);
}


int main()
{
    BString str = "Beezer Tracker add-on";
    str << "\n" << "© 2005 Ramshankar.\n\n";
    str << "This must be placed in:\n";
    str << "/boot/home/config/add-ons/Tracker\n\n";
    str << "To use this add-on choose the files you with to archive, right-click them and choose this add-on" B_UTF8_ELLIPSIS;

    str.Prepend("alert \"");
    str.Append("\"");
    system(str.String());
    return 0;
}
