//
// Copyright (C) 2015-2016 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __PNGSTEGO_BZ2_COMPRESSION_H
#define __PNGSTEGO_BZ2_COMPRESSION_H

#include <vector>
#include <cstdint>

namespace PNGStego {
namespace bzip2 {

/** Uses bzip2 to compress given data */
std::vector<char> compress(const std::vector<char> &source);

/** Uses bzip2 to decompress given data */
std::vector<char> decompress(const std::vector<char> &source);

/** Uses bzip2 to compress given data */
std::vector<uint8_t> compress(const std::vector<uint8_t> &source);

/** Uses bzip2 to decompress given data */
std::vector<uint8_t> decompress(const std::vector<uint8_t> &source);

} // namespace bzip2
} // namespace PNGStego
#endif