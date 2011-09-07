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

#include "AppConstants.h"
#include "LangStrings.h"

static int _language = LANG_ENGLISH;

const char* strR5 (int whichLang, int language);

static const char * _englishStrings [NUM_STRINGS] =
{
    "English",
    "Untitled",
    "Developer Info…",
    "File",
    "New…",
    "Open…",
    "<empty>",
    "Close",
    "Archive Info…",
    "Password…",
    "Delete…",
    "Help…",
    "About",
    "Quit",
    "Edit",
    "Copy row as text",
    "Select All",
    "Folders",
    "Files",
    "Deselect All",
    "Invert Selection",
    "Expand All",
    "Expand Selected",
    "Collapse All",
    "Collapse Selected",
    "Actions",
    "Extract",
    "Extract To…",
    "Extract Selected…",
    "View File…",
    "Open With…",
    "Test…",
    "Add…",
    "Delete",
    "Rename…",
    "Search Archive…",
    "Deep Search…",
    "Saving Comment...",
    "Comment…",
    "Create Folder…",
    "View",
    "While Opening",
    "Show all items folded",
    "Show first level unfolded",
    "Show first 2 levels unfolded",
    "Show all items unfolded",
    "Toolbar",
    "Infobar",
    "Action Log",
    "Save to Archive",
    "Save interace and folding settings to the archive?",
    "Save as Defaults",
    "Save interface and folding settings as defaults?",
    "Don't save",
    "Save",
    "Save %s settings as defaults for %s files?",
    "Save %s settings to the archive?",
    "Columns",
    "Sort By",
    "Windows",
    "Settings",
    "Preferences…",
    "Window…",
    "New",
    "Create a new archive",
    "Preferences",
    "Edit application preferences",
    "Open",
    "Open an existing archive",
    "Recent",
    "Open an recently opened archive",
    "Close",
    "Close current archive",
    "Search",
    "Search for files in the archive",
    "Extract",
    "Extract contents of archive",
    "View",
    "View selected file(s)",
    "Add",
    "Add files to archive",
    "Delete",
    "Delete selected files & folders",
    "of",
    "OK",
    "Extract",
    "Detecting format... ",
    "Failed!",
    "Verifying archiver... ",
    "Partially successful.",
    "Success.",
    "Loading archive... ",
    "Done.",
    "Error!",
    "Viewing: ",
    "Extracting...",
    "Extracting To: ",
    "Extract To",
    "Unexpected error.",
    "Done extracting.",
    "Delete",
    "Move To Trash",
    "Cancel",
    "Delete the selected item(s)?\nThis operation "
        "cannot be reverted.",
    "Delete this archive from disk?\nThis operation "
        "cannot be reverted.",
    "Name",
    "Size",
    "Packed",
    "Ratio",
    "Path",
    "Date",
    "Method",
    "CRC",
    "Sort Order",
    "None",
    "Ascending",
    "Descending",
    "The archiver has discovered errors in the format.\n"
        "Do you want to view a detailed report?",
    "View report",
    "Cancel",
    "Close",
    "Cannot open the archive:\n\t" B_UTF8_OPEN_QUOTE "%s" B_UTF8_CLOSE_QUOTE "\n"
        "The file is not an archive or is an unsupported one.",
    "Archiver binary missing. Cannot continue...",
    "The archiver could not find an optional binary. As a result of this "
        "some features may not be available.\n\nFor further help regarding this "
        "refer the documentation.",
    "Error initializing archive.",
    "Extract selected entries of ",
    "Extract archive ",
    "Preparing to extract...",
    "Please wait...",
    "Cancel",
    "Yes",
    "No",
    "Close",
    "Extraction cancelled.",
    "Couldn't initialize output directory.",
    "Couldn't initialize archive path.",
    "Couldn't initialize archive path.\nThe location of the archive has been changed.\n\nEither "
        "the archive is deleted or moved to another location.\n\n-- Operation failed!",
    "Deleting... ",
    "Preparing to delete...",
    "Done.",
    "Cancelled.",
    "Error(s) found.",
    "The archiver has discovered errors while deleting.\nDo you want "
        "to view a detailed report?",
    "Testing... ",
    "Preparing to test...",
    "Done.",
    "Cancelled.",
    "Error(s) found.",
    "The archiver has discovered one or more errors/warnings.\nDo you want "
        "to view a detailed report?",
    "Test completed successfully without any errors.\nDo you want "
        "to view a detailed report?",
    "Errors & Warnings: ",
    "Test Results: ",
    "About",
    "An error was encountered while trying to load resources for the About window. Try "
        "fixing the error by opening this binary using QuickRes tool.\n\nIf you still "
        "can't fix the bug, try mailing the author for help.",
    "CREDITS",
    "LEGAL MUMBO JUMBO",
    "SPECIAL THANKS TO",
    "[ Programming ]",
    "[ ColumnListView ]",
    "[ SplitPane ]",
    "[ ToolTips ]",
    "[ URLView ]",
    "[ BeShare ]",
    "[ 7zip Add-on ]",
    "[ Disclaimer ]",
    "Version " K_APP_VERSION "\n"
        "$DEBUG_BUILD$\n\n"
        "Compiled On:\n$BUILD_DATE$\n\n*  *  *\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
        "$S_PROGRAMMING$\n" K_APP_AUTHOR "\n" K_APP_AUTHOR_MAIL "\n\n"
        K_APP_AUTHOR_2 "\n" K_APP_AUTHOR_2_MAIL "\n\n*  *  *\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
        "$S_CREDITS$\n\n"
        "$S_COLUMN_LIST_VIEW$\nBrian Tietz\n\n"
        "$S_SPLITPANE$\nYNOP Talton\n\n"
        "$S_BUBBLE_HELP$\nMarco Nelissen\n\n"
        "$S_7ZIP$\nMarcin Konicki\n\n"
        "Thank you for your contributions\n\n*  *  *\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
        "A special round of applause\n"
        "to BeShare/IRC members:\n\n"
        "Zartan\nxyphn\nBiPolar\nTechnix\ntschertel\nDaaT\nEsperantolo\nIcewarp\nAlienSoldier\nSoulbender\ndisreali\n"
        "RepairmanJack\nJack Burton\nslaad\nBegasus\nWilliam Kakes\nJeremy Friesner\n"
        "BeGeistert\nBGA\nhUMUNGUs\nmmu_man\nmahlzeit\nBryan\nPahtz\nfyysik\nSir Mik\nIngenu\nTenzin\nand "
        "others...\n\n"
        "for testing, feedback, suggestions\n"
        "& some timely help as well... :)\n\n*  *  *\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
        "Also thanks to:\n\n"
        "BeIDE\nbdb\nEvaluator\nRL's Icon Collection\nJess Tipton (BeWicked Icons)\n\n"
        "... for various reasons :^)\n\n*  *  *\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
        "$S_LEGAL_MUMBO_JUMBO$\n\n"
        "This program is distributed under\n"
        "the 3-clause BSD license and the\n"
        "the license is attached to each\n"
        "source file of this program\n\n"

        "For third party code, the license's\n"
        "terms and conditions are explicitly\n"
        "stated and the author disclaimed of\n"
        "any and all liabilities\n\n"

        "For the full license read the\n"
        "License section of the documentation\n\n"
        "*  *  *\n\n\n\n\n\n\n\n\n\n\n\n\n\n"

        "$S_DISCLAIMER$\n\n"
        "Because the software is licensed\n"
        "free of charge, there is no warranty\n"
        "for the software. The copyright\n"
        "holders and/or other parties provide\n"
        "the software \"AS IS\" without warranty\n"
        "of any kind, either expressed or\n"
        "implied, including, but not limited to,\n"
        "the implied warranties of merchantability\n"
        "and fitness for a particular purpose.\n"
        "The entire risk as to the quality and\n"
        "performance of the software is with you.\n"
        "Should the software prove defective, you\n"
        "assume the cost of all necessary\n"
        "servicing, repair or correction.\n\n"

        "In no event will the copyright holder,\n"
        "or any other party who may modify and/or\n"
        "redistribute the software as permitted\n"
        "above, be liable to you for damages,\n"
        "including any general, special, incidental\n"
        "or consequential damages arising out of\n"
        "the use or inability to use the software\n"
        "(including but not limited to the loss of\n"
        "data or data being rendered inaccurate or\n"
        "losses sustained by you or third parties\n"
        "or a failure of the software to operate\n"
        "with any other programs), even if such\n"
        "holder or other party has been advised\n"
        "of the possibility of such damages.\n\n\n\n\n\n\n\n\n\n\n\n\n\n"

        "$S_SPECIAL_THANKS$\n\n"
        "Be Inc., for making this OS\n"
        "in the first place\n\n"
        "Haiku Inc. for their efforts with\n"
        "Haiku\n\n"
        "BeBits.com, BeGroovy.com, BeZip.de and\n"
        "other BeOS related sites for their\n"
        "continued enthusiasm and effort!\n\n"
        "BeOS, Haiku programmers, designers, artists for\n"
        "their contributions to the OS' growth\n\n"
        "and a big applause goes to the\n"
        "community\n\n*  *  *\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
        "OK...\n\nYou can close this window now :)\n\n\n\n\n",

    "You are opening %ld files simultenously.\n"
        "Are you sure you wish to proceed?",
    "Continue",
    "The archiver add-on was unable to find its executable file.",
    "Welcome to " K_APP_TITLE,
    "Create a new archive",
    "Open an existing archive",
    "Open a recently opened archive",
    "Edit application preferences",
    "No archivers found. Cannot continue to load " K_APP_TITLE ".",
    "Quit",
    "Open archive",
    "View",
    "Open With…",
    "Extract…",
    "Delete",
    "Copy row as text",
    "Select Folder",
    "Deselect Folder",
    "Clear Log",
    "Copy Log",
    "Save Log…",
    "Archive Information",
    "Compression ratio:",
    "Compressed size:",
    "Original size:",
    "Number of files:",
    "Number of folders:",
    "Total entries:",
    "Type:",
    "archive",
    "Path:",
    "Created On:",
    "Last Modified:",
    "Search Archive",
    "Column:",
    "Which column to search in",
    "The type of matching to\n"
        "do in the search",
    "Starts with",
    "Ends with",
    "Contains",
    "Matches wildcard expression",
    "Matches regular expression",
    "Find:",
    "What you are searching for",
    "Scope",
    "All entries",
    "Searches the entire archive\n"
        "(Automatically unfolds all folded\n"
        "directories before searching)",
    "Visible entries",
    "Searches all visible entries\n"
        "(Contents of folded directories\n"
        "will not be searched)",
    "Selected entries",
    "Searches only selected entries",
    "Options",
    "Add to selection",
    "Adds found items to existing selection (if any)",
    "Deselect unmatched entries",
    "Deselects those entries that\n"
        "don't match the search criteria",
    "Ignore case",
    "Treats uppercase and lowercase\n"
        "letters as the same",
    "Invert",
    "Selects all entries that don't match\n"
        "the search criteria i.e. inverts the\n"
        "result of the search",
    "Persistent window",
    "Prevents closing of this window before searching",
    "Search",
    "Error in regular expression:",
    "Searching for \"%s\"...",
    "Done.",
    "The archive seems to be missing.\nThe current operation has failed.",
    "Entries: ",
    "Bytes: ",
    "Close",
    "Save",
    "Comment",
    "About %s's Author",
    "Author:",
    "Website:",
    "Nickname:",
    "E-Mail:",
    "Close",
    "Age:",
    "Nationality:",
    "Occupation:",
    "Extract Archive",
    "file",
    "folder",
    "files",
    "folders",
    "to extract",
    "Add",
    "Add To:",
    "The archiver does not support this operation.",
    "The archive is read-only, or is in a read-only partition.",
    "Operation unavailable.",
    "Preparing to add...",
    "Adding...",
    "Gathering information",
    "Copying...",
    "Cancelled",
    "Done.",
    "Done.",
    "Adding %s of data could take some time.\n"
        "Are you sure you wish to proceed?",
    "Continue",
    "Create archive",
    "Create",
    "Type: ",
    "A critical operation has been cancelled. Due to the nature of this "
        "operation the archive is at an indeterminate state.\n\nCannot continue, closing window...",
    "New folder name: ",
    "Create Directory In:\n   %s\n",
    ":Root:",
    "Cannot add " B_UTF8_OPEN_QUOTE "%s" B_UTF8_CLOSE_QUOTE " as a folder with the same name already exists.",
    "Cannot add " B_UTF8_OPEN_QUOTE "%s" B_UTF8_CLOSE_QUOTE " as a file with the same name already exists.",
    "A directory named " B_UTF8_OPEN_QUOTE "%s" B_UTF8_CLOSE_QUOTE " already exists. Replace it with the "
        "one being added?",
    "You are trying to replace the file:\n\t%s1\n\t(%z1, %d1)\nwith:\n\t%s2\n\t(%z2, %d2)\n\nWould you "
        "like to replace it with the one you are adding?",
    "You are trying to add the file:\n\t%s1\n\t(%z1, %d1)\nwhile this:\n\t%s2\n\t(%z2, %d2)\nalready exists.\n\n"
        "Are you sure you wish to append another file with the same name?",
    "Proceed to add the dropped files to the archive?",
    "Replace",
    "Append",
    "Skip",
    "Creating folder...",
    "A folder with this name already exists. Cannot create folder...",
    "Creation of destination folder failed. This could be because there exists a file with the same name.",
    "Password protected file(s) found.",
    "Enter the password to be used while extracting and adding files\n",
    "Password: ",
    "Error (password protected file)",
    "Set password…",
    "A password protection error has occured.\nPlease set the correct password and retry...",
    "An error occured",
    "Preparing to open...",
    "The file type: " B_UTF8_OPEN_QUOTE "%t" B_UTF8_CLOSE_QUOTE "\nis currently associated with %o.\n\n"
        "Make %s the preferred app for this type?",
    "Install " B_UTF8_OPEN_QUOTE "%t" B_UTF8_CLOSE_QUOTE "\n and associate it with %s?",
    "Yes, make it preferred",
    "Register all types!!",                                           // 0.06 (only 2 new in 0.06)
    "Completed registering of file types. Associated %ld file types with %s.",
    "%s has already been associated with all supported archive types.",
    "Couldn't locate the help files.",
    "Preferences",
    "Save",
    "Discard",
    "Revert",
    "Extract",
        "Options related to extraction",
        "Open destination folder after extracting",
        "Close window after extracting",
        "Quit %s after extracting",
    "State",
        "Configure what to tag along with archives as attributes",
        "Store automatically",
        "Restore automatically",
        "Interface state",
        "Archiver settings",
    "Windows",
        "Configure remembering of windows' settings",
        "Search window",
        "Log window",
        "Comment window",
        "Archive Info window",
        "Welcome window",
        "Preferences window",                                        // 0.06 (see above)
    "Paths",
        "Configure default & favourite paths",
        "Default Paths:",
        "Select",
        "Open Path: ",
        "Select Default Open Path",
        "Add Path: ",
        "Select Default Add Path",
        "Extract Path:",
        "Select Default Extract Path",
        "Same directory as source (archive) file",
        "Use: ",
        "Favourite Extract Paths:",
        "Add a favourite extract path",
        "List more paths (using archive name)",
        "The path " B_UTF8_OPEN_QUOTE "%s" B_UTF8_CLOSE_QUOTE " is already present in your "
           "list of favourite paths.",
    "Recent",
        "Configure recent archives & extract locations",
        "Recent archives",
        "Recent extract paths",
        "Show full path in recent archives",
        "Number of recent archives",
        "Number of recent extract paths",
    "Add",
        "Options related to adding of entries to archives",
        "Replace files:",
        "Never replace",
        "Without asking",
        "Ask me before replacing",
        "When new file is more recent",
        "Confirm when adding more than ",
        "MB",
        "Confirm when adding through drag 'n drop",
        "Sort after add (n/a for reloading archivers)",
    "Interface",
        "Configure graphical user interface options",
        "Full length Toolbar & Infobar",
        "Configure colours: ",
        "Selected text colour",
        "Selected background colour",
    "Miscellaneous",
        "Other general options",
        "When last archive is closed: ",
        "Show welcome window",
        "Quit %s",
        "Show comments (if any) after opening an archive",
        "Check file types at startup",
        "Register file types now",
        "Default Archiver: ",
        "When %s starts: ",
        "Show create archive panel",
        "Show open archive panel",

    "Tools",                             // 0.07 starts here
    "Tools",
    "Additional tools",
    "File Splitter",
    "This tool lets you split a file into several small pieces.",
    "The pieces can later be rejoined to recreate the original file.",
    "Debug Release",
    "Public Release",
    "File to split:",
    "Folder for pieces:",
    "Size of pieces:",
    "Number of pieces:",
    "File size:",
    "Custom size:",
    "Custom size... ",
    "1.44 MB - (floppy)",               // Must be of the format !!     <size> - (description)
    "1.20 MB - (floppy)",
    "100 MB - (zip disk)",
    "250 MB - (zip disk)",
    "650 MB - (CD)",
    "700 MB - (CD)",
    "800 MB - (CD)",
    "1 GB - (jaz disk)",
    "2 GB - (jaz disk)",
    "TB",
    "GB",
    "MB",
    "KB",
    "Bytes",
    "Create self-joining executable",
    "Split",
    "Creating: ",
    "Select ",
    "Select folder for pieces:",
    "Incorrect split size!",
    "Too many pieces!",
    "The file has been successfully split!",
    "Splitting of the file was cancelled...",
    "An unknown error occured while splitting the file.",
    "Open pieces folder after splitting",
    "Splitting is in progress, force it to stop?",
    "Force",
    "Don't force",
    "File number separator:",
    "Select file to split",
    "Close window after splitting",
    "Stub directory not found, cannot create self-joining executable...",
    "_Create_%s",        // The underscore at the beginning is just to make the stub file alphabetically on top


    "File Joiner",                                           // Joiner strings
    "This tool lets you join several files into a single file.",
    "The order of joining is based on number suffixes in the filenames...",
    "First file piece:",
    "Folder for output file:",
    "Select file or folder to join",
    "Select folder to create the output (joined) file",
    "Select ",
    "Open destination folder after joining",
    "Delete pieces after joining them",
    "Output file size:",
    "Refresh",
    "Join",
    "Joining: ",
    "The file was successfully joined!",
    "Joining of the file was cancelled...",
    "An unknown error occured while joining the files.",
    "Joining is in progress, force it to stop?",
    "Close window after joining",

    "Extract with full path during drag 'n drop",        // 0.09 starts here
    "QuickCreate Archive",
    "Archive Name:",
    "Archive Type:",
    "Drop files to create an archive...",
    "Help…",
    "Create",
    "Ready for creating archive.",
    "Waiting for input files...",
    "Adding files to the archive...",
    "Warning! File already exists and will be overwritten!",
    "Error! Destination directory is incorrect.",
    "Fatal error, no add-ons found! You cannot create any archives using Beezer.",
    "Adding: ",
    "Settings: ",
    "Password: ",
    "QuickCreate Archive\n\nAdding is in progress, force it to stop?",

    "Default Interface settings:",                  // 0.09 preferences::interface
    "Show Toolbar",
    "Show Infobar",
    "Show Action Log",
    "Show Columns",
    "Folding: ",
};

//=============================================================================================================//

int GetLanguage ()
{
    return _language;
}

//=============================================================================================================//

void SetLanguage (int whichLang)
{
    if ((whichLang >= 0) && (whichLang < NUM_LANGUAGES))
        _language = whichLang;
}

//=============================================================================================================//

const char* GetLanguageName (int whichLang, bool native)
{
    switch (whichLang)
    {
        case LANG_ENGLISH: return strR5 (S_ENGLISH, native ? LANG_ENGLISH : _language);

        default: debugger ("bad language index!"); return 0;
    }
}

//=============================================================================================================//

const char* strR5 (int whichLang, int language)
{
    const char **base = NULL;
    switch ((language < 0) ? _language : language)
    {
        case LANG_ENGLISH:        base = _englishStrings;        break;
    }

    return ((base) && (whichLang >= 0) && (whichLang < NUM_STRINGS)) ? base[whichLang] : NULL;
}

//=============================================================================================================//
