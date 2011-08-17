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

#include <Debug.h>
#include <Bitmap.h>
#include <List.h>
#include <Roster.h>
#include <Window.h>
#include <Directory.h>
#include <Menu.h>
#include <File.h>
#include <FindDirectory.h>
#include <Application.h>

#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "Archiver.h"
#include "AppUtils.h"
#include "HashTable.h"
#include "ListEntry.h"
#include "ArchiveEntry.h"
#include "ListSorter.h"
#include "MsgConstants.h"
#include "AppConstants.h"

//=============================================================================================================//

Archiver::Archiver ()
{
    m_settingsDirectoryPath = NULL;
    m_settingsMenu = NULL;
    m_cachedPath = NULL;
    m_hashTable = NULL;
    m_tempDirPath = NULL;
    m_foldingLevel = 3;
    m_passwordRequired = false;

    m_typeStr = strdup ("");
    m_extensionStr = strdup ("");
}

//=============================================================================================================//

Archiver::~Archiver ()
{
    // No need to delete m_settingsMenu as our MainWindow will do it automatically
    // i.e. it never removes it from its Settings Menu so it will be deleted automatically
    // If the window relinquishes the menu from the Settings menu then we will need to delete
    // it, but this is not the case as one-window-one-addon ONLY is what main app follows

    free ((char*)m_typeStr);
    free ((char*)m_extensionStr);
    free ((char*)m_settingsDirectoryPath);
    if (m_tempDirPath != NULL)
        free ((char*)m_tempDirPath);

    int32 entryCount = m_entriesList.CountItems();
    for (int32 i = 0; i < entryCount; i++)
        delete reinterpret_cast<ArchiveEntry*>(m_entriesList.ItemAtFast (i));

    delete m_hashTable;

    int32 mimeCount = m_mimeList.CountItems();
    for (int32 i = 0; i < mimeCount; i++)
        free (reinterpret_cast<char*>(m_mimeList.ItemAtFast (i)));

    ResetCache();
}

//=============================================================================================================//

inline void Archiver::ResetCache ()
{
    if (m_cachedPath != NULL)
    {
        free ((char*)m_cachedPath);
        m_cachedPath = NULL;
    }
}

//=============================================================================================================//

status_t Archiver::InitCheck () const
{
    return m_error;
}

//=============================================================================================================//

void Archiver::SetFoldingLevel (int8 level)
{
    m_foldingLevel = level;
}

//=============================================================================================================//

void Archiver::SetTempDirectoryPath (const char *path)
{
    m_tempDirPath = strdup (path);
}

//=============================================================================================================//

const char* Archiver::TempDirectoryPath () const
{
    return m_tempDirPath;
}

//=============================================================================================================//

void Archiver::SetArchiveType (const char *type)
{
    free ((char*)m_typeStr);
    m_typeStr = strdup (type);
}

//=============================================================================================================//

const char* Archiver::ArchiveType () const
{
    return m_typeStr;
}

//=============================================================================================================//

void Archiver::SetArchiveExtension (const char *extension)
{
    free ((char*)m_extensionStr);
    m_extensionStr = strdup (extension);
}

//=============================================================================================================//

const char* Archiver::ArchiveExtension () const
{
    return m_extensionStr;
}

//=============================================================================================================//

HashTable* Archiver::Table () const
{
    return m_hashTable;
}

//=============================================================================================================//

BList* Archiver::MimeTypeList () const
{
    return const_cast<BList*>(&m_mimeList);
}

//=============================================================================================================//

BMessage* Archiver::ErrorMessage () const
{
    return const_cast<BMessage*>(&m_errorDetails);
}

//=============================================================================================================//

