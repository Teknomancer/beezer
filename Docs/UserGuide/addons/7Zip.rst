
==========
7zip AddOn
==========


The 7zip archiver gives you the following options:

**Compression Level**

   Allows you to specify the amount of compression while creating
   archives on an odd scale of 0 to 9. 9 is maximum compression, 1 is
   minimum and 0 (zero) is no compression (just storage).

**While Adding**

   -  **Use solid blocks**: Utilizes 7zip's solid block algorithm, this
      results in higher compression but makes the archive sort of like a
      magnetic tape meaning, to extract one file in the block you will have
      to extract all files upto the file. In general, its best not to use
      this option unless you want that slight bit of extra compression
   -  **Use multi-threading (for multi-CPU PCs)**: Asks 7zip to make use of
      multi-threading across all your CPUs, useful if your system has more
      than one CPU

**While Extracting**

   -  **Always overwrite (Default)**: Always overwrite any existing files
   -  **Never overwrite existing files**: Any files that already exist are
      not overwritten while extracting
   -  **Rename existing files**: Rather than overwriting, it renames the
      existing file
   -  **Rename extracted files**: Rather than overwriting, the newly
      extracted file from the archive is created with a different name

For information on saving these settings to the archive or as defaults
read the :ref:`ArchiveWindow:\< archiver \>` menu information.
