# [The Pasclang Pseudo-Pascal compiler](https://gitlab.com/abonnet/pasclang/)

[Version Française/French version](README-fr.md)

[Website](http://arnaud.medichon.fr/pasclang.html)

## The project

Pasclang is a compiler for the educational language Pseudo-Pascal. It uses the LLVM infrastructure for code generation on different targets. The goal is to show how to implement a full compiler for a simple imperative language using CMake and the LLVM infrastructure and as few external libraries as possible.

## The Pseudo-Pascal language

Pseudo-Pascal is a Pascal dialect which features similar syntax and semantics but has several limitations. For example, only single-file programs can be compiled. The supported types are integers and booleans. Only three built-in functions exist: `readln()` which reads an integer from the standard input, `write(x)` and `writeln(x)` where `x` is an integer printed to the standard output (with or with no new line). The [test directory](test/) features several examples of programs written in Pseudo-Pascal. More examples will be added as features get implemented.
For those who are not familiar with the language, here is a sample program that computes the given Fibonacci number until we input 0:

```pascal
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

The current stable version is [Pasclang 1.2](https://gitlab.com/abonnet/pasclang/tree/1.2). Latest development version can be found in the [master branch](https://gitlab.com/abonnet/pasclang/tree/master).

The compiler operates via successive passes. The first one consists of parsing the input and producing the corresponding syntax tree (lexical and syntactic analyses). The next step is done with the type checker (semantic analysis) before using LLVM to produce the output.

Pasclang aims to produce useful diagnostics. Here is an example syntactically incorrect program:
```pascal
$ cat syntax.pp 
program
var a: array of integer;
procedure f(i : integer; a : array of integer; b : boolean);
begin
    writeln(a)
end
begin
    b := true
    a := new array of array of integer[a];
    a := f(156, a,)
end.
$ bin/pasclang syntax.pp -f
error: at line 7
        unexpected token begin when expecting any of the following: ;

begin
^^^^^
note: 
        Pasclang will now look for additional syntax errors. However since the input already contains an error, some reports may be wrong.

error: at line 10
        unexpected token ) when expecting any of the following: boolean literal, int literal, identifier, (

    a := f(156, a,)
                  ^
```
and for the semantic pass:
```pascal
$ cat type.pp 
program
var a: array of integer;
procedure f(i : integer; a : array of integer; b : boolean);
begin
    writeln(a, i)
end;
begin
    b := true;
    a := new array of array of integer[a];
    a := f(156, a, b)
end.
$ bin/pasclang type.pp -f
error: at line 5
        wrong number of arguments in call to writeln

    writeln(a, i)
    ^^^^^^^^^^^^^
error: at line 5
        unexpected type int[1] instead of int[0] 

    writeln(a, i)
            ^^
warning: 
        unused variable b in function f

warning: 
        unused variable i in function f

error: at line 8
        undefined symbol b

    b := true;
    ^^^^^^
error: at line 9
        unexpected type int[1] instead of int[0] 

    a := new array of array of integer[a];
                                       ^^
error: at line 9
        unexpected type int[2] instead of int[1] 

    a := new array of array of integer[a];
         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: at line 10
        invalid call to procedure or function f

    a := f(156, a, b)
         ^^^^^^^^^^^^
error: at line 10
        unexpected type int[2] instead of int[1] 

    a := f(156, a, b)
         ^^^^^^^^^^^^
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

Pasclang is developed and tested on an amd64 Linux installation. It is known to build and pass all tests on Debian 9 and Fedora 25 with stable gcc >= 6.3 and clang >= 4.0.0 as well as LLVM >= 4.0.0. Stable Pasclang versions have also been tested on musl-based distributions (Alpine Linux) as well as OpenBSD 6.1 and FreeBSD 11.0 (using LLVM and clang 3.9 for BSD distributions). Note that you may have to install static development libraries on some systems that do not ship them by default, e.g. if the linker gives the `cannot find -lc/-lstdc++/-lm` error. It should work on any Unix-like system with no change. Other platforms might require some adjustments. This is especially the case for the currently used way to link files after objects are built, see the `#warning` in `src/main.cpp`.

## Source tree

### [include](include/)

The `include` folder contains the header files necessary for the compiler's development and to use the project as a library. Currently these aren't installed with the `install` target. It is split in subfolders each corresponding to a namespace.

### [src](src/)

The `src` directory contains the sources for object files organized the same way as header files. Only the driver's entry point (`main`) is not in the `pasclang` namespace and thus lies in the root of `src`.

### [rt](rt/)

This is the source directory for the runtime library of Pseudo-Pascal. The driver statically links each executable to the generated `libpasclang-rt.a` archive to provide the functions `readln`, `writeln` and the procedures required for dynamic memory allocation of arrays.

### [test](test/)

Finally we have the Pseudo-Pascal source files used for testing. During the test each file `test.pp` is built, then given an input file `test.in`, the output is compared to `test.out` in order to check whether the test passed or failed.

## Acknowledgements

* [Luc Maranget's course](http://gallium.inria.fr/~maranget/X/compil/poly/index.html) was the main reason I chose Pseudo-Pascal (in French).
* [François Pottier](http://cristal.inria.fr/~fpottier) kindly allowed me to use his [MIPS compiler](http://cristal.inria.fr/~fpottier/X/INF564/petit.tar.gz)'s tests for Pasclang. They are available using the `make runmoretests` command.
* The [LLVM Kaleidoscope Tutorial](http://llvm.org/docs/tutorial/index.html) is a thoroughly explained document on how to use the LLVM C++ and OCaml API for a functional programming language.

