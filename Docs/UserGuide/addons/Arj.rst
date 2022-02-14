
=========
Arj AddOn
=========


The arj binary (or the arj format itself) has some distinct
differences that limit some actions that are generally available with
other formats. The arj binary/format (ark v 3.10, 15 Feb 2005 Build)
has the following limitations/issues:

   #. Adding empty folders to an arj archive will not work. Hence the
      :doc:`Create folder <../CreatingFolders>` option isn't available.
   #. :doc:`Deleting <../DeletingFiles>` is messed. Meaning deleting a
      file named "Hello.txt" from the root of the archive will also
      remove all files named "Hello.txt" from all sub-directories as
      well. Dangerous!!
   #. Adding files and folders at the same time makes a mess of things,
      as ARJ apparently erroneous back-recursing with parent folders...
      extremely time consuming, ugly result. You can for the time being
      add folders seperately, and files seperately as two different Add
      actions. The ARJ binary v3.10 (15-02-2005 build) is probably to
      blame as I tried adding folders even from the command-line and it
      wouldn't behave like it should (either it won't find the folders
      or will not traverse links properly or other errors).

My guess is that the above limitations are imposed by the arj format
itself and isn't the fault of the arj binary port of BeOS...

The arj archiver gives you the following options:

**Compression Level**

   Allows you to specify the amount of compression while creating
   archives on a scale of 0 to 4. 1 is maximum compression, 4 is minimum
   and 0 (zero) is no compression (just storage).

**While Adding**

   -  **Recurse into folder**: Adds all the contents of folders being
      added, including sub-directories and its contents

**While Extracting**

   -  **Update files (new and newer)**: Overwrites existing file if archive
      file is newer, creates files that are missing
   -  **Freshen existing files**: Only existing files are overwritten if
      they are older than the ones in the archive, no new files are
      extracted
   -  **Enable multiple volumes**: Enables extraction of multi-volume
      archives (recommended)

For information on saving these settings to the archive or as defaults
read the :ref:`ArchiveWindow:\< archiver \>` menu information.
