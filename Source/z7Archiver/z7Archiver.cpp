// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2005 Ramshankar (aka Teknomancer).
// Copyright (c) 2005 Marcin Konicki (aka ahwayakchih).
// Copyright (c) 2011 Chris Roberts.
// All rights reserved.

#include "z7Archiver.h"
#include "ArchiveEntry.h"
#include "AppUtils.h"

#include <NodeInfo.h>
#include <Messenger.h>
#include <MenuItem.h>

#ifdef HAIKU_ENABLE_I18N
#include <Catalog.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "7zipArchiver"
#else
#define B_TRANSLATE(x) x
#endif


// keep track of our custom options/menuitems
static const char
    //*kArchiveAttrs    = "Add attributes", // unused?
    *kSolidBlocks     = "Use solid blocks",
    *kMultiThread     = "Use multi-threading (for multi-core CPUs)",
    *kOverwriteFiles  = "Always overwrite (default)",
    *kNoOverwrite     = "Never overwrite existing files",
    *kRenameExisting  = "Rename existing files",
    *kRenameExtracted = "Rename extracted files";


Archiver* load_archiver(const char* addonImagePath)
{
    return new z7Archiver(addonImagePath);
}


z7Archiver::z7Archiver(const char* addonImagePath)
    : Archiver(addonImagePath)
{
    // Detect 7z binary
    if (GetBinaryPath(m_7zPath, "7za") == true)
        m_error = BZR_DONE;
    else
    {
        m_error = BZR_BINARY_MISSING;
        return;
    }
}


status_t z7Archiver::ReadOpen(FILE* fp)
{
    uint16 len = B_PATH_NAME_LENGTH + 500;
    char lineString[len],
         attrStr[25], sizeStr[25], packedStr[20], dayStr[5],
         monthStr[5], yearStr[8], hourStr[5], minuteStr[5], secondStr[5], dateStr[90],
         pathStr[B_PATH_NAME_LENGTH + 1];
    uint32 lineCount = 0;

    while (fgets(lineString, len, fp) && !feof(fp))
    {
        // skip first 8 lines
        if (lineCount++ < 17)
            continue;

        // list ends with "----" line, and last line contains sums, so break on first line with "-" as first char.
        if (lineString[0] == '-')
            break;

        lineString[strlen(lineString) - 1] = '\0';
        packedStr[0] = 0;

        sscanf(lineString,
               "%[0-9]-%[0-9]-%[0-9] %[0-9]:%[0-9]:%[0-9] %[^ ] %[0-9] %[0-9]", yearStr, monthStr, dayStr,
               hourStr, minuteStr, secondStr, attrStr, sizeStr, packedStr);

        // packed size is not there if file is in solid block with others, only first from block has packed
        // size which shows size for whole block.
        if (!strcmp(packedStr, ""))
            strcpy(packedStr, "0");

        // file name always starts from 53rd char
        strncpy(pathStr, lineString+53, B_PATH_NAME_LENGTH);

        struct tm timeStruct; time_t timeValue;
        MakeTime(&timeStruct, &timeValue, dayStr, monthStr, yearStr, hourStr, minuteStr, secondStr);

        // Check if it's directory
        if (strstr(attrStr, "D") != NULL)
        {
            // Beezer's window will not show empty directory if it doesn't have "/" at the and of name :(
            strcpy(pathStr + strlen(pathStr), "/");

            m_entriesList.AddItem(new ArchiveEntry(true, pathStr, sizeStr, packedStr, timeValue, "", ""));
        }
        else
            m_entriesList.AddItem(new ArchiveEntry(false, pathStr, sizeStr, packedStr, timeValue, "", ""));
    }

    return BZR_DONE;
}


status_t z7Archiver::Open(entry_ref* ref, BMessage* fileList)
{
    m_archiveRef = *ref;
    m_archivePath.SetTo(ref);

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_7zPath << "l" << m_archivePath.Path();

    FILE* out;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;        // Handle 7zip unloadable error here

    status_t exitCode;
    resume_thread(tid);

    close(errdes[1]);
    close(outdes[1]);

    out = fdopen(outdes[0], "r");
    exitCode = ReadOpen(out);
    fclose(out);

    close(outdes[0]);
    close(errdes[0]);

    return exitCode;
}


