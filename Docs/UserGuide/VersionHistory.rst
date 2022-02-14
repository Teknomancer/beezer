
===============
Version History
===============


A list of important changes that have come accross Beezer.

**Version 1.00**

   -  Add basic support for multipart rar archives
   -  Disable arj support and creating/writing of rar archives until
      Haiku has modern ports of these
   -  Enhanced path checking for worker binaries
   -  Added support for xz archives
   -  Removed obsolete ZETA code
   -  Migrate settings folder to B_USER_SETTINGS_DIRECTORY, to be more
      consistent with Haiku standards
   -  Statically link tar archiver into gzip, bzip2 and variants
   -  Added tooltips
   -  Bug Fix: Fixed reading and testing of 7z files for latest 7z
      binaries
   -  Bug Fix: Minor UI fixes for Haiku
   -  Compiled and built under Haiku natively using CMake, abandoned
      legacy BeIDE projects

**Version 0.09 - (Released)**

   -  Made file extension stripping in Extract context menu case
      insensitive
   -  Included an option for extracting without full path during drag 'n
      drop extraction
   -  Fixed a minor bug when reporting info about unavailable operation
      in the log
   -  Made hidden unavailable columns for ARJ archives, namely CRC &
      Method
   -  Included default archive window settings to Preferences' Interface
      panel
   -  Fixed a bug in ARJ' s extract process by removing the recurse
      option while extracting
   -  Fixed a bug while adding files to Rar archives by ignoring warning
      issued by Rar when it couldn't get hold of a file's permission
   -  New QuickCreate Tracker Add-on available for Beezer, this is a
      hack of sorts in that Bezeer is invoked to do the creation process
      in one step...
   -  Some 'interfacial' changes to Preferences window :)

**Version 0.08 - (Released)**

   -  Made Gzip/Bzip2 add-on use ".tar" extension when the mime-type
      doesn't update itself correctly during the open process
   -  Fixed embarrasing bug where the 7zip script was left off (instead
      was a link). Now 7zip should work again...

**Version 0.07 - (Released)**

   -  Fixed a bug that prevents choosing a file in-place of a folder
      (during extraction)
   -  New tools: FileSplitter & File Joiner with ability to create
      self-joining executables
   -  Ability to make self-joining BeOS executables, which work on BeOS
      systems without Beezer
   -  Updated 7z add-on to work with the latest version of p7zip
   -  Fixed a bug in 7z add-on that caused error to be reported for
      empty 7z archives
   -  Some fancy drawing added for PrefsListItem

**Version 0.06 - (Unreleased)**

   -  Fixed a bug while extracting multiple files with "Quit after
      extract" option (thanks to Jess for reporting this)
   -  Fixed a bug in Welcome window as it now resizes properly to fits
      its contents regardless of label lengths
   -  Added an option to save Preferences window position
   -  Renamed "Prefs" in Welcome window to "Preferences" to be more
      intuitive to the complete newbie :)
   -  Added an option to register all archive types with Beezer rather
      than confirming each one, as requested by Begasus :)
   -  Added some ARJ add-on's delete operation switches (not available
      to the user)
   -  Updated 7zip add-on to work with the p7zip 4.20
   -  Made viewing of archive's within an archive that open by Beezer
      itself work
   -  Fixed a bug where RefsReceived() was getting called twice for each
      archive
   -  Changed Beezer's icon a bit
   -  Added code to enable reporting of an unsupported operation by an
      add-on beforehand
   -  Fixed a typo, thanks to DaaT for reporting it

**Version 0.05 - (Unreleased)**

   -  New ARJ add-on included, (has minor issues due to the ARJ format
      itself)
   -  Fixed a bug in 7zip launch script, no longer prints 7zip output to
      Terminal while opening files
   -  Fixed a bug while cancelling create/open panels on startup
   -  Error messages while attempting to write archives in read-only
      partitions are shown
   -  Internal clean-ups in Rar add-on, disabled percentage display for
      neater output
   -  Made the main app code a bit cleaner
   -  Minor bug fix in ImageButton (no practical effect, only
      theoritically)
   -  Fixed an out-of-the-blue bug in Delete of Zip add-on (!)
   -  Better implementation of archive name generation in "extract"
      context menu
   -  Now pressing SHIFT while dropping files to an archive correctly
      updates drop indicator color rather than updating the colour in
      the next MouseMoved() event
   -  Fixed a resizing bug where Column headers were getting graphically
      corrupt when resizing to least height

**Version 0.04 - (Released)**

   -  Shiny new 7zip add-on included, with password support and partial
      delete support (written by Marcin Konicki, with inputs and fixes
      by me)
   -  Fixed a bug in some add-ons for filenames starting with spaces
      (thanks to Marcin Konicki for suggesting this)
   -  Fixed a bug in Rar add-on that caused crash due to unconventional
      permission strings (thanks to AlienSoldier and Miguel Guerreiro
      for finding this and providing archives that crash)
   -  Fixed a bug in cancelling Open panel when using non-default
      startup option works like it should
   -  Fixed a bug in some add-ons to not use extract options while
      simply viewing files
   -  Fixed a bug in Zip add-on where files with -0%, -1% ratio were
      skipped

**Version 0.03 - (Released)**

   -  Rar add-on now has full password support - password can be used
      for extracting and adding files
   -  Fixed a minor bug in rar add-on that caused progress bar to
      unnecessarily update in test/extract functions
   -  Rar add-on uses the newer unrar 3.10 for extraction purposes and
      older rar 2.x for creating/adding (many thanks to Miguel Guerreiro
      for suggesting this and for providing test files). Because of the
      difference in rar and unrar versions, you will not be able to add
      files to a 3.x password-protected rar archive
   -  Minor change to ZipArchiver improving the possibility of handling
      erroneous adding/deleting
   -  Now right-clicking anywhere on the Infobar will toggles its
      visibility
   -  Added startup options - can be accessed from Miscellaneous panel
   -  Added color preferences - can be accessed from Interface panel
   -  Fixed a bug when detecting archives without mime/extension

**Version 0.02 - (Released)**

   -  Associated "Delete" key to the delete files operation
   -  Added a button in extract file panel to allow extraction to
      currently displayed folder
   -  Fixed a rare bug that caused crash when trying to modify a
      corrupt/invalid archive
   -  Rar add-on, with rudimentary password support, is now included
      (doesn't support deleting from rar archives)
   -  Minor internal change caused all add-ons, main app to be
      recompiled
   -  Replace "/" in Infobar by "of", makes it easier to read
   -  Added a new Preference panel - "Interface", currently only one
      option, will be useful in future for adding color prefs
   -  Double-clicking a column now auto-sizes it to fit all visible
      column contents, similar to Tracker
   -  Fixed a bug that showed "Adding error" or "Deleting error" when
      the archive resides on a filesystem without mime (thanks to Tim de
      Jong/Sir Mik for reporting)

**Version 0.01 - (Released)**

   -  First public released. All looks well :)
