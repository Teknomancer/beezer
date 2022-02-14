
==============
FileType Rules
==============


Beezer uses mimetype corrections to rectify incorrect mimetypes for
archives that you try to open using Beezer. It also uses these rules
to open archives that resides in file systems that don't support mime
(such as FAT32), in such cases the file extension would be critical.
The mimetype correction rules are necessary if you want Beezer to
open archives with incorrect mimetypes/extensions (often the case
when archives are downloaded from the Internet).

In general, you shouldn't alter the mimetype corrections file. The
file named \_bzr_rules.txt in the Beezer's settings folder is the
mimetype corrections file. It's a plain text file whose format is
explained within the file itself.

Should anything happen to this file (deleted/renamed/moved), Beezer
will for every archive you open re-assign the mimetype which isn't an
efficient thing to do. Hence it's advised you don't do anything to
this file.

You can use this file to add new mimetypes that your system has
(provided Beezer can open them), remove unwanted mimetypes etc.