status_t z7Archiver::Extract(entry_ref* refToDir, BMessage* message, BMessenger* progress,
                             volatile bool* cancel)
{
    BEntry dirEntry;
    entry_ref dirRef;

    dirEntry.SetTo(refToDir);
    status_t exitCode = BZR_DONE;
    if (progress)        // Perform output directory checking only when a messenger is passed
    {
        if (dirEntry.Exists() == false || dirEntry.IsDirectory() == false)
            return BZR_EXTRACT_DIR_INIT_ERROR;
    }

    BPath dirPath(refToDir);
    BEntry archiveEntry(&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
        return BZR_ARCHIVE_PATH_INIT_ERROR;

    int32 count = 0L;
    if (message)
    {
        uint32 type;
        message->GetInfo(kPath, &type, &count);
        if (type != B_STRING_TYPE)
            return BZR_UNKNOWN;
    }

    // Setup argv, fill with selection names if needed
    m_pipeMgr.FlushArgs();
    BString combo("-o");
    combo << dirPath.Path();
    m_pipeMgr << m_7zPath << "x" << combo.String();

    if (progress)    // Only enable extract options when user is NOT viewing
    {
        if (m_settingsMenu->FindItem(B_TRANSLATE(kNoOverwrite))->IsMarked())
            m_pipeMgr << "-aos";
        else if (m_settingsMenu->FindItem(B_TRANSLATE(kRenameExisting))->IsMarked())
            m_pipeMgr << "-aot";
        else if (m_settingsMenu->FindItem(B_TRANSLATE(kRenameExtracted))->IsMarked())
            m_pipeMgr << "-aou";
        else
            m_pipeMgr << "-aoa";
    }
    else
        m_pipeMgr << "-aoa";

    combo = Password();
    if (combo != "")
        combo.Prepend("-p");
    else
        combo.Prepend("-p-");
    m_pipeMgr << combo.String();
    m_pipeMgr << m_archivePath.Path();

    int32 i = 0L;
    for (; i < count; i ++)
    {
        const char* pathString = NULL;
        if (message->FindString(kPath, i, &pathString) == B_OK)
            m_pipeMgr << SupressWildcards(pathString);
    }

    FILE* out;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;           // Handle 7zip unloadable error here

    if (progress)
        resume_thread(tid);
    else
    {
        status_t exitCode;
        wait_for_thread(tid, &exitCode);
    }

    close(errdes[1]);
    close(outdes[1]);

    if (progress)
    {
        out = fdopen(outdes[0], "r");
        exitCode = ReadExtract(out, progress, cancel);
        fclose(out);
    }

    close(outdes[0]);
    close(errdes[0]);

    // Send signal to quit archiver only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    m_pipeMgr.FlushArgs();
    return exitCode;
}


status_t z7Archiver::ReadExtract(FILE* fp, BMessenger* progress, volatile bool* cancel)
{
    status_t exitCode = BZR_DONE;

    // Reads output of 7zip while extracting files and updates progress window (thru messenger)
    char lineString[928];
    BString buf;

    // Prepare message to update the progress bar
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (fgets(lineString, 927, fp))
    {
        if (cancel && *cancel == true)
            return BZR_CANCEL_ARCHIVER;

        lineString[strlen(lineString) - 1] = '\0';
        buf = lineString;
        int32 found = buf.FindFirst("Extracting");

        if (found == 0)
        {
            updateMessage.RemoveName("error");
            buf.ReplaceFirst("Extracting  ", "");
            if ((found = buf.FindLast("     ")) > 0)
            {
                updateMessage.AddString("error", buf.String()+found+5);
                buf.Truncate(found);
                exitCode = BZR_ERRSTREAM_FOUND;
            }

            updateMessage.RemoveName("text");
            updateMessage.AddString("text", LeafFromPath(buf.String()));
            progress->SendMessage(&updateMessage, &reply);
        }
        else if ((found = buf.FindFirst("No files to process")) == 0)
        {
            updateMessage.RemoveName("error");
            updateMessage.AddString("error", "Files not found");
            updateMessage.RemoveName("text");
            progress->SendMessage(&updateMessage, &reply);
            exitCode = BZR_ERROR;
        }
    }

    return exitCode;
}


status_t z7Archiver::Test(char*& outputStr, BMessenger* progress, volatile bool* cancel)
{
    // Setup the archive testing process
    BEntry archiveEntry(&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
    {
        outputStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_7zPath << "t";

    // 0.07: Added password input while testing, also
    //        Added "-bd" flag to disable percentage display in output
    BString combo = Password();
    if (combo != "")
        combo.Prepend("-p");
    else
        combo.Prepend("-p-");

    m_pipeMgr << "-bd" << combo.String() << m_archivePath.Path();

    FILE* out;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
    {
        outputStr = NULL;        // Handle 7zip unloadable error here
        return B_ERROR;
    }

    resume_thread(tid);

    close(outdes[1]);
    close(errdes[1]);

    // 7za outputs everything to stderr, so 7zip worker redirects everything to stdout
    out = fdopen(outdes[0], "r");
    status_t exitCode = ReadTest(out, outputStr, progress, cancel);
    fclose(out);

    close(errdes[0]);
    close(outdes[0]);

    // Send signal to quit thread only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    return exitCode;
}


status_t z7Archiver::ReadTest(FILE* fp, char*& outputStr, BMessenger* progress, volatile bool* cancel)
{
    // Simply read the entire output of the test process and dump it to the error window (though it need not
    // be an error, it will simply report the output of 7za t
    status_t exitCode = BZR_DONE;
    BString fullOutputStr;

    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    bool startedActualTest = false;
    bool errFlag = false;
    char lineString[999];
    uint64 lineCount = 0L;

    while (fgets(lineString, 998, fp))
    {
        if (cancel && *cancel == true)
        {
            exitCode = BZR_CANCEL_ARCHIVER;
            break;
        }

        lineString[strlen(lineString) - 1] = '\0';
        fullOutputStr << lineString << "\n";
        lineCount++;

        // Skip blank lines
        if (strlen(lineString) > 1)
        {
            char* testingStr = lineString;
            testingStr += CountCharsInFront(testingStr, ' ');

            if (strncmp(testingStr, "Testing", 7) == 0)
            {
                BString pathStr = testingStr;
                pathStr.Remove(0, 12);         // Removes "Testing     "

                // Trim right side spaces as this is only a test operation
                while (pathStr[pathStr.Length() - 1] == ' ')     // Trim right hand side spaces
                    pathStr.RemoveLast(" ");

                updateMessage.RemoveName("text");
                updateMessage.AddString("text", FinalPathComponent(pathStr.String()));
                progress->SendMessage(&updateMessage, &reply);
            }
            else if (strncmp(testingStr, "Processing archive:", 19) == 0)         // test process started
                startedActualTest = true;
            else if (strncmp(testingStr, "Everything is Ok", 16) == 0 && errFlag == false)
            {
                // Important we check this before error, error should be the last to be checked
                exitCode = BZR_DONE;
                // Set startedActualTest to false after we've found this string
                // Newer versions of 7za have output after this
                startedActualTest = false;
            }
            else if (startedActualTest == true)
            {
                // Check for empty archive warning, in which case it isn't an error, so skip setting errFlag
                if (strncmp(testingStr, "No files to process", 19) != 0)
                {
                    // most possibly an error as test has started and it isn't one of the above lines
                    errFlag = true;
                    exitCode = BZR_ERRSTREAM_FOUND;
                }
            }
        }
    }

    outputStr = new char[fullOutputStr.Length() + 1];
    strcpy(outputStr, fullOutputStr.String());

    return exitCode;
}


bool z7Archiver::SupportsFolderEntity() const
{
    // p7zip will delete whole folder with it's content given folder name
    return true;
}


bool z7Archiver::CanReplaceFiles() const
{
    // TODO: if p7zip can't replace files in solid blocks, maybe we should return false here if we notice
    //    that archive contains any blocks?
    // In any case, if we return false, Beezer will reload the entire archive for every addition
    return true;
}


bool z7Archiver::CanPartiallyOpen() const
{
    // TODO: when p7zip gets fixed output for progress info, and we can be sure about file names (right now it
    //    adds space chars after file name, which makes handling file names with space chars at the end impossible),
    //    make this return true, so there will be no need to list archive each time
    return false;
}


bool z7Archiver::SupportsPassword() const
{
    return true;
}


status_t z7Archiver::Add(bool createMode, const char* relativePath, BMessage* message, BMessage* addedPaths,
                         BMessenger* progress, volatile bool* cancel)
{
    // Don't check if archive exists in createMode, otherwise check
    if (createMode == false)
    {
        BEntry archiveEntry(&m_archiveRef, true);
        if (archiveEntry.Exists() == false)
            return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    m_pipeMgr.FlushArgs();

    BString levelStr;
    levelStr.SetToFormat("-mx%ld", GetCompressionLevel());
    m_pipeMgr << m_7zPath << "a" << levelStr.String();

    if (m_settingsMenu->FindItem(B_TRANSLATE(kMultiThread))->IsMarked() == true)
        m_pipeMgr << "-mmt";

    BString combo = Password();
    if (combo != "")
    {
        combo.Prepend("-p");
        m_pipeMgr << combo.String();
    }

    // 7za can't delete/update files inside solid blocks :(
    // Delete/update works only on whole solid block (so all files in such block have to be targetted).
    // Also files inside solid blocks do not show compressed size.
    // Of course using -msoff will not help with handling solid blocks in archives made by other apps.
    if (m_settingsMenu->FindItem(B_TRANSLATE(kSolidBlocks))->IsMarked() == false)
        m_pipeMgr << "-ms=off";

    // 0.07: Added "-bd" switch to prevent percentage display in output
    m_pipeMgr << "-bd" << m_archivePath.Path();

    int32 count = 0L;
    uint32 type;
    message->GetInfo(kPath, &type, &count);
    if (type != B_STRING_TYPE)
        return BZR_UNKNOWN;

    int32 i = 0L;
    for (; i < count; i ++)
    {
        const char* pathString = NULL;
        if (message->FindString(kPath, i, &pathString) == B_OK)
            m_pipeMgr << pathString;
    }

    // 7z[a] outputs everything to stderr! so 7zip worker redirects everything to stdout
    FILE* out;
    int outdes[2], errdes[2];

    if (relativePath)
        chdir(relativePath);

    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
        return B_ERROR;        // Handle 7zip unloadable error here

    resume_thread(tid);
    close(errdes[1]);
    close(outdes[1]);

    out = fdopen(outdes[0], "r");
    status_t exitCode = ReadAdd(out, addedPaths, progress, cancel);
    fclose(out);

    close(errdes[0]);
    close(outdes[0]);

    // Send signal to quit archiver only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    m_pipeMgr.FlushArgs();
    SetMimeType();
    return exitCode;
}


status_t z7Archiver::ReadAdd(FILE* fp, BMessage* addedPaths, BMessenger* progress, volatile bool* cancel)
{
    // Read output while adding files to archive
    status_t exitCode = BZR_DONE;
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0);

    char lineString[999];
    bool noError = false;

    while (fgets(lineString, 998, fp))
    {
        if (cancel && *cancel == true)
        {
            exitCode = BZR_CANCEL_ARCHIVER;
            break;
        }

        lineString[strlen(lineString) - 1] = '\0';
        if (strncmp(lineString, "Compressing  ", 13) == 0)
        {
            BString filePath = lineString + 13;
            const char* fileName = FinalPathComponent(filePath.String());

            // Don't update progress bar for folders
            if (fileName[strlen(fileName) - 1] != '/' && progress)
            {
                updateMessage.RemoveName("text");
                updateMessage.AddString("text", fileName);
                progress->SendMessage(&updateMessage, &reply);
            }

            addedPaths->AddString(kPath, filePath.String());
        }
        else if (strncmp(lineString, "Everything is Ok", 16) == 0)
            noError = true;
    }

    if (noError == false)
        exitCode = BZR_ERRSTREAM_FOUND;

    return exitCode;

//    int readingPercent = 0;
//    while (1)
//    {
//        BString buf;
//        int c;
//        while (1)
//        {
//           c = fgetc (fp);
//           if (c == EOF || c == '\r' || c == '\n')
//           {
//               readingPercent = 0;
//               break;
//           }
//           else if (c == '')
//           {
//               readingPercent++;
//               if (readingPercent == 1 && currentFileName.Length() < 1)
//                  break;
//           }
//           else if (c == '%' && readingPercent > 0)
//           {
//               readingPercent = 0;
//               break;
//           }
//           else
//               buf << (char)c;
//        }
//
//        if (c == EOF)
//           break;
//
//        if (cancel && *cancel == true)
//        {
//           exitCode = BZR_CANCEL_ARCHIVER;
//           break;
//        }
//
//        if (c == '\r')
//           continue;
//        else if (c == '' && readingPercent == 1 && currentFileName.Length() < 1)
//        {
//           int32 found = buf.FindFirst ("Compressing");
//           if (found == 0)
//           {
//               buf.ReplaceFirst ("Compressing  ", "");
//               // TODO: again problem with spaces... p7zip puts space chars between file name and  staring percents
//               //        since it's just for reading progress output we can leave those spaces for now.
//               //        When p7zip will fix that, we can add better checking here ;)
//               currentFileName = buf;
//
//               // TODO: uncomment this when Beezer will let for partial updates (ie. 25%, 33%...).
//               //updateMessage.RemoveName ("text");
//               //updateMessage.AddString ("text", LeafFromPath (buf.String()));
//               //updateMessage.ReplaceFloat("delta", 0.1);
//               //progress->SendMessage (&updateMessage, &reply);
//           }
//        }
//        else if (c == '%')
//        {
//           int percent = atoi(buf.String());
//           if (percent > 0)
//           {
//               // TODO: uncomment this when Beezer will let for partial updates (ie. 25%, 33%...).
//               //updateMessage.ReplaceFloat("delta", percent / 100);
//               //progress->SendMessage (&updateMessage, &reply);
//           }
//        }
//        else if (c == '\n' && currentFileName.Length() > 0)
//        {
//           // TODO: comment out this when Beezer will let for partial updates (ie. 25%, 33%...)?
//           updateMessage.RemoveName ("text");
//           updateMessage.AddString ("text", LeafFromPath (currentFileName.String()));
//
//           // TODO: uncomment this when Beezer will let for partial updates (ie. 25%, 33%...).
//           //updateMessage.ReplaceFloat("delta", 1.0);
//           progress->SendMessage (&updateMessage, &reply);
//           addedPaths->AddString (kPath, currentFileName.String());
//           currentFileName = "";
//        }
//        else
//        {
//           int32 found = buf.FindFirst("Error:");
//           if (found == 0) {
//               exitCode = BZR_ERRSTREAM_FOUND;
//           }
//        }
//    }

    return exitCode;
}


status_t z7Archiver::Delete(char*& outputStr, BMessage* message, BMessenger* progress,
                            volatile bool* cancel)
{
    // WARNING! 7z currently can't delete/update specific files in solid block, and will return
    //    error when trying to delete such file

    // Setup deleting process
    BEntry archiveEntry(&m_archiveRef, true);
    if (archiveEntry.Exists() == false)
    {
        outputStr = NULL;
        return BZR_ARCHIVE_PATH_INIT_ERROR;
    }

    int32 count = 0L;
    if (message)
    {
        uint32 type;
        message->GetInfo(kPath, &type, &count);
        if (type != B_STRING_TYPE)
            return BZR_UNKNOWN;
    }

    m_pipeMgr.FlushArgs();
    m_pipeMgr << m_7zPath << "d" << m_archivePath.Path();

    int32 i = 0L;
    for (; i < count; i ++)
    {
        const char* pathString = NULL;
        if (message->FindString(kPath, i, &pathString) == B_OK)
            m_pipeMgr << SupressWildcardSet(pathString);
    }

    // 7z outputs everything to stderr!
    FILE* out;
    int outdes[2], errdes[2];
    thread_id tid = m_pipeMgr.Pipe(outdes, errdes);

    if (tid == B_ERROR || tid == B_NO_MEMORY)
    {
        outputStr = NULL;        // Handle 7zip unloadable error here
        return B_ERROR;
    }

    resume_thread(tid);

    close(errdes[1]);
    close(outdes[1]);

    out = fdopen(outdes[0], "r");
    status_t exitCode = ReadDelete(out, outputStr, progress, cancel);
    fclose(out);

    close(outdes[0]);
    close(errdes[0]);

    // Send signal to quit thread only AFTER pipes are closed
    if (exitCode == BZR_CANCEL_ARCHIVER)
        TerminateThread(tid);

    m_pipeMgr.FlushArgs();
    SetMimeType();
    return exitCode;
}


status_t z7Archiver::ReadDelete(FILE* fp, char*& outputStr, BMessenger* progress,
                                volatile bool* cancel)
{
    status_t exitCode = B_ERROR;
    BString fullOutputStr;

    int32 len = B_PATH_NAME_LENGTH + strlen("Delete ") + 2;
    char lineString[len];

    // Prepare message to update the progress bar
    BMessage updateMessage(BZR_UPDATE_PROGRESS), reply('DUMB');
    updateMessage.AddFloat("delta", 1.0f);

    while (fgets(lineString, len - 1, fp))
    {
        if (cancel && *cancel == true)
            return BZR_CANCEL_ARCHIVER;

        lineString[strlen(lineString) - 1] = '\0';
        fullOutputStr << lineString << "\n";

        if (strstr(lineString, "Everything is Ok") == lineString)
            exitCode = BZR_DONE;

        if (strstr(lineString, "Error:") == lineString)
            exitCode = BZR_ERRSTREAM_FOUND;
    }

    outputStr = new char[fullOutputStr.Length() + 1];
    strcpy(outputStr, fullOutputStr.String());
    return exitCode;
}


status_t z7Archiver::Create(BPath* archivePath, const char* relPath, BMessage* fileList, BMessage* addedPaths,
                            BMessenger* progress, volatile bool* cancel)
{
    // true=>normalize path, which means everything otherthan the leaf must exist,
    // meaning we have everything ready and only need to create the leaf (by add)
    m_archivePath.SetTo(archivePath->Path(), NULL, true);

    status_t result = Add(true, relPath, fileList, addedPaths, progress, cancel);

    // Once creating is done, set m_archiveRef to pointed to the existing archive file
    if (result == BZR_DONE)
    {
        BEntry tempEntry(m_archivePath.Path(), true);
        if (tempEntry.Exists())
            tempEntry.GetRef(&m_archiveRef);

        SetMimeType();
    }

    return result;
}


void z7Archiver::BuildDefaultMenu()
{
    BMenu* addMenu;
    BMenu* extractMenu;
    BMenuItem* item;

    m_settingsMenu = new BMenu(m_typeStr);

    // Build the header-level sub-menu
    m_compressionMenu = new BMenu(B_TRANSLATE(kCompressionLevelString));
    m_compressionMenu->SetRadioMode(true);

    BString menuStr("0");
    menuStr << " " << B_TRANSLATE("(none)");
    m_compressionMenu->AddItem(new BMenuItem(menuStr, NULL));
    m_compressionMenu->AddItem(new BMenuItem("1", NULL));
    menuStr = "5";
    menuStr << " " << B_TRANSLATE("(default)");
    BMenuItem* defaultItem = new BMenuItem(menuStr, NULL);
    m_compressionMenu->AddItem(defaultItem);
    m_compressionMenu->AddItem(new BMenuItem("7", NULL));
    menuStr = "9";
    menuStr << " " << B_TRANSLATE("(best)");
    m_compressionMenu->AddItem(new BMenuItem(menuStr, NULL));

    defaultItem->SetMarked(true);

    // Build the "While adding" sub-menu
    addMenu = new BMenu(B_TRANSLATE("While adding"));
    addMenu->SetRadioMode(false);

    item = new BMenuItem(B_TRANSLATE(kSolidBlocks), new BMessage(BZR_MENUITEM_SELECTED));
    item->SetMarked(false);
    addMenu->AddItem(item);

    item = new BMenuItem(B_TRANSLATE(kMultiThread), new BMessage(BZR_MENUITEM_SELECTED));
    item->SetMarked(true);
    addMenu->AddItem(item);

    // Build the "While extracting" sub-menu
    extractMenu = new BMenu(B_TRANSLATE("While extracting"));
    extractMenu->SetRadioMode(true);

    defaultItem = new BMenuItem(B_TRANSLATE(kOverwriteFiles), NULL);
    extractMenu->AddItem(defaultItem);
    extractMenu->AddItem(new BMenuItem(B_TRANSLATE(kNoOverwrite), NULL));
    extractMenu->AddItem(new BMenuItem(B_TRANSLATE(kRenameExisting), NULL));
    extractMenu->AddItem(new BMenuItem(B_TRANSLATE(kRenameExtracted), NULL));

    defaultItem->SetMarked(true);

    // Add sub-menus to settings menu
    m_settingsMenu->AddItem(m_compressionMenu);
    m_settingsMenu->AddItem(addMenu);
    m_settingsMenu->AddItem(extractMenu);
}


void z7Archiver::SetMimeType()
{
    // It seems the 7zip binary resets the mime-type of archives to "application/octet-stream", after
    // operations such as delete, add, create - this function sets it back to the correct type
    BNode node(m_archivePath.Path());
    if (node.InitCheck() == B_OK)
    {
        BNodeInfo nodeInfo(&node);
        nodeInfo.SetType((const char*)m_mimeList.ItemAtFast(0L));
    }
}
