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
application/zip=.zip\n\
application/x-zip-compressed=.zip\n\
\n\
application/x-tar=.tar\n\
\n\
application/x-gzip=.tar.gz\n\
application/x-gzip=.tgz\n\
application/x-gzip=.gz\n\
\n\
application/x-bzip2=.bz2\n\
\n\
application/x-lharc=.lha\n\
application/x-lzh=.lzh\n\
\n\
application/x-rar-compressed=.rar\n\
application/x-rar=.rar\n\
\n\
application/x-7zip-compressed=.7z\n\
application/x-7zip-compressed=.7zip\n\
application/x-7z-compressed=.7z\n\
application/x-7z-compressed=.7zip\n\
\n\
application/x-arj-compressed=.arj\n\
";

#endif	// _RULEDEFAULTS_H_
