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
#include <cstdint>

namespace PNGStego {
/**
 * Adds 'addition' to 'filename', before the file extension if it exists
 * Returns result
 */
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
 * Sets bytes at [dest; dest + size) to zeros
 * (HOPEFULLY)
 */
#if defined (__GNUC__) || defined(__MINGW32__)
void zeroMemory(void *dest, size_t size);
#elif defined(_WIN32)
void* zeroMemory(void *dest, size_t size);
#elif defined (__APPLE__)
errno_t zeroMemory(void *dest, size_t size);
#else
void* zeroMemory(void *dest, size_t size);
#endif


} // namespace PNGStego
#endif