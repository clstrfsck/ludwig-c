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

OBJS =	arrow.o		caseditto.o	ch.o		charcmd.o	\
	code.o		const.o		dfa.o		eqsgetrep.o	\
	exec.o		execimmed.o	filesys.o	frame.o		\
	fyle.o		help.o		helpfile.o	line.o		\
	ludwig.o	lwgetopt.o	mark.o		newword.o	\
	nextbridge.o	opsys.o		patparse.o	quit.o		\
	recognize.o	screen.o	span.o		swap.o		\
	sys_linux.o	text.o		tpar.o		user.o		\
	validate.o	value.o		var.o		vdu.o		\
	version.o	window.o	word.o

# Either g++ or clang++ should work.
# Tested with clang++ 6.0.0 and g++ 7.3.0
CXX = g++
CC  = gcc

ifdef NDEBUG
# These are release flags.
# Works for g++, clang++ doesn't understand -Wno-maybe-uninitialized
CXXFLAGS = -Wno-maybe-uninitialized -O3 -Wall -std=c++14 -fdiagnostics-color=never
CFLAGS   = -O3 -Wall -DMKSTEMP
else
# These are debug flags. Works for both g++ and clang++.
DEFS     = -DDEBUG -D_GLIBCXX_DEBUG
CXXFLAGS = -g -Wall -std=c++14 $(DEFS) -fdiagnostics-color=never
CFLAGS   = -g -Wall -DMKSTEMP
endif

.PHONY: all tests clean

all:	ludwig ludwighlp.idx ludwignewhlp.idx
	echo Done.

ludwig:	$(OBJS)
	$(CXX) $(CXXFLAGS) -o ludwig $(OBJS) -lncurses

ludwighlpbld:	ludwighlpbld.cpp
	$(CXX) $(CXXFLAGS) -o ludwighlpbld ludwighlpbld.cpp

ludwighlp.idx:  ludwighlpbld ludwighlp.t
	./ludwighlpbld ludwighlp.t ludwighlp.idx

ludwignewhlp.idx:  ludwighlpbld ludwignewhlp.t
	./ludwighlpbld ludwignewhlp.t ludwignewhlp.idx

tests:
	$(MAKE) -C tests all

clean:
	rm -f $(OBJS) ludwighlpbld ludwighlp.idx ludwignewhlp.idx
	$(MAKE) -C tests $@

arrow.o: arrow.cpp arrow.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h var.h vdu.h line.h mark.h text.h screen.h
caseditto.o: caseditto.cpp caseditto.h type.h const.h parray.h prange.h \
 perange.h penumset.h prangeset.h ch.h var.h vdu.h mark.h text.h screen.h
charcmd.o: charcmd.cpp charcmd.h type.h const.h parray.h prange.h \
 perange.h penumset.h prangeset.h ch.h var.h vdu.h mark.h text.h screen.h
ch.o: ch.cpp ch.h type.h const.h parray.h prange.h perange.h penumset.h \
 prangeset.h
code.o: code.cpp code.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h var.h vdu.h exec.h line.h mark.h tpar.h frame.h \
 screen.h
const.o: const.cpp const.h
dfa.o: dfa.cpp dfa.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h var.h screen.h
eqsgetrep.o: eqsgetrep.cpp eqsgetrep.h type.h const.h parray.h prange.h \
 perange.h penumset.h prangeset.h ch.h var.h dfa.h mark.h text.h screen.h \
 charcmd.h patparse.h recognize.h
exec.o: exec.cpp exec.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h var.h vdu.h code.h fyle.h help.h line.h mark.h \
 quit.h span.h swap.h text.h tpar.h user.h word.h arrow.h frame.h opsys.h \
 screen.h window.h charcmd.h newword.h validate.h caseditto.h eqsgetrep.h \
 nextbridge.h
execimmed.o: execimmed.cpp execimmed.h type.h const.h parray.h prange.h \
 perange.h penumset.h prangeset.h var.h vdu.h code.h exec.h fyle.h line.h \
 mark.h quit.h text.h screen.h
filesys.o: filesys.cpp filesys.h type.h const.h parray.h prange.h \
 perange.h penumset.h prangeset.h sys.h screen.h lwgetopt.h
frame.o: frame.cpp frame.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h ch.h dfa.h var.h vdu.h line.h mark.h span.h \
 tpar.h user.h screen.h version.h
fyle.o: fyle.cpp fyle.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h ch.h var.h vdu.h exec.h line.h mark.h tpar.h \
 screen.h filesys.h
help.o: help.cpp help.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h ch.h var.h screen.h helpfile.h
helpfile.o: helpfile.cpp type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h
line.o: line.cpp line.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h ch.h var.h screen.h
ludwig.o: ludwig.cpp var.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h sys.h vdu.h exec.h fyle.h quit.h user.h frame.h \
 value.h screen.h filesys.h execimmed.h
lwgetopt.o: lwgetopt.cpp lwgetopt.h
mark.o: mark.cpp mark.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h var.h screen.h
newword.o: newword.cpp newword.h type.h const.h parray.h prange.h \
 perange.h penumset.h prangeset.h var.h line.h mark.h text.h
nextbridge.o: nextbridge.cpp nextbridge.h type.h const.h parray.h \
 prange.h perange.h penumset.h prangeset.h var.h mark.h
opsys.o: opsys.cpp opsys.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h ch.h line.h filesys.h
parray.o: parray.cpp type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h
patparse.o: patparse.cpp patparse.h type.h const.h parray.h prange.h \
 perange.h penumset.h prangeset.h var.h tpar.h screen.h
quit.o: quit.cpp quit.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h sys.h var.h vdu.h fyle.h mark.h screen.h
recognize.o: recognize.cpp recognize.h type.h const.h parray.h prange.h \
 perange.h penumset.h prangeset.h var.h
screen.o: screen.cpp screen.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h var.h vdu.h line.h
span.o: span.cpp span.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h var.h code.h fyle.h line.h mark.h screen.h
swap.o: swap.cpp swap.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h var.h mark.h text.h
sys_linux.o: sys_linux.cpp sys.h
text.o: text.cpp text.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h var.h vdu.h line.h mark.h screen.h
tpar.o: tpar.cpp tpar.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h ch.h sys.h var.h span.h screen.h
user.o: user.cpp user.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h sys.h var.h vdu.h code.h mark.h span.h text.h \
 tpar.h screen.h
validate.o: validate.cpp validate.h type.h const.h parray.h prange.h \
 perange.h penumset.h prangeset.h var.h screen.h
value.o: value.cpp value.h var.h type.h const.h parray.h prange.h \
 perange.h penumset.h prangeset.h
var.o: var.cpp var.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h
vdu.o: vdu.cpp vdu.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h sys.h
version.o: version.cpp version.h
window.o: window.cpp window.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h var.h vdu.h line.h mark.h frame.h screen.h
word.o: word.cpp word.h type.h const.h parray.h prange.h perange.h \
 penumset.h prangeset.h var.h line.h mark.h text.h screen.h
zz.o: zz.cpp
