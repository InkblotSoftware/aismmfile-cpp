aismmfile-cpp
=============

Reader for the "positional AIS memory mapped file" format, originally implemented
in [aismmfile](https://github.com/InkblotSoftware/aismmfile).

This (binary) format offers fast and easy to use access to positional AIS data,
in a portable, cross-language manner.

See the original project for details of the format, and a program to generate files.
We also have a Java reader and writer implementation in the works.

This project exposes a single file header only C++ library, with no dependencies
outside the standard library, and a pair of utility programs to investigate and
read aismmfile files.

Currently the library and programs only compile on POSIX, as we haven't yet included
a Windows mmap wrapper, but that's pretty easy to add if anyone is interested.


C++ library - aismmfile.hpp
---------------------------

Just add `include/` to your include path to use this, or copy `include/aismmfile.hpp`
to your project directory.

Allows zero-copy access to per-mmsi track message data, exposed as arrays of structs.

Example:

```c++
// #include <aismmfile.hpp>

auto mmf = aismmf::AISMmFile {"path/to/file"};

if (mmf.hasMmsi (12345)) {
    auto track = mmf.mmsi (12345);
    std::cout << track.size() << std::endl;  // number of messages in track

    // Iterate over contiguous array of aismmf::AisMsg structs
    for (auto msg : track) {
        std::cout << msg.mmsi   << "," << msg.timestamp << ","
                  << msg.lat    << "," << msg.lon       << ","
                  << msg.course << "," << msg.speed
                  << "\n";
    }
}
```


Helper programs
---------------

Use these to look inside aismmf files without having to write any code.

**aismmfile_ls**

```
USAGE:
  aismmfile_ls FILE.aismmf
Prints a CSV of the MMSIs and per-MMSI counts in the file to stdout.
Also writes the total number of messages and MMSIs to stderr.
```

**aismmfile_mmsi**

```
USAGE:
  aismmfile_mmsi FILE.aismmf MMSI
Prints a CSV containing all the messages stored for MMSI
in FILE to stdout.
Also prints the total number of messages to stderr.
```


System install
--------------

Call `make && sudo make install` in the usual way.

By default installs to `/usr/local`; specify an alternative path prefix
as follows: `sudo make PREFIX=/my/other/prefix install`


Self tests
----------

Run `make run-tests` to build and run the test runner, which uses the pre-made
file `test/testdata.aismmf`. It informs on success or failure.


Copyright and license
---------------------

Project copyright (c) 2018 Inkblot Software Limited.

Licensed under the terms of the Mozilla Public License v2.0. This means you
can permissively use this library in your own (perhaps proprietary) projects,
but any changes you make to this project's files stay open.
