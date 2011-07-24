/*
 *    Open Tracker License
 *    Terms and Conditions
 *    Copyright (c) 1991-2000, Be Incorporated. All rights reserved.
 *
 *    Permission is hereby granted, free of charge, to any person obtaining a copy of
 *    this software and associated documentation files (the "Software"), to deal in
 *    the Software without restriction, including without limitation the rights to
 *    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *    of the Software, and to permit persons to whom the Software is furnished to do
 *    so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice applies to all licensees
 *    and shall be included in all copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE, MERCHANTABILITY,
 *    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *    BE INCORPORATED BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 *    AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION
 *    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *    Except as contained in this notice, the name of Be Incorporated shall not be
 *    used in advertising or otherwise to promote the sale, use or other dealings in
 *    this Software without prior written authorization from Be Incorporated.
 *
 *    Tracker(TM), Be(R), BeOS(R), and BeIA(TM) are trademarks or registered trademarks
 *    of Be Incorporated in the United States and other countries. Other brand product
 *    names are registered trademarks or trademarks of their respective holders.
 *    All rights reserved.
 *
 *
 *    This code was taken from the Open Tracker project and is modified to suit
 *    _this_ project.
 *
 *    Originally this file contained TrackerString class. This was simply renamed
 *    as RegExString class for namesake. Other changes are only decorative. Above is
 *    the license for this file. What follows is the copyright information for _this_
 *    project.
 *
 *    Beezer
 *    Copyright (c) 2002 Ramshankar (aka Teknomancer)
 *    See "License.txt" for licensing info.
 *
 *    Changes to this code is marked as such
*/

#include <OS.h>
#include <Debug.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "RegExString.h"

//=============================================================================================================//

RegExString::RegExString()
{
}

//=============================================================================================================//

RegExString::RegExString (const char *string)
    : BString (string)
{
}

//=============================================================================================================//

RegExString::RegExString (const RegExString &string)
    : BString (string)
{
}

//=============================================================================================================//

RegExString::RegExString (const char *string, int32 maxLength)
    : BString (string, maxLength)
{
}

//=============================================================================================================//

RegExString::~RegExString()
{
}

//=============================================================================================================//

bool RegExString::Matches (const char *string, bool caseSensitivity,
                      RegExStringExpressionType expressionType) const
{
    switch (expressionType)
    {
        case kStartsWith:
           return StartsWith(string, caseSensitivity);
           
        case kEndsWith:
           return EndsWith(string, caseSensitivity);
    
        case kContains:
           return Contains(string, caseSensitivity);

        case kGlobMatch:
           return MatchesGlob(string, caseSensitivity);

        case kRegexpMatch:
           return MatchesRegExp(string, caseSensitivity);

        default:
        case kNone:
           return false;
    }
}

//=============================================================================================================//

bool RegExString::MatchesRegExp (const char *pattern, bool caseSensitivity) const
{
    BString patternString (pattern);
    BString textString (String());
    
    if (caseSensitivity == false)
    {
        patternString.ToLower();
        textString.ToLower();
    }
    
    RegExp expression (patternString);
    
    if (expression.InitCheck() != B_OK)
        return false;

    return expression.Matches (textString);
}

//=============================================================================================================//

bool RegExString::MatchesGlob (const char *string, bool caseSensitivity) const
{
    return StringMatchesPattern (String(), string, caseSensitivity);
}

//=============================================================================================================//

bool RegExString::EndsWith (const char *string, bool caseSensitivity) const
{
    // BUG-FIX by Ram - 11:35 AM 13/5/2004 India
    // Made sure foundIndexPos is not -1 while checking with FindLast (string)
    // Eg: "lib" comparing with ".cpp", now FindLast (".cpp") will be -1 and
    // Length() - strlen (".cpp") will be -1. In which case EndsWith would return
    // true but it should NOT it must return false, the below check ensures it
    // returns false correctly
    int32 foundIndexPos = Length() - (int32)strlen (string);
    if (foundIndexPos < 0)
        return false;
    
    if (caseSensitivity)
        return FindLast (string) == foundIndexPos;
    else
        return IFindLast (string) == foundIndexPos;
}

//=============================================================================================================//

bool RegExString::StartsWith (const char *string, bool caseSensitivity) const
{
    if (caseSensitivity)
        return FindFirst (string) == 0;
    else 
        return IFindFirst (string) == 0;
}

//=============================================================================================================//

