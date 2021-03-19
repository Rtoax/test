Just-In-Time Compilation (libgccjit.so)

看个评论: libgccjit简直就只是一个玩具，根本没有办法拿来做即时编译器[所以我决定不学libgccjit]

https://gcc.gnu.org/wiki/JIT
-------------------------------------------------------
GCC can be built as a shared library "libgccjit.so", for generating machine code from API calls, using GCC as the backend.
This shared library can then be dynamically-linked into bytecode interpreters and other such programs that want to generate machine code "on the fly" at run-time.
It can also be used for ahead-of-time code generation, for building standalone compilers (so the "jit" part of the name is now something of a misnomer).
The library provides a C API, along with a C++ wrapper API, with bindings for languages available from 3rd parties (see below).
The API is very high-level, and is designed in terms of C semantics (since you're probably going to want to interface with C code).
