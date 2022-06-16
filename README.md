# UnicornSC
A SystemC wrapper for the Unicorn Engine (https://www.unicorn-engine.org/)

## What is the Unicorn Engine (https://www.unicorn-engine.org/)

### Unicorn - The Ultimate CPU emulator

Unicorn is a lightweight multi-platform, multi-architecture CPU emulator framework.

Highlight features:

* Multi-architectures: ARM, ARM64 (ARMv8), m68k, MIPS, PowerPC, RISC-V, S390x (SystemZ), SPARC, TriCore & x86 (include x86_64).
* Clean/simple/lightweight/intuitive architecture-neutral API.
* Implemented in pure C language, with bindings for Pharo, Crystal, Clojure, Visual Basic, Perl, Rust, Haskell, Ruby, Python, Java, Go, D, Lua, JavaScript, .NET, Delphi/Pascal & MSVC available.
* Native support for Windows & *nix (with macOS, Linux, Android, *BSD & Solaris confirmed).
* High performance by using Just-In-Time compiler technique.
* Support fine-grained instrumentation at various levels.
* Thread-safe by design.

## Build it

One need to have conan and cmake (>=3.16) installed as well as a decent gcc.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build 
```

To run the example one also needs the RSIC-V gcc toolsuite installed.

```
make -C example/fw/hello-world
build/example/unicorn-sc-ex example/fw/hello-world/hello
```

## TODO

* add gdb server to allow debugging
