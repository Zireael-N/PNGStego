//
// Copyright (C) 2015 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __PNG_WRAPPER_H
#define __PNG_WRAPPER_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <cryptopp/serpent.h>

typedef unsigned char byte;

namespace PNGStego {

class PNGFile {
public:
	struct Pixel {
		uint8_t blue;
		uint8_t green;
		uint8_t red;
		uint8_t alpha;

		bool operator==(const PNGFile::Pixel &other) const {
			return this->blue  == other.blue  &&
			       this->red   == other.red   &&
			       this->green == other.green &&
			       this->alpha == other.alpha;
		}

	};

	/**
	 ** Creates an empty object
	 ** It's necessary to load an image
	 ** If you wish to further work with it
	 **/
	PNGFile();
	/** Copies content of one PNG file into another */
	PNGFile(const PNGFile &other);
	/** Copies content of one PNG file into another */
	PNGFile(PNGFile &&other);
	/** Loads a PNG file from a file with the given filename */
	PNGFile(const std::string &filename);
	/** Loads a PNG file from the given std::istream */
	PNGFile(std::istream &stream);

	/** Swaps content between two files */
	void swap(PNGFile &other);

	/** Copies content of one PNG file into another */
	PNGFile& operator=(const PNGFile &other);
	/** Copies content of one PNG file into another */
	PNGFile& operator=(PNGFile &&other);
	/** Loads a PNG file from a file with the given filename */
	PNGFile& operator=(const std::string &container);

	/** Returns the image's width */
	uint32_t getWidth();
	/** Returns the image's height */
	uint32_t getHeight();
	/** Returns a constant reference to an internal representation of the image */
	const std::vector<Pixel>& getPixels();

	/** Loads a PNG file from a file with the given filename */
	void load(const std::string &filename);
	/** Saves a PNG file with a given filename */
	void save(const std::string &filename);

	/** Loads a PNG file from the given std::istream */
	void load(std::istream &stream);
	/** Outputs a PNG file into the given std::ostream */
	void save(std::ostream &stream);

	/** Sets a function that gets called each time decode/encode do something */
	void setOutputFn(const std::function<void(const std::string&)> &fn);
	/** Sets a function that gets called each time decode/encode do something */
	void setOutputFn(std::function<void(const std::string&)> &&fn);

	/** Returns capacity of the PNG file with the given seed, in bytes */
	uint32_t capacity(uint32_t seed) const noexcept;
	/** Embeds data from a file with the given filename into the PNG file, using the given key */
	void encode(const std::string &filename, const std::string &key);
	/** Embeds data from a given vector and string using the given key */
	void encode(const std::vector<uint8_t> &data, const std::string &extension, const std::string &key);
	/**
	 ** Extracts data from the PNG file using the given key and saves it
	 ** into a file with the given filename.
	 ** In case of file I/O failure if the 3rd parameter is not nullptr, puts data there.
	 **/
	void decode(std::string filename, const std::string &key, std::vector<uint8_t> *backup = nullptr) const;
	/** Extracts data from the PNG file using the given key, puts it into the 1st and 2nd parameters. */
	void decode(std::vector<uint8_t> &data, std::string &extension, const std::string &key) const;

	~PNGFile();

	/**
	 ** !!! DO NOT CALL THIS UNLESS YOU ABSOLUTELY SURE YOU KNOW WHAT YOU DO !!!
	 ** This function replaces a PRNG for IV & salt
	 ** It exists purely for testing & debugging.
	 ** By default PRNG used *IS* cryptographically secure.
	 **/
	void setCSPRNG(const std::function<void(uint8_t *, size_t)> &fn);
	/**
	** !!! DO NOT CALL THIS UNLESS YOU ABSOLUTELY SURE YOU KNOW WHAT YOU DO !!!
	** This function replaces a PRNG for IV & salt
	** It exists purely for testing & debugging.
	** By default PRNG used *IS* cryptographically secure.
	**/
	void setCSPRNG(std::function<void(uint8_t *, size_t)> &&fn);
private:
	struct {
		uint32_t width, height, BitsPerPixel;
		int32_t BitDepth, ColorType, InterlaceType, CompressionType, FilterType, Channels;
	} params;
	
	std::vector<Pixel> pixels;	
	std::vector<uint8_t> salt;
	std::vector<byte> iv;
	std::function<void(const std::string &)> outputFn;
	std::function<void(uint8_t *, size_t)> CSPRNG;

	void ReadIV();
	void WriteIV();
	void ReadSalt();
	void WriteSalt();
};

} // namespace PNGStego
#endif