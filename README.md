# My experimental Libraries for C++ (LCXX)

* These libraries are all experimental! In general, they might not be finished, they might not be debugged, they might lack of appropriate tests or documentation... they are just a collection of work-in-progress.

* These libraries target modern C++ compilers and they might leverage features added to the language for and after C++11 (even if it might technically possible to rework them, especially using Boost, to compile prior to C++11).

Libraries:

## ISystem
Emulate GCC -isystem on all compilers using the preprocessor.

## ScopeExit
Execute code at scope exit, without macros (useful together with C++11 lambdas).

## Singleton
Manage single-instance objects (thread-safe on C++11).

## Separate
Implement concurrent model similar to Simple Concurrent Object-Oriented Programming (SCOOP) of the Eiffel programming language.

License:
Distributed under the [Boost Software License, Version 1.0](http://boost.org/LICENSE_1_0.txt).
