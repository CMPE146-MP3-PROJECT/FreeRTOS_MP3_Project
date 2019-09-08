# SJ2-C

This is a sample project for the SJ2 board, written in C that anyone can understand easily.


## Build System

We use [SCons](https://scons.org/) as a build platform. The developers of SJ2-C applied experience of diverse set of build systems acquired over many years, and resorted to this one. The candidates were:

- SCons (used at Tesla, and many ohter companies)
- Bazel (used at Google, Zoox and many other companies)
- makefile (really?)

From experience, we can state that `bazel` is really cool, but hard to extend. SCons dependencies are tricky, but it is based on python, and easy to understand and extend. SCons is also portable and able to run on multiple platforms (Mac, Linux, Windows).


## History

We wanted to build a strong foundational sample project for SJ-2 development board that is used at San Jose State University (SJSU). 

Originally, Preet created "SJ1" development board, which meant that there was one development board across multiple classes at SJSU. I was an enthusiast and created a hybrid project composed of C and C++ sources. I love C++ a little more than C because I can express artistic designs in the language, but a language is really a tool, and you need to select the right tool for the job. Presently I work on Embedded 'Firmware' code for automotive industry which is in C, and 'Software' code in C++, because C is the right tool for firmware, and C++ is the right tool for software. "SJ2" was also designed by an enthusiast (Khalil), who is a very talented person, but expressing a starter project in C++ increased the complexity so much that many developers had a difficult time adopting it.

This is where the "SJ2-C" was born, which was completely re-designed to write simple code that everyone could understand. It was not just a mere C++ to C conversion, but an overhaul to apply year of experience to make things simpler to read, understand, and extend. Do you have friends or students in freshmen level class? Then SJ2-C is your best bet... do you have advanced designers that wish to use C++? Sure, you can also use SJ2-C and extend it with C++ libraries.
