// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// All rights reserved.

#include "AppUtils.h"
#include "Shared.h"

#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


BString StringFromBytes(uint64 val)
{
    char buf[64];
    if (val < kKiBSize)
        sprintf(buf, "%" B_PRIu64 " bytes", val);
    else if (val < kMiBSize)
        sprintf(buf, "%.2f KiB", (double)val / kKiBSize);
    else if (val < kGiBSize)
        sprintf(buf, "%.2f MiB", (double)val / kMiBSize);
    else if (val < kTiBSize)
        sprintf(buf, "%.2f GiB", (double)val / kGiBSize);
    else if (val < kPiBSize)
        sprintf(buf, "%.2f TiB", (double)val / kTiBSize);
    else if (val < kEiBSize)
        sprintf(buf, "%.2f PiB", (double)val / kPiBSize);
    else
        sprintf(buf, "%.2f EiB", (double)val / kEiBSize);

    BString str(buf);
    return str;
}


inline uint64 DigitalUnitToBytes(char *str)
{
    assert(str);
    if (!strcasecmp(str, "bytes") || !strcasecmp(str, "B"))
        return 1;
    if (!strcasecmp(str, "KB") || !strcasecmp(str, "KiB"))
        return kKiBSize;
    if (!strcasecmp(str, "MB") || !strcasecmp(str, "MiB"))
        return kMiBSize;
    if (!strcasecmp(str, "GB") || !strcasecmp(str, "GiB"))
        return kGiBSize;
    if (!strcasecmp(str, "TB") || !strcasecmp(str, "TiB"))
        return kTiBSize;
    if (!strcasecmp(str, "PB") || !strcasecmp(str, "PiB"))
        return kPiBSize;
    if (!strcasecmp(str, "EB") || !strcasecmp(str, "EiB"))
        return kEiBSize;
    return 0;
}


BString StringFromDigitalSize(char *size, char *unit)
{
    char buf[128];
    memset(&buf[0], 0, sizeof(buf));
    assert(size);
    assert(unit);

    // Sometimes archivers comma-separate floating point numbers.
    // But "atof" (below) doesn't understand ",". Therefore we need to feed
    // sanitized floating point numbers (just digits and decimal) to "atof".
    size_t i = 0;
    while (*size && i < sizeof(buf) - 1)
    {
        if (isdigit(*size) || *size == '.')
            buf[i++] = *size;
        *size++;
    }

    uint64 const bytes = (uint64)(atof(buf) * (double)DigitalUnitToBytes(unit));
    BString bytesStr;
    bytesStr << bytes;
    return bytesStr;
}


int8 MonthStrToNum(const char* month)
{
    if (strcasecmp(month, "Jan") == 0) return 1;
    if (strcasecmp(month, "Feb") == 0) return 2;
    if (strcasecmp(month, "Mar") == 0) return 3;
    if (strcasecmp(month, "Apr") == 0) return 4;
    if (strcasecmp(month, "May") == 0) return 5;
    if (strcasecmp(month, "Jun") == 0) return 6;
    if (strcasecmp(month, "Jul") == 0) return 7;
    if (strcasecmp(month, "Aug") == 0) return 8;
    if (strcasecmp(month, "Sep") == 0) return 9;
    if (strcasecmp(month, "Oct") == 0) return 10;
    if (strcasecmp(month, "Nov") == 0) return 11;
    if (strcasecmp(month, "Dec") == 0) return 12;

    return 1;
}


int32 LastOccurrence(const char* str, char whatChar)
{
    // Move "str" to the last occurrence of "whatChar" also count and return number of occurrences
    int32 count = 0L;
    while (*str)
    {
        if (*str == whatChar)
            count++;

        str++;
    }

    return count;
}


int32 CountCharsInFront(char* str, char whatChar)
{
    // Simply count "whatChar"s in the beginning of "str" without modifying "str" pointer
    int32 count = 0;
    while (str[count] == ' ')
        count++;

    return count;
}


bool StrEndsWith(const char* str, const char* end)
{
    assert(str);
    assert(end);
    // Check if the given "str" ends with "end"
    size_t const endLen = strlen(end);
    size_t const strLen = strlen(str);
    if (endLen <= strLen)
        str += strLen - endLen;
    if (!strcmp(str, end))
        return true;
    return false;
}


const char* FinalPathComponent(const char* path)
{
    // Return the final path component be it a file or folder
    // Archivers like zip etc will use this function to spit out either the final dir name
    // or the final file name - LeafFromPath will return ONLY final filename from a path
    // and will return "" if the path is a folder, whereas this function will return
    // the final folder name even if the path is a folder and NOT ""
    // We do NOT change LeafFromPath() as Archiver.cpp uses LeafFromPath in its critical
    // FillLists() function
    // Only problem is this function will return directory names with a trailing slash
    // We don't want to waste time by removing the trailing slash, if it was a leading
    // slash we can simply increment the pointer, but removing a trailing slash involves
    // making copy of the path that is being returned, then manipulating it, we avoid that
    // for sake of speed.
    int32 len = strlen(path);

    if (path[len - 1] == '/')
        len--;

    while (len > 0)
        if (path[--len] == '/')
            break;

    const char* leafStr = path;
    leafStr += len > 0 ? ++len : len;

    return leafStr;
}


const char* LeafFromPath(const char* path)
{
    // Return the filename (pointer to where filename starts rather) from a full path string
    int32 len = strlen(path);

    while (len > 0)
        if (path[--len] == '/')
            break;

    const char* leafStr = path;
    leafStr += len > 0 ? ++len : len;        // Workaround for no slashes ie root directory

    return leafStr;
}


char* ParentPath(const char* pathStr, bool truncateSlash)
{
    // Returns the parent path given a path,
    // e.g: "be/book" is passed, "be" will be returned
    //      if, "dir" is passed then "" will be returned and NOT NULL!
    int32 parentLen = strlen(pathStr) - strlen(FinalPathComponent(pathStr));
    char* parent = NULL;
    if (parentLen > 0)
    {
        parent = (char*)malloc(parentLen + 1);
        if (pathStr[parentLen - 1] == '/' && truncateSlash == true)
            parentLen--;
        strncpy(parent, pathStr, parentLen);
    }
    else
        parent = (char*)malloc(1);

    parent[parentLen] = 0;
    return parent;
}


char* Extension(const char* fileName, int extLen)
{
    // Return the extension in lowercase, without '.' in front -- freeing left to the caller
    int32 len = strlen(fileName);
    int32 lenCopy = len;
    char* m = new char[extLen + 1];
    bool dotEncountered = false;
    while (len >= lenCopy - extLen - 1)
        if (fileName[--len] == '.')
        {
            dotEncountered = true;
            break;
        }

    if (len >= 0 && dotEncountered == true)
        len++;
    else
        return NULL;

    int32 i = 0L;
    for (; i < extLen; i++, len++)
        m[i] = tolower(fileName[len]);

    m[i] = 0;

    return m;
}


BString SupressWildcards(const char* str)
{
    BString s = str;
    s.ReplaceAll("[", "\\[");
    s.ReplaceAll("*", "\\*");
    return s;
}


BString SupressWildcardSet(const char* str)
{
    BString s = str;
    s.ReplaceAll("[", "\\[");
    return s;
}
