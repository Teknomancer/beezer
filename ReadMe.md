# ![Beezer Icon](Icons/Beezer.svg) Beezer

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
~/Beezer> cd Source
~/Beezer/Source> cmake .
~/Beezer/Source> make
```
If you have a powerful computer you may speed up compilation by using multiple jobs. For example:
```
~/Beezer/Source> make -j8
```

Once the build succeeds, the binaries should be available in the `build` directory.  Check the wiki for a list of the [build options](https://github.com/Teknomancer/beezer/wiki/CMake-Build-Options) available.

## Build Requirements

You will need to have `sphinx` installed if you want to build the user guide.  It can be installed with the python `pip` command.  Depending on which python package you have installed the name of the `pip` command may be slightly different.  For example, to use python 3.8...

```
~> pkgman install pip_python38
~> pip3.8 install sphinx
```
