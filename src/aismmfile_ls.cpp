//  ======================================================================
//  Copyright (c) 2018 Inkblot Software Limited.
// 
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at http://mozilla.org/MPL/2.0/.
//  ======================================================================

#include <cstdio>
#include <stdexcept>

#include "../include/aismmfile.hpp"


//  ----------------------------------------------------------------------
//  Utils

void dieWithUsage () {
    fprintf (stderr,
             "USAGE:\n"
             "  aismmfile_ls FILE.aismmf\n"
             "Prints a CSV of the MMSIs and per-MMSI counts in the file to stdout.\n"
             "Also writes the total number of messages and MMSIs to stderr.\n");
    exit (1);
}


//  ----------------------------------------------------------------------
//  main()

int main (int argc, char **argv) {
    try {
        if (argc != 2)
            dieWithUsage ();
        auto mmf = aismmf::AISMmFile {argv [1]};

        puts ("mmsi,count");
    
        for (auto mmsi : mmf.mmsis())
            printf ("%d,%zu\n", mmsi, mmf.mmsi(mmsi).size());

        fprintf (stderr, "Count MMSIs:    %zu\n", mmf.mmsis().size());
        fprintf (stderr, "Count messages: %zu\n", mmf.all().size());

    } catch (std::exception &e) {
        fprintf (stderr, "Died with exception: %s\n", e.what());
        dieWithUsage ();
    }
}
