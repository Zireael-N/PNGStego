//
// Copyright (C) 2015 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __BYTE_ENCRYPTION_H
#define __BYTE_ENCRYPTION_H

#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <cryptopp/whrlpool.h>
#include <cryptopp/pwdbased.h>

const int TAG_SIZE = 12;

namespace PNGStego {
namespace Encryption {
/**
 * Generates a hash of your key using PBKDF2 with given salt
 * Then encrypts data stored in the given std::vector with both AES and Serpent, using that hash and given IV
 * Returns an std::vector with encrypted data
 */
std::vector<uint8_t> encrypt(const std::vector<uint8_t> &source, const std::string &key, const std::vector<byte> &iv, const std::vector<byte> &salt);

/**
 * Generates a hash of your key using PBKDF2 with given salt
 * Then decrypts data stored in the given std::vector with both AES and Serpent, using that hash and given IV
 * Returns an std::vector with decrypted data
 */
std::vector<uint8_t> decrypt(const std::vector<uint8_t> &source, const std::string &key, const std::vector<byte> &iv, const std::vector<byte> &salt);

/** Encrypts data stored in the given std::vector with AES (Rijndael) using CBC mode and given key + IV. */
std::vector<uint8_t> AESEncrypt(const std::vector<uint8_t> &source, const byte *key, size_t keylength, const byte *iv, size_t ivlength);

/** Decrypts data stored in the given std::vector with AES (Rijndael) using CBC mode and given key + IV. */
std::vector<uint8_t> AESDecrypt(const std::vector<uint8_t> &source, const byte *key, size_t keylength, const byte *iv, size_t ivlength);

/** Encrypts data stored in the given std::vector with Serpent using CBC mode and given key + IV. */
std::vector<uint8_t> SerpentEncrypt(const std::vector<uint8_t> &source, const byte *key, size_t keylength, const byte *iv, size_t ivlength);

/** Decrypts data stored in the given std::vector with Serpent using CBC mode and given key + IV. */
std::vector<uint8_t> SerpentDecrypt(const std::vector<uint8_t> &source, const byte *key, size_t keylength, const byte *iv, size_t ivlength);

/**
 * Generates a hash of your key using PBKDF2 with the given salt
 * Uses 2nd template argument to determine iterations of Whirlpool
 * (500 000 by default)
 */
template <int hashSize, int iterations = 500000>
std::array<byte, hashSize> hashKey(const std::string &key, const std::vector<byte> &salt) {
	std::array<byte, hashSize> derived;

	CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::Whirlpool> pbkdf2;
	pbkdf2.DeriveKey(derived.data(), derived.size(), 0, reinterpret_cast<const byte *>(key.data()), key.size(), salt.data(), salt.size(), iterations);

	return derived;
}

} // namespace Encryption
} // namespace PNGStego
#endif