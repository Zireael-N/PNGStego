//
// Copyright (C) 2015 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#include "helperfunctions.h"

#include <iterator>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>

#if defined(_WIN32)
#include <boost/nowide/convert.hpp>
#define DIRECTORYDELIM '\\'
#elif defined(__unix__) || defined(__APPLE__)
#define DIRECTORYDELIM '/'
#endif

#ifdef _WIN32
#include <Windows.h>
#elif defined(__GNUC__) || defined (__APPLE__)
#include <cstring>
#endif

namespace PNGStego {

	std::string getExtension(const std::string &filename) {
		size_t insertpos = filename.size();
		if (insertpos == 0)
			throw std::runtime_error("Empty string used as a function parameter");
		for (size_t i = insertpos - 1; ; --i) {
			if (filename[i] == '.' && i > 0 && filename[i - 1] != DIRECTORYDELIM) {
				insertpos = i + 1;
			}
			if (filename[i] == DIRECTORYDELIM || i == 0)
				break;
		}
		return filename.substr(insertpos);
	}

	std::string addToFilename(std::string filename, const std::string& addition) {
		size_t insertpos = filename.size();
		if (insertpos == 0)
			throw std::runtime_error("Empty string used as a function parameter");
		for (size_t i = insertpos - 1; ; --i) {
			if (filename[i] == '.' && i > 0 && filename[i - 1] != DIRECTORYDELIM)
				insertpos = i;
			if (filename[i] == DIRECTORYDELIM || i == 0)
				break;
		}
		filename.insert(insertpos, addition);
		return filename;
	}

	std::string shortenFilename(const std::string &filename) {
		size_t insertpos = filename.size();
		if (insertpos == 0)
			throw std::runtime_error("Empty string used as a function parameter");
		for (size_t i = insertpos - 1; ; --i) {
			if (filename[i] == DIRECTORYDELIM) {
				insertpos = i + 1;
				break;
			}
			if (i == 0) {
				insertpos = 0;
				break;
			}
		}
		return filename.substr(insertpos);
	}

	std::ifstream::pos_type fileSize(const std::string &filename)
	{
#if defined(__MINGW32__)
		HANDLE in = CreateFileW(boost::nowide::widen(filename).c_str(), GENERIC_READ, 
			FILE_SHARE_READ, NULL, OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL, NULL);
		if (in==INVALID_HANDLE_VALUE)
			throw std::runtime_error("Couldn't open the file");

		LARGE_INTEGER size;
		if (!GetFileSizeEx(in, &size))
		{
			CloseHandle(in);
			throw std::runtime_error("Couldn't get the file's size");
		}

		CloseHandle(in);
		return size.QuadPart;
#else
	#if defined(_WIN32)
		std::ifstream in(boost::nowide::widen(filename), std::ifstream::ate | std::ifstream::binary);
	#else
		std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	#endif
		if (!in)
			throw std::runtime_error("Couldn't open the file");
		return in.tellg();
#endif
	}

	void cutLineEndings(std::string &source) {
		if (source.size() == 0) // empty string
			return;
		size_t pos = source.size() - 1;
		if (source[pos] != '\n' && source[pos] != '\r') // no line endings
			return;

		while (pos != 0) {
			--pos;
			if (source[pos] != '\r' && source[pos] != '\n') {
				source.erase(source.begin() + pos + 1, source.end());
				return;
			}
		}

		if (pos == 0)
			source.erase(source.begin(), source.end());

		return;
	}

	std::vector<uint8_t> stringToVector(const std::string &source) {
		std::vector<uint8_t> result;
		std::copy(source.begin(), source.end(), std::back_inserter(result));
		return result;
	}

#if defined (__GNUC__) || defined (__MINGW32__)
	void zeroMemory(void *dest, size_t size) {
		memset(dest, 0x00, size);
		asm volatile ("" : : : "memory");
	}
#elif defined(_WIN32)
	void* zeroMemory(void *dest, size_t size) {
		return SecureZeroMemory(dest, size);
	}
#elif defined (__APPLE__)
	errno_t zeroMemory(void *dest, size_t size) {
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