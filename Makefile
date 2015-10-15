CXX = g++
CXXFLAGS = --std=c++11 -O3 -Wall -Wextra
LIBS = -lboost_iostreams -lbz2 -lcryptopp -lpthread -lpng -lz
SOURCES = pngwrapper.cpp helperfunctions.cpp bz2compression.cpp byteencryption.cpp
OBJECTS = $(SOURCES:.cpp=.o)

SRCDIR = ./src/
HEADERS = ./include/

ENCODER = pngstego
DECODER = pngdestego

default: all

all: $(ENCODER) $(DECODER)

$(ENCODER): $(OBJECTS) $(SRCDIR)main-stego.cpp $(HEADERS)bz2compression.h $(HEADERS)byteencryption.h $(HEADERS)helperfunctions.h $(HEADERS)pngwrapper.h
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -o $@ $(SRCDIR)main-stego.cpp $(OBJECTS) $(LIBS)

$(DECODER): $(OBJECTS) $(SRCDIR)main-destego.cpp $(HEADERS)bz2compression.h $(HEADERS)byteencryption.h $(HEADERS)helperfunctions.h $(HEADERS)pngwrapper.h
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -o $@ $(SRCDIR)main-destego.cpp $(OBJECTS) $(LIBS)

pngwrapper.o: $(SRCDIR)pngwrapper.cpp $(HEADERS)bz2compression.h $(HEADERS)byteencryption.h $(HEADERS)helperfunctions.h $(HEADERS)pngwrapper.h
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -c $(SRCDIR)pngwrapper.cpp

helperfunctions.o: $(SRCDIR)helperfunctions.cpp $(HEADERS)helperfunctions.h
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -c $(SRCDIR)helperfunctions.cpp

bz2compression.o: $(SRCDIR)bz2compression.cpp $(HEADERS)bz2compression.h
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -c $(SRCDIR)bz2compression.cpp

byteencryption.o: $(SRCDIR)byteencryption.cpp $(HEADERS)byteencryption.h $(HEADERS)helperfunctions.h
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -c $(SRCDIR)byteencryption.cpp


install: $(ENCODER) $(DECODER)
	cp $(ENCODER) /usr/bin/$(ENCODER)
	cp $(DECODER) /usr/bin/$(DECODER)

uninstall:
	if [ -a /usr/bin/$(ENCODER) ] ; \
	then \
		rm /usr/bin/$(ENCODER) ; \
	fi;
	if [ -a /usr/bin/$(DECODER) ] ; \
	then \
		rm /usr/bin/$(DECODER) ; \
	fi;

.PHONY: clean
clean: 
	rm -f $(OBJECTS) $(ENCODER) $(DECODER)