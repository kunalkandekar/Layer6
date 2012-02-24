#include ../makefile.common
#
# Makes layer6
#

CFLAGS  = -g -O3 -Wall
#CFLAGS  = -g -Wall  #uncomment for debugging
LDFLAGS = -shared -fPIC
LDFLAGS = -shared -fPIC
LIBEXT  = so
SRC     =  
OBJ     = $(SRC:.c,.cpp=.o)
CC      = gcc
CPPC    = g++
SRCDIR  = ./src/ccpp
OBJDIR  = ./build
BINDIR  = ./dist

#for  MAC OS X, which uses -dynamic instead
LDFLAGS = -dynamiclib 
LIBEXT  = dylib

LINK    = -lm -lpthread

INCLUDE = -I./ -I/usr/include

DEPENDENCIES = #nothing

all: libl6.so l6test l6testcpp

libl6.so: clean layer6.o layer6cpp.o $(DEPENDENCIES)
	$(CPPC) $(LDFLAGS) -o $(BINDIR)/libl6.$(LIBEXT) $(OBJDIR)/layer6.o $(OBJDIR)/layer6cpp.o $(DEPENDENCIES)

l6test: clean layer6.o $(DEPENDENCIES) 
	$(CC) $(CFLAGS)   -o $(BINDIR)/l6test    $(SRCDIR)/l6test.c   -ll6 -L. -L$(BINDIR) $(LINK) $(INCLUDE)

l6testcpp: clean layer6.o  $(DEPENDENCIES) 
	$(CPPC) $(CFLAGS) -o $(BINDIR)/l6testcpp $(SRCDIR)/l6test.cpp -ll6 -L. -L$(BINDIR) $(LINK) $(INCLUDE)

#********************************* Layer 6 ******************************
layer6.o:
	$(CC) $(CFLAGS)   -o $(OBJDIR)/layer6.o    -c $(SRCDIR)/layer6.c   $(INCLUDE)

layer6cpp.o:
	$(CPPC) $(CFLAGS) -o $(OBJDIR)/layer6cpp.o -c $(SRCDIR)/layer6.cpp $(INCLUDE)

#********************************* CLEAN ********************************
clean:
	rm -f $(BINDIR)/l6test $(BINDIR)/l6testcpp $(BINDIR)/*.exe $(BINDIR)/*.dll $(BINDIR)/*.so $(BINDIR)/*.dylib $(OBJDIR)/*.o 
	rm -rf $(OBJDIR)/*.dSYM
