#!/bin/sh

# Remove Beezer.o file from the Objects directory so that it will
# be recompiled each time the application is compiled. This script
# will be run during pre-compile stage from BeIDE's ".proj" file and
# ensures that compile time etc. are computed for each compile

if [ -f "./(Objects._Beezer)/Beezer.o" ]; then
	rm "./(Objects._Beezer)/Beezer.o"
fi
