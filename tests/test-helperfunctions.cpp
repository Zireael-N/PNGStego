//
// Copyright (C) 2015 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include "helperfunctions.h"

bool testGetExtension();
bool testRemoveExtension();
bool testAddToFilename();
bool testBaseFilename();
bool testBaseFilenameNoExtension();
bool testCutLineEndings();
bool testEndsWith();
bool testStringToVector();

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

	TEST("Testing getExtension()...: ", testGetExtension)
	TEST("Testing removeExtension()...: ", testRemoveExtension)
	TEST("Testing addToFilename()...: ", testAddToFilename)
	TEST("Testing baseFilename()...: ", testBaseFilename)
	TEST("Testing baseFilenameNoExtension()...: ", testBaseFilenameNoExtension)
	TEST("Testing cutLineEndings()...: ", testCutLineEndings)
	TEST("Testing endsWith()...: ", testEndsWith)
	TEST("Testing stringToVector()...: ", testStringToVector)

	std::cout << "\nTESTS: " << tests;
	std::cout << "\nPASSED: " << successes;
	std::cout << "\nFAILED: " << tests - successes << std::endl;

	return !(successes == tests);
}

#if defined(_WIN32)
#define HOMEDIR "C:\\Users\\Test\\"
#else
#define HOMEDIR "/home/test/"
#endif

using namespace PNGStego;

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

bool testCutLineEndings() {
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
		cutLineEndings(temp);
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