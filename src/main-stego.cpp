//
// Copyright (C) 2015 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

// STL
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <clocale>

/*
  Boost::Nowide provides UTF-8 support on Windows
  Windows, by default, uses UTF-16 for Unicode.
*/
#ifdef _WIN32
#include <boost/nowide/args.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/nowide/iostream.hpp>
#else
namespace boost {
	namespace nowide {
		using std::cout;
		using std::cerr;
		using std::cin;
		using std::clog;
	}
}
#endif

#include "bz2compression.h"
#include "byteencryption.h"
#include "pngwrapper.h"
#include "helperfunctions.h"

int main(int argc, char **argv) {
	std::setlocale(LC_ALL, "");
#ifdef _WIN32
	boost::nowide::args argsConverter(argc, argv);
#endif
	std::string containerFilename, dataFilename, key;
	bool silentMode = false;
	if (argc < 4) {
		boost::nowide::cout << "Usage: " << PNGStego::shortenFilename(argv[0]) << " [path-to-container] [input-file] [key] [--silent]" << std::endl;
	}

	if (argc > 1) {
		containerFilename = argv[1];
	}
	else {
		boost::nowide::cout << "Enter the file name of the container: ";
		std::getline(boost::nowide::cin, containerFilename);
		
		PNGStego::cutLineEndings(containerFilename);
	}
	if (argc > 2) {
		dataFilename = argv[2];
	}
	else {
		boost::nowide::cout << "Enter the name of the file you wish to embed: ";
		std::getline(boost::nowide::cin, dataFilename);
		
		PNGStego::cutLineEndings(dataFilename);
	}
	if (argc > 3) {
		key = argv[3];
	}
	else {
		boost::nowide::cout << "Enter the key: ";
		std::getline(boost::nowide::cin, key);
		
		PNGStego::cutLineEndings(key);
	}
	if (argc > 4) {
		silentMode = (argv[4] == std::string("--silent") || argv[4] == std::string("-s"));
	}

	try {
		PNGStego::PNGFile container(containerFilename);
		container.setOutput(!silentMode);
		if (!silentMode)
			container.setOutputStream(boost::nowide::cout);
		container.encode(dataFilename, key);
		std::string newfile = PNGStego::addToFilename(containerFilename, " (copy)");
		if (!silentMode)
			boost::nowide::cout << "Saving the output to \"" << PNGStego::shortenFilename(newfile) << "\"..." << std::endl;
		PNGStego::zeroMemory(&key[0], key.size());

		container.save(newfile);
		if (!silentMode)
			boost::nowide::cout << "Done." << std::endl;
	}
	catch (std::exception &e) {
		boost::nowide::cerr << "Fatal error: " << e.what() << std::endl;
#if defined(_WIN32)
		system("pause");
#endif
		return 1;
	}

#if defined(_WIN32)
	if (!silentMode)
		system("pause");
#endif
	return 0;
}