bool RegExString::Contains (const char *string, bool caseSensitivity) const
{
    if (caseSensitivity)
        return FindFirst(string) > -1;
    else
        return IFindFirst(string) > -1;
}

//=============================================================================================================//

int32 RegExString::FindFirst (const BString &string) const
{
    return FindFirst (string.String(), 0);
}

//=============================================================================================================//

int32 RegExString::FindFirst (const char *string) const
{
    return FindFirst (string, 0);
}

//=============================================================================================================//

int32 RegExString::FindFirst(const BString &string, int32 fromOffset) const
{
    return FindFirst(string.String(), fromOffset);
}

//=============================================================================================================//

int32 RegExString::FindFirst (const char *string, int32 fromOffset) const
{
    if (!string)
        return -1;
    
    int32 length = Length ();
    uint32 stringLength = strlen (string);
    
    // The following two checks are required to be compatible with BString:
    if (length <= 0)
        return -1;
    
    if (stringLength == 0)
        return fromOffset;
    
    int32 stop = length - static_cast<int32>(stringLength);
    int32 start = MAX (0, MIN (fromOffset, stop));
    int32 position = -1;
    
    for (int32 i = start; i <= stop; i++)
        if (string[0] == ByteAt(i))
           // This check is to avoid mute str*cmp() calls. Performance.
           if (strncmp (string, String() + i, stringLength) == 0)
           {
               position = i;
               break;
           }
               
    return position;
}

//=============================================================================================================//

int32 RegExString::FindFirst(char ch) const
{
    char string[2] = {ch, '\0'};
    return FindFirst (string, 0);
}

//=============================================================================================================//

int32 RegExString::FindFirst (char ch, int32 fromOffset) const
{
    char string[2] = {ch, '\0'};
    return FindFirst (string, fromOffset);
}

//=============================================================================================================//

int32 RegExString::FindLast (const BString &string) const
{
    return FindLast (string.String(), Length() - 1);
}

//=============================================================================================================//

int32 RegExString::FindLast (const char *string) const
{
    return FindLast (string, Length() - 1);
}

//=============================================================================================================//

int32 RegExString::FindLast (const BString &string, int32 beforeOffset) const
{
    return FindLast (string.String(), beforeOffset);
}

//=============================================================================================================//

int32 RegExString::FindLast (const char *string, int32 beforeOffset) const
{
    if (!string)
        return -1;
    
    int32 length = Length();
    uint32 stringLength = strlen(string);
    
    // The following two checks are required to be compatible with BString
    if (length <= 0)
        return -1;
        
    if (stringLength == 0)
        return beforeOffset;
    
    int32 start = MIN (beforeOffset, length - static_cast<int32>(stringLength));    
    int32 stop = 0;
    int32 position = -1;
    
    for (int32 i = start; i >= stop; i--)
        if (string[0] == ByteAt(i))
           // This check is to avoid mute str*cmp() calls. Performance.
           if (strncmp(string, String() + i, stringLength) == 0)
           {
               position = i;
               break;
           }
               
    return position;
}

//=============================================================================================================//

int32 RegExString::FindLast(char ch) const
{
    char string[2] = {ch, '\0'};
    return FindLast (string, Length() - 1);
}

//=============================================================================================================//

int32 RegExString::FindLast (char ch, int32 beforeOffset) const
{
    char string[2] = {ch, '\0'};
    return FindLast (string, beforeOffset);
}

//=============================================================================================================//

int32 RegExString::IFindFirst (const BString &string) const
{
    return IFindFirst (string.String(), 0);
}

//=============================================================================================================//

int32 RegExString::IFindFirst(const char *string) const
{
    return IFindFirst (string, 0);
}

//=============================================================================================================//

int32 RegExString::IFindFirst (const BString &string, int32 fromOffset) const
{
    return FindFirst (string.String(), fromOffset);
}

//=============================================================================================================//

int32 RegExString::IFindFirst (const char *string, int32 fromOffset) const
{
    if (!string)
        return -1;
    
    int32 length = Length();
    uint32 stringLength = strlen(string);
    
    // The following two checks are required to be compatible with BString:
    if (length <= 0)
        return -1;
        
    if (stringLength == 0)
        return fromOffset;
    
    int32 stop = length - static_cast<int32>(stringLength);    
    int32 start = MAX (0, MIN(fromOffset, stop));
    int32 position = -1;
        
    for (int32 i = start; i <= stop; i++)
        if (tolower(string[0]) == tolower(ByteAt(i)))
           // This check is to avoid mute str*cmp() calls. Performance.
           if (strncasecmp(string, String() + i, stringLength) == 0) {
               position = i;
               break;
           }
               
    return position;
}

