PNAME=cstlemma
VERSION=2
MINOR_NUMBER=1

PNAMESTATIC=$(PNAME)s
PNAMEDYNAMIC=$(PNAME)
PNAMEDYNAMICLIB=$(PNAME)dl
LINKERNAME=lib$(PNAME).so
SONAME=$(LINKERNAME).$(VERSION)
REALNAME=$(SONAME).$(MINOR_NUMBER)


#GCCINC+=-I../../src

CC=/usr/bin/g++ -O3 -Wall -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align\
	-Wwrite-strings -Wconversion -Waggregate-return -Wstrict-prototypes\
	-Wlong-long -Woverloaded-virtual
# 	-Wtraditional -Wmissing-prototypes
#       -Wmissing-declarations -Wredundant-decls -Wnested-externs
#	-Winline 
CC=/usr/bin/g++ -O3 -Wall

# -fPIC or -fpic: enable 'position independent code' generation. Necessary for shared libs
# -fpic may generate smaller and faster code, but will have platform-dependent limitations
PIC=-fPIC
#PIC=

CCLINKSTATIC=/usr/bin/g++ -static
CCLINKDYNAMIC=/usr/bin/g++ 
# Linux:
CCCREATELIB=/usr/bin/g++ -shared -Wl,-soname,$(SONAME)
# Solaris gcc 2.95.3:
#CCCREATELIB=g++ -shared

#DEBUG=-g
DEBUG=

# Solaris:
#GCCLINK=-L/usr/local/lib/gcc-lib/sparc-sun-solaris2.8/2.95.3 -L/usr/local/lib -lstdc++
# Linux:
#GCCLINK=-L/usr/local/lib -lstdc++
GCCLINK=-L/usr/lib -lstdc++

RM=rm -f

LEMMATISERSRC=applyrules.cpp\
	argopt.cpp\
	basefrm.cpp\
	basefrmpntr.cpp\
	caseconv.cpp\
	dictionary.cpp\
	field.cpp\
	flex.cpp\
	freqfile.cpp\
	function.cpp\
	functiontree.cpp\
	hash.cpp\
	lemmatiser.cpp\
	lemmtags.cpp\
	lext.cpp\
	makedict.cpp\
	option.cpp\
	outputclass.cpp\
	readfreq.cpp\
	readlemm.cpp\
	tags.cpp\
	text.cpp\
	word.cpp\
	letter.cpp\
	letterfunc.cpp\
	utf8func.cpp

CSTLEMMASRC=cstlemma.cpp


LEMMATISEROBJS=applyrules.o\
	argopt.o\
	basefrm.o\
	basefrmpntr.o\
	caseconv.o\
	dictionary.o\
	field.o\
	flex.o\
	freqfile.o\
	function.o\
	functiontree.o\
	hash.o\
	lemmatiser.o\
	lemmtags.o\
	lext.o\
	makedict.o\
	option.o\
	outputclass.o\
	readfreq.o\
	readlemm.o\
	tags.o\
	text.o\
	word.o\
	letter.o\
	letterfunc.o\
	utf8func.o


CSTLEMMAOBJS=cstlemma.o

#                               #
#   Dependencies start here.    #
#                               #


$(PNAMEDYNAMIC): $(CSTLEMMAOBJS) $(LEMMATISEROBJS)
	$(CCLINKDYNAMIC) $(CSTLEMMAOBJS) $(LEMMATISEROBJS) -o $@ $(GCCLINK)

$(PNAMESTATIC): $(CSTLEMMAOBJS) $(LEMMATISEROBJS)
	$(CCLINKSTATIC) $(CSTLEMMAOBJS) $(LEMMATISEROBJS) -o $@ $(GCCLINK)

$(REALNAME) : $(LEMMATISEROBJS)
	$(CCCREATELIB) -o $@ $(LEMMATISEROBJS)
	ln -sf $(REALNAME) $(SONAME)
	ln -sf $(SONAME) $(LINKERNAME)

$(PNAMEDYNAMICLIB): $(REALNAME) $(CSTLEMMAOBJS)
	$(CCLINKDYNAMIC) $(CSTLEMMAOBJS) -L. -l $(PNAME) -o $@ $(GCCLINK)


all: $(PNAMESTATIC) $(PNAMEDYNAMIC) $(REALNAME) $(PNAMEDYNAMICLIB)

.cpp.o:
	$(CC) $(PIC) $(DEBUG) $(GCCINC) -c $(CSTLEMMASRC) $(LEMMATISERSRC)

#.cpp.o:
#	$(CC) $(PIC) $(DEBUG) $(GCCINC) -c $?

clean:
	$(RM) *.o
	$(RM) $(LINKERNAME)
	$(RM) $(SONAME)
	$(RM) $(REALNAME)
