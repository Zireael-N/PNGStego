//
// Copyright (C) 2015 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#include "bz2compression.h"
#include "byteencryption.h"
#include "helperfunctions.h"
#include "pngwrapper.h"
#include <png.h>
#include <climits>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <cstring>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4297)
#endif
/*
  Access to:
  /dev/random & /dev/urandom on Linux
  /dev/srandom & /dev/urandom on BSD
  CryptGenRandom() on Windows
  For OS X it's probably the same as for BSD, it's not mentioned on Crypto++'s Wiki. 

  Used for generating IV and Salt.
*/
#include <cryptopp/osrng.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

/*
  While Mersenne Twister is a specific algorithm and C++11 standart implementation,
  in my experience, returns the same results on Linux, Windows and OS X,
  std::uniform_int_distribution tends to vary, thus I chose to use boost::random
  since I need to get the same values if the same seed is used.
  
  Used for generating offsets.
*/
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

/*
  Boost::Nowide provides UTF-8 support on Windows
  Windows, by default, uses UTF-16 for Unicode.
*/
#ifdef _WIN32
#include <boost/nowide/args.hpp>
#include <boost/nowide/fstream.hpp>
#else
namespace boost {
	namespace nowide {

		using std::basic_ifstream;
		using std::basic_ofstream;
		using std::basic_fstream;
		using std::ifstream;
		using std::ofstream;
		using std::fstream;
	}
}
#endif

const int PNG_MIN_OFFSET = 1;
const int PNG_MAX_OFFSET = 3;
const int EXTENSION_BYTES = 1; // 8 bits
const int SIZE_BYTES = 4;      // 32 bits
const int IV_BYTES = 12;       // 96 bits
const int SALT_BYTES = 16;     // 128 bits

namespace PNGStego {

	/** For reading using ifstream rather than FILE* (helper function) */
	void ReadFromStream(png_structp pngPointer, png_bytep data, png_size_t length) 
	{
		std::ifstream *Stream = reinterpret_cast<std::ifstream*>(png_get_io_ptr(pngPointer));
		Stream->read(reinterpret_cast<char *>(data), length);
	}

	/** For writing using ofstream rather than FILE* (helper function) */
	void WriteToStream(png_structp pngPointer, png_bytep data, png_size_t length)  
	{
		std::ofstream *Stream = reinterpret_cast<std::ofstream*>(png_get_io_ptr(pngPointer));
		Stream->write(reinterpret_cast<char *>(data), length);
	}

	PNGFile::PNGFile() : pixels(std::vector<BGRA>()), salt(std::vector<uint8_t>()), iv(std::vector<byte>()), outputEnabled(false), outputStream(&std::cout) { }

	PNGFile::PNGFile(const PNGFile& other) : pixels(std::vector<BGRA>()), salt(std::vector<uint8_t>()) {
		std::copy(other.pixels.begin(), other.pixels.end(), std::back_inserter(this->pixels));
		std::copy(other.salt.begin(), other.salt.end(), std::back_inserter(this->salt));
		for (int i = 0; i < CryptoPP::Serpent::BLOCKSIZE; ++i)
			this->iv[i] = other.iv[i];
		
		this->params.width = other.params.width;
		this->params.height = other.params.height;
		this->params.BitsPerPixel = other.params.BitsPerPixel;
		this->params.BitDepth = other.params.BitDepth;
		this->params.ColorType = other.params.ColorType;
		this->params.InterlaceType = other.params.InterlaceType;
		this->params.CompressionType = other.params.CompressionType;
		this->params.FilterType = other.params.FilterType;
		this->params.Channels = other.params.Channels;
		this->outputEnabled = other.outputEnabled;
		this->outputStream = other.outputStream;
	}

	PNGFile::PNGFile(PNGFile &&other) : PNGFile() {
			other.swap(*this);
	}

	PNGFile::PNGFile(const std::string& filename) : outputEnabled(false), outputStream(&std::cout) {
		this->load(filename);
	}

