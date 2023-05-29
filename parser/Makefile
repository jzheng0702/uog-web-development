CFLAGS = -Wall -g -std=c11
APP = ../
INC = include/
SRC = src/
XML_PATH = /usr/local/opt/libxml2/include/libxml2

#UNAME Shell Variable
UNAME_S := $(shell uname -s)

# OS X Dependant Variables
ifeq ($(UNAME_S), Darwin)
    LIB := sharedLib.dylib
# GNU/Linux Depedant Variables
else ifeq ($(UNAME_S), Linux)
    LIB := sharedLib.so
endif

# Standard common makefile
parser: $(APP)$(LIB)

$(APP)$(LIB): $(APP)SVGParser.o $(APP)LinkedListAPI.o
	gcc -shared -o $(APP)$(LIB) $(APP)SVGParser.o $(APP)LinkedListAPI.o -lxml2 -lm

$(APP)SVGParser.o: $(SRC)SVGParser.c $(INC)LinkedListAPI.h $(INC)SVGParser.h
	gcc -c -fpic $(CFLAGS) -I$(XML_PATH) -I$(INC) $(SRC)SVGParser.c -o $(APP)SVGParser.o

$(APP)LinkedListAPI.o: $(SRC)LinkedListAPI.c $(INC)LinkedListAPI.h
	gcc -c -fpic $(CFLAGS) -I$(INC) $(SRC)LinkedListAPI.c -o $(APP)LinkedListAPI.o

clean:
	rm $(APP)*.o $(APP)$(LIB)
