# TSan Projektarbeit

## Requirements

- CMAKE 3.13.4 or higher
- One of the following OS:
  - Android (aarch64, x86_64)
  - Darwin (arm64, x86_64)
  - FreeBSD (64-bit)
  - Linux (aarch64, x86_64, powerpc64, powerpc64le)
  - NetBSD (64-bit)

## Build

1. Clone project
2. `mkdir build && cd build`
3. `cmake -DLLVM_ENABLE_PROJECTS="clang;compiler-rt;" DCMAKE_BUILD_TYPE=Release ../llvm_source`
    This will build clang and the runtime libraries including the ThreadSanitizer project.
    For debug puprposes use `CMAKE_BUILD_TYPE=Debug`
4. `cmake --build . -jn`  to build with `n` cores.
5. (optional) `make check-tsan` to build and run tests for TSan

More information about llvm and the build flags can be found [here](https://llvm.org/docs/GettingStarted.html#requirements).

## Usage

In order to compile your program with TSan enabled, you have to options.

### Option 1 (CMake)

1. Add the following to the cmake file of your program:

    ```cmake
    set(CMAKE_CXX_COMPILER "path/to/your/llvm/project/build/bin/clang++")

    add_link_options(-fsanitize=thread)

    add_executable(program_name your_program.cc)
    add_executable(program_name_sanitized  your_programm.cc)
    set_target_properties(program_name_sanitized PROPERTIES COMPILE_FLAGS "-fsanitize=thread -fPIE -pie -g -O1")
    ```

2. `mkdir build && cd build`
3. `cmake path/to/source`
4. `make`
5. Run your programm with `./program_name_sanitized 2>log.txt`, which will write the TSan output into `log.txt`
6. Open the log file `log.txt` to analyze the TSan output

### Option 2 (build manually)

1. Set the flag to run with tsan and use the right compiler `path/to/your/llvm/project/build/bin/clang++ source.cpp -fsanitize=thread -fPIE -pie -g -O1`
2. Run your program with `./a.out 2> log.txt` to log the TSan output into `log.txt`
3. Open the log `log.txt` file to analyze the TSan output

## Logging

The following TSan events are logged:

- Read/write operations.
- Vector clocks synchronizations (not yet finished)

We use the `Printf` function provided by the sanitizer library in order to output log messages to `std::err`. The normal output of the program is written to `std::out`.

TSan uses instrumentation, where it injects its own code into the program being analyzed to monitor memory accesses and detect data races. Operations such as opening, closing, reading, and writing can cause the execution of the program to be interleaved with the execution of the runtime library.
This causes TSan to loose information, generate false positives or even crash.
For that reason we only can use the build in `Printf` function wich is a thread-safe and does not interleave with the program execution.

### Defining what will be logged

In the file `log.h`, several `#defines` are used that determine how the logging is done. To disable logging at the given positons, the appropriate line has to be commented out.

- `LOG_MUTEX_EPOCH_INCREMENTS`: Enables logging of epoch increments in the mutex (implemented in `tsan_rtl_mutex.cpp`)
- `LOG_MUTEX_ACTIONS`: Enables logging of actions in the mutex (implemented in `tsan_rtl_mutex.cpp`)

### Logging vector clocks

To log the vector clock of a given thread, the following code is used:

```cpp
void PrintVectorClock(__tsan::Context* ctx, __tsan::ThreadState* thr) {
    unsigned long nthread, nlive;
    ctx->thread_registry.GetNumberOfThreads(&nthread, &nlive);
    __sanitizer::Printf("Vector clock: [ ");
    for (int i = 0; i < nthread; ++i) {
        __sanitizer::Printf("%d ", (int)thr->clock.clk_[i]);
    }
    __sanitizer::Printf("]\n");
}
```

![grafik](https://user-images.githubusercontent.com/73063108/212671463-88f1ef17-4b7c-47bc-b56b-4f0006feaf1a.png)

This method prints the state of the vector clock at a certain time.

Where can we find the events in the code?
