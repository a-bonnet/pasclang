# The Pasclang Pseudo-Pascal compiler

## The project

Pasclang is a compiler for the educational language Pseudo-Pascal. It uses the LLVM infrastructure for code generation on different targets. The goal is to show how to implement a full compiler for a simple imperative language using CMake and the LLVM infrastructure and as few external libraries as possible.

## The Pseudo-Pascal language

Pseudo-Pascal is a Pascal dialect which features similar syntax and semantics but has several limitations. For example, only single-file programs can be compiled. The supported types are integers and booleans and only two built-in functions exist: `readln()` which reads an integer from the standard input, and `writeln(x)` where `x` is an integer printer to the standard output. The [test directory](test/) features several examples of programs written in Pseudo-Pascal. More examples will be added as features get (re)implemented.

## Building

Pasclang uses [CMake](https://cmake.org) as its build system since it is the one required to build LLVM.
In order to build Pasclang, you should [build LLVM](http://llvm.org/docs/CMake.html) and install it with development libraries, then make sure CMake can find it. This is usually done by having it installed in the default `/usr/...` path or defining an environment variable `LLVM_DIR` equal to the installation path. Then you can create your build directory and let CMake do the rest:

``mkdir build
cd build
cmake ..
make -j<number of jobs>``

## Using

Command-line usage is documented by invoking the pasclang executable (present in the bin subfolder of the build directory) with no argument.

## Support

Pasclang is developed and tested on an amd64 Linux installation. It should work on any Unix-like system with no change, other platforms might require some adjustments. This is especially the case for the currently used way to link files after objects are built, see the `#warning` in `src/main.cpp`.

## Source tree

### include

The `include` folder contains the header files necessary for the compiler's development and to use the project as a library. Currently these aren't installed with the `install` target. It is split in subfolders each corresponding to a namespace.

### src

The `src` directory contains the sources for object files organized the same way as header files. Only the driver's entry point (`main`) is not in the `pasclang` namespace and thus lies in the root of `src`.

### rt

This is the source directory for the runtime library of Pseudo-Pascal. The driver statically links each executable to the generated `libpasclang-rt.a` archive to provide the functions `readln`, `writeln` and the procedures required for dynamic memory allocation of arrays.

### test

Finally we have the Pseudo-Pascal source files used for testing. During the test each file `test.pp` is built, then given an input file `test.in`, the output is compared to `test.out` in order to check whether the test passed or failed.

## Acknowledgements

* [Luc Maranget's course](http://gallium.inria.fr/~maranget/X/compil/poly/index.html) was the main reason I chose Pseudo-Pascal (in French).
* The [LLVM Kaleidoscope Tutorial](http://llvm.org/docs/tutorial/index.html) is a thoroughly explained document on how to use the LLVM C++ and OCaml API for a functional programming language.