# PNGStego [![Build Status](https://travis-ci.org/Zireael-N/PNGStego.svg?branch=master)](https://travis-ci.org/Zireael-N/PNGStego)

## About

PNGStego is a command-line application that hides files within [PNG](https://en.wikipedia.org/wiki/Portable_Network_Graphics) images using [LSB](https://en.wikipedia.org/wiki/Least_significant_bit) [steganography](https://en.wikipedia.org/wiki/Steganography).
To embed the data, the program's user must provide a password. It's impossible to extract that data without knowing the password.
To achieve its goal, PNGStego first compresses the data with the [bzip2](https://en.wikipedia.org/wiki/Bzip2) algorithm. Then it uses [PBKDF2](https://en.wikipedia.org/wiki/PBKDF2) with hundreds of thousands iterations to derive two 256-bit keys and a seed for [PRNG](https://en.wikipedia.org/wiki/Pseudorandom_number_generator) from the given password.
[PRNG](https://en.wikipedia.org/wiki/Pseudorandom_number_generator) is used to generate offsets, so the program sometimes skips 1-2 pixels instead of writing into every single one. The two keys are used to encrypt the data with both [AES (Rijndael)](https://en.wikipedia.org/wiki/Advanced_Encryption_Standard) and [Serpent](https://en.wikipedia.org/wiki/Serpent_(cipher)).

[Salt](https://en.wikipedia.org/wiki/Salt_(cryptography)) and [IV](https://en.wikipedia.org/wiki/Initialization_vector) are generated using CryptGenRandom() on Windows, /dev/random on Linux and so on.
This ensures that even if you use the same container to embed the same data using the same key, the ciphertext will be different each time.

PNGStego has been successfully compiled with:

* Windows
  * Microsoft Visual Studio 2015's compiler
  * MinGW 4.9.2
  * MinGW-w64 5.2.0
* Linux
  * GCC 5.2.0
  * Clang 3.7.0
* OS X
  * Xcode v.7.0.1's Clang

PNGStego has been tested with [StegExpose](https://github.com/b3dk7/StegExpose), reports for containers containing data were the same as for clean pictures.
Even with `threshold` as low as **0.05**. The default value is **0.20** and the author recommends using **0.15** to reduce false negatives.

Sadly, I couldn't build [stegdetect](https://github.com/abeluck/stegdetect).

To be sure that you use the program properly, please check the recommendations at the end of this Readme.

## Why?
There are [countries](https://en.wikipedia.org/wiki/Key_disclosure_law) where one may be forced to provide either keys or decrypted information, there may be countries where using encryption is illegal. 
However, I believe that privacy is a fundamental human right. And while using encryption alone in such countries is not enough, steganography conceals the fact that there's hidden data.

## Usage

### Windows

You can either double click the binaries, or you can invoke them from the cmd.exe.

#### Double click

Simply double click either PNGStego.exe or PNGDeStego.exe, then enter what they ask for.

You can also drag&drop files onto them.
To embed the data, select the container, **then** hold Ctrl and select the file you want to embed and drag&drop them onto PNGStego.exe. Then you'll be asked to enter the password.
To extract the data, select the container and drag&drop it onto PNGDeStego.exe. You'll be asked to enter the file name for extracted data and to enter the password.

#### cmd.exe

Syntax is:

```
PNGStego.exe [path-to-container] [input-file] [key] [--silent]
PNGDeStego.exe [path-to-container] [output-file] [key] [--silent]
```

For example:

```
PNGStego.exe cats.png passwords.txt el!tepa55word
PNGDeStego.exe "cats (copy).png" mylovelyhiddenfile el!tepa55word --silent
```

`--silent` key turns off any output, unless there's an error that didn't let the program do its job.

### OS X

You can either simply double click the binaries or invoke them from the terminal.

Note that double clicking them actually just creates a terminal window that invokes them.
But it does so from your Home folder. It matters because the program will look for files within your Home folder, unless you'll type the full path.

Invoking them manually is preferable because you can specify arguments they are called with.
Syntax is:

```
$ ./PNGStego [path-to-container] [input-file] [key] [--silent]
$ ./PNGDestego [path-to-container] [output-file] [key] [--silent]
```

For example:

```
$ ./PNGStego cats.png passwords.txt el!tepa55word
$ ./PNGDestego "cats (copy).png" mylovelyhiddenfile el!tepa55word --silent
```

`--silent` key turns off any output, unless there's an error that didn't let the program do its job.

Please note that most shells record commands used by default, so it's better to either avoid invoking the program with a 3rd parameter or turn off shell history before doing so / erase shell history after doing so.

### Linux

I believe you guys don't need help figuring things out.

```
$ ./pngstego [path-to-container] [input-file] [key] [--silent]
$ ./pngdestego [path-to-container] [output-file] [key] [--silent]
```

For example:

```
$ ./pngstego cats.png passwords.txt el!tepa55word
$ ./pngdestego "cats (copy).png" mylovelyhiddenfile el!tepa55word --silent
```

`--silent` key turns off any output, unless there's an error that didn't let the program do its job.

Please note that most shells record commands used by default, so it's better to either avoid invoking the program with a 3rd parameter or turn off shell history before doing so / erase shell history after doing so.

It doesn't support pipelines because I honestly do not know how one should go about that when the program needs two files for embedding the data.
Though `load()` and `save()` are already capable of working with `std::istream` and `std::ostream`. `encode()` and `decode()` would require modifications though.

The only flag it supports is `--silent`/`-s` and the order of args matters.
Partly because I wanted drag&drop to work on Windows, partly because I'm lazy, I didn't implement proper args parsing; that's also the reason for two binaries.

## Installation

### Windows

Go to [releases](https://github.com/Zireael-N/PNGStego/releases) and download one of the builds.
Usually, you'd want the one built with MSVC, but if you have an outdated Windows and/or you can't get it to launch, you'll need the one built with MinGW.

#### File integrity verification

To verify that you've downloaded binaries compiled by the repository's owner, you'll need [GnuPG](https://en.wikipedia.org/wiki/Gpg4win) and this key:

```
gpg --keyserver hkp://pool.sks-keyservers.net --recv-keys C32F513D
gpg --fingerprint C32F513D
```

If the output of the 2nd command is:

```
pub   4096R/C32F513D 2015-10-23 [expires: 2018-10-22]
      Key fingerprint = 3242 2012 8FF2 6EB9 FCB4  69D6 11D2 951F C32F 513D
uid                  Zireael <zireael.nk@gmail.com>
sub   4096R/EABFE0D4 2015-10-23 [expires: 2018-10-22]
```

Then everything's alright. To verify that you've downloaded a ZIP-archive that is signed using this key, you can either right click on .asc file, choose "More GpgEX Options -> Verify" or execute this command:

`gpg --verify <FILENAME>.asc`

The output should say whether the signature is valid and what key's been used to create this signature.

### Anything else
You'll need to build it yourself.

## Building from source files

First of all, if you intend to modify the source files, please note that there are constants that, if changed, will make your binaries incompatible with others.
If you really want to achieve that, be sure that you have a backup of your binaries / modified source files. Otherwise, you might never extract your data again.

### Windows

You'll need to compile these libraries with the compiler of your choice:

* [Boost](http://www.boost.org/)
* [Boost Nowide](https://github.com/artyom-beilis/nowide)
* [Crypto++](https://www.cryptopp.com/)
* [libbzip2](http://www.bzip.org/)
* [libpng](http://www.libpng.org/pub/png/libpng.html)
* [zlib](http://www.zlib.net/)

#### Microsoft Visual Studio 2015

Grab the solution & projects from the repository. Make sure that the IDE knows where the headers and compiled libs are. Compile.

If your Visual Studio is outdated, then you'll need to create a solution with 2 projects.
Add all header files and pngwrapper.cpp, helperfunctions.cpp, bz2compression.cpp, byteencryption.cpp to both projects.
Add main-destego.cpp and main-stego.cpp to each project accordingly.
Then link against the libs mentioned above.

#### MinGW

Open Makefile with a text editor.
Change `-lboost_nowide-mgw52-mt-1_59 -lboost_iostreams-mgw52-mt-1_59` if needed.
Then open a cmd.exe and run the following command:

```
mingw32-make
```

### OS X

First, you'll need to install Xcode and the libraries PNGStego depends on:

* [Boost](http://www.boost.org/)
* [Crypto++](https://www.cryptopp.com/)
* [libbzip2](http://www.bzip.org/)
* [libpng](http://www.libpng.org/pub/png/libpng.html)
* [zlib](http://www.zlib.net/)

To install the libraries the easy way, you can use [Homebrew](https://github.com/Homebrew/homebrew):

```bash
$ brew install lbzip2
$ brew install lzlib
$ brew install libpng
$ brew install cryptopp
$ brew install boost
```

Then you can either use Xcode or just build it with a terminal.

#### Xcode
You'll need to use the Xcode project from this repository. Make a few changes:

```
Product - Scheme - Edit Scheme...
Build - Build - + - PNGDestego - Add
```

To ensure that you build both binaries.

```
File - Project Settings... - Advanced...
```

To change where Xcode will create binaries. Now you can build the project.

#### Terminal

You'll still need to have a compiler obviously. Either install Xcode's command line tools or get yourself gcc from Homebrew.

Execute this in your terminal to build binaries: 

```bash
$ make
```

### Linux

You'll need these libraries:

* [Boost](http://www.boost.org/)
* [Crypto++](https://www.cryptopp.com/)
* [libbzip2](http://www.bzip.org/)
* [libpng](http://www.libpng.org/pub/png/libpng.html)
* [zlib](http://www.zlib.net/)

After getting them you can simply compile the project with 

```bash
$ make
```

To install it as a system app you can use:

```bash
$ sudo make install
```

However, this is **not recommended**. Why? Because your package manager won't know about the program.
If your distributive uses .deb-packages, check [this](https://wiki.debian.org/HowToPackageForDebian); if your distributive uses .rpm-packages, check [this](https://fedoraproject.org/wiki/How_to_create_an_RPM_package); if your distributive uses [pacman](https://wiki.archlinux.org/index.php/Pacman), check [this](https://wiki.archlinux.org/index.php/Creating_packages).

## Recommendations

* Use a strong password.
You can easily calculate how many possible combinations there are of a given alphabet and fixed length. Let's say you use only digits (10) and small english letters (26) and your password is 6 characters long. That'd be **(10 + 26)^6** combinations, **2 176 782 336** combinations. That's not that much for bruteforcing. The attacker doesn't necessary have the same hardware as you. Throw in capital letters, symbols, brackets, make it 15 characters long and suddenly there are **395 * 10^27** possible combinations.
But please note that simple bruteforcing is not the only way to crack the password. Check [this](https://en.wikipedia.org/wiki/Dictionary_attack).
* Use a picture with a lot of different tones/colors. Such as a photo of nature: waterfalls, skies, jungles. Or a screenshot from a videogame. An image with a solid color background is a *bad* choice, that's for sure.
* At all costs avoid using a picture that you can easily find on the Internet. It'd be possible to compare your version containing something with its source. If you absolutely have no other choice, change it somehow so it'd make sense why it is different from its source: apply color correction, use blur/sharpen, use some filters. Something of that sort.
* To calculate **minimum** size of data the image is able to contain, multiply the image's width by its height, divide that by 3 and that'd be it, in bits. For a Full HD picture, that's **1920 * 1080 / 3 / 8 = 86 400** bytes.
Note that 5 bytes are reserved for storing data's size + the embedded file's extension length, 24 bytes are reserved for cryptographic purposes.
And keep in mind that the embedded's file extension is also stored. However, the file's extension and its contents are compressed before embedding.