void Archiver::SetIconList (BList *list)
{
    m_iconList = list;

    // Store named pointer variables - for convienience and quick reference
    m_folderBmp = (BBitmap*)m_iconList->ItemAtFast (0L);
    m_binaryBmp = (BBitmap*)m_iconList->ItemAtFast (1L);
    m_htmlBmp = (BBitmap*)m_iconList->ItemAtFast (2L);
    m_textBmp = (BBitmap*)m_iconList->ItemAtFast (3L);
    m_sourceBmp = (BBitmap*)m_iconList->ItemAtFast (4L);
    m_audioBmp = (BBitmap*)m_iconList->ItemAtFast (5L);
    m_archiveBmp = (BBitmap*)m_iconList->ItemAtFast (6L);
    m_packageBmp = (BBitmap*)m_iconList->ItemAtFast (7L);
    m_pdfBmp = (BBitmap*)m_iconList->ItemAtFast (8L);
    m_imageBmp = (BBitmap*)m_iconList->ItemAtFast (9L);
}

//=============================================================================================================//

void Archiver::GetLists (BList *&fileList, BList *&folderList) const
{
    fileList = (BList*)&m_fileList;
    folderList = (BList*)&m_folderList;
}

//=============================================================================================================//

void Archiver::TerminateThread (thread_id tid) const
{
    thread_info t_info;
    if (get_thread_info (tid, &t_info) == B_OK)        // Verify if thread is still running then terminate
        send_signal ((pid_t)tid, SIGTERM);
}

//=============================================================================================================//

void Archiver::FillLists (BList *files, BList *dirs)
{
    // Reset cache before filling our lists - bug fixed
    ResetCache();

    // Dynamically size hash table for improved performance
    int32 entryCount = m_entriesList.CountItems();
    if (!m_hashTable)
    {
        m_hashTable = new HashTable (HashTable::OptimalSize (entryCount * 3.5L));
        if (!m_hashTable)
           debugger ("couldn't alloc hash table");
    }

    BList fileList, dirList;
    int32 i = 0L;

    // First time opening, CountItems() will be zero, thereby we won't delete and reallocate hash
    // table
    if (CanPartiallyOpen() == false && m_hashTable->CountItems() > 0)
    {
        m_hashTable->DeleteTable();
        m_hashTable = new HashTable (HashTable::OptimalSize (entryCount * 3.5L));
        m_fileList.MakeEmpty();
        m_folderList.MakeEmpty();
    }

    // Create the file items in our list
    for (; i < entryCount; i++)
    {
        ArchiveEntry *entry = reinterpret_cast<ArchiveEntry*>(m_entriesList.ItemAtFast (i));
        if (entry->m_dirStr != NULL)
            AddDirPathToTable (&dirList, entry->m_dirStr);

        // Don't call with folder items to filepath table function
        if (entry->m_isDir == false)
        {
           HashEntry *item = AddFilePathToTable (&fileList, entry->m_pathStr);

           // Get rid of trailing slash
           if (entry->m_dirStr != NULL)
               entry->m_dirStr[strlen(entry->m_dirStr) - 1] = '\0';

           // Check for folders without any files, in which case ArchiveEntry will exist but will have its
           // fileName as "" (not NULL but "")
           BBitmap *icon = BitmapForExtension (entry->m_nameStr);
           ListEntry *listItem;
           listItem = new ListEntry (0UL, false, false, icon, entry->m_nameStr,
                                StringFromBytes (atol (entry->m_sizeStr)).String(),
                                StringFromBytes (atol (entry->m_packedStr)).String(), entry->m_ratioStr,
                                entry->m_dirStr, entry->m_dateStr, entry->m_methodStr, entry->m_crcStr,
                                entry->m_dirStr, entry->m_pathStr, atol (entry->m_sizeStr),
                                atol (entry->m_packedStr), entry->m_timeValue);

           // If file doesn't exist simply set its HashItem to have its listentry
           if (item != NULL)
               item->m_clvItem = listItem;
           else
           {
               // Or else update old listentry with new one
               HashEntry *existingItem = m_hashTable->Find (entry->m_pathStr);
               ListEntry *existingEntry = existingItem->m_clvItem;
               if (existingEntry && existingEntry->IsSuperItem() == false)
                  existingItem->m_clvItem->Update (listItem);

               // Now if existingEntry is NULL that means there exists a folder and file with the same
               // name in the archive, we don't handle such an odd situation

               delete listItem;
           }
        }

        delete entry;
    }

    m_entriesList.MakeEmpty();

    // Create folder items also add to hash table for quick finding & uniqueness
    int32 uniqueDirCount = dirList.CountItems();
    for (i = 0L; i < uniqueDirCount; i++)
    {
        HashEntry *item = reinterpret_cast<HashEntry*>(dirList.ItemAtFast(i));
        const char *dirPath = item->m_pathStr;

        if (item->m_clvItem != NULL)
           continue;

        uint32 level = LastOccurrence (dirPath, '/');

        // Try to determine whether to show the dir item expanded or not as quick as possible
        bool expand = false;
        if (m_foldingLevel == 3)
           expand = true;
        else if (m_foldingLevel == 1 || m_foldingLevel == 2)
           if ((int8)level < m_foldingLevel)
               expand = true;

        // Get parent's path without the slash (true = truncate slash)
        char *parentDirPath = ParentPath (dirPath, true);
        ListEntry *itemEntry = new ListEntry (level, true, expand, m_folderBmp, LeafFromPath (dirPath),
                                NULL, NULL, NULL, NULL, NULL, NULL, NULL, parentDirPath, dirPath, 0, 0, 0);
        item->m_clvItem = itemEntry;
        free ((char*)parentDirPath);
    }

    if (files) files->AddList (&fileList);
    if (dirs) dirs->AddList (&dirList);

    m_fileList.AddList (&fileList);
    m_folderList.AddList (&dirList);
}

