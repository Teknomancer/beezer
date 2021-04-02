Adding support for a new archive format
=======================================

1. Locate and install the command-line worker for your format.
2. Implement the new add-on for your format in a sub-folder under `Source/` (e.g., `Source/FancyArchiver/`).
3. Add mime-type/extension rules for your format in `Beezer/RuleDefaults.h`.
4. Ensure `_bzr_rules.txt` is deleted so a new one will be generated when Beezer launches with newly added rules.
   This file is typically found in `~/config/settings/Beezer/`
5. If too many compression levels are supported by the format, try reducing it to a few meaningful values to keep the UI sensible
   (as currently they are implemented as a drop-down menu). However, always allow the fastest and the highest compression (best) regardless
   of how many levels you expose in-between them.
