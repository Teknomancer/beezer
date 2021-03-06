// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#include "JoinerStub.h"
#include "JoinerWindow.h"

#include "Shared.h"


JoinerStub::JoinerStub()
    : BApplication(K_JOINER_STUB_SIGNATURE),
    m_joinWnd(NULL)
{
    Run();
}


void JoinerStub::ReadyToRun()
{
    m_joinWnd = new JoinerWindow();

    return BApplication::ReadyToRun();
}


int main()
{
    delete new JoinerStub();
    return B_OK;
}
