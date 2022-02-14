
=========
Rar AddOn
=========


Currently the rar archiver available for Haiku doesn't support
creating rar archives, you can only extract them.

The rar archiver gives you the following options:

**While Extracting**

   -  **Always overwrite (Default)**: Always overwrite any existing
      files
   -  **Never overwrite existing files**: Any files that already exist
      are not overwritten while extracting
   -  **Update files, create if needed**: Overwrites existing file if
      archive file is newer, creates files that are missing
   -  **Freshen existing files, create none**: Only existing files are
      overwritten if they are older than the ones in the archive, no new
      files are extracted

**Other options**

   -  **Process attributes**: Allows adding/extracting of Haiku file
      attributes. Its generally best to leave this option turned ON
      especially if you plan to use the archive in other Haiku partitions
   -  **Keep broken extracted files**: Files that are found corrupt after
      extraction are not deleted by Rar

For information on saving these settings to the archive or as defaults
read the :ref:`ArchiveWindow:\< archiver \>` menu information.
