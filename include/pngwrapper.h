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
#include <cstdint>
#include <cryptopp/serpent.h>

typedef unsigned char byte;

namespace PNGStego {

class PNGFile {
public:
	/**
	 * Creates an empty object
	 * It's necessary to load an image
	 * If you wish to further work with it
	 */
	PNGFile();
	/** Loads a PNG file from a file with the given filename */
	PNGFile(const std::string& filename);
	/** Copies content of one PNG file into another */
	PNGFile(const PNGFile& other);
	/** Copies content of one PNG file into another */
	PNGFile(PNGFile &&other);

	/** Swaps content between two files */
	void swap(PNGFile &other);

	/** Copies content of one PNG file into another */
	PNGFile& operator=(const PNGFile& other);
	/** Copies content of one PNG file into another */
	PNGFile& operator=(PNGFile&& other);
	/** Loads a PNG file from a file with the given filename */
	PNGFile& operator=(const std::string& container);

	/** Loads a PNG file from a file with the given filename */
	void load(const std::string& filename);
	/** Saves a PNG file with a given filename */
	void save(const std::string& filename);

	/** Loads a PNG file from the given std::istream */
	void load(std::istream& stream);
	/** Outputs a PNG file into the given std::ostream */
	void save(std::ostream& stream);

	/** Returns capacity of the PNG file with the given seed, in bytes */
	uint32_t capacity(uint32_t seed);
	/** Embeds data from a file with the given filename into the PNG file, using the given key. */
	void encode(const std::string& filename, const std::string& key);
	/** Extracts data from the PNG file using the given key and saves it into a file with the given filename. */
	void decode(std::string filename, const std::string& key);

	/** Returns whether the PNGFile class outputs its actions into a stream */
	bool getOutput();
	/** Sets whether the PNGFile class should output its actions into a stream */
	void setOutput(bool output);
	/**
	 * Sets a stream where the PNGFile should output its actions
	 * By default it's std::cout
	 */
	void setOutputStream(std::ostream& stream);
	~PNGFile();
private:
	struct {
		uint32_t width, height, BitsPerPixel;
		int32_t BitDepth, ColorType, InterlaceType, CompressionType, FilterType, Channels;
	} params;

	union BGRA
	{
		uint32_t Color;
		struct
		{
			unsigned char B, G, R, A;
		} RGBA;
	};
	
	std::vector<BGRA> pixels;	
	std::vector<uint8_t> salt;
	std::vector<byte> iv;
	bool outputEnabled;
	std::ostream *outputStream;

	void ReadIV();
	void WriteIV();
	void ReadSalt();
	void WriteSalt();
};

} // namespace PNGStego
#endif