//
// Copyright (C) 2015-2016 Zireael (zireael dot nk at gmail dot com)
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

	std::string getExtension(const std::string &filename) noexcept {
		size_t pos = filename.size();
		if (pos == 0)
			return std::string("");
		for (size_t i = pos - 1; ; --i) {
			if (filename[i] == '.' && i > 0 && filename[i - 1] != DIRECTORYDELIM) {
				pos = i + 1;
			}
			if (filename[i] == DIRECTORYDELIM || i == 0)
				break;
		}
		return filename.substr(pos);
	}

	std::string removeExtension(const std::string &filename) noexcept {
		size_t pos = filename.size();
		if (pos == 0)
			return std::string("");
		for (size_t i = pos - 1; ; --i) {
			if (filename[i] == '.' && i > 0 && filename[i - 1] != DIRECTORYDELIM) {
				pos = i;
			}
			if (filename[i] == DIRECTORYDELIM || i == 0)
				break;
		}
		return filename.substr(0, pos);
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

	std::string baseFilename(const std::string &filename) noexcept {
		std::string::size_type pos = filename.rfind(DIRECTORYDELIM);
		if (pos == std::string::npos)
			return filename;

		return filename.substr(pos + 1);
	}

	std::string baseFilenameNoExtension(const std::string &filename) noexcept {
		size_t to = filename.size();
		if (to == 0)
			return filename;
		to -= 1;

		std::string::size_type from = filename.rfind(DIRECTORYDELIM);
		if (from == std::string::npos)
			from = 0;
		else
			from += 1;

		for (size_t i = to - 1; ; --i) {
			if (filename[i] == '.' && i > 0 && filename[i - 1] != DIRECTORYDELIM)
				to = i - 1;
			if (filename[i] == DIRECTORYDELIM || i == 0)
				break;
		}

		return filename.substr(from, to - from + 1);
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

	void removeLineEndings(std::string &source) noexcept {
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

		source.erase(source.begin(), source.end());
		return;
	}

	bool endsWith(const std::string &str1, const std::string &str2) noexcept {
		if (!str2.empty() && str1.length() >= str2.length()) {
			return (str1.compare(str1.length() - str2.length(), str2.length(), str2) == 0);
		}
		else {
			return false;
		}
	}

	std::vector<uint8_t> stringToVector(const std::string &source) noexcept {
		return std::vector<uint8_t>(source.begin(), source.end());
	}

} // namespace PNGStego