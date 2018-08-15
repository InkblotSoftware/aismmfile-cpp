//  ======================================================================
//  Copyright (c) 2018 Inkblot Software Limited.
// 
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at http://mozilla.org/MPL/2.0/.
//  ======================================================================

#ifdef NDEBUG
#  error "Can't compile tests with NDEBUG set"
#endif

#include <algorithm>
#include <vector>
#include <cassert>

#include "../include/aismmfile.hpp"

using std::vector;

using aismmf::enforce;
using aismmf::AisMsg;


//  ----------------------------------------------------------------------
//  Utils

template <typename T>
bool equal (const T &o1, const T &o2) {
    return std::equal (o1.begin(), o1.end(), o2.begin(), o2.end()); }

// NB this is exact equality, not allowing for floating point approx
bool operator== (const AisMsg &m1, const AisMsg &m2) {
    return    m1.mmsi   == m2.mmsi   && m1.timestamp == m2.timestamp
           && m1.lat    == m2.lat    && m1.lon       == m2.lon
           && m1.course == m2.course && m1.speed     == m2.speed; }


//  ----------------------------------------------------------------------
//  main()

int main (int argc, char **argv) {
    //enforce (argc == 2, "Provide one arg, path to test data holding aismmf file");
    assert (argc == 2 && "Provide one arg, path to test data holding aismmf file");
    auto mmf = aismmf::AISMmFile {argv [1]};

    auto mmsis = mmf.mmsis ();
    auto targ_mmsis = std::vector<int32_t> {999, 90909};
    assert (equal (mmsis, targ_mmsis));
    
    // Check track 1 right
    {
        auto track = mmf.mmsi (999);
        assert (track.size() == 2);

        const AisMsg *cur = track.begin ();

        auto msg1 = AisMsg {1.1, 2.2,
                            999, 1234,
                            5.5f, 6.6f};
        assert (*cur == msg1);

        ++cur;
        auto msg2 = AisMsg {2.1, 3.2,
                            999, 1294,
                            6.5f, 7.6f};
        assert (*cur == msg2);
        
        ++cur;
        assert(cur == track.end());
    }

    // Check track 2 right
    {
        auto track = mmf.mmsi (90909);
        assert (track.size() == 2);

        const AisMsg *cur = track.begin ();

        auto msg1 = AisMsg {3.1, 4.1,
                            90909, 10123,
                            1.1f, 2.1f};
        assert (*cur == msg1);

        ++cur;
        auto msg2 = AisMsg {4.1, 5.1,
                            90909, 10193,
                            2.1f, 3.1f};
        assert (*cur == msg2);

        ++cur;
        assert (cur == track.end());
    }

    fprintf (stderr, "-- aismmfile tests passed\n");
}

