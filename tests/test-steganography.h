//
// Copyright (C) 2015-2016 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdint>
#include <vector>
#include <string>

void init();
bool testEncode();
bool testDecode();
bool testDecodeSelf();

const std::string password = "StrongPasswordNotReally";

const std::vector<uint8_t> encodedData = { 'P', 'N', 'G', 'S', 't', 'e', 'g', 'o' };
const std::string encodedExtension = "txt";

inline bool fileExists(const std::string& name) {
	std::ifstream f(name);
	return f.good();
}