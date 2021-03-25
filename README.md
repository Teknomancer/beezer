# Beezer

Beezer is an archiver application for the Haiku operating system (and earlier versions for BeOS and Zeta operating systems). 

Beezer is currently only developed/maintained for Haiku. The final version available for Zeta and BeOS is 0.09 with no further updates planned. Beezer does -not- work on Windows, GNU/Linux or macOS.

Beezer is functionally similar to programs like WinZip, Ark (on KDE) and Archive Manager (on Gnome).

Beezer is a native Haiku application written from scratch in C++ with the aim of having an slick, responsive and easy-to-use GUI while offering powerful archiving features. Under the hood, Beezer controls command-line programs, most of which come pre-installed with the operating system, to seamlessly perform compression and decompression.

Beezer was designed for a pluggable module architecture and supports several popular archive formats such as `zip`, `tar`, `gzip`, `bzip2`, `lha` (`lzh`) and `7zip` using pluggable add-ons. Adding support for more archive formats is thus relatively little work.

Beezer is open source and published under the BSD 3-clause license. Beezer was originally written by Ramshankar (Teknomancer) for BeOS and later Zeta. He open sourced it in 26 September 2009 following which Chris Roberts (cpr) ported it to Haiku in 2011. Beezer is now being developed/maintained by the two of them in their spare time.

Contributions in the form of patches are welcome.

Beezer also supports some additional file formats that may require you to download the binaries if they don't ship as part of the operating system.

Beezer's official home page can be found here: https://ramshankar.org/software/beezer/

## Build Instructions

```
cd Source/
cmake
make
```
Once the build succeeds, the binaries should be available in the `build` directory.
