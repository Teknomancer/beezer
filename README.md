# Beezer

Beezer is an archiver application for the Haiku operating system and functionally similar to programs like WinZip, Ark (on KDE) and Archive Manager (on Gnome). Beezer does **not** work on Windows, GNU/Linux or MacOS.

![Screenshot](/Pictures/Screenshots/Beezer_Haiku.png)

------------------------------------------------------------

Beezer is a native Haiku application written from scratch in C++ with the aim of having an slick, responsive and easy-to-use GUI while offering powerful archiving features. Under the hood, Beezer controls command-line programs, most of which come pre-installed with the operating system, to seamlessly perform compression and decompression.

Beezer was designed for a pluggable module architecture and supports several popular archive formats such as `zip`, `tar`, `gzip`, `bzip2`, `xz`, `lha` (`lzh`) and `7zip` using pluggable add-ons.  Beezer also supports some additional file formats that may require you to download the binaries if they don't ship as part of the operating system.  Adding support for more archive formats is thus relatively little work.

Beezer is open source and published under the BSD 3-clause license. Beezer was originally written by Ramshankar (Teknomancer) for BeOS and later Zeta. He open sourced it in 26 September 2009 following which Chris Roberts (cpr) ported it to Haiku in 2011. Beezer is now being developed/maintained by the two of them in their spare time.  Beezer is currently only developed/maintained for Haiku. The final version available for Zeta and BeOS is 0.09 with no further updates planned.

Contributions in the form of patches are welcome.

Beezer's official home page can be found here: https://ramshankar.org/software/beezer/

## Build Instructions

```
cd Source/
cmake .
make
```
If you have a powerful computer you may speed up compilation by using multiple jobs. For example:
```
make -j8
```

Once the build succeeds, the binaries should be available in the `build` directory.  Check the wiki for a list of the [build options](https://github.com/Teknomancer/beezer/wiki/CMake-Build-Options) available.

If you're using gcc2, you will need the CMake 3.19.4 package. Newer CMake doesn't seem to work with gcc2.
