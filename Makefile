OBJS = \
	arrow.o \
	caseditto.o \
	ch.o \
	charcmd.o \
	code.o \
	const.o \
	dfa.o \
	eqsgetrep.o \
	exec.o \
	execimmed.o \
	filesys.o \
	frame.o \
	fyle.o \
	help.o \
	helpfile.o \
	line.o \
	ludwig.o \
	lwgetopt.o \
	mark.o \
	msdos.o \
	newword.o \
	nextbridge.o \
	opsys.o \
	patparse.o \
	quit.o \
	recognize.o \
	screen.o \
	span.o \
	swap.o \
	text.o \
	tpar.o \
	user.o \
	validate.o \
	value.o \
	var.o \
	vdu.o \
	version.o \
	window.o \
	word.o

.cpp.o:
	clang++ -fdiagnostics-color=never -g -c -Wall -std=c++14 -DDEBUG -DWINDOWCHANGE -D_GLIBCXX_DEBUG $*.cpp

all:	ludwig
	echo Done.

ludwig:	$(OBJS)
	clang++ -lncurses -o ludwig $(OBJS)

clean:
	rm -f *.o