//=============================================================================================================//

inline int Archiver::CompareHashEntries (const void *a, const void *b)
{
    HashEntry *x (reinterpret_cast<HashEntry*>(&a));
    HashEntry *y (reinterpret_cast<HashEntry*>(&b));
    return strcasecmp (x->m_pathStr, y->m_pathStr);
}

//=============================================================================================================//

void Archiver::AddDirPathToTable (BList *dirList, const char *path)
{
    // Check with previous path (For consecutive files of same folder - quick check and exit)
    if (m_cachedPath != NULL && strcmp (m_cachedPath, path) == 0)
        return;

    // Make sure "path" is unique
    if (m_hashTable->IsFound (path) == true)
        return;

    // Store the full-path of the dir in the cache, so that files in consecutive directories get
    // a significant speed boost as we won't enter the loop below (the check at the beginning of this
    // function will make sure of that)
    ResetCache();
    m_cachedPath = strdup (path);

    // Break folders from the path, add each folder to the hash table.
    // e.g. bebook/art/deskbar/ will be added as 3 items: bebook, bebook/art, bebook/art/deskbar
    int32 len = strlen (path);
    for (int32 i = 0; i < len; i++)
    {
        if (path[i] != '/')
           continue;

        char *t = new char[i+1];
        strncpy (t, path, i);
        t[i] = '\0';

        bool insertFailed;
        m_hashTable->Insert (t, &insertFailed, false);
        if (insertFailed == false)
           dirList->AddItem ((void*)(m_hashTable->LastAddedEntry()));
        // For efficiency: we made Hashtable to use 't' rather than copy it, so don't delete[] t here.
    }
}

//=============================================================================================================//

HashEntry *Archiver::AddFilePathToTable (BList *fileList, const char *path)
{
    // Try and insert the file's path into the hash table
    if (strlen(path) > 0)
    {
        if (CanReplaceFiles() == true)
        {
           bool insertFailed;
           m_hashTable->Insert ((char*)path, &insertFailed, true);    // copy path into hashtable
           if (insertFailed == false)
           {
               HashEntry *addedItem = m_hashTable->LastAddedEntry();
               fileList->AddItem ((void*)addedItem);

               return addedItem;
           }
        }
        else
        {
           HashEntry *addedItem = m_hashTable->ForceInsert ((char*)path, true);
           fileList->AddItem ((void*)addedItem);
           return addedItem;
        }
    }

    return NULL;
}

//=============================================================================================================//

