//
// Copyright (C) 2015 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __HELPER_FUNCTIONS_H
#define __HELPER_FUNCTIONS_H

#include <string>
#include <vector>
#include <fstream>

#ifdef _WIN32
#include <Windows.h>
#elif defined(__GNUC__) || defined (__APPLE__)
#include <cstring>
#endif

namespace PNGStego {
/**
 ** Adds 'addition' to 'filename', before the file extension if it exists
 ** Returns result
 **/
std::string addToFilename(std::string filename, const std::string& addition);

/** Returns extension of a file, without the leading dot */
std::string getExtension(const std::string &filename);

/** Cuts all the directories, returns result */
std::string shortenFilename(const std::string &filename);

/** Returns the size of a given file */
std::ifstream::pos_type fileSize(const std::string &filename);

/** Converts std::string to std::vector<uint8_t> */
std::vector<uint8_t> stringToVector(const std::string &source);

void cutLineEndings(std::string &source);

/**
 ** Sets bytes at [dest; dest + size) to zeros
 ** (HOPEFULLY)
 **/
#if defined (__GNUC__) || defined (__MINGW32__)
inline void zeroMemory(void *dest, size_t size) {
	memset(dest, 0x00, size);
	asm volatile ("" : : : "memory");
}
#elif defined(_WIN32)
FORCEINLINE void* zeroMemory(void *dest, size_t size) {
	return SecureZeroMemory(dest, size);
}
#elif defined (__APPLE__)
inline errno_t zeroMemory(void *dest, size_t size) {
	return memset_s(dest, size, 0x00, size);
}
#else
void* zeroMemory(void *dest, size_t size) {
	volatile char *t = static_cast<volatile char*>(dest);
	while (size--)
		*t++ = 0;
	return dest;
}
#endif


} // namespace PNGStego
#endif