//=============================================================================================================//

int32 RegExString::IFindLast(const BString &string) const
{
    return IFindLast (string.String(), Length() - 1);
}

//=============================================================================================================//

int32 RegExString::IFindLast(const char *string) const
{
    return IFindLast (string, Length() - 1);
}

//=============================================================================================================//

int32 RegExString::IFindLast (const BString &string, int32 beforeOffset) const
{
    return IFindLast (string.String(), beforeOffset);
}

//=============================================================================================================//

int32 RegExString::IFindLast(const char *string, int32 beforeOffset) const
{
    if (!string)
        return -1;
    
    int32 length = Length();
    uint32 stringLength = strlen(string);
    
    // The following two checks are required to be compatible with BString:
    if (length <= 0)
        return -1;
        
    if (stringLength == 0)
        return beforeOffset;
    
    int32 start = MIN (beforeOffset, length - static_cast<int32>(stringLength));    
    int32 stop = 0;
    int32 position = -1;
    
    for (int32 i = start; i >= stop; i--)
        if (tolower(string[0]) == tolower(ByteAt(i)))
           // This check is to avoid mute str*cmp() calls. Performance.
           if (strncasecmp(string, String() + i, stringLength) == 0)
           {
               position = i;
               break;
           }
               
    return position;
}

//=============================================================================================================//

// MatchesBracketExpression() assumes 'pattern' to point to the
// character following the initial '[' in a bracket expression.
// The reason is that an encountered '[' will be taken literally.
// (Makes it possible to match a '[' with the expression '[[]').
bool RegExString::MatchesBracketExpression(const char *string, const char *pattern,
    bool caseSensitivity) const
{
    bool GlyphMatch = IsStartOfGlyph (string[0]);
    
    if (IsInsideGlyph (string[0]))
        return false;

    char testChar = ConditionalToLower (string[0], caseSensitivity);
    bool match = false;

    bool inverse = *pattern == '^' || *pattern == '!';
    // We allow both ^ and ! as a initial inverting character.
    
    if (inverse)
        pattern++;    
           
    while (!match && *pattern != ']' && *pattern != '\0')
    {
        switch (*pattern)
        {
           case '-':
           {
               char start = ConditionalToLower (*(pattern - 1), caseSensitivity),
                  stop = ConditionalToLower (*(pattern + 1), caseSensitivity);
               
               if (IsGlyph (start) || IsGlyph (stop))
                  return false;               // Not a valid range!
               
               if ((islower (start) && islower (stop))   
                  || (isupper (start) && isupper (stop))
                  || (isdigit (start) && isdigit (stop)))
                      match = start <= testChar && testChar <= stop;
               else
                  return false;
           }
           break;
           
           default:
               if (GlyphMatch)
                  match = UTF8CharsAreEqual (string, pattern);
               else
                  match = CharsAreEqual (testChar, *pattern, caseSensitivity);
               break;
        }
        
        if (!match)
        {
           pattern++;
           if (IsInsideGlyph (pattern[0]))
               pattern = MoveToEndOfGlyph (pattern);
        }
    } 

    // Consider an unmatched bracket a failure
    // (i.e. when detecting a '\0' instead of a ']'.)
    if (*pattern == '\0')
        return false;
    
    return (match ^ inverse) != 0;
}

//=============================================================================================================//

