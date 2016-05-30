//
// Copyright (C) 2015-2016 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <array>
#include <tuple>
#include <cstring>
#include "pngwrapper.h"
#include "bz2compression.h"
#include "byteencryption.h"
#include "helperfunctions.h"
#include "constants.h"

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
	initCrypto();
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
	TEST("Testing (de-/en-)cryption with random data...: ", testEncryptionWithRandomData)

	TEST("\nTesting compress() with precomputed data...: ", testCompress)
	TEST("Testing decompress() with precomputed data...: ", testDecompress)
	TEST("Testing (de-)compress() with random data...: ", testCompressionWithRandomData)

	TEST("\nTesting getExtension()...: ", testGetExtension)
	TEST("Testing removeExtension()...: ", testRemoveExtension)
	TEST("Testing addToFilename()...: ", testAddToFilename)
	TEST("Testing baseFilename()...: ", testBaseFilename)
	TEST("Testing baseFilenameNoExtension()...: ", testBaseFilenameNoExtension)
	TEST("Testing removeLineEndings()...: ", testRemoveLineEndings)
	TEST("Testing endsWith()...: ", testEndsWith)
	TEST("Testing stringToVector()...: ", testStringToVector)


	std::cout << "\nI/O Initialization: ";
	if (initStego()) {
		std::cout << "Done.\n";

		TEST("Testing encode() with precomputed data...: ", testEncode)
		TEST("Testing decode() with precomputed data...: ", testDecode)
		TEST("Testing decode() with data previously calculated with encode()...: ", testDecodeSelf)
	} else {
		std::cout << "Missing files, can't do the final tests.\n";
		tests += 3;
	}

	std::cout << "\nTESTS: " << tests;
	std::cout << "\nPASSED: " << successes;
	std::cout << "\nFAILED: " << tests - successes << std::endl;

	return !(successes == tests);
}

using namespace PNGStego::Encryption;

std::vector<uint8_t> IV;
std::vector<uint8_t> salt;
std::array<uint8_t, 64> hashedKey;

