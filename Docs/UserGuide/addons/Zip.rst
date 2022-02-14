
=========
Zip AddOn
=========


The zip archiver gives you the following options:

**Compression Level**

   Allows you to specify the amount of compression while creating
   archives on a scale of 0 to 9. 9 is maximum compression, 1 is minimum
   and 0 (zero) is no compression (just storage).

**While Adding**

   -  **Add attributes**: Stores Haiku file attributes to the zip. Its
      generally best to leave this option turned ON especially if you plan
      to use the archive in other Haiku partitions
   -  **Recurse into folder**: Adds all the contents of folders being
      added, including sub-directories and its contents

**While Extracting**

   -  **Extract attributes**: Restores Haiku file attributes from the zip.
      Its generally best to leave this option turned ON
   -  **Extract folders**: Allows extraction of folders
   -  **Never overwrite existing files**: Doesn't overwrite files that
      already exist in the destination folder while extracting
   -  **Update files, create if needed**: Overwrites existing file if
      archive file is newer, creates files that are missing
   -  **Freshen existing files, create none**: Only existing files are
      overwritten if they are older than the ones in the archive, no new
      files are extracted

For information on saving these settings to the archive or as defaults
read the :ref:`ArchiveWindow:\< archiver \>` menu information.
