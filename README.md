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

## Usage & Installation

Please check the Wiki:

* [Linux](https://github.com/Zireael-N/PNGStego/wiki/Linux)
* [Mac OS X](https://github.com/Zireael-N/PNGStego/wiki/OS-X)
* [Windows](https://github.com/Zireael-N/PNGStego/wiki/Windows)


## Recommendations

* Use a strong password.
You can easily calculate how many possible combinations there are of a given alphabet and fixed length. Let's say you use only digits (10) and small english letters (26) and your password is 6 characters long. That'd be **(10 + 26)^6** combinations, **2 176 782 336** combinations. That's not that much for bruteforcing. The attacker doesn't necessary have the same hardware as you. Throw in capital letters, symbols, brackets, make it 15 characters long and suddenly there are **395 * 10^27** possible combinations.
But please note that simple bruteforcing is not the only way to crack the password. Check [this](https://en.wikipedia.org/wiki/Dictionary_attack).
* Use a picture with a lot of different tones/colors. Such as a photo of nature: waterfalls, skies, jungles. Or a screenshot from a videogame. An image with a solid color background is a *bad* choice, that's for sure.
* At all costs avoid using a picture that you can easily find on the Internet. It'd be possible to compare your version containing something with its source. If you absolutely have no other choice, change it somehow so it'd make sense why it is different from its source: apply color correction, use blur/sharpen, use some filters. Something of that sort.
* To calculate **minimum** size of data the image is able to contain, multiply the image's width by its height, divide that by 3 and that'd be it, in bits. For a Full HD picture, that's **1920 * 1080 / 3 / 8 = 86 400** bytes.
Note that 5 bytes are reserved for storing data's size + the embedded file's extension length, 24 bytes are reserved for cryptographic purposes.
And keep in mind that the embedded's file extension is also stored. However, the file's extension and its contents are compressed before embedding.