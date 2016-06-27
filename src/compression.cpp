//
// Copyright (C) 2015-2016 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#include "compression.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4244)
#endif

#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace PNGStego {
namespace bzip2 {

	std::vector<char> compress(const std::vector<char> &source) {
		std::vector<char> compressed;
		boost::iostreams::filtering_streambuf< boost::iostreams::output > out;
		out.push(boost::iostreams::bzip2_compressor());
		out.push(std::back_inserter(compressed));
		boost::iostreams::copy(boost::make_iterator_range(source), out);
		return compressed;
	}

	std::vector<char> decompress(const std::vector<char> &source) {
		std::vector<char> decompressed;
		boost::iostreams::filtering_streambuf< boost::iostreams::input > in;
		in.push(boost::iostreams::bzip2_decompressor());
		in.push(boost::make_iterator_range(source));
		boost::iostreams::copy(in, std::back_inserter(decompressed));
		return decompressed;
	}

	std::vector<uint8_t> compress(const std::vector<uint8_t> &source) {
		std::vector<char> temp = compress(std::vector<char>(source.begin(), source.end()));
		return std::vector<uint8_t>(temp.begin(), temp.end());
	}

	std::vector<uint8_t> decompress(const std::vector<uint8_t> &source) {
		std::vector<char> temp = decompress(std::vector<char>(source.begin(), source.end()));
		return std::vector<uint8_t>(temp.begin(), temp.end());
	}

} // namespace bzip2
} // namespace PNGStego