	void PNGFile::swap(PNGFile &other) {
		std::swap(this->salt, other.salt);
		std::swap(this->pixels, other.pixels);
		for (int i = 0; i < CryptoPP::Serpent::BLOCKSIZE; ++i)
			std::swap(this->iv[i], other.iv[i]);

		std::swap(this->params.width, other.params.width);
		std::swap(this->params.height, other.params.height);
		std::swap(this->params.BitsPerPixel, other.params.BitsPerPixel);
		std::swap(this->params.BitDepth, other.params.BitDepth);
		std::swap(this->params.ColorType, other.params.ColorType);
		std::swap(this->params.InterlaceType, other.params.InterlaceType);
		std::swap(this->params.CompressionType, other.params.CompressionType);
		std::swap(this->params.FilterType, other.params.FilterType);
		std::swap(this->params.Channels, other.params.Channels);
		std::swap(this->outputEnabled, other.outputEnabled);
		std::swap(this->outputStream, other.outputStream);
	}

	PNGFile& PNGFile::operator=(const PNGFile& other) {
		if (this != &other) {
			PNGFile(other).swap(*this);
		}
		return *this;
	}

	PNGFile& PNGFile::operator=(PNGFile&& other) {
		if (this != &other) {
			other.swap(*this);
		}
		return *this;
	}

	PNGFile& PNGFile::operator=(const std::string& container) {
		this->load(container);
		return *this;
	}

	void PNGFile::load(const std::string& filename) {
		boost::nowide::ifstream File(filename.c_str(), std::ifstream::binary | std::ifstream::in);
		if (!File) {
			throw std::invalid_argument("Cannot open " + filename);
		}
		this->load(File);
	}

	void PNGFile::save(const std::string& filename) {
		boost::nowide::ofstream File(filename.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!File) {
			throw std::invalid_argument("Cannot open " + filename);
		}
		this->save(File);
	}

