/*
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
*/

#ifndef _INPUT_ALERT_H
#define _INPUT_ALERT_H

#include <Alert.h>
#include <TextView.h>
#include <TextControl.h>
#include <String.h>
#include <Button.h>

#ifdef B_ZETA_VERSION
#include <interface/StringView.h>
#else
#include <StringView.h>
#endif

#define kInputText        "input_field"
#define kButtonIndex    "button_index"

enum
{
    kButton0 = '_b0_',
    kButton1 = '_b1_',
    kButton2 = '_b2_',
    kInputBox = '_ip_'
};

class InputAlert : public BAlert
{
    public:
        InputAlert (const char *title, const char *text, const char *initialText, bool hideTyping,
               const char *button1, const char *button2 = NULL, const char *button3 = NULL,
               button_width width = B_WIDTH_AS_USUAL, alert_type type = B_INFO_ALERT);
        InputAlert (const char *title, const char *text, const char *initialText, bool hideTyping,
               const char *button1, const char *button2, const char *button3, button_width width,
               button_spacing spacing, alert_type type = B_INFO_ALERT);
        
        // Inherited hooks
        void                MessageReceived (BMessage *message);
        
        // Additional hooks
        BMessage            GetInput (BWindow *window);
        BTextControl        *TextControl () const;
        
        // Static constants
        static const uint32  kInputMessage = 'inpt';
        
    private:
        // Private hooks
        void                InitInputAlert (const char *title, const char *label, const char *initialText,
                             bool hideTyping);
        
        // Private members
        BTextControl        *m_inputBox;
        BStringView        *m_bytesView;
        BButton            *m_farRightButton;
        BString             m_inputText;
        volatile bool        m_isQuitting;
        int32               m_buttonIndex;
};

#endif /* _INPUT_ALERT_H */