void initCrypto() {
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

bool testEncryptionWithRandomData() {
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

using namespace PNGStego::bzip2;

bool testCompress() {
    std::vector<uint8_t> temp = compress(originalData);
    return temp == bz2Data;
}

bool testDecompress() {
    std::vector<uint8_t> temp = decompress(bz2Data);
    return temp == originalData;
}

bool testCompressionWithRandomData() {
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

using namespace PNGStego;

#if defined(_WIN32)
#define HOMEDIR "C:\\Users\\Test\\"
#else
#define HOMEDIR "/home/test/"
#endif

bool testGetExtension() {
	const std::pair<std::string, std::string> TestsAndExpectedResults[] = {
	                                                                        { HOMEDIR "",                 ""        },
	                                                                        { HOMEDIR ".gitignore",       ""        },
	                                                                        { HOMEDIR "Makefile",         ""        }, 
	                                                                        { HOMEDIR "image.png",        "png"     }, 
	                                                                        { HOMEDIR "backup.tar.bz2",   "tar.bz2" },
	                                                                        {         "",                 ""        },
	                                                                        {         ".gitignore",       ""        },
	                                                                        {         "Makefile",         ""        }, 
	                                                                        {         "image.png",        "png"     }, 
	                                                                        {         "backup.tar.bz2",   "tar.bz2" },
	                                                                      };
	for (auto &p : TestsAndExpectedResults) {
		std::string temp = getExtension(p.first);
		if (temp != p.second)
			return false;
	}
	return true;
}

bool testRemoveExtension() {
	const std::pair<std::string, std::string> TestsAndExpectedResults[] = {
	                                                                        { HOMEDIR "",                 HOMEDIR ""           },
	                                                                        { HOMEDIR ".gitignore",       HOMEDIR ".gitignore" },
	                                                                        { HOMEDIR "Makefile",         HOMEDIR "Makefile"   }, 
	                                                                        { HOMEDIR "image.png",        HOMEDIR "image"      }, 
	                                                                        { HOMEDIR "backup.tar.bz2",   HOMEDIR "backup"     },
	                                                                        {         "",                         ""           },
	                                                                        {         ".gitignore",               ".gitignore" },
	                                                                        {         "Makefile",                 "Makefile"   }, 
	                                                                        {         "image.png",                "image"      }, 
	                                                                        {         "backup.tar.bz2",           "backup"     },
	                                                                      };
	for (auto &p : TestsAndExpectedResults) {
		std::string temp = removeExtension(p.first);
		if (temp != p.second)
			return false;
	}
	return true;
}

bool testAddToFilename() {
	const std::pair<std::string, std::string> TestsAndExpectedResults[] = {
	                                                                        { HOMEDIR ".gitignore",       HOMEDIR ".gitignore (TEST)"     },
	                                                                        { HOMEDIR "Makefile",         HOMEDIR "Makefile (TEST)"       }, 
	                                                                        { HOMEDIR "image.png",        HOMEDIR "image (TEST).png"      }, 
	                                                                        { HOMEDIR "backup.tar.bz2",   HOMEDIR "backup (TEST).tar.bz2" },
	                                                                        {         ".gitignore",               ".gitignore (TEST)"     },
	                                                                        {         "Makefile",                 "Makefile (TEST)"       }, 
	                                                                        {         "image.png",                "image (TEST).png"      }, 
	                                                                        {         "backup.tar.bz2",           "backup (TEST).tar.bz2" },
	                                                                      };
	for (auto &p : TestsAndExpectedResults) {
		std::string temp = addToFilename(p.first, " (TEST)");
		if (temp != p.second)
			return false;
	}
	return true;
}

bool testBaseFilename() {
	const std::pair<std::string, std::string> TestsAndExpectedResults[] = {
	                                                                        { HOMEDIR "",                 ""               },
	                                                                        { HOMEDIR ".gitignore",       ".gitignore"     },
	                                                                        { HOMEDIR "Makefile",         "Makefile"       }, 
	                                                                        { HOMEDIR "image.png",        "image.png"      }, 
	                                                                        { HOMEDIR "backup.tar.bz2",   "backup.tar.bz2" },
	                                                                        {         ".gitignore",       ".gitignore"     },
	                                                                        {         "Makefile",         "Makefile"       }, 
	                                                                        {         "image.png",        "image.png"      }, 
	                                                                        {         "backup.tar.bz2",   "backup.tar.bz2" },
	                                                                      };
	for (auto &p : TestsAndExpectedResults) {
		std::string temp = baseFilename(p.first);
		if (temp != p.second)
			return false;
	}
	return true;
}

bool testBaseFilenameNoExtension() {
	const std::pair<std::string, std::string> TestsAndExpectedResults[] = {
	                                                                        { HOMEDIR "",                 ""            },
	                                                                        { HOMEDIR ".gitignore",       ".gitignore"  },
	                                                                        { HOMEDIR "Makefile",         "Makefile"    }, 
	                                                                        { HOMEDIR "image.png",        "image"       }, 
	                                                                        { HOMEDIR "backup.tar.bz2",   "backup"      },
	                                                                        {         "",                 ""            },
	                                                                        {         ".gitignore",       ".gitignore"  },
	                                                                        {         "Makefile",         "Makefile"    }, 
	                                                                        {         "image.png",        "image"       }, 
	                                                                        {         "backup.tar.bz2",   "backup"      },
	                                                                      };
	for (auto &p : TestsAndExpectedResults) {
		std::string temp = baseFilenameNoExtension(p.first);
		if (temp != p.second)
			return false;
	}
	return true;
}

bool testRemoveLineEndings() {
	const std::pair<std::string, std::string> TestsAndExpectedResults[] = {
	                                                                        { "",                 ""            },
	                                                                        { "\r\n\r\n\r\n",     ""            },
	                                                                        { "Message\r\n",      "Message"     }, 
	                                                                        { "Message",          "Message"     }, 
	                                                                        { "\r\nMessage\r\n",  "\r\nMessage" },
	                                                                        { "\r\nMessage",      "\r\nMessage" },
	                                                                      };
	for (auto &p : TestsAndExpectedResults) {
		std::string temp = p.first;
		removeLineEndings(temp);
		if (temp != p.second)
			return false;
	}
	return true;
}

bool testEndsWith() {
	const std::tuple<std::string, std::string, bool> TestsAndExpectedResults[] = {
	                                                                               std::make_tuple(HOMEDIR "image.png", ".png",         true),
	                                                                               std::make_tuple(HOMEDIR "image.png", ".jpg",         false),
	                                                                               std::make_tuple(HOMEDIR "image.png", "",             false),
	                                                                               std::make_tuple(HOMEDIR "image.png", "bigimage.png", false),
	                                                                               std::make_tuple(        "",          ".odt",         false),
	                                                                             };
	for (auto &p : TestsAndExpectedResults) {
		bool result = endsWith(std::get<0>(p), std::get<1>(p));
		if (result != std::get<2>(p))
			return false;
	}
	return true;
}

bool testStringToVector() {
	const std::pair<std::string, std::vector<unsigned char> > TestsAndExpectedResults[] = {
	                                                                                        std::make_pair("",           std::vector<unsigned char>({                                             }) ),
	                                                                                        std::make_pair("message",    std::vector<unsigned char>({ 'm', 'e', 's', 's', 'a', 'g', 'e'           }) ),
	                                                                                        std::make_pair("hey there",  std::vector<unsigned char>({ 'h', 'e', 'y', ' ', 't', 'h', 'e', 'r', 'e' }) ),
	                                                                                      };
	for (auto &p : TestsAndExpectedResults) {
		std::vector<unsigned char> temp = stringToVector(p.first);
		if (temp != p.second)
			return false;
	}
	return true;
}

PNGFile original;
PNGFile precalculatedContainer;
PNGFile container;

#if defined(_WIN32)
#define TESTDIR ".\\tests\\"
#else
#define TESTDIR "./tests/"
#endif

bool initStego() {
	if (fileExists(TESTDIR "original.png")) {
		original.load(TESTDIR "original.png");
	} else if (fileExists("original.png")) {
		original.load("original.png");
	} else {
		return false;
	}

	if (fileExists(TESTDIR "container.png")) {
		precalculatedContainer.load(TESTDIR "container.png");
	} else if (fileExists("container.png")) {
		precalculatedContainer.load("container.png");
	} else {
		return false;
	}

	return true;
}

bool testEncode() {
	container = original;

	// Instead of using a PRNG fill IV and Salt with predefined values
	container.setCSPRNG(std::bind(memset, std::placeholders::_1,
	                                0x7F, std::placeholders::_2));
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