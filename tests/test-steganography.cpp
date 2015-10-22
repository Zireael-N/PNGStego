//
// Copyright (C) 2015 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include "pngwrapper.h"
#include "bz2compression.h"
#include "test-steganography.h"

#define TEST(name, fn)  std::cout << name;             \
                        ++tests;                       \
                        if (fn()) {                    \
                            std::cout << "PASSED!\n";  \
                            ++successes;               \
                        } else {                       \
                            std::cout << "FAILED!\n";  \
                        }

int main() {
	std::cout << "Initialization: ";
	init();
	std::cout << "Done.\n";

	int tests = 0;
	int successes = 0;

	TEST("Testing encode() with precomputed data...: ", testEncode)
	TEST("Testing decode() with precomputed data...: ", testDecode)
	TEST("Testing decode() with data previously calculated with encode()...: ", testDecodeSelf)

	std::cout << "\nTESTS: " << tests;
	std::cout << "\nPASSED: " << successes;
	std::cout << "\nFAILED: " << tests - successes << std::endl;

	return !(successes == tests);
}

using namespace PNGStego;

PNGFile original;
PNGFile precalculatedContainer;
PNGFile container;

#if defined(_WIN32)
#define TESTDIR ".\\tests\\"
#else
#define TESTDIR "./tests/"
#endif

void init() {
	if (fileExists(TESTDIR "original.png")) {
		original.load(TESTDIR "original.png");
	} else if (fileExists("original.png")) {
		original.load("original.png");
	} else {
		exit(1);
	}

	if (fileExists(TESTDIR "container.png")) {
		precalculatedContainer.load(TESTDIR "container.png");
	} else if (fileExists("container.png")) {
		precalculatedContainer.load("container.png");
	} else {
		exit(1);
	}
}

bool testEncode() {
	container = original;

	// Instead of using a PRNG fill IV and Salt with predefined values
	container.setCSPRNG(std::bind(memset, std::placeholders::_1, 0x7F, std::placeholders::_2));
	container.encode(encodedData, encodedExtension, password);

	return container.getPixels() == precalculatedContainer.getPixels() &&
	       container.getWidth()  == precalculatedContainer.getWidth()  &&
	       container.getHeight() == precalculatedContainer.getHeight();
}

bool testDecode() {
	std::vector<uint8_t> temp1;
	std::string temp2;
	precalculatedContainer.decode(temp1, temp2, password);
	return temp1 == encodedData &&
	       temp2 == encodedExtension;
}

bool testDecodeSelf() {
	std::vector<uint8_t> temp1;
	std::string temp2;
	container.decode(temp1, temp2, password);
	return temp1 == encodedData &&
	       temp2 == encodedExtension;
}