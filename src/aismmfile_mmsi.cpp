//  ======================================================================
//  Copyright (c) 2018 Inkblot Software Limited.
// 
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at http://mozilla.org/MPL/2.0/.
//  ======================================================================

#include <cstdio>
#include <stdexcept>
#include <string>

#include "../include/aismmfile.hpp"


//  ----------------------------------------------------------------------
//  Utils

void dieWithUsage () {
    fprintf (stderr,
             "USAGE:\n"
             "  aismmfile_mmsi FILE.aismmf MMSI\n"
             "Prints a CSV containing all the messages stored for MMSI \n"
             "in FILE to stdout.\n"
             "Also prints the total number of messages to stderr.\n");
    exit (1);
}


//  ----------------------------------------------------------------------
//  main()

int main (int argc, char **argv) {
    try {
        if (argc != 3)
            dieWithUsage ();
        auto mmf = aismmf::AISMmFile {argv [1]};
        int32_t mmsi = std::stoi (argv [2]);

        if (!mmf.hasMmsi (mmsi)) {
            fprintf (stderr, "MMSI %d not found in file\n", mmsi);
            exit (1);
        }

        auto track = mmf.mmsi (mmsi);

        puts ("mmsi,timestamp,lat,lon,course,speed");

        for (auto msg : track) {
            printf ("%d,%d,%f,%f,%f,%f\n",
                    msg.mmsi, msg.timestamp, msg.lat, msg.lon,
                    msg.course, msg.speed);
        }

        fprintf (stderr, "Count messages: %zu\n", track.size());
        
    } catch (std::exception e) {
        fprintf (stderr, "-- Program died with exception: %s\n", e.what());
        dieWithUsage ();
    }
}
