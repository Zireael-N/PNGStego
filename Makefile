CXX = g++
CXXFLAGS = --std=c++11 -O3 -Wall -Wextra
LIBS = -lboost_iostreams -lbz2 -lcryptopp -lpthread -lpng -lz
SOURCES = pngwrapper.cpp helperfunctions.cpp bz2compression.cpp byteencryption.cpp
OBJECTS = $(SOURCES:.cpp=.o)

SRCDIR = ./src/
HEADERS = ./include/

TESTSDIR = ./tests/
TESTSLIBS = -lboost_iostreams -lbz2 -lcryptopp -lpthread -lpng -lz

ENCODER = pngstego
DECODER = pngdestego

default: all

all: $(ENCODER) $(DECODER)

$(ENCODER): $(OBJECTS) $(SRCDIR)main-stego.cpp $(HEADERS)bz2compression.h $(HEADERS)byteencryption.h $(HEADERS)helperfunctions.h $(HEADERS)pngwrapper.h $(HEADERS)pngstegoversion.h
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -o $@ $(SRCDIR)main-stego.cpp $(OBJECTS) $(LIBS)

$(DECODER): $(OBJECTS) $(SRCDIR)main-destego.cpp $(HEADERS)bz2compression.h $(HEADERS)byteencryption.h $(HEADERS)helperfunctions.h $(HEADERS)pngwrapper.h $(HEADERS)pngstegoversion.h
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -o $@ $(SRCDIR)main-destego.cpp $(OBJECTS) $(LIBS)

pngwrapper.o: $(SRCDIR)pngwrapper.cpp $(HEADERS)bz2compression.h $(HEADERS)byteencryption.h $(HEADERS)helperfunctions.h $(HEADERS)pngwrapper.h $(HEADERS)pngstegoversion.h
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

clean-test:
	rm -f $(TESTSDIR)test-byteencryption $(TESTSDIR)test-helperfunctions $(TESTSDIR)test-bz2compression $(TESTSDIR)test-steganography

test: $(TESTSDIR)test-helperfunctions $(TESTSDIR)test-bz2compression $(TESTSDIR)test-byteencryption $(TESTSDIR)test-steganography
	$(TESTSDIR)test-helperfunctions
	$(TESTSDIR)test-bz2compression
	$(TESTSDIR)test-byteencryption
	$(TESTSDIR)test-steganography

$(TESTSDIR)test-helperfunctions: $(TESTSDIR)test-helperfunctions.cpp $(HEADERS)helperfunctions.h helperfunctions.o
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -o $(TESTSDIR)test-helperfunctions $(TESTSDIR)test-helperfunctions.cpp helperfunctions.o $(TESTSLIBS)

$(TESTSDIR)test-bz2compression: $(TESTSDIR)test-bz2compression.cpp $(TESTSDIR)test-bz2compression.h $(HEADERS)bz2compression.h bz2compression.o
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -o $(TESTSDIR)test-bz2compression $(TESTSDIR)test-bz2compression.cpp bz2compression.o $(TESTSLIBS)

$(TESTSDIR)test-byteencryption: $(TESTSDIR)test-byteencryption.cpp $(TESTSDIR)test-byteencryption.h $(HEADERS)byteencryption.h byteencryption.o helperfunctions.o
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -o $(TESTSDIR)test-byteencryption $(TESTSDIR)test-byteencryption.cpp byteencryption.o helperfunctions.o $(TESTSLIBS)

$(TESTSDIR)test-steganography: $(TESTSDIR)test-steganography.cpp $(TESTSDIR)test-steganography.h $(HEADERS)pngwrapper.h $(OBJECTS)
	$(CXX) $(CXXFLAGS) -I$(HEADERS) -o $(TESTSDIR)test-steganography $(TESTSDIR)test-steganography.cpp $(OBJECTS) $(TESTSLIBS)