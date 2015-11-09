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
#define WIN32_LEAN_AND_MEAN
#ifndef __MINGW32__
#define NOMINMAX
#endif
#include <Windows.h>
#ifndef __MINGW32__
#undef NOMINMAX
#endif
#undef WIN32_LEAN_AND_MEAN
#elif defined(__GNUC__) || defined (__APPLE__)
#include <cstring>
#endif

namespace PNGStego {
/**
 ** Adds 'addition' to 'filename', before the file extension if it exists
 ** Returns result
 **/
std::string addToFilename(std::string filename, const std::string &addition);

/** Returns extension of a file, without the leading dot */
std::string getExtension(const std::string &filename) noexcept;

/** Returns a filename without an extension */
std::string removeExtension(const std::string &filename) noexcept;

/** Returns a filename without directories */
std::string baseFilename(const std::string &filename) noexcept;

/** Returns a filename without directories and extension */
std::string baseFilenameNoExtension(const std::string &filename) noexcept;

/** Returns the size of a given file */
std::ifstream::pos_type fileSize(const std::string &filename);

/** Converts std::string to std::vector<uint8_t> */
std::vector<uint8_t> stringToVector(const std::string &source) noexcept;

/** If the source ends with '\r' and '\n' symbols, remove them */
void removeLineEndings(std::string &source) noexcept;

/** Returns whether str2 is the ending of the str1 */
bool endsWith(const std::string &str1, const std::string &str2) noexcept;

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
inline void* zeroMemory(void *dest, size_t size) {
	volatile char *t = static_cast<volatile char*>(dest);
	while (size--)
		*t++ = 0;
	return dest;
}
#endif


} // namespace PNGStego
#endif