BBitmap* Archiver::BitmapForExtension (const char *str) const
{
    // It seems this is fine, implementing a hashmap or similar structure won't give a significant speed
    // boost according to NathanW of BeShare
    char *extn = Extension (str, 4);
    BBitmap *icon = m_binaryBmp;
    if (extn)
    {
        if (strcmp (extn, "htm") == 0|| strcmp (extn, "html") == 0)
           icon = m_htmlBmp;
        else if (strcmp (extn, "cpp") == 0 || strcmp (extn, "c") == 0 || strcmp (extn, "h") == 0 ||
               strcmp (extn, "py") == 0)
           icon = m_sourceBmp;
        else if (strcmp (extn, "txt") == 0 || strcmp (extn, "sh") == 0 || strcmp (extn, "doc") == 0)
           icon = m_textBmp;
        else if (strcmp (extn, "bmp") == 0 || strcmp (extn, "gif") == 0 || strcmp (extn, "png") == 0    ||
                  strcmp (extn, "jpg") == 0 || strcmp (extn, "jpeg") == 0 || strcmp (extn, "tga") == 0 ||
                  strcmp (extn, "tiff") == 0)
           icon = m_imageBmp;
        else if (strcmp (extn, "z") == 0 || strcmp (extn, "zip") == 0 || strcmp (extn, "gz") == 0 ||
               strcmp (extn, "gzip") == 0 || strcmp (extn, "tgz") == 0)
           icon = m_archiveBmp;
        else if (strcmp (extn, "wav") == 0 || strcmp (extn, "mp3") == 0 || strcmp (extn, "mp2") == 0    ||
               strcmp (extn, "aiff") == 0 || strcmp (extn, "riff") == 0 || strcmp (extn, "ogg") == 0 ||
               strcmp (extn, "mod") == 0 || strcmp (extn, "mid") == 0 || strcmp (extn, "midi") == 0)
           icon = m_audioBmp;
        else if (strcmp (extn, "pdf") == 0)
           icon = m_pdfBmp;
        else if (strcmp (extn, "pkg") == 0)
           icon = m_packageBmp;

        delete[] extn;
    }

    return icon;
}

//=============================================================================================================//

status_t Archiver::ReadErrStream (FILE *fp, const char *escapeSeq)
{
    // Read entire stream into a BString, check for errors
    BString fullErrorString;
    ReadStream (fp, fullErrorString);

    if ((fullErrorString.Length() > 0L))
    {
        // An option string to escape ("Empty archive warning" lines etc. can be passed here)
        // and in general any line which must not be treated as an error
        if (escapeSeq && fullErrorString.FindFirst (escapeSeq) >= 0L)
           return BZR_DONE;

        m_errorDetails.RemoveName (kErrorString);
        m_errorDetails.AddString (kErrorString, fullErrorString.String());
        return BZR_ERRSTREAM_FOUND;
    }

    return BZR_DONE;
}

//=============================================================================================================//

void Archiver::ReadStream (FILE *fp, BString &str) const
{
    // Read entire stream into a BString
    while (!feof (fp))
    {
        unsigned char c = fgetc (fp);

        // Check for end of pipe, else append
        if (c != 255)
           str << (char)c;
    }
}

//=============================================================================================================//

BList Archiver::HiddenColumns (BList *columns) const
{
    // By default return all columns as available (ie empty hidden list) columns for the archiver
    // Derived classes will over-ride removing unwanted column indices
    // Indices are: 0-name 1-size 2-packed 3-ratio 4-path 5-date 6-method 7-crc
    BList retList;
    return retList;
}

//=============================================================================================================//

bool Archiver::SupportsComment () const
{
    // By default we don't support comments - Derived classes will override as necessary
    // Zip, for instance, will override this function and return true - others like gzip won't
    return false;
}

//=============================================================================================================//

bool Archiver::CanDeleteFiles () const
{
    // Archivers (Rar at the moment) can't delete files accurately, hence will override this and return false
    return true;
}

//=============================================================================================================//

bool Archiver::CanAddEmptyFolders () const
{
    // Arj for example, can't add empty folders, hence will override this and return false
    return true;
}

//=============================================================================================================//

