// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2011 Chris Roberts.
// Copyright (c) 2021 Ramshankar (aka Teknomancer).
// All rights reserved.

#ifndef _RULEDEFAULTS_H_
#define _RULEDEFAULTS_H_

const char* kDefaultRules = "\
# Mime correction rules for Beezer\n\
# \n\
# Beezer checks the file extension and accordingly checks the\n\
# mime type. If the extension and mime type doesn't match,\n\
# Beezer removes the wrong mime type and asks BeOS\n\
# to re-correct the mime type.\n\
# \n\
# If you don't understand this you probably shouldn't alter\n\
# anything. The rules are of the obvious format:\n\
# mimetype=extension\n\
\n\
#application/zip=.zip\n\
#application/x-zip-compressed=.zip\n\
\n\
#application/x-tar=.tar\n\
\n\
#application/x-gzip=.tar.gz\n\
#application/x-gzip=.tgz\n\
#application/x-gzip=.gz\n\
\n\
#application/x-bzip2=.bz2\n\
#application/x-bzip2=.tbz\n\
#application/x-bzip2=.tar.bz2\n\
\n\
#application/x-lharc=.lha\n\
#application/x-lzh=.lzh\n\
\n\
#application/x-rar-compressed=.rar\n\
#application/x-rar=.rar\n\
\n\
#application/x-7zip-compressed=.7z\n\
#application/x-7zip-compressed=.7zip\n\
#application/x-7z-compressed=.7z\n\
#application/x-7z-compressed=.7zip\n\
\n\
#application/x-xz=.xz\n\
#application/x-xz=.xzip\n\
#application/x-xz=.txz\n\
#application/x-xz=.tar.xz\n\
\n\
#application/x-arj-compressed=.arj\n\
\n\
#application/x-zstd-compressed-tar=.tar.zst\n\
#application/x-zstd=.zst\n\
#application/x-zstd=.zstd\n\
\n\
#application/x-vnd.haiku-package=.hpkg\n\
";

#endif  // _RULEDEFAULTS_H_
