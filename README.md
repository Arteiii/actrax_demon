# Actrax Demon - Early Development

well basically i currently dont have the time to work on this project

so i decided to make it public and maybe some contributors will help me work on it  
i will continue maintaining the project and reviewing the pull requests

well that said its not really tested the c2 probably doesnt work correctly
and there is no frontend its basically just a restapi...

## Overview

> [!CAUTION]  
> This software is provided for educational purposes only. It is intended to be used in accordance with all applicable
> laws and regulations. The authors and contributors are not responsible for any misuse, illegal activities, or damage
> caused by the use of this software. Users are solely responsible for ensuring that their use of this software complies
> with all local, state, and federal laws.

## Early Development

Please note that Actrax Demon is currently in early development.
I will continue working on it as time permits.
**Contributions and feature requests are welcome.**
Feel free to open an issue or submit a pull request if you'd like to contribute!

## C2 Server

The Command and Control (C2) server for this project can be found at:  
[Arteii - Actrax](https://github.com/Arteiii/Actrax)

## Features

This project is a simple C2 demon client with a minimal feature set. The current implementation is primarily focused on
basic functionality and integrity checks. Key features include:

- **Registry-based Integrity Checks**: The client performs basic integrity checks through registry operations.
- **Anti-Analysis Checks**: The client checks if analysis tools are currently loaded on the system, offering simple
  protection against common analysis tools.
- **Basic Anti-VM**: The client includes rudimentary checks to identify virtual machine environments, offering basic
  protection against VM-based analysis.
- **Simple Load Library Injector**: A basic load library injector is implemented.
- **Core Modules for Reuse**: The core modules of the project are designed as independent libraries, making them
  reusable in other projects. However, some modules may have dependencies, such as the `RegistryGuard` depending on the
  `RegistryHelper`.

## Preview

![Screencast](https://github.com/Arteiii/actrax_demon/blob/main/Screencast%20from%202024-12-11%2013-58-40.gif)

## Detection

some public yara rules detected "anti sandbox" and "anti analysis" based on strings
(i started adding str encryptions but didn't test it yet...)

you can use [weak Alcatraz](https://github.com/weak1337/Alcatraz) for example

[VirusTotal](https://www.virustotal.com/gui/file/03fe91637ce92c3df60e15536c251de2536e4399c33ad3d4cec30d049c350e53/detection)

## Building the Project

To learn how to build and manage the project using the provided Makefile, refer to the [Build Instructions](BUILD.md).

## Building with Toolchains

This project includes toolchain files for cross-compiling with GCC on Windows using MinGW. Toolchain files are located
in the `cmake/toolchains` directory.

To use a toolchain file, specify it with the `-DCMAKE_TOOLCHAIN_FILE` option when configuring the build:

```bash
# Mingw/GCC
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw.cmake -DCMAKE_BUILD_TYPE=Debug ..
```

## License

This project is licensed under the GNU General Public License (GPL) v3.0.