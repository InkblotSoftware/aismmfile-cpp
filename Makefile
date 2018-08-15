##  ======================================================================
##  Copyright (c) 2018 Inkblot Software Limited.
## 
##  This Source Code Form is subject to the terms of the Mozilla Public
##  License, v. 2.0. If a copy of the MPL was not distributed with this
##  file, You can obtain one at http:##mozilla.org/MPL/2.0/.
##  ======================================================================

SHELL=/bin/bash -o pipefail
.DELETE_ON_ERROR:

default: all


##  ======================================================================
##  User-facing binaries

CXXFLAGS := -g -Wall -Wextra -Werror -std=c++14 -O2

bin/aismmfile_ls: src/aismmfile_ls.cpp include/aismmfile.hpp
	@mkdir -p $(dir $@)
	c++ $(CXXFLAGS) $< -o $@

bin/aismmfile_mmsi: src/aismmfile_mmsi.cpp include/aismmfile.hpp
	@mkdir -p $(dir $@)
	c++ $(CXXFLAGS) $< -o $@

bin/run_tests: src/run_tests.cpp include/aismmfile.hpp
	@mkdir -p $(dir $@)
	c++ $(CXXFLAGS) $< -o $@


##  ======================================================================
##  System-wide install

### Install prefix; override at 'make' invocation if desired
PREFIX := /usr/local

install: bin/aismmfile_ls
	cp include/aismmfile.hpp $(DESTDIR)$(PREFIX)/include/
	cp bin/aismmfile_ls $(DESTDIR)$(PREFIX)/bin
	cp bin/aismmfile_mmsi $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/include/aismmfile.hpp
	rm -f $(DESTDIR)$(PREFIX)/bin/aismmfile_ls
	rm -f $(DESTDIR)$(PREFIX)/bin/aismmfile_mmsi


##  ======================================================================
##  Utilities

all: bin/aismmfile_ls   \
     bin/aismmfile_mmsi \
     bin/run_tests

clean:
	rm -f bin/run_tests
	rm -f bin/aismmfile_ls

run-tests: bin/run_tests
	./$< test/testdata.aismmf
