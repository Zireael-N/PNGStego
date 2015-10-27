//
// Copyright (C) 2015 Zireael (zireael dot nk at gmail dot com)
//  Distributed under the Boost Software License, Version 1.0.
//       (See accompanying file LICENSE.md or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __PNGSTEGO_VERSION_H
#define __PNGSTEGO_VERSION_H

// for compatibility with the .rc file
#define __PNGSTEGO_VER_MANOR   1
#define __PNGSTEGO_VER_MINOR   0
#define __PNGSTEGO_VER_RELEASE 3
#define __PNGSTEGO_VER_STRING  "1.0.3"
#define __PNGSTEGO_VER_FULL    10003

// define __PNGSTEGO_VER_KEEP_DEFINES within the .rc file
#ifndef __PNGSTEGO_VER_KEEP_DEFINES
namespace PNGStego {

constexpr struct {
	uint16_t major;
	uint16_t minor;
	uint16_t release;
	const char *string;
	uint32_t full;
} version { __PNGSTEGO_VER_MANOR, __PNGSTEGO_VER_MINOR,  __PNGSTEGO_VER_RELEASE,
                                  __PNGSTEGO_VER_STRING, __PNGSTEGO_VER_FULL };


#undef __PNGSTEGO_VER_MANOR
#undef __PNGSTEGO_VER_MINOR
#undef __PNGSTEGO_VER_RELEASE
#undef __PNGSTEGO_VER_STRING
#undef __PNGSTEGO_VER_FULL

} // namespace PNGStego

#endif // !defined(__PNGSTEGO_VER_KEEP_DEFINES)

#endif