bool RegExString::StringMatchesPattern (const char *string, const char *pattern,
    bool caseSensitivity) const
{
    // One could do this dynamically, counting the number of *'s,
    // but then you have to free them at every exit of this
    // function, which is awkward and ugly.
    const int32 kWildCardMaximum = 100;
    const char *pStorage[kWildCardMaximum];
    const char *sStorage[kWildCardMaximum];

    int32 patternLevel = 0;
    
    if (string == NULL || pattern == NULL)
        return false;
    
    while (*pattern != '\0')
    {
        switch (*pattern)
        {
           case '?':
           {
               pattern++;
               string++;
               if (IsInsideGlyph (string[0]))
                  string = MoveToEndOfGlyph (string);
               break;
           }
               
           case '*':
           {
               // Collapse any ** and *? constructions:
               while (*pattern == '*' || *pattern == '?')
               {
                  pattern++;
                  if (*pattern == '?' && string != '\0')
                  {
                      string++;
                      if (IsInsideGlyph (string[0]))
                         string = MoveToEndOfGlyph (string);
                  }
               }
               
               if (*pattern == '\0')                  // An ending * matches all strings.
                  return true;
               
               bool match = false;
               const char *pBefore = pattern - 1;

               if (*pattern == '[')
               {
                  pattern++;

                  while (!match && *string != '\0')
                      match = MatchesBracketExpression (string++, pattern, caseSensitivity);
           
                  // Skip the rest of the bracket:
                  while (*pattern != ']' && *pattern != '\0')
                      pattern++;
    
                  // Failure if no closing bracket;
                  if (*pattern == '\0')
                      return false;
                  
               }
               else
               {
                  // No bracket, just one character:
                  while (!match && *string != '\0')
                  {
                      if (IsGlyph (string[0]))
                         match = UTF8CharsAreEqual(string++, pattern);
                      else
                         match = CharsAreEqual(*string++, *pattern, caseSensitivity);
                  }
               }
    
               if (!match)
                      return false;
               else
               {
                  pStorage[patternLevel] = pBefore;
                  if (IsInsideGlyph (string[0]))
                      string = MoveToEndOfGlyph (string);
                  sStorage[patternLevel++] = string;
                  if (patternLevel > kWildCardMaximum)
                      return false;
                  pattern++;
                  if (IsInsideGlyph (pattern[0]))
                      pattern = MoveToEndOfGlyph (pattern);
               }
           }
           break;           
    
           case '[':
               pattern++;
               
               if (!MatchesBracketExpression (string, pattern, caseSensitivity))
                  if (patternLevel > 0)
                  {
                      pattern = pStorage[--patternLevel];
                      string = sStorage[patternLevel];
                  }
                  else
                      return false;
               else
               {
                  // Skip the rest of the bracket:
                  while (*pattern != ']' && *pattern != '\0')
                      pattern++;
    
                  // Failure if no closing bracket;
                  if (*pattern == '\0')
                      return false;
                  
                  string++;
                  if (IsInsideGlyph (string[0]))
                      string = MoveToEndOfGlyph (string);
                  pattern++;
               }
               break;
               
           default:
           {
               bool equal = false;
               if (IsGlyph (string[0]))
                  equal = UTF8CharsAreEqual (string, pattern);
               else
                  equal = CharsAreEqual (*string, *pattern, caseSensitivity);
               
               if (equal)
               {
                  pattern++;
                  if (IsInsideGlyph (pattern[0]))
                      pattern = MoveToEndOfGlyph (pattern);
                  string++;
                  if (IsInsideGlyph (string[0]))
                      string = MoveToEndOfGlyph (string);
               }
               else if (patternLevel > 0)
               {
                  pattern = pStorage[--patternLevel];
                  string = sStorage[patternLevel];
               }
               else 
                  return false;
           }
           break;        
        }
    
        if (*pattern == '\0' && *string != '\0' && patternLevel > 0)
        {
           pattern = pStorage[--patternLevel];
           string = sStorage[patternLevel];
        }
    }
    
    return *string == '\0' && *pattern == '\0';
}

//=============================================================================================================//

bool RegExString::UTF8CharsAreEqual (const char *string1, const char *string2) const
{
    const char *s1 = string1;
    const char *s2 = string2;
    
    if (IsStartOfGlyph (*s1) && *s1 == *s2)
    {
        s1++;
        s2++;
        
        while (IsInsideGlyph (*s1) && *s1 == *s2)
        {
           s1++;
           s2++;
        }
        
        return !IsInsideGlyph(*s1) && !IsInsideGlyph(*s2) && *(s1 - 1) == *(s2 - 1);
    }
    else
        return false;
}

//=============================================================================================================//

const char *RegExString::MoveToEndOfGlyph (const char *string) const
{
    const char *ptr = string;
    while (IsInsideGlyph (*ptr))
        ptr++;
        
    return ptr;
}

//=============================================================================================================//

bool RegExString::IsGlyph (char ch) const
{
    return (ch & 0x80) == 0x80;
}

//=============================================================================================================//

bool RegExString::IsInsideGlyph(char ch) const
{
    return (ch & 0xC0) == 0x80;
}

//=============================================================================================================//

bool RegExString::IsStartOfGlyph (char ch) const
{
    return (ch & 0xC0) == 0xC0;
}

//=============================================================================================================//