	void PNGFile::load(std::istream& stream) {
		uint8_t header[8];

		// Check the file's signature
		stream.read(reinterpret_cast<char*>(&header), sizeof(header));
		if (png_sig_cmp(header, 0, 8))
		{
			throw std::invalid_argument("Invalid file format");
		}

		// Initializations needed by libpng
		png_structp PngPointer = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if (!PngPointer)
		{
			throw std::runtime_error("Cannot allocate memory");
		}
		
		png_infop InfoPointer = png_create_info_struct(PngPointer);
		if (!InfoPointer)
		{
			png_destroy_read_struct(&PngPointer, nullptr, nullptr);
			throw std::runtime_error("Cannot allocate memory");
		}

		if (setjmp(png_jmpbuf(PngPointer)))
		{
			png_destroy_read_struct(&PngPointer, &InfoPointer, nullptr);
			throw std::runtime_error("Cannot set jump pointer");
		}

		png_set_sig_bytes(PngPointer, sizeof(header));
		png_set_read_fn(PngPointer, reinterpret_cast<void*>(&stream), ReadFromStream);

		// Get the image's parameters
		png_read_info(PngPointer, InfoPointer);
		params.Channels = png_get_channels(PngPointer, InfoPointer);
		png_get_IHDR(PngPointer, InfoPointer, &params.width, &params.height, &params.BitDepth, &params.ColorType, &params.InterlaceType, &params.CompressionType, &params.FilterType);
		
		// Convert to 32-bits if needed
		png_set_strip_16(PngPointer);
		png_set_packing(PngPointer);
		switch (params.ColorType)
		{
		case PNG_COLOR_TYPE_GRAY:
		{
			png_set_gray_to_rgb(PngPointer);
			png_set_filler(PngPointer, 0xFF, PNG_FILLER_AFTER);
			png_set_bgr(PngPointer);
			params.BitsPerPixel = 24;
			break;
		}
		case PNG_COLOR_TYPE_PALETTE:
		{
			// Check whether there's a tRNS chunk
			png_bytep transparency_vals = nullptr;
			png_get_tRNS(PngPointer, InfoPointer, &transparency_vals, nullptr, nullptr);

			// Convert to RGB
			png_set_palette_to_rgb(PngPointer);
			png_set_bgr(PngPointer);

			// If there was no tRNS chunk,
			// add an alpha channel
			if (transparency_vals != nullptr) {
				params.BitsPerPixel = 32;
			}
			else {
				png_set_filler(PngPointer, 0xFF, PNG_FILLER_AFTER);
				params.BitsPerPixel = 24;
			}			
			break;
		}

		case PNG_COLOR_TYPE_RGB:
		{
			png_set_filler(PngPointer, 0xFF, PNG_FILLER_AFTER);
			png_set_bgr(PngPointer);
			params.BitsPerPixel = 24;			
			break;
		}

		case PNG_COLOR_TYPE_GRAY_ALPHA:
		{
			png_set_gray_to_rgb(PngPointer);
			png_set_bgr(PngPointer);
			params.BitsPerPixel = 32;
			break;
		}

		case PNG_COLOR_TYPE_RGBA:
		{
			png_set_bgr(PngPointer);
			params.BitsPerPixel = 32;
			break;
		}

		default:
			png_destroy_read_struct(&PngPointer, &InfoPointer, nullptr);
			throw std::runtime_error("Not supported PNG Type");
			break;
		}
		
		// Update the image's parameters
		png_read_update_info(PngPointer, InfoPointer);
		params.Channels = png_get_channels(PngPointer, InfoPointer);
		png_get_IHDR(PngPointer, InfoPointer, &params.width, &params.height, &params.BitDepth, &params.ColorType, &params.InterlaceType, &params.CompressionType, &params.FilterType);

		/*
		  Instead of storing the image in a 2D-array, I store it in a 1D-array.
		  Since png_read_image() accepts a pointer to a pointer as an argument,
		  I need to create a temporary std::vector storing pointers
		  to addresses of 1st pixels for each row.
		*/
		pixels.resize(params.width * params.height);
		std::vector<unsigned char*> RowPointers(params.height);
		size_t BytesPerLine = params.width << 2; // (x << 2) == (x * 4). 4 channels: RGB and Alpha.
		unsigned char *ptr = reinterpret_cast<unsigned char*>(pixels.data());
		for (size_t i = 0; i < params.height; ++i, ptr += BytesPerLine)
			RowPointers[i] = ptr;

		// Read pixels
		png_read_image(PngPointer, RowPointers.data());
		png_destroy_read_struct(&PngPointer, &InfoPointer, nullptr);

		// Read cryptographic stuff
		this->ReadIV();
		this->ReadSalt();		
	}

