# UnicornSC
A SystemC wrapper for the Unicorn Engine (https://www.unicorn-engine.org/)

## Build it

One need to have conan and cmake (>=3.16) installed as well as a decent gcc.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build 
```

To run teh example one also needs the RSIC-V gcc toolsuite installed.

```
make -C example/fw/hello-world
build/example/unicorn-sc-ex example/fw/hello-world/hello
```

## TODO

* add gdb server to allow debugging