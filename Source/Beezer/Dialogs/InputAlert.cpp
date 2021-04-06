// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "InputAlert.h"
#include "UIConstants.h"

#include <Button.h>
#include <TextControl.h>
#include <Layout.h>
#include <GroupLayout.h>

#include <cassert>

static const uint32 kButton0    = '_b0_';
static const uint32 kButton2    = '_b2_';
static const uint32 kButton1    = '_b1_';
static const uint32 kInputBox   = '_ip_';

const uint32 InputAlert::kInputMessage      = 'inpt';
const char* InputAlert::kInputText          = "input_field";
const char* InputAlert::kButtonIndex        = "button_index";


InputAlert::InputAlert(const char* title, const char* text, const char* initialText, bool hideTyping,
                       const char* button1, const char* button2, const char* button3, button_width width,
                       alert_type type)
    : BAlert(title, "\n\n", button1, button2, button3, width, type)
    , m_LastButton(NULL)
{
    InitInputAlert(title, text, initialText, hideTyping);
}


InputAlert::InputAlert(const char* title, const char* text, const char* initialText, bool hideTyping,
                       const char* button1, const char* button2, const char* button3, button_width width,
                       button_spacing spacing, alert_type type)
    : BAlert(title, "\n\n", button1, button2, button3, width, spacing, type)
    , m_LastButton(NULL)
{
    InitInputAlert(title, text, initialText, hideTyping);
}


void InputAlert::InitInputAlert(const char* title, const char* label, const char* initialText, bool hideTyping)
{
    font_height fntHt;
    be_plain_font->GetHeight(&fntHt);
    float lineHeight = fntHt.ascent + fntHt.descent + fntHt.leading;

    BTextView* textView = TextView();
    textView->SetText(title);

    // Do our own Go() type function. Nice workaround used - See MessageReceived() and GetInput()
    // work in conjunction. Not too much CPU usage infact. What we do here is we erase BAlert's
    // button message and assign our own. Then trap it in MessageReceived and set "m_inputText"
    // then set "m_isQuitting" to true so that our GetInput() function adds the "m_inputText" to be
    // returned, then quit using the PostMessage() call.
    BButton* button0 = ButtonAt(0);
    if (button0)
    {
        button0->SetMessage(new BMessage(kButton0));
        m_LastButton = button0;
    }

    BButton* button1 = ButtonAt(1);
    if (button1)
    {
        button1->SetMessage(new BMessage(kButton1));
        m_LastButton = button1;
    }

    BButton* button2 = ButtonAt(2);
    if (button2)
    {
        button2->SetMessage(new BMessage(kButton2));
        m_LastButton = button2;
    }
    assert(m_LastButton);

    m_inputBox = new BTextControl(BRect(K_MARGIN, K_MARGIN, 400, 2 * K_MARGIN + lineHeight),
                                  "_textInput_", label, NULL, NULL, B_FOLLOW_LEFT, B_WILL_DRAW | B_NAVIGABLE);

    m_inputBox->SetDivider(m_inputBox->StringWidth(label) + 10);
    m_inputBox->SetModificationMessage(new BMessage(kInputBox));
    m_inputBox->TextView()->HideTyping(hideTyping);
    m_inputBox->SetText(initialText);

    if (strlen(initialText) == 0)
        m_LastButton->SetEnabled(false);

    BView* parentView = FindView("_tv_");
    assert(parentView);
    BWindow *parentWindow = parentView->Window();
    assert(parentWindow);

    BLayout* layout = GetLayout();
    assert(layout);
    BView* alertView = FindView("TAlertView");
    assert(alertView);
    BGroupLayout *group = (BGroupLayout*)layout;
    group->SetOrientation(B_VERTICAL);
    group->SetSpacing(K_MARGIN * 2);
    int32 const textViewIndex = layout->IndexOfView(alertView);

    layout->AddView(textViewIndex + 1, m_inputBox);

    parentWindow->ResizeTo(m_inputBox->Frame().Width(), 180);
    parentWindow->CenterOnScreen();
    m_inputBox->MakeFocus(true);
}


BMessage InputAlert::GetInput(BWindow* window)
{
    // Show and thus start the message loop.
    BMessage msg(kInputMessage);
    m_isQuitting = false;
    Show();

    // Wait till "m_isQuitting" turns true (meaning the user has finished typing and has pressed
    // one of the buttons. Till then wait and update the owner window (so it Draws when alert is moved
    // over its views) */
    while (m_isQuitting == false)
    {
        if (window)
            window->UpdateIfNeeded();

        snooze(10000);
    }

    // OK time to return the things we need to
    msg.AddInt32(kButtonIndex, m_buttonIndex);
    msg.AddString(kInputText, m_inputText);
    return msg;
}


void InputAlert::MessageReceived(BMessage* message)
{
    switch (message->what)
    {
        case kButton0: case kButton1 : case kButton2:
        {
            int32 w = message->what;
            m_buttonIndex = w == kButton0 ? 0 : w == kButton1 ? 1 : 2;
            m_inputText = m_inputBox->Text();
            m_isQuitting = true;

            snooze(20000);
            PostMessage(B_QUIT_REQUESTED);
            break;
        }

        case kInputBox:
        {
            int32 len = strlen(m_inputBox->Text());
            if (len > 0L)
                m_LastButton->SetEnabled(true);
            else
                m_LastButton->SetEnabled(false);
            break;
        }

        default:
            BAlert::MessageReceived(message);
            break;
    }
}


BTextControl* InputAlert::TextControl() const
{
    // Return pointer to our BTextControl incase caller needs to fiddle with it
    return m_inputBox;
}
