VERSION=4
MINOR_NUMBER=0

BIN=/var/csttools/bin/
LIB=/usr/lib/

PNAME=cstlemma

PNAMESTATIC=$(PNAME)s
PNAMEDYNAMIC=$(BIN)$(PNAME)
PNAMEDYNAMICLIB=$(PNAME)dl
LINKERNAME=$(LIB)lib$(PNAME).so
SONAME=$(LINKERNAME).$(VERSION)
REALNAME=$(SONAME).$(MINOR_NUMBER)

PNAME2=$(BIN)allinone
PNAME3=$(BIN)makedictionary
PNAME4=$(BIN)makesuffixrules
PNAME5=$(BIN)makeaffixrules
PNAME6=$(BIN)combiflex



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

CSTLEMMASRC=cstlemma.cpp

CSTLEMMAOBJ=cstlemma.o

COMMONSRC=\
	argopt.cpp\
	caseconv.cpp\
	lemmatiser.cpp\
	letter.cpp\
	letterfunc.cpp\
	option.cpp\
	utf8func.cpp

LEMMATISESRC=\
	applyrules.cpp\
	basefrm.cpp\
	basefrmpntr.cpp\
	dictionary.cpp\
	field.cpp\
	flattext.cpp\
	flex.cpp\
	function.cpp\
	functiontree.cpp\
	hash.cpp\
	lemmatise.cpp\
	lemmtags.cpp\
	lext.cpp\
	outputclass.cpp\
	sgmltag.cpp\
	tags.cpp\
	text.cpp\
	word.cpp\
	XMLtext.cpp

MAKEDICTSRC=\
	dictionary.cpp\
	freqfile.cpp\
	makedict.cpp\
	readfreq.cpp\
	readlemm.cpp\
	sgmltag.cpp

MAKESUFFIXRULESSRC=\
	flex.cpp\
	makesuffixflex.cpp\
	readlemm.cpp
				
MAKEAFFIXRULESSRC=\
	applyaffrules.cpp\
	comp.cpp\
	graph.cpp\
	isofunc.cpp\
	letter.cpp\
	letterfunc.cpp\
	simil.cpp\
	utf8func.cpp
				
LEMMATISERSRC=\
	applyrules.cpp\
	basefrm.cpp\
	basefrmpntr.cpp\
	dictionary.cpp\
	field.cpp\
	flattext.cpp\
	flex.cpp\
	freqfile.cpp\
	function.cpp\
	functiontree.cpp\
	hash.cpp\
	lemmatise.cpp\
	lemmtags.cpp\
	lext.cpp\
	makedict.cpp\
	makesuffixflex.cpp\
	outputclass.cpp\
	readfreq.cpp\
	readlemm.cpp\
	sgmltag.cpp\
	tags.cpp\
	text.cpp\
	word.cpp\
	XMLtext.cpp

COMBIFLEXSRC=combiflex.cpp

COMBIFLEXOBJ=combiflex.o

COMMONOBJ=\
	argopt.o\
	caseconv.o\
	lemmatiser.o\
	letter.o\
	letterfunc.o\
	option.o\
	utf8func.o

LEMMATISEOBJ=\
	applyrules.o\
	basefrm.o\
	basefrmpntr.o\
	dictionary.o\
	field.o\
	flattext.o\
	flex.o\
	function.o\
	functiontree.o\
	hash.o\
	lemmatise.o\
	lemmtags.o\
	lext.o\
	outputclass.o\
	sgmltag.o\
	tags.o\
	text.o\
	word.o\
	XMLtext.o

MAKEDICTOBJ=\
	dictionary.o\
	freqfile.o\
	makedict.o\
	readfreq.o\
	readlemm.o\
	sgmltag.o

MAKESUFFIXRULESOBJ=\
	flex.o\
	makesuffixflex.o\
	readlemm.o

MAKEAFFIXRULESOBJ=\
	applyaffrules.o\
	comp.o\
	graph.o\
	isofunc.o\
	letter.o\
	letterfunc.o\
	simil.o\
	utf8func.o
				
LEMMATISEROBJ=\
	applyrules.o\
	basefrm.o\
	basefrmpntr.o\
	dictionary.o\
	field.o\
	flattext.o\
	flex.o\
	freqfile.o\
	function.o\
	functiontree.o\
	hash.o\
	lemmatise.o\
	lemmtags.o\
	lext.o\
	makedict.o\
	makesuffixflex.o\
	outputclass.o\
	readfreq.o\
	readlemm.o\
	sgmltag.o\
	tags.o\
	text.o\
	word.o\
	XMLtext.o

