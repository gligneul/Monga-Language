# Monga Language

Compiling and running tests:
```
make tests
```
Running benchmarks:
```
make benchmarks
```

Running examples:
```
Example:
    ./bin/monga < examples/sort.mng

Usage:
    monga [options] < [input]

Options:
    -h             Shows this message
    -bc            Exports the llvm bytecode file
    -dump          Dumps the llvm module
    -no-execution  Doesn't execute the monga program
```
