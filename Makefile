# Makefile for BMS 2 project
# Fridolin Pokorny
# fridex.devel@gmail.com

CXX=g++
LDFLAGS=-lbass
CXXFLAGS=-std=c++11 -O2 -fomit-frame-pointer

SRCS=bms2.cpp cfg.cpp epginfo.cpp proginfo.cpp stream.cpp tspacket.cpp tsstream.cpp tsstream_stats.cpp
HDRS=cfg.h epginfo.h proginfo.h stream.h table.h ts.h tspacket.h tsstream.h
AUX=Makefile

PACKNAME=project.zip

all: bms2

.PHONY: clean pack

bms2: bms2.cpp tspacket.o tsstream.o tsstream_stats.o epginfo.o proginfo.o cfg.o stream.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) bms2.cpp tspacket.o epginfo.o tsstream.o tsstream_stats.o proginfo.o stream.o cfg.o -o $@

pack:
	zip -R $(PACKNAME) $(SRCS) $(HDRS) $(AUX)

clean:
	@rm -f *.o bms2 $(PACKNAME)