	void PNGFile::save(std::ostream& stream) {
		if (pixels.empty()) {
			throw std::runtime_error("Trying to save an empty PNG");
		}
		
		// Initializations needed by libpng
		png_structp PngPointer = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if (!PngPointer)
		{
			throw std::runtime_error("Cannot allocate memory");
		}

		png_infop InfoPointer = png_create_info_struct(PngPointer);
		if (!InfoPointer)
		{
			png_destroy_write_struct(&PngPointer, nullptr);
			throw std::runtime_error("Cannot allocate memory");
		}

		if (setjmp(png_jmpbuf(PngPointer)))
		{
			png_destroy_write_struct(&PngPointer, &InfoPointer);
			throw std::runtime_error("Cannot set jump pointer");
		}

		// Set PNG parameters
		png_set_IHDR(PngPointer, InfoPointer, params.width, params.height, params.BitDepth, params.BitsPerPixel == 24 ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_RGBA,
						params.InterlaceType, params.CompressionType, params.FilterType);

		/*
		  Instead of storing the image in a 2D-array, I store it in a 1D-array.
		  Since png_set_rows() accepts a pointer to a pointer as an argument,
		  I need to create a temporary std::vector storing pointers
		  to addresses of 1st pixels for each row.
		*/
		std::vector<unsigned char*> RowPointers(params.height);
		size_t BytesPerLine = params.width << 2; // (x << 2) == (x * 4). 4 channels: RGB and Alpha.
		unsigned char *ptr = reinterpret_cast<unsigned char*>(pixels.data());
		for (size_t i = 0; i < params.height; ++i, ptr += BytesPerLine)
			RowPointers[i] = ptr;

		// Write data to file
		png_set_bgr(PngPointer);
		png_set_write_fn(PngPointer, reinterpret_cast<void*>(&stream), WriteToStream, nullptr);
		// png_set_rows() takes a pointer to a non-const data as its
		// 3rd argument, making it not possible to declare save() as const
		// without using const_cast on pixels.data(), I'd rather not do that.
		png_set_rows(PngPointer, InfoPointer, RowPointers.data());
		png_write_png(PngPointer, InfoPointer, params.BitsPerPixel == 24 ? PNG_TRANSFORM_STRIP_FILLER_AFTER : PNG_TRANSFORM_IDENTITY, NULL);
		png_destroy_write_struct(&PngPointer, &InfoPointer);
	}

	uint32_t PNGFile::capacity(uint32_t seed) const {
		/*
		  I kinda doubt there'd be a picture able to hold more
		  than 0.5 GiB in near future, so uint32_t should be enough.
		  Even if we use 4K (4096 * 3112), it's only
		  12 746 752 pixels, and if we store information in
		  every single one, that'd be no more than ~1.52 MiB.
		*/

		if (pixels.empty()) {
			throw std::runtime_error("PNG's empty");
		}

		boost::random::mt19937 gen(seed);
		boost::random::uniform_int_distribution<uint16_t> offset(PNG_MIN_OFFSET, PNG_MAX_OFFSET);
		PNGStego::zeroMemory(&seed, sizeof(seed));

		uint32_t capacity = 0;
		uint32_t pos = 0;
		uint32_t size = static_cast<uint32_t>(pixels.size());
		while(pos < size) {
			++capacity;
			pos += offset(gen);
		}

		return (capacity / 8) > (SIZE_BYTES + EXTENSION_BYTES) ? (capacity / 8) - (SIZE_BYTES + EXTENSION_BYTES) : 0;
	}

