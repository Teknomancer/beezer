# Release instructions

Things to do before creating an official release version of Beezer:

* Update `VersionHistory.html`

* Make sure the `AppConstants.h` has the correct version number

* Remove the `DDEBUG` flag in _BeIDE_.
  This is a pain as it will cause a full rebuild when its turned back on, but don't do a release with debug symbols.

* Move any older versions of Beezer and its SoftwareValet package files to `_OlderVersions` folder located in this folder's parent folder.

- Ramshankar
