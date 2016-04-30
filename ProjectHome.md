This library will unpack PKLITE, LZEXE, EXEPACK and Knowledge Dynamics LZW COMPRESSOR compressed MS-DOS exe files

Supported versions:
  * PKLITE: 1.12, 1.15, 1.50 (both standard and extended modes)
  * LZEXE:  0.90, 0.91, 0.91e
  * Knowledge Dynamics (no rellocation tables)

Supported compilers: GCC, CLANG, Intel Compiler, SunCC, VisualStudio

Both 32/64 bits and Little/Big endian architectures are supported


Credits:
  * Knowledge Dynamics is based on https://sourceforge.net/p/openkb/code/ci/master/tree/src/tools/unexecomp.c
  * Unlzexe is based on http://bellard.org/lzexe.html


TODO:
  * More robust detection of Knowledge Dynamics compressed files
  * Support for EXEPACK (in progress)
  * Support for PKLITE 1.20 and 2.01
  * Memory mapped I/O