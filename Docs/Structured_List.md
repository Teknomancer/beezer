# Structured List Implementation Notes:

This file explains how I got the structured list in Beezer.

The structured list refers to the hierarchical structure that is shown for an 
archive.

1. First the path name is parsed and the directory is pulled away from the 
   filename. The directory path is further parsed into unique sub paths 
   (AddDirPathToTable() and AddFilePathToTable() functions do this).

2. The unique directory paths are all stored in the hash table and then file 
   items into a pointer BList and folder items into another pointer BList. 
   Always, directory entries are made unique while file items are
   not unique. This is because archiving systems like "tar" (GNU) allows several 
   files with the same name to exist (yes, in the same folder) in a tar archive. 
   Since we must support these basic tar archives, we don't store only unique 
   file names. Only directories are unique.

3. The BList is no longer sorted - this is because both files and folders
   are both added by looking up their parent in the hashtable. The parent will 
   contain a pointer to the list item, and AddUnder() will add a file/folder under 
   its correct parent. So sorting is no longer needed.

4. Archiver's FillList() is that magic function that creates the file ListEntry 
   and folder ListEntry items and stores them in the 2 BLists.

5. Now the MainWindow adds all directory items to the window. Uses AddUnder() 
   for adding sub-directories. Hash table lookup is essential here.

6. Then uses AddUnder() to add all file items to the window. Here too,
   hash table lookup is critical.

7. The NULL parent directory entries that files that don't belong to any
   folder are handled separately.

**NOTE:** The ArchiveEntries list in Archiver is deleted after it is used by 
FillLists(), this is because it will hold duplicate entries. Also while 
deleting items from archives, we update the fileList and folderList BLists of 
the Archiver class.

\- Ramshankar.

