Build with

```bash
cmake -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DLLVM_ENABLE_PROJECTS="clang;compiler-rt;" -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" -DCMAKE_BUILD_TYPE=Release COMPILER_RT_USE_LIBCXX=ON-G "Unix Makefiles" ../llvm
```

Run with
```bash
./program_name 2>tsan.log
```