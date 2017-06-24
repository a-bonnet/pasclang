# The Pasclang Pseudo-Pascal compiler

## The project

Pasclang is a compiler for the educational language Pseudo-Pascal. It uses the LLVM infrastructure for code generation on different targets. The goal is to show how to implement a full compiler for a simple imperative language using CMake and the LLVM infrastructure and as few external libraries as possible.

## The Pseudo-Pascal language

Pseudo-Pascal is a Pascal dialect which features similar syntax and semantics but has several limitations. For example, only single-file programs can be compiled. The supported types are integers and booleans and only two built-in functions exist: `readln()` which reads an integer from the standard input, and `writeln(x)` where `x` is an integer printer to the standard output. The [test directory](test/) features several examples of programs written in Pseudo-Pascal. More examples will be added as features get implemented.
For those who are not familiar with the language, here is a sample program that computes the given Fibonacci number until 0 is given:

```delphi
program
var
    a : integer;
function fibonacci(n : integer) : integer;
begin
    if n = 0 then
        fibonacci := 0
    else if n = 1 then
        fibonacci := 1
    else
        fibonacci := fibonacci(n-1) + fibonacci(n-2)
end;
begin
    a := 1;
    while a > 0 do
        begin
            a := readln();
            writeln(fibonacci(a))
        end
end.
```

## The compiler

Pasclang aims to provide useful and diagnostics. Here is an example incorrect program:
```delphi
program
var
    a, b : integer;
    c : array of integer;
function proc(a : integer) : boolean;
begin
    b := 5;
    a := b + 1;
    writeln(b)
end;
begin
    c := new array of array of boolean[6];
    c[2] := readln();
    writeln(c[2]);
    b := proc(true);
    proc(true)
end.
```
when running `pasclang program.pp -f`, we get the following diagnostics:
```
warning: 
	unused variable a in function proc

error: at line 12
	unexpected type bool[2] instead of int[2] 

    c := new array of array of boolean[6];
         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: at line 15
	unexpected type bool[0] instead of int[0] 

    b := proc(true);
              ^^^^^
error: at line 15
	unexpected type bool[0] instead of int[0] 

    b := proc(true);
         ^^^^^^^^^^
error: at line 16
	unexpected type bool[0] instead of int[0] 

    proc(true)
         ^^^^^
error: at line 16
	invalid call to procedure or function proc

    proc(true)
    ^^^^^^^^^^
warning: 
	unused variable a in main body
```

## Building

Pasclang uses [CMake](https://cmake.org) as its build system since it is the one required to build LLVM.
In order to build Pasclang, you should [build LLVM](http://llvm.org/docs/CMake.html) and install it with development libraries, then make sure CMake can find it. This is usually done by having it installed in the default `/usr/...` path or defining an environment variable `LLVM_DIR` equal to the installation path. Then you can create your build directory and let CMake do the rest:

```bash
mkdir build
cd build
cmake ..
make -j<number of jobs>
```

You can then run the tests by using `make runtest`.

## Using

Command-line usage is documented by invoking the pasclang executable (present in the bin subfolder of the build directory) with no argument.

## Support

Pasclang is developed and tested on an amd64 Linux installation. It is known to build and pass all tests on Debian 9 and Fedora 25 with gcc >= 6.3 and clang >= 4.0.0 as well as LLVM >= 4.0.0. It has also been tested on musl-based distributions (Alpine Linux) as well as OpenBSD 6.1 and FreeBSD 11.0. Note that you may have to install static development libraries on some systems that do not ship them by default, e.g. if the linker gives the `cannot find -lc/-lstdc++/-lm` error. It should work on any Unix-like system with no change. Other platforms might require some adjustments. This is especially the case for the currently used way to link files after objects are built, see the `#warning` in `src/main.cpp`.

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
