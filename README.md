# UnicornSC
A SystemC wrapper for the Unicorn Engine (https://www.unicorn-engine.org/)

## Build it

One need to have conan and cmake (>=3.16) installed as well as a decent gcc.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build 
```

## TODO

* add gdb server to allow debugging