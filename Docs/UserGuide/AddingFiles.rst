
============
Adding Files
============


Beezer allows you to add files & folders (entries) to any folder
inside the archive and not just at the root of the archive.

Before adding entries you need to choose where to add them (unless
you are adding files through drag 'n drop). The following paragraph
explains where Beezer will add new files:

   -  **If you have selected a file**, newly added entries will be added
      in the same folder as the selected file.
   -  **If you have selected a folder**, newly added entries will be
      added inside the selected folder.
   -  **If you have no selection** newly added entries will be added to
      the root of the archive.

If you have multiple selections before adding entries, the first
visible selected item will be used.

Once you have chosen where to add the files, you can choose any of
the following actions:

   -  From Actions –> Add, or
   -  From toolbar: Add

A file requester will pop-up asking you to choose which files and
folders you wish to add. After choosing them, press the "Add" button
and the selected entries will be added to the archive.

Drag 'n Drop
============

   You can also add entries to the archive by dropping them from
   *Tracker* to the :ref:`ArchiveWindow:TreeView`. In this
   case, the selected entry will NOT be used to determine where the new
   entries will be added. Rather a horizontal marker (that appears as
   you drag the entries to be added) will determine where new entries
   are added.

   When you drag your mouse over the treeview you will see a horizontal
   line. This is the location where the dropped entries will be added.
   If you want to add the files at the root of the archive, press
   **SHIFT** and you will see the horizontal line turn red.

   Thus dropping entries when the line is blue, adds them to that
   location, dropping entries when the line is red adds the entries root
   of the archive.

The process of adding is shown by a progress bar. If the files you are
adding already exist in the archive, the replacement action in
:ref:`Preferences: Add <Preferences:Add>` would be used.

Cancelling the operation
========================

   You can cancel this operation while it is in progress, but doing so will
   leave the archive in an indeterminate state - regarding the physical
   changes to the archive. This is because different archivers react
   differently when they are stopped before completing their task. For
   example, some archives automatically undo the changes while some don't.
   Thus you will have to re-load the archive.

   In general it's not advisable to cancel this operation, as it can leave
   your archive corrupted. You have been warned! :)
 
