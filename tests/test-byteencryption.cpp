//
// Copyright (C) 2015 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <vector>
#include <random>
#include <array>
#include "byteencryption.h"
#include "test-byteencryption.h"

#define TEST(name, fn)  std::cout << name;             \
                        ++tests;                       \
                        if (fn()) {                    \
                            std::cout << "PASSED!\n";  \
                            ++successes;               \
                        } else {                       \
                            std::cout << "FAILED!\n";  \
                        }

int main() {
	std::cout << "Initializing IV, salt and a derived key: ";
	init();
	std::cout << "Done.\n";

	int tests = 0;
	int successes = 0;

	TEST("Testing AESEncrypt() with precomputed data...: ", testAESEncryption)
	TEST("Testing SerpentEncrypt() with precomputed data...: ", testSerpentEncryption)
	TEST("Testing AESDecrypt() with precomputed data...: ", testAESDecryption)
	TEST("Testing SerpentDecrypt() with precomputed data...: ", testSerpentDecryption)
	TEST("Comparing derived keys with precomputed ones...: ", testPBKDF2)
	TEST("Testing double encryption with precomputed data...: ", testDoubleEncryption)
	TEST("Testing double decryption with precomputed data...: ", testDoubleDecryption)
	TEST("Testing (de-/en-)cryption with random data...: ", testWithRandomData)

	std::cout << "\nTESTS: " << tests;
	std::cout << "\nPASSED: " << successes;
	std::cout << "\nFAILED: " << tests - successes << std::endl;

	return !(successes == tests);
}

using namespace PNGStego::Encryption;

std::vector<uint8_t> IV;
std::vector<uint8_t> salt;
std::array<uint8_t, 64> hashedKey;

void init() {
	const int IV_BYTES = 12;       // 96 bits
	const int SALT_BYTES = 16;     // 128 bits

	IV.resize(IV_BYTES);
	for (int i = 0; i < IV_BYTES; ++i) {
		IV[i] = static_cast<uint8_t>(i);
	}

	salt.resize(SALT_BYTES);
	for (int i = 0; i < SALT_BYTES; ++i) {
		salt[i] = static_cast<uint8_t>(SALT_BYTES - i);
	}

	hashedKey = hashKey<64>(password, salt);
}

bool testAESEncryption() {
	std::vector<uint8_t> temp = AESEncrypt(originalData, hashedKey.data(), hashedKey.size() - 32, IV.data(), IV.size());
	return temp == AESEncrypted;
}

bool testSerpentEncryption() {
	std::vector<uint8_t> temp = SerpentEncrypt(originalData, hashedKey.data() + 32, hashedKey.size() - 32, IV.data(), IV.size());
	return temp == SerpentEncrypted;
}

bool testAESDecryption() {
	std::vector<uint8_t> temp = AESDecrypt(AESEncrypted, hashedKey.data(), hashedKey.size() - 32, IV.data(), IV.size());
	return temp == originalData;
}

bool testSerpentDecryption() {
	std::vector<uint8_t> temp = SerpentDecrypt(SerpentEncrypted, hashedKey.data() + 32, hashedKey.size() - 32, IV.data(), IV.size());
	return temp == originalData;
}

bool testPBKDF2() {
	std::array<uint8_t, 64> temp = hashKey<64, 100000>(password, salt);
	return temp == hashedPassword100k &&
	       hashedKey == hashedPassword500k;
}

bool testDoubleEncryption() {
	std::vector<uint8_t> temp = encrypt(originalData, password, IV, salt);
	return temp == doubleEncrypted;
}

bool testDoubleDecryption() {
	std::vector<uint8_t> temp = decrypt(doubleEncrypted, password, IV, salt);
	return temp == originalData;
}

bool testWithRandomData() {
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<size_t> randomSize(100, 200);
	std::uniform_int_distribution<uint8_t>  randomData(0, 0xFF);


	std::vector<uint8_t> container;
	std::vector<uint8_t> encryptedData;
	const int iterations = 100;
	for (int i = 0; i < iterations; ++i) {
		size_t size = randomSize(mt);
		container.resize(size);
		for (size_t j = 0; j < size; ++j) {
			container[j] = randomData(mt);
		}
		encryptedData = SerpentEncrypt(container, hashedKey.data() + 32, hashedKey.size() - 32, IV.data(), IV.size());
		encryptedData = AESEncrypt(encryptedData, hashedKey.data(), hashedKey.size() - 32, IV.data(), IV.size());
		std::vector<uint8_t> decryptedData;
		decryptedData = AESDecrypt(encryptedData, hashedKey.data(), hashedKey.size() - 32, IV.data(), IV.size());
		decryptedData = SerpentDecrypt(decryptedData, hashedKey.data() + 32, hashedKey.size() - 32, IV.data(), IV.size());
		if (decryptedData != container)
			return false;
	}
	return true;
}