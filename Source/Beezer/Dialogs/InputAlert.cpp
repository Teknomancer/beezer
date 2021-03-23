/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011, Chris Roberts
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


#include <Button.h>
#include <String.h>
#include <TextControl.h>

#include <string.h>

#include "InputAlert.h"



InputAlert::InputAlert(const char* title, const char* text, const char* initialText, bool hideTyping,
                       const char* button1, const char* button2, const char* button3, button_width width,
                       alert_type type)
    : BAlert(title, "\n\n", button1, button2, button3, width, type)
{
    InitInputAlert(title, text, initialText, hideTyping);
}



InputAlert::InputAlert(const char* title, const char* text, const char* initialText, bool hideTyping,
                       const char* button1, const char* button2, const char* button3, button_width width,
                       button_spacing spacing, alert_type type)
    : BAlert(title, "\n\n", button1, button2, button3, width, spacing, type)
{
    InitInputAlert(title, text, initialText, hideTyping);
}



void InputAlert::InitInputAlert(const char* title, const char* label, const char* initialText, bool hideTyping)
{
    BTextView* textView = TextView();
    BView* parent = textView->Parent();
    textView->SetText(title);

    float height = textView->TextHeight(0, 2E6) + 6;
    float width = 0;
    int numlines = textView->CountLines();
    int linewidth;

    for (int i = 0; i < numlines; i++)
        if ((linewidth = int (textView->LineWidth(i))) > width)
            width = linewidth;

    textView->ResizeTo(width + 2, height);

    // Do our own Go() type function. Nice workaround used - See MessageReceived() and GetInput()
    // work in conjunction. Not too much CPU usage infact. What we do here is we erase BAlert's
    // button message and assign our own. Then trap it in MessageReceived and set "m_inputText"
    // then set "m_isQuitting" to true so that our GetInput() function adds the "m_inputText" to be
    // returned, then quit using the PostMessage() call.
    BButton* button0 = (BButton*)parent->FindView("_b0_");
    BButton* button1 = (BButton*)parent->FindView("_b1_");
    BButton* button2 = (BButton*)parent->FindView("_b2_");
    float extremeRight = 200;
    if (button0)
    {
        button0->SetMessage(new BMessage(kButton0));
        extremeRight = button0->Frame().right;
        m_farRightButton = button0;
    }

    if (button1)
    {
        button1->SetMessage(new BMessage(kButton1));
        extremeRight = button1->Frame().right;
        m_farRightButton = button1;
    }

    if (button2)
    {
        button2->SetMessage(new BMessage(kButton2));
        extremeRight = button2->Frame().right;
        m_farRightButton = button2;
    }

    m_inputBox = new BTextControl(BRect(textView->Frame().left, textView->Frame().bottom,
                                        extremeRight, 0), "_textInput_", label, NULL, NULL, B_FOLLOW_LEFT,
                                  B_WILL_DRAW | B_NAVIGABLE);

    m_inputBox->SetDivider(m_inputBox->StringWidth(label) + 10);
    m_inputBox->SetModificationMessage(new BMessage(kInputBox));
    m_inputBox->TextView()->HideTyping(hideTyping);
    m_inputBox->SetText(initialText);

    if (strlen(initialText) == 0)
        m_farRightButton->SetEnabled(false);

    parent->Window()->ResizeBy(0, m_farRightButton->Frame().Height() + m_inputBox->Frame().Height());
    parent->AddChild(m_inputBox);
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
                m_farRightButton->SetEnabled(true);
            else
                m_farRightButton->SetEnabled(false);
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


