//
// Copyright (C) 2015 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <vector>
#include <random>
#include "bz2compression.h"
#include "test-bz2compression.h"


#define TEST(name, fn)  std::cout << name;             \
                        ++tests;                       \
                        if (fn()) {                    \
                            std::cout << "PASSED!\n";  \
                            ++successes;               \
                        } else {                       \
                            std::cout << "FAILED!\n";  \
                        }

int main() {
	int tests = 0;
	int successes = 0;

	TEST("Testing compress() with precomputed data...: ", testCompress)
	TEST("Testing decompress() with precomputed data...: ", testDecompress)
	TEST("Testing (de-)compress() with random data...: ", testWithRandomData)

	std::cout << "\nTESTS: " << tests;
	std::cout << "\nPASSED: " << successes;
	std::cout << "\nFAILED: " << tests - successes << std::endl;

	return !(successes == tests);
}

using namespace PNGStego::bzip2;

bool testCompress() {
	std::vector<uint8_t> temp = compress(originalData);
	return temp == bz2Data;
}

bool testDecompress() {
	std::vector<uint8_t> temp = decompress(bz2Data);
	return temp == originalData;
}

bool testWithRandomData() {
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<size_t> randomSize(100, 200);
	std::uniform_int_distribution<uint8_t>  randomData(0, 0xFF);


	std::vector<uint8_t> container;
	std::vector<uint8_t> compressedData;
	const int iterations = 100;
	for (int i = 0; i < iterations; ++i) {
		size_t size = randomSize(mt);
		container.resize(size);
		for (size_t j = 0; j < size; ++j) {
			container[j] = randomData(mt);
		}
		compressedData = compress(container);
		std::vector<uint8_t> decompressedData = decompress(compressedData);
		if (decompressedData != container)
			return false;
	}
	return true;
}