#                               #
#   Dependencies start here.    #
#                               #


$(PNAMEDYNAMIC): $(CSTLEMMASRC) $(COMMONSRC) $(LEMMATISESRC)
	$(CC) $(PIC) $(DEBUG) $(GCCINC) -DPROGLEMMATISE -c $(CSTLEMMASRC) $(COMMONSRC) $(LEMMATISESRC)
	$(CCLINKDYNAMIC) $(CSTLEMMAOBJ) $(COMMONOBJ) $(LEMMATISEOBJ) -o $@ $(GCCLINK)

$(PNAME2): $(CSTLEMMAOBJ) $(COMMONOBJ) $(LEMMATISEROBJ)
	$(CCLINKDYNAMIC) $(CSTLEMMAOBJ) $(COMMONOBJ) $(LEMMATISEROBJ) -o $@ $(GCCLINK)

$(PNAME3): $(CSTLEMMASRC) $(COMMONSRC) $(MAKEDICTSRC)
	$(CC) $(PIC) $(DEBUG) $(GCCINC) -DPROGMAKEDICT -c $(CSTLEMMASRC) $(COMMONSRC) $(MAKEDICTSRC)
	$(CCLINKDYNAMIC) $(CSTLEMMAOBJ) $(COMMONOBJ) $(MAKEDICTOBJ) -o $@ $(GCCLINK)

$(PNAME4): $(CSTLEMMASRC) $(COMMONSRC) $(MAKESUFFIXRULESSRC)
	$(CC) $(PIC) $(DEBUG) $(GCCINC) -DPROGMAKESUFFIXFLEX -c $(CSTLEMMASRC) $(COMMONSRC) $(MAKESUFFIXRULESSRC)
	$(CCLINKDYNAMIC) $(CSTLEMMAOBJ) $(COMMONOBJ) $(MAKESUFFIXRULESOBJ) -o $@ $(GCCLINK)

$(PNAME5): $(MAKEAFFIXRULESSRC)
	$(CC) $(PIC) $(DEBUG) $(GCCINC) -c $(MAKEAFFIXRULESSRC)
	$(CCLINKDYNAMIC) $(MAKEAFFIXRULESOBJ) -o $@ $(GCCLINK)

$(PNAME6): $(COMBIFLEXSRC)
	$(CC) $(PIC) $(DEBUG) $(GCCINC) -c $(COMBIFLEXSRC)
	$(CCLINKDYNAMIC) $(COMBIFLEXOBJ) -o $@ $(GCCLINK)

$(PNAMESTATIC): $(CSTLEMMASRC) $(COMMONSRC) $(LEMMATISESRC)
	$(CC) $(PIC) $(DEBUG) $(GCCINC) -DPROGLEMMATISE -c $(CSTLEMMASRC) $(COMMONSRC) $(LEMMATISESRC)
	$(CCLINKSTATIC) $(CSTLEMMAOBJ) $(COMMONOBJ) $(LEMMATISEOBJ) -o $@ $(GCCLINK)

$(REALNAME) : $(COMMONOBJ) $(LEMMATISEOBJ) $(PNAMEDYNAMIC)
	$(CCCREATELIB) -o $@ $(COMMONOBJ) $(LEMMATISEOBJ)
	ln -sf $(REALNAME) $(SONAME)
	ln -sf $(SONAME) $(LINKERNAME)

$(PNAMEDYNAMICLIB): $(REALNAME) $(CSTLEMMAOBJ)
	$(CCLINKDYNAMIC) $(CSTLEMMAOBJ) -L. -l $(PNAME) -o $@ $(GCCLINK)


all: $(PNAMESTATIC) $(PNAMEDYNAMIC) $(REALNAME) $(PNAMEDYNAMICLIB) $(PNAME2) $(PNAME3) $(PNAME4) $(PNAME5) $(PNAME6)

.cpp.o:
	$(CC) $(PIC) $(DEBUG) $(GCCINC) -c $(CSTLEMMASRC) $(COMMONSRC) $(LEMMATISERSRC)

#.cpp.o:
#	$(CC) $(PIC) $(DEBUG) $(GCCINC) -c $?

clean:
	$(RM) *.o
	$(RM) $(PNAMESTATIC)
	$(RM) $(PNAMEDYNAMIC)
	$(RM) $(PNAMEDYNAMICLIB)
	$(RM) $(LINKERNAME)
	$(RM) $(SONAME)
	$(RM) $(REALNAME)
	$(RM) $(PNAME2)
	$(RM) $(PNAME3)
	$(RM) $(PNAME2)
