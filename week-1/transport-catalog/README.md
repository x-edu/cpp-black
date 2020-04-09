```bash
$ clang++-6.0 *.cpp -o main.o -std=c++17 -fsanitize=address -g -fno-omit-frame-pointer -fno-optimize-sibling-calls
$ cat input.txt | ASAN_SYMBOLIZER_PATH="/usr/lib/llvm-6.0/bin/llvm-symbolizer" ./main.o
```
