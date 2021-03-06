## P2C-Client

[![C++](https://img.shields.io/badge/language-C%2B%2B-%23f34b7d.svg?style=plastic)](https://en.wikipedia.org/wiki/C%2B%2B) 
[![Windows](https://img.shields.io/badge/platform-Windows-0078d7.svg?style=plastic)](https://en.wikipedia.org/wiki/Microsoft_Windows) 
[![x86](https://img.shields.io/badge/arch-x86-red.svg?style=plastic)](https://en.wikipedia.org/wiki/X86) 

Open-source P2C Loader designed for CS:GO, Built around [KeyAuth](https://keyauth.win/). Uses the [Themida](https://www.oreans.com/Themida.php) SDK.

## Dependencies

- [Dear imgui](https://github.com/ocornut/imgui).
- [cryptoPP](https://github.com/weidai11/cryptopp).
- [curl](https://github.com/curl/curl)
- [json](https://github.com/nlohmann/json)
- [openssl](https://github.com/openssl/openssl)

## Instructions

Download the dependencies [here](https://files.catbox.moe/qtx3kt.zip) <br>
Place them into the main directory (along with all the other folders) <br>
Extract them, then rename the folder to "Dependencies" <br>

Change your keyauth credentials [here](https://github.com/UntitledEntity/P2C-Client/blob/main/UserInterface/UserInterface.cpp#L5) <br>
Add your file IDs [here](https://github.com/UntitledEntity/P2C-Client/blob/main/auth/Auth.hpp#L457) and [here](https://github.com/UntitledEntity/P2C-Client/blob/main/auth/Auth.hpp#L486)

## FAQ

### Help
I will not help with any errors as after the dependencies are extracted, and the directories are fixed, the solution will build with no errors. Apoligies.

## License

> Copyright (c) 2022 Jacob Gluska/UntitledEntity

This project is licensed under the [MIT License](https://opensource.org/licenses/mit-license.php) - see the [LICENSE](https://github.com/UntitledEntity/P2C-Client/blob/main/LICENSE) file for details.