bool Archiver::CanAddFiles () const
{
    // Gzip, bzip2 for example, can't add files, hence they will override this and return false
    return true;
}

//=============================================================================================================//

bool Archiver::CanReplaceFiles () const
{
    // Tar, for example, can't replace existing files while adding new ones with the same name,
    // in which case it will override this and return false
    return true;
}

//=============================================================================================================//

bool Archiver::CanPartiallyOpen () const
{
    // Tar, for example, needs to reload everytime a file is added (appended) which will override this
    // and return false
    return true;
}

//=============================================================================================================//

bool Archiver::NeedsTempDirectory () const
{
    // For archivers like tar.gzip it will set this to true, so that the main app
    // will create and pass the path of the temporary directory which is needed by
    // gzip as it works on a temp dir unlike zip
    return false;
}

//=============================================================================================================//

bool Archiver::SupportsFolderEntity () const
{
    // An ugly thing we need to do for zip-because zip binary will not delete all files under
    // a folder given the folder name, but things like tar will - and tar will not work fully
    // when folder/* is given (eg: files like .doomrc won't be deleted by tar when /* is given)
    // so for things like tar this will be true, zip will override this function and return false
    // so that zip will be given dir/* to delete a dir while tar will be given dir/ to delete the
    // folder, zip don't support folder entities
    return true;
}

//=============================================================================================================//

void Archiver::SetPassword (const char *password)
{
    m_password = password;
}

//=============================================================================================================//

BString Archiver::Password () const
{
    return m_password;
}

//=============================================================================================================//

bool Archiver::SupportsPassword () const
{
    return false;
}

//=============================================================================================================//

bool Archiver::PasswordRequired() const
{
    // "m_passwordRequired" will be true/false depending on the derived class' Open() function
    // -- rar for example, can find out while "listing" an archive contents whether it requires password
    // or not in which case it will set m_passwordRequired to true
    return m_passwordRequired;
}

//=============================================================================================================//

status_t Archiver::GetComment (char *&commentStr)
{
    commentStr = NULL;
    return BZR_DONE;
}

//=============================================================================================================//

status_t Archiver::SetComment (char *commentStr, const char *tempDirPath)
{
    return BZR_DONE;
}

//=============================================================================================================//

bool Archiver::IsBinaryFound (char *filePath, const char *fileName) const
{
    // Check if the given fileName exists in the given dir, if so copy the full path of fileName to filePath
    // Path priority  <appdir>/workers -> B_SYSTEM_BIN_DIRECTORY -> B_COMMON_BIN_DIRECTORY
    BPath binPath;
    app_info appInfo;
    be_app->GetAppInfo (&appInfo);
    BEntry appEntry (&appInfo.ref);
    appEntry.GetParent (&appEntry);
    binPath.SetTo(&appEntry);
    binPath.Append(K_BIN_DIR_NAME, true);
    binPath.Append(fileName, true);
    BEntry workersEntry(binPath.Path(), true);
    if (workersEntry.Exists())
    {
        strcpy(filePath, binPath.Path());
        return true;
    }

    if (find_directory(B_SYSTEM_BIN_DIRECTORY, &binPath) == B_OK)
    {
        binPath.Append(fileName);
        BEntry entry(binPath.Path(), true);
        if (entry.Exists())
        {
            strcpy(filePath, binPath.Path());
            return true;
        }
    }

    if (find_directory(B_COMMON_BIN_DIRECTORY, &binPath) == B_OK)
    {
        binPath.Append(fileName);
        BEntry entry(binPath.Path(), true);
        if (entry.Exists())
        {
            strcpy(filePath, binPath.Path());
            return true;
        }
    }

    // TODO full search of $PATH

    filePath = '\0';
    return false;
}

//=============================================================================================================//

void Archiver::SetSettingsDirectoryPath (const char *path)
{
    if (m_settingsDirectoryPath != NULL)
        free ((char*)m_settingsDirectoryPath);

    m_settingsDirectoryPath = strdup (path);
}

//=============================================================================================================//

BMenu* Archiver::SettingsMenu () const
{
    return m_settingsMenu;
}

