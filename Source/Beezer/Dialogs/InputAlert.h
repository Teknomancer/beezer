// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#ifndef _INPUT_ALERT_H
#define _INPUT_ALERT_H

#include <Alert.h>

class BStringView;
class BTextControl;

class InputAlert : public BAlert
{
    public:
        InputAlert(const char* title, const char* text, const char* initialText, bool hideTyping,
                   const char* button1, const char* button2 = NULL, const char* button3 = NULL,
                   button_width width = B_WIDTH_AS_USUAL, alert_type type = B_INFO_ALERT);
        InputAlert(const char* title, const char* text, const char* initialText, bool hideTyping,
                   const char* button1, const char* button2, const char* button3, button_width width,
                   button_spacing spacing, alert_type type = B_INFO_ALERT);

        // Inherited hooks
        void                     MessageReceived(BMessage* message);

        // Additional hooks
        BMessage                 GetInput(BWindow* window);
        BTextControl*            TextControl() const;

        static uint32 const      kInputMessage;
        static const char* const kInputText;
        static const char* const kButtonIndex;

    private:
        void                     InitInputAlert(const char* title, const char* label, const char* initialText,
                                                bool hideTyping);

        BButton*                 m_LastButton;
        BTextControl*            m_inputBox;
        BStringView*             m_bytesView;
        const char*              m_inputText;
        volatile bool            m_isQuitting;
        int32                    m_buttonIndex;

};

#endif /* _INPUT_ALERT_H */
