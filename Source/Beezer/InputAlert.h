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
#define kButtonIndex      "button_index"

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
        BTextControl       *TextControl () const;
        
        // Static constants
        static const uint32 kInputMessage = 'inpt';
        
    private:
        // Private hooks
        void                InitInputAlert (const char *title, const char *label, const char *initialText,
                             bool hideTyping);
        
        // Private members
        BTextControl       *m_inputBox;
        BStringView        *m_bytesView;
        BButton            *m_farRightButton;
        BString             m_inputText;
        volatile bool       m_isQuitting;
        int32               m_buttonIndex;
};

#endif /* _INPUT_ALERT_H */
