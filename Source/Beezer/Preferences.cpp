// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2002 Ramshankar (aka Teknomancer).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include <Debug.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <Path.h>
#include <String.h>

#include <malloc.h>
#include <string.h>

#include "Preferences.h"

Preferences _prefs_colors,
            _prefs_paths,
            _prefs_extract,
            _prefs_add,
            _prefs_state,
            _prefs_windows,
            _prefs_recent,
            _prefs_lang,
            _prefs_misc,
            _prefs_recent_archives,
            _prefs_recent_extract,
            _prefs_interface,
            _prefs_recent_splitfiles,
            _prefs_recent_splitdirs;



Preferences::Preferences(const char* dir, const char* file)
{
    Init(dir, file);
}



Preferences::Preferences()
    : m_prefsPathStr(NULL)
{
}



Preferences::~Preferences()
{
    // Write settings to the file before dying
    WritePrefs();
    FreePathString();
}



void Preferences::FreePathString()
{
    if (m_prefsPathStr)
        free((char*)m_prefsPathStr);

    m_prefsPathStr = NULL;
}



void Preferences::Init(const char* dir, const char* file)
{
    BString locationStr = dir;
    locationStr << '/' << file;
    locationStr.ReplaceAll("//", "/");         // BugFix: Incase of invalid paths
    SetLocation(locationStr.String());
    ReadPrefs();
}



void Preferences::SetLocation(const char* path)
{
    FreePathString();
    m_prefsPathStr = strdup(path);
}



const char* Preferences::Location() const
{
    return m_prefsPathStr;
}



void Preferences::WritePrefs()
{
    // Serialize contents of "prefs" to a file on disk
    if (m_prefsPathStr && IsEmpty() == false)
    {
        // Create settings directory if need be
        BPath filePath(m_prefsPathStr);
        BPath settingsDirPath;
        filePath.GetParent(&settingsDirPath);

        BEntry settingsDirEntry(settingsDirPath.Path(), true);
        if (settingsDirEntry.Exists() == false)
            create_directory(settingsDirPath.Path(), 0777);

        BFile file(m_prefsPathStr, B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY);
        if (file.InitCheck() == B_OK)
            Flatten(&file);
    }
}



void Preferences::ReadPrefs()
{
    // Load "prefs" from the preferences file
    if (m_prefsPathStr)
    {
        BFile file(m_prefsPathStr, B_READ_ONLY);
        Unflatten(&file);
    }
}



status_t Preferences::SetBool(const char* name, bool b)
{
    if (HasBool(name) == true)
        return ReplaceBool(name, 0, b);

    return AddBool(name, b);
}



status_t Preferences::SetInt8(const char* name, int8 i)
{
    if (HasInt8(name) == true)
        return ReplaceInt8(name, 0, i);

    return AddInt8(name, i);
}



status_t Preferences::SetInt16(const char* name, int16 i)
{
    if (HasInt16(name) == true)
        return ReplaceInt16(name, 0, i);

    return AddInt16(name, i);
}



status_t Preferences::SetInt32(const char* name, int32 i)
{
    if (HasInt32(name) == true)
        return ReplaceInt32(name, 0, i);

    return AddInt32(name, i);
}



status_t Preferences::SetInt64(const char* name, int64 i)
{
    if (HasInt64(name) == true)
        return ReplaceInt64(name, 0, i);

    return AddInt64(name, i);
}



status_t Preferences::SetFloat(const char* name, float f)
{
    if (HasFloat(name) == true)
        return ReplaceFloat(name, 0, f);

    return AddFloat(name, f);
}



status_t Preferences::SetDouble(const char* name, double f)
{
    if (HasDouble(name) == true)
        return ReplaceDouble(name, 0, f);

    return AddDouble(name, f);
}



status_t Preferences::SetString(const char* name, const char* s)
{
    if (HasString(name) == true)
        return ReplaceString(name, 0, s);

    return AddString(name, s);
}



status_t Preferences::SetPoint(const char* name, BPoint p)
{
    if (HasPoint(name) == true)
        return ReplacePoint(name, 0, p);

    return AddPoint(name, p);
}



status_t Preferences::SetRect(const char* name, BRect r)
{
    if (HasRect(name) == true)
        return ReplaceRect(name, 0, r);

    return AddRect(name, r);
}



status_t Preferences::SetMessage(const char* name, const BMessage* message)
{
    if (HasMessage(name) == true)
        return ReplaceMessage(name, 0, message);

    return AddMessage(name, message);
}



status_t Preferences::SetColor(const char* name, rgb_color& col)
{
    RemoveName(name);
    return AddInt32(name, (((uint32)col.red) << 24) | (((uint32)col.green) << 16)
                    | (((uint32)col.blue) << 8)    | (((uint32)col.alpha) << 0));
}



status_t Preferences::SetFlat(const char* name, const BFlattenable* obj)
{
    if (HasFlat(name, obj) == true)
        return ReplaceFlat(name, 0, (BFlattenable*)obj);

    return AddFlat(name, (BFlattenable*) obj);
}



bool Preferences::FindBoolDef(const char* name, bool defaultValue)
{
    bool v;
    status_t result = FindBool(name, &v);
    if (result == B_OK)
        return v;
    else
        return defaultValue;
}



int8 Preferences::FindInt8Def(const char* name, int8 defaultValue)
{
    int8 v;
    status_t result = FindInt8(name, &v);
    if (result == B_OK)
        return v;
    else
        return defaultValue;
}



int16 Preferences::FindInt16Def(const char* name, int16 defaultValue)
{
    int16 v;
    status_t result = FindInt16(name, &v);
    if (result == B_OK)
        return v;
    else
        return defaultValue;
}



rgb_color Preferences::FindColorDef(const char* name, rgb_color defaultValue)
{
    uint32 val;
    rgb_color retCol;

    if (FindInt32(name, (int32*)&val) == B_NO_ERROR)
    {
        retCol.red   = (val >> 24);
        retCol.green = (val >> 16);
        retCol.blue  = (val >> 8);
        retCol.alpha = (val >> 0);
    }
    else
        retCol = defaultValue;

    return retCol;
}


