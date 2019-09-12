# ToDo List for Beezer

* To a Quick-Edit feature that allows extraction of file to temporary location and each
  time it's saved it will be re-added to the archive.

* Fix bug in Quit after Extract when multiple files are being extracted. Test with BeBook.zip
  and a smaller zip with BeBook.zip is extracting, use extract on smaller file and when
  the smaller file completes it will forcefully quit Beezer without waiting for BeBook to 
  complete. 
  (Done)
        -> Fixed using something like m_criticalSection variable.

* File Splitter.
  (Done)

* Register all file types button
  (Done)

* Change R5's str () function to detect if OS is Zeta and use _X function (where the
  current str will be renamed as say str2) thus making R5 code compile in Zeta. Then all
  we need to do is hold 2 different PROJ files with different libraries... the rest will be
  handled by #ifdef B_ZETA_VERSION flag... cool :)
  (Done)

* Make add-ons save settings only when user clicks "Save as Defaults".
  (Done)

* Consider giving the add full path option, for eg, adding "/boot/home/temp.txt" will add full
  directory hierarchy to the archiver rather than just adding "temp.txt" inside the selected folder.
  (Closed)
      -> Not feasible, the options were explored, problems were analyzed,
         I came to the conclusion this isn't possible in our architecture.

* Use sscanf() for reading piped output in add-ons.
  (Done)
      -> No loss in terms of read speed was found.

* Implement proper date, time parsing & keep the timestamp for each file as a time_t value. Will be
  needed while sorting, replacing/adding files to archive and so on.
  (Done)

* Try and implement the option - Replace files as follows (When newer, always ask, always replace,
  never replace). 
  (Done)
      -> We now know the timestamp of each file in the archive. 
         We do an optimized hash-table lookup, compare the
         timestamps accordingly and take action.

* Add "freshen and update" options to "While adding" menu of Zip add-on. Do NOT give the "-m" option
  to Zip add-on as it will delete the original file from the disk - can be dangerous and we will avoid this...
  (Closed)
      -> Don't know if this is such a critical thing as we have implemented the Replace files as follows option, and
         that could conflict with this because, while we ask whether to replace or not, zip might not actually replace
         (due to freshen or update option) and thus could lead the user into thinking it is actually replaced.

\- Ramshankar.