//=============================================================================================================//

void Archiver::SetSettingsMenu (BMenu *menu)
{
    if (m_settingsMenu)
        delete m_settingsMenu;
    m_settingsMenu = menu;
}

//=============================================================================================================//

void Archiver::BuildDefaultMenu ()
{
    // Empty implementation (i.e. m_settingsMenu will be NULL if control comes here)
}

//=============================================================================================================//

void Archiver::SaveSettingsMenu ()
{
    if (!m_settingsMenu)
        return;

    // Make sure our settings folder exists!
    // Now when the app launches if there is NO folder our m_settingsDirectoryPath
    // would be NULL in which case don't bother creating/saving
    if (m_settingsDirectoryPath == NULL)
        return;

    BDirectory settingsDirectory (m_settingsDirectoryPath);
    //create_directory (m_settingsDirectoryPath, 0777);
    if (settingsDirectory.InitCheck() != B_OK)
        return;

    BMessage settingsMsg;
    BString settingsFilePath;
    BFile settingsFile;

    // Remove the first 3 items which will be "Save as Defaults", "Save to archive" and separator item
    // Then save the rest of the items,
    BMenuItem *item0 = m_settingsMenu->RemoveItem (0L);
    BMenuItem *item1 = m_settingsMenu->RemoveItem (0L);
    BMenuItem *item2 = m_settingsMenu->RemoveItem (0L);

    m_settingsMenu->Archive (&settingsMsg, true);

    // Restore menu to its original form
    m_settingsMenu->AddItem (item0, 0);
    m_settingsMenu->AddItem (item1, 1);
    m_settingsMenu->AddItem (item2, 2);

    BString temp = m_typeStr;
    temp.ToLower();        // Use lowercase filenames :)

    settingsFilePath = m_settingsDirectoryPath;
    settingsFilePath << "/" << temp.String() << "_settings";

    // Setup the file for writing the settings onto
    settingsFile.SetTo (settingsFilePath.String(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);

    // Write message to the file
    settingsMsg.Flatten (&settingsFile);
}

//=============================================================================================================//

void Archiver::LoadSettingsMenu ()
{
    // Load settings file of archiver
    BString settingsFilePath;
    BString temp = m_typeStr;
    temp.ToLower();           // Use lowercase filenames :)

    settingsFilePath = m_settingsDirectoryPath;
    settingsFilePath << "/" << temp.String() << "_settings";

    BFile settingsFile;
    if (settingsFile.SetTo (settingsFilePath.String(), B_READ_ONLY) == B_OK)
    {
        BMessage settingsMsg;
        settingsMsg.Unflatten (&settingsFile);

        m_settingsMenu = new BMenu (&settingsMsg);
        if (m_settingsMenu == NULL)
           BuildDefaultMenu();
    }
    else
        BuildDefaultMenu();
}

//=============================================================================================================//

void Archiver::MakeTime (struct tm *timeStruct, time_t *timeValue, const char *day, const char *month,
                  const char *year, const char *hour, const char *min, const char *sec)
{
    // Zero-initialize time structure
    memset ((void*)timeStruct, 0, sizeof (*timeStruct));

    int16 numYear = atoi (year);
    int8 numMonth = atoi (month) - 1;
    if (numYear > 999)               // Convert long year to short (as tm requires short year)
        numYear = numYear % 100;

    if (numYear < 70)
        numYear += 100;

    // Set the fields we have
    timeStruct->tm_sec = atoi (sec);
    timeStruct->tm_min = atoi(min);
    timeStruct->tm_hour = atoi(hour);
    timeStruct->tm_mon = numMonth;
    timeStruct->tm_year = numYear;
    timeStruct->tm_mday = atoi(day);

    *timeValue = mktime (timeStruct);
}

//=============================================================================================================//

void Archiver::FormatDate (char *dateStr, size_t bufSize, struct tm *timeStruct)
{
    strftime (dateStr, bufSize, "%d/%m/%Y, %I:%M %p", timeStruct);
}

//=============================================================================================================//
