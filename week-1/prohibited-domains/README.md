```bash
$ # Build
$ clang++-6.0 ok.cpp -o main -std=c++17 -fsanitize=address -g -fno-omit-frame-pointer -fno-optimize-sibling-calls 

$ # Find symbolizer
$ dpkg -L llvm-6.0 | grep symbolizer

$ # Run
$ ASAN_SYMBOLIZER_PATH="/usr/lib/llvm-6.0/bin/llvm-symbolizer" ./main
```
