//  ======================================================================
//  Copyright (c) 2018 Inkblot Software Limited.
// 
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at http://mozilla.org/MPL/2.0/.
//  ======================================================================

// Header-only reader library for positional AIS holding memory mapped
// binary file format. Fairly efficient - holds each MMSI's data as a
// contiguous array, in timestamp-increasing order.
// 
// Project URL: https://github.com/InkblotSoftware/aismmfile-cpp
// See README and examples here.
//
// Usually you want to create an AISMmFile object with a file path,
// call mmsis() or .hasMmsi(int32_t) to see what it contains, then access
// a single MMSI's track (zero copy) with .mmsi(int32_t).
//
// Other classes and functions are exposed to the user as a convenience.
// These carry no stability guarantee, but are unlikely to change in a
// backwards incompatable way.
//
// Currently this library only works on POSIX, as it's missing the mmap
// code for Windows. This is pretty easy to add, so do PR the change if
// you need it.

#ifndef AISMMFILE_HPP_INCLUDED
#define AISMMFILE_HPP_INCLUDED

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cstdio>

namespace aismmf {

//  ======================================================================
//  == Condition enforcement
//  ======================================================================

struct EnforcementError : public std::runtime_error {
    explicit EnforcementError (const std::string &msg) :std::runtime_error{msg} {} };

void enforce (bool cond, const std::string &msg) {
    if (!cond)
        throw EnforcementError (msg);
}


//  ======================================================================
//  == Byte type used in this library
//  ======================================================================

using byte = unsigned char;    
    

//  ======================================================================
//  ==  Simple memory-mapped file manager
//  ==    Curently this only supports posix systems, as we use mman.h. Expanding
//  ==    to include Windows is easy to do.
//  ======================================================================

class MMFile {
    const byte *_mappedRegion;
    size_t      _length;
    int         _fd;

public:
    //  --------------------------------------------------
    //  Ctr/dtr
    
    explicit MMFile (const std::string &filepath) {
        _fd = open (filepath.c_str(), O_RDONLY);
        enforce (_fd != -1, "Error opening mmaped file");

        struct stat sb;
        enforce (fstat (_fd, &sb) != -1,
                 "Error calling stat() on file");
        _length = sb.st_size;

        const void *ptr = mmap (NULL, _length, PROT_READ,
                              // TODO experiment with and w/o populate
                              //MAP_SHARED | MAP_POPULATE,
                              MAP_SHARED,
                              _fd, 0);
        enforce (ptr != MAP_FAILED, "MMAP call failed");
        _mappedRegion = reinterpret_cast<const byte *> (ptr);
    }

    ~MMFile () {
        munmap ((void *)_mappedRegion, _length);
        close (_fd);
    }

    //  --------------------------------------------------
    //  Accessors

    const byte * data () const { return _mappedRegion; }
    size_t       size () const { return _length; }

    const byte * begin () const { return data(); }
    const byte * end   () const { return data() + size(); }
};

    
//  ======================================================================
//  == Span over const array of T. Correctly aligned for T.

template <typename T>
class CSpan {
    const T *_data;
    size_t   _size;

    // Check memory alignment
    template <typename TTest>
    bool isAlignedFor (const void *ptr) {
        return (uintptr_t)ptr % alignof(TTest) == 0; }

public:
    typedef unsigned char byte;
    
    //  --------------------------------------------------
    //  Ctrs etc
    
    explicit CSpan (const T *data, size_t size) :_data{data}, _size{size} {
        enforce (isAlignedFor<T> (data), "CSpan pointer not aligned for type"); }
    explicit CSpan () :CSpan{nullptr, 0} {}

    // Make a span within this, from relative offset of given length
    CSpan<T> subspan (size_t offset, size_t length) const {
        enforce (offset < _size, "Subspan starts outside span");
        enforce (offset + length <= _size, "Subspan ends outside span");
        return CSpan<T> {_data + offset, length}; }
    
    template <typename TNew>
    CSpan<TNew> asSpan () const {
        enforce ((size() * sizeof(T)) % sizeof(TNew) == 0,
                 "Span size doesn't fit target type during asSpan() conversion");
        // Ctr does aligment check
        return CSpan<TNew> {reinterpret_cast<const TNew *> (_data),
                            (size() * sizeof(T)) / sizeof(TNew)}; }

    CSpan<byte> bytes() const { return asSpan<byte>(); }

    CSpan<T> & operator= (const CSpan<T> &rhs) {
        _data = rhs._data;
        _size = rhs._size;
        return *this; }

    //  --------------------------------------------------
    //  Accessors

    const T * data() const { return _data; }
    size_t    size() const { return _size; }

    const T * begin() const { return data(); }
    const T * end()   const { return data() + size(); }
};


//  ======================================================================
//  == Core AIS message type stored in the file

struct AisMsg {
    double lat;
    double lon;
    
    int32_t mmsi;
    int32_t timestamp;
    
    float course;
    float speed;

    // To help debugging etc
    void fprint (FILE *out) const {
        fprintf (out, "(AisMsg){ mmsi: %d, timestamp: %d, lat: %f, lon: %f, "
                 "course: %f, speed: %f\n",
                 mmsi, timestamp, lat, lon, course, speed);
    }
};
static_assert (sizeof(AisMsg) == 32, "");


//  ======================================================================
//  == AISMmFile class - core public export
//  ======================================================================

class AISMmFile {
    // File header entry
    struct HeadEnt { int32_t mmsi, offset, length, _padding; };
    static_assert (sizeof (HeadEnt) == 16, "");
    
    MMFile  _file;
    int64_t _numMmsis;
    CSpan<HeadEnt> _header;
    CSpan<AisMsg>  _body;

public:
    //  --------------------------------------------------
    //  Ctrs

    explicit AISMmFile (const std::string &filepath) :_file{filepath} {
        auto bytes = CSpan<byte> {_file.data(), _file.size()};
        
        _numMmsis = bytes.subspan (0, sizeof(int64_t))
                         .asSpan<int64_t> ()
                         .data()
                         [0];
        
        _header = bytes.subspan (sizeof(int64_t), _numMmsis * sizeof(HeadEnt))
                       .asSpan<HeadEnt> ();

        auto doneSize = sizeof(int64_t) + _header.bytes().size();
        _body = bytes.subspan (doneSize, bytes.size() - doneSize)
                     .asSpan<AisMsg> ();

        enforce (   bytes.size()
                 == sizeof(int64_t) + _header.bytes().size() + _body.bytes().size(),
                 "File length integrity error");
    }

    //  --------------------------------------------------
    //  Accessors

    // Does the file contain messages from the given mmsi?
    bool hasMmsi (int32_t mmsi) const {
        for (auto he : _header) {
            if (he.mmsi == mmsi)
                return true;
        }
        return false;
    }

    // Get a freshly allocated vector of all the mmsis in the file
    std::vector<int32_t> mmsis () const {
        auto res = std::vector<int32_t> {};
        for (auto he : _header) res.push_back (he.mmsi);
        return res; }

    // Get one mmsi's track data
    CSpan<AisMsg> mmsi (int32_t mmsi) const {
        for (auto he : _header) {
            if (he.mmsi == mmsi)
                return _body.subspan (he.offset, he.length);
        }
        throw EnforcementError {"Requested MMSI not found in file"};
    }

    // Get all messages
    CSpan<AisMsg> all () const { return _body; }
};

}  // namespace aismmf

#endif  // AISMMFILE_HPP_INCLUDED
