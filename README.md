# SJ2-C Project Introduction

This is the development infrastructure for the SJ2 board, written in C that anyone can understand easily. It was designed to **learn the low level workings of a microcontroller platform with an RTOS**:

Project highlights:

- Fully implemented in C
- Minimalistic design with little to no abstractions
- Follows good coding principles (such as YAGNI and DRY)

Infrastructure highlights:

- Support Mac, Linux, Windows out of the box
- Version controlled toolchains and other supplementary tools
- No VMs
- No WSL dependency on Windows

----

## Build System

We use [SCons](https://scons.org/) as our build platform. The developers of SJ2-C applied experience of diverse set of build systems acquired over many years, and resorted to this one. The candidates were:

- SCons (used at Tesla, and many other companies)
- Bazel (used at Google, Zoox and many other companies)
- Make

SCons discusses the advantages and disadvantages of other existing build systems: [Build System Comparison](https://github.com/SCons/scons/wiki/sconsvsotherbuildtools)

From experience, we can state that `bazel` is really cool, but hard to extend. SCons dependencies are tricky, but it is based on python and easy to understand and extend. SCons takes advantage of a Python interpreter making it portable across multiple platforms (Mac, Linux, Windows).

----

## History

We wanted to build a strong foundational sample project for SJ-2 development board that is used at San Jose State University (SJSU).

Originally, Preet created "SJ1" development board, which meant that there was one development board across multiple classes at SJSU. I was an enthusiast and created a hybrid project composed of C and C++ sources. I love C++ a little more than C because I can express artistic designs in the language, but a language is really a tool, and you need to select the right tool for the job. Presently I work on Embedded 'Firmware' code for automotive industry which is in C, and 'Software' code in C++, because C is the right tool for firmware, and C++ is the right tool for software. "SJ2" [original](https://github.com/kammce/SJSU-Dev2) software was also designed by an enthusiast (Khalil), who is a very talented person, but expressing a starter project in C++ increased the complexity so much that many developers had a difficult time adopting it.

This is where the "SJ2-C" was born, which was completely re-designed to write simple code that everyone could understand. Many students are new to programming and it was not expected that C++ would come naturally. The code may appear less fancy, but it is simple to understand and traceable with minimal abstractions. The goal is to avoid designing yet another Arduino platform. There is no such thing as magic in the field of firmware engineering.
