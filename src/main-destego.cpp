//
// Copyright (C) 2015-2016 Zireael (zireael dot nk at gmail dot com)
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
  Boost::Nowide provides UTF-8 support on Windows.
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

#include "compression.h"
#include "encryption.h"
#include "pngwrapper.h"
#include "helpers.h"
#include "pngstegoversion.h"

int main(int argc, char **argv) {
#ifdef _WIN32
	// Convert args to UTF8
	boost::nowide::args argsConverter(argc, argv);

	// Check whether the program's been launched from explorer
	HWND consoleHandle = GetConsoleWindow();
	DWORD processID;
	GetWindowThreadProcessId(consoleHandle, &processID);
	bool ownsConsole = GetCurrentProcessId() == processID;
#endif
	
	std::setlocale(LC_ALL, "");

	bool silentMode = false;
	if (argc > 4) {
		silentMode = (argv[4] == std::string("--silent")
		                || argv[4] == std::string("-s"));
	}

	if (!silentMode)
		boost::nowide::cout << "PNGStego " << PNGStego::version.string << "\nCopyright (C) 2015 Zireael"
		            "\nDistributed under Boost Software License: http://www.boost.org/LICENSE_1_0.txt\n";

	if (argc < 4) {
		boost::nowide::cout << "Usage: " << PNGStego::baseFilename(argv[0]) << " [path-to-container] [output-file] [key] [--silent]\n";
	}

	if (!silentMode)
		boost::nowide::cout << std::endl;

	std::string containerFilename, outputFilename, key;
	if (argc > 1) {
		containerFilename = argv[1];
	}
	else {
		boost::nowide::cout << "Enter the file name of the container: ";
		std::getline(boost::nowide::cin, containerFilename);

		PNGStego::removeLineEndings(containerFilename);
	}
	if (argc > 2) {
		outputFilename = argv[2];
	}
	else {
		boost::nowide::cout << "Enter the name of the output file: ";
		std::getline(boost::nowide::cin, outputFilename);

		PNGStego::removeLineEndings(outputFilename);
	}
	if (argc > 3) {
		key = argv[3];
	}
	else {
		boost::nowide::cout << "Enter the key: ";
		std::getline(boost::nowide::cin, key);

		PNGStego::removeLineEndings(key);
	}

	try {
		PNGStego::PNGFile container(containerFilename);
		if (!silentMode)
			container.setOutputFn([](const std::string &event) {
				boost::nowide::cout << event << std::endl;
			});
		container.decode(outputFilename, key);
		if (!silentMode)
			boost::nowide::cout << "Done." << std::endl;
	}
	catch (const std::exception &e) {
		boost::nowide::cerr << "Fatal error: " << e.what() << std::endl;
		PNGStego::zeroMemory(&key[0], key.size());
#ifdef _WIN32
		if (ownsConsole) {
			boost::nowide::cerr << "Press enter to terminate the program" << std::endl;
			boost::nowide::cin.get();
		}
#endif
		return 1;
	}
	PNGStego::zeroMemory(&key[0], key.size());

#ifdef _WIN32
	if (!silentMode && ownsConsole) {
		boost::nowide::cerr << "Press enter to terminate the program" << std::endl;
		boost::nowide::cin.get();
	}
#endif
	return 0;
}