	void PNGFile::encode(const std::string& filename, const std::string& key) {
		if (pixels.empty()) {
			throw std::runtime_error("Trying to encode data into an empty PNG");
		}
		if (key.empty()) {
			throw std::runtime_error("An empty key was given");
		}

		boost::nowide::ifstream File(filename.c_str(), std::ios::in | std::ios::binary);
		if (!File) {
			throw std::invalid_argument("Cannot open " + filename);
		}
		std::string extension = getExtension(filename);
		uint32_t dataSize = static_cast<uint32_t>(fileSize(filename));
		uint8_t extensionSize = static_cast<uint8_t>(extension.size());
		std::vector<uint8_t> binaryData(stringToVector(extension));
		binaryData.resize(extensionSize + dataSize);
		File.read(reinterpret_cast<char *>(binaryData.data() + extensionSize), dataSize);

		iv.resize(IV_BYTES);
		CryptoPP::OS_GenerateRandomBlock(true, iv.data(), iv.size());

		std::array<uint8_t, 4> t = PNGStego::Encryption::hashKey<4, 150000>(key, iv);
		uint32_t offsetSeed = 0;
		for (int i = 0; i < 4; ++i) {
			offsetSeed <<= 8;
			offsetSeed += t[i];
		}
		PNGStego::zeroMemory(t.data(), t.size());

		if (outputEnabled)
			(*outputStream) << "Compressing data..." << std::endl;
		binaryData = PNGStego::bzip2::compress(binaryData);
		dataSize = static_cast<uint32_t>(binaryData.size());
		dataSize += (TAG_SIZE * 2);

		if (dataSize <= capacity(offsetSeed)) {
			boost::random::mt19937 gen(offsetSeed);
			boost::random::uniform_int_distribution<uint16_t> offset(PNG_MIN_OFFSET, PNG_MAX_OFFSET);
			PNGStego::zeroMemory(&offsetSeed, sizeof(offsetSeed));

			if (outputEnabled)
				(*outputStream) << "Encrypting data..." << std::endl;

			salt.resize(SALT_BYTES);
			CryptoPP::OS_GenerateRandomBlock(true, salt.data(), salt.size());
			this->WriteSalt();
			this->WriteIV();

			binaryData = Encryption::encrypt(binaryData, key, iv, salt);
			dataSize = static_cast<uint32_t>(binaryData.size());

			if (outputEnabled)
				(*outputStream) << "Embedding data..." << std::endl;
			int PixelPos = 0;
			for (int i = 0; i < 8 * EXTENSION_BYTES; ++i) {
				if (extensionSize & (1 << i)) {
					pixels[PixelPos].RGBA.B |= 1;
				}
				else {
					pixels[PixelPos].RGBA.B &= ~1;
				}
				PixelPos += offset(gen);
			}
			for (int i = 0; i < 8 * SIZE_BYTES; ++i) {
				if (dataSize & (1 << i)) {
					pixels[PixelPos].RGBA.B |= 1;
				}
				else {
					pixels[PixelPos].RGBA.B &= ~1;
				}
				PixelPos += offset(gen);
			}
			for (size_t i = 0; i < dataSize * 8; ++i) {
				if (binaryData[i / 8] & (1 << (i % 8)))
					pixels[PixelPos].RGBA.B |= 1;
				else
					pixels[PixelPos].RGBA.B &= ~1;
				PixelPos += offset(gen);
			}
		}
		else {
			throw std::runtime_error("The image can't contain data that large");
		}		
	}

	void PNGFile::decode(std::string filename, const std::string& key) const {
		if (pixels.empty()) {
			throw std::runtime_error("Trying to extract data from an empty PNG");
		}
		if (key.empty()) {
			throw std::runtime_error("An empty key was given");
		}

		uint32_t dataSize = 0;
		uint8_t extensionSize = 0;
		
		std::array<uint8_t, 4> t = PNGStego::Encryption::hashKey<4, 150000>(key, iv);
		uint32_t offsetSeed = 0;
		for (int i = 0; i < 4; ++i) {
			offsetSeed <<= 8;
			offsetSeed += t[i];
		}
		PNGStego::zeroMemory(t.data(), t.size());

		boost::random::mt19937 gen(offsetSeed);
		boost::random::uniform_int_distribution<uint16_t> offset(PNG_MIN_OFFSET, PNG_MAX_OFFSET);
		PNGStego::zeroMemory(&offsetSeed, sizeof(offsetSeed));
	
		int PixelPos = 0;
		for (int i = 0; i < 8 * EXTENSION_BYTES; ++i) {
			extensionSize |= ((pixels[PixelPos].RGBA.B & 1) << i);
			PixelPos += offset(gen);
		}
		for (int i = 0; i < 8 * SIZE_BYTES; ++i) {
			dataSize |= ((pixels[PixelPos].RGBA.B & 1) << i);
			PixelPos += offset(gen);
		}

		if (dataSize <= capacity(offsetSeed)) {

			std::vector<uint8_t> binaryData(dataSize);
			if (outputEnabled)
				(*outputStream) << "Extracting data..." << std::endl;
			for (size_t i = 0; i < dataSize * 8; ++i) {
				if (pixels[PixelPos].RGBA.B & 1)
					binaryData[i / 8] |= (1 << (i % 8));
				else
					binaryData[i / 8] &= ~(1 << (i % 8));
				PixelPos += offset(gen);
			}
			if (outputEnabled)
				(*outputStream) << "Decrypting data..." << std::endl;
			binaryData = Encryption::decrypt(binaryData, key, iv, salt);
			if (outputEnabled)
				(*outputStream) << "Decompressing data..." << std::endl;
			binaryData = PNGStego::bzip2::decompress(binaryData);

			if (extensionSize)
				filename.push_back('.');
			for (uint8_t i = 0; i < extensionSize; ++i) {
				filename.push_back(binaryData[i]);
			}

			if (outputEnabled)
				(*outputStream) << "Writing data..." << std::endl;
			boost::nowide::ofstream File(filename.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
			if (!File) {
				throw std::invalid_argument("Cannot open " + filename);
			}
			File.write(reinterpret_cast<char *>(binaryData.data() + extensionSize), binaryData.size() - extensionSize);
		}
		else {
			// Basically, if dataSize happens to be larger than the result of capacity()
			// then something's not right, so we throw an exception.
			throw std::runtime_error("Corrupted header");
		}
	}

	bool PNGFile::getOutput() const {
		return outputEnabled;
	}

	void PNGFile::setOutput(bool output) {
		outputEnabled = output;
	}

	void PNGFile::setOutputStream(std::ostream& stream) {
		outputStream = &stream;
	}

	/**
	 * Reads IV
	 * Gets data from 8 * IV_BYTES pixels that are in the middle of the image, using LSB of the red channel.
	 */
	void PNGFile::ReadIV() {
		iv.resize(IV_BYTES);
		size_t pos = pixels.size() / 2;
		if (pos < (8 * IV_BYTES / 2))
			throw std::runtime_error("The image's too small");
		pos -= (8 * IV_BYTES / 2);
		for (size_t i = 0; i < 8 * IV_BYTES; ++i) {
			if (pixels[pos + i].RGBA.R & 1)
				iv[i / 8] |= (1 << (i % 8));
			else
				iv[i / 8] &= ~(1 << (i % 8));
		}
	}

	/**
	* Writes IV
	* Writes data to (8 * IV_BYTES) pixels that are in the middle of the image, using LSB of the red channel.
	*/
	void PNGFile::WriteIV() {
		size_t bits = iv.size() * 8;
		size_t pos = pixels.size() / 2;
		if (pos < (bits / 2))
			throw std::runtime_error("The image's too small");
		pos -= (bits / 2);
		for (size_t i = 0; i < bits; ++i) {
			if (iv[i / 8] & (1 << (i % 8)))
				pixels[pos + i].RGBA.R |= 1;
			else
				pixels[pos + i].RGBA.R &= ~1;
		}
	}

	/**
	* Reads IV
	* Gets data from first (8 * SALT_BYTES) pixels, using LSB of the green channel.
	*/
	void PNGFile::ReadSalt() {
		salt.resize(SALT_BYTES);
		if (pixels.size() < SALT_BYTES * 8)
			throw std::runtime_error("The image's too small");
		for (size_t i = 0; i < 8 * SALT_BYTES; ++i) {
			if (pixels[i].RGBA.G & 1)
				salt[i / 8] |= (1 << (i % 8));
			
			else
				salt[i / 8] &= ~(1 << (i % 8));		
		}
	}

	/**
	* Writes IV
	* Writes data to first (8 * SALT_BYTES) pixels, using LSB of the green channel.
	*/
	void PNGFile::WriteSalt() {
		size_t bits = salt.size() * 8;
		if (pixels.size() < bits)
			throw std::runtime_error("The image's too small");
		for (size_t i = 0; i < bits; ++i) {
			if (salt[i / 8] & (1 << (i % 8)))
				pixels[i].RGBA.G |= 1;
			else
				pixels[i].RGBA.G &= ~1;
		}
	}

	PNGFile::~PNGFile() {
		// Wipe memory
		PNGStego::zeroMemory(iv.data(), iv.capacity());
		PNGStego::zeroMemory(salt.data(), salt.capacity());
	}

} // namespace PNGStego