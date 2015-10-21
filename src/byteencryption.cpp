//
// Copyright (C) 2015 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#include <vector>
#include <array>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4297)
#endif

#include <cryptopp/filters.h>
#include <cryptopp/aes.h>
#include <cryptopp/serpent.h>
#include <cryptopp/gcm.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "byteencryption.h"
#include "helperfunctions.h"

namespace PNGStego {
namespace Encryption {

	std::vector<uint8_t> encrypt(const std::vector<uint8_t> &source, const std::string &key,
	                             const std::vector<byte> &iv, const std::vector<byte> &salt) {
		constexpr int keylength = CryptoPP::AES::MAX_KEYLENGTH + CryptoPP::Serpent::MAX_KEYLENGTH;

		std::array<byte, keylength> hashedKey = hashKey<keylength>(key, salt);
		std::vector<uint8_t> encrypted;
		encrypted = SerpentEncrypt(source, hashedKey.data() + CryptoPP::AES::MAX_KEYLENGTH,
			                                                  CryptoPP::Serpent::MAX_KEYLENGTH, iv.data(), iv.size());
		encrypted = AESEncrypt(encrypted, hashedKey.data(), CryptoPP::AES::MAX_KEYLENGTH, iv.data(), iv.size());
		PNGStego::zeroMemory(hashedKey.data(), hashedKey.size());

		return encrypted;
	}

	std::vector<uint8_t> decrypt(const std::vector<uint8_t> &source, const std::string &key,
	                             const std::vector<byte> &iv, const std::vector<byte> &salt) {
		constexpr int keylength = CryptoPP::AES::MAX_KEYLENGTH + CryptoPP::Serpent::MAX_KEYLENGTH;

		std::array<byte, keylength> hashedKey = hashKey<keylength>(key, salt);
		std::vector<uint8_t> decrypted;
		decrypted = AESDecrypt(source, hashedKey.data(), CryptoPP::AES::MAX_KEYLENGTH, iv.data(), iv.size());
		decrypted = SerpentDecrypt(decrypted, hashedKey.data() + CryptoPP::AES::MAX_KEYLENGTH,
		                                                        CryptoPP::Serpent::MAX_KEYLENGTH, iv.data(), iv.size());
		PNGStego::zeroMemory(hashedKey.data(), hashedKey.size());

		return decrypted;
	}

	std::vector<uint8_t> AESEncrypt(const std::vector<uint8_t> &source, const byte *key, size_t keylength,
	                                                                    const byte *iv,  size_t ivlength) {
		std::string encrypted;
		CryptoPP::GCM< CryptoPP::AES >::Encryption e;
		e.SetKeyWithIV(key, keylength, iv, ivlength);
		CryptoPP::ArraySource ss(source.data(), source.size(), true,
			new CryptoPP::AuthenticatedEncryptionFilter(e,
				new CryptoPP::StringSink(encrypted), false, TAG_SIZE
				) // AuthenticatedEncryptionFilter
		); // ArraySource

		return PNGStego::stringToVector(encrypted);
	}

	std::vector<uint8_t> AESDecrypt(const std::vector<uint8_t> &source, const byte *key, size_t keylength,
	                                                                    const byte *iv,  size_t ivlength) {
		std::string decrypted;
		CryptoPP::GCM< CryptoPP::AES >::Decryption d;
		d.SetKeyWithIV(key, keylength, iv, ivlength);

		CryptoPP::AuthenticatedDecryptionFilter df(d,
			new CryptoPP::StringSink(decrypted),
			CryptoPP::AuthenticatedDecryptionFilter::DEFAULT_FLAGS,
			TAG_SIZE
		);
		CryptoPP::ArraySource ss(source.data(), source.size(), true,
			new CryptoPP::Redirector(df /*, PASS_EVERYTHING */)
		); // ArraySource

		if (false == df.GetLastResult())
			throw std::runtime_error("The data's corrupted.");

		return PNGStego::stringToVector(decrypted);
	}

	std::vector<uint8_t> SerpentEncrypt(const std::vector<uint8_t> &source, const byte *key, size_t keylength,
	                                                                        const byte *iv,  size_t ivlength) {
		std::string encrypted;
		CryptoPP::GCM< CryptoPP::Serpent >::Encryption e;
		e.SetKeyWithIV(key, keylength, iv, ivlength);
		CryptoPP::ArraySource ss(source.data(), source.size(), true,
			new CryptoPP::AuthenticatedEncryptionFilter(e,
				new CryptoPP::StringSink(encrypted), false, TAG_SIZE
				) // AuthenticatedEncryptionFilter
		); // ArraySource

		return PNGStego::stringToVector(encrypted);
	}

	std::vector<uint8_t> SerpentDecrypt(const std::vector<uint8_t> &source, const byte *key, size_t keylength,
	                                                                        const byte *iv,  size_t ivlength) {
		std::string decrypted;
		CryptoPP::GCM< CryptoPP::Serpent >::Decryption d;
		d.SetKeyWithIV(key, keylength, iv, ivlength);

		CryptoPP::AuthenticatedDecryptionFilter df(d,
			new CryptoPP::StringSink(decrypted),
			CryptoPP::AuthenticatedDecryptionFilter::DEFAULT_FLAGS,
			TAG_SIZE
		);
		CryptoPP::ArraySource ss(source.data(), source.size(), true,
			new CryptoPP::Redirector(df /*, PASS_EVERYTHING */)
		); // ArraySource

		if (false == df.GetLastResult())
			throw std::runtime_error("The data's corrupted.");

		return PNGStego::stringToVector(decrypted);
	}

} // namespace Encryption
} // namespace PNGStego