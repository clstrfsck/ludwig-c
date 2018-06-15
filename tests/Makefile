########################################################################
#                                                                      #
#            L      U   U   DDDD   W      W  IIIII   GGGG              #
#            L      U   U   D   D   W    W     I    G                  #
#            L      U   U   D   D   W ww W     I    G   GG             #
#            L      U   U   D   D    W  W      I    G    G             #
#            LLLLL   UUU    DDDD     W  W    IIIII   GGGG              #
#                                                                      #
########################################################################
#                                                                      #
#   Makefile Copyright (C) 2018                                        #
#   Martin Sandiford, Adelaide, Australia                              #
#   All rights reserved.                                               #
#                                                                      #
########################################################################

OBJS =	parray.o

# Either g++ or clang++ should work.
# Tested with clang++ 6.0.0 and g++ 7.3.0
CXX = g++

# Flags used for all builds
AFLAGS = -I. -I.. -Iinclude

# Flags used for code coverage
GFLAGS = -fprofile-arcs -ftest-coverage

# These are debug flags. Works for both g++ and clang++.
DFLAGS = -g -Wall -std=c++14 -DDEBUG -D_GLIBCXX_DEBUG
# These are release flags.
# Works for g++, clang++ doesn't understand -Wno-maybe-uninitialized
RFLAGS = -Wno-maybe-uninitialized -O3 -Wall -std=c++14 $(AFLAGS)

# Actual flags we use
CXXFLAGS = $(AFLAGS) $(GFLAGS) $(DFLAGS)

.PHONY: all clean

all:	tests
	./tests

tests:	$(OBJS) gtest-all.o gtest_main.o
	$(CXX) -o tests $(CXXFLAGS) $(OBJS) gtest-all.o gtest_main.o -lpthread

clean:
	rm -f tests $(OBJS) gtest-all.o gtest_main.o

gtest-all.o:	gtest-all.cc

gtest_main.o:	gtest_main.cc
