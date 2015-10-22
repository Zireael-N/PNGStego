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

#if defined(_WIN32)
#include <boost/nowide/convert.hpp>
#define DIRECTORYDELIM '\\'
#elif defined(__unix__) || defined(__APPLE__)
#define DIRECTORYDELIM '/'
#endif

namespace PNGStego {

	std::string getExtension(const std::string &filename) {
		size_t insertpos = filename.size();
		if (insertpos == 0)
			throw std::invalid_argument("Empty string used as a function parameter");
		for (size_t i = insertpos - 1; ; --i) {
			if (filename[i] == '.' && i > 0 && filename[i - 1] != DIRECTORYDELIM) {
				insertpos = i + 1;
			}
			if (filename[i] == DIRECTORYDELIM || i == 0)
				break;
		}
		return filename.substr(insertpos);
	}

	std::string addToFilename(std::string filename, const std::string &addition) {
		size_t insertpos = filename.size();
		if (insertpos == 0)
			throw std::invalid_argument("Empty string used as a function parameter");
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
			throw std::invalid_argument("Empty string used as a function parameter");
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
#ifdef _WIN32
		HANDLE in = CreateFileW(boost::nowide::widen(filename).c_str(),
		    GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
		    FILE_ATTRIBUTE_NORMAL, NULL);
		if (in==INVALID_HANDLE_VALUE)
			throw std::invalid_argument("Couldn't open " + filename);

		LARGE_INTEGER size;
		if (!GetFileSizeEx(in, &size))
		{
			CloseHandle(in);
			throw std::runtime_error("Couldn't get the file's size");
		}

		CloseHandle(in);
		return size.QuadPart;
#else
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	if (!in)
		throw std::invalid_argument("Couldn't open " + filename);
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

	bool endsWith(const std::string &str1, const std::string &str2) {
		if (!str2.empty() && str1.length() >= str2.length()) {
			return (0 == str1.compare(str1.length() - str2.length(), str2.length(), str2));
		}
		else {
			return false;
		}
	}

	std::vector<uint8_t> stringToVector(const std::string &source) {
		return std::vector<uint8_t>(source.begin(), source.end());
	}

} // namespace PNGStego