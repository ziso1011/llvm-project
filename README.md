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

1. Clone this project
2. `mkdir build && cd build`
3. `cmake -DLLVM_ENABLE_PROJECTS="clang;compiler-rt;" DCMAKE_BUILD_TYPE=Release ../llvm_source`
    This will build clang and the runtime libraries including the ThreadSanitizer project.
    For debug puprposes use `CMAKE_BUILD_TYPE=Debug`
4. `cmake --build . -jn`  to build with `n` cores.
5. (optional) `make check-tsan` to build and run tests for TSan

More information about llvm and the build flags can be found [here](https://llvm.org/docs/GettingStarted.html#requirements).

## Usage

In order to compile your program with TSan enabled, you have two options.

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

**note:** To just get it as console output, remove the `2> log.txt`. 

## Logging

The following TSan events can be logged:

- Read/Write operations.
- Fork/Join/Finished operations for Threads.
- Lock/Unlock mutex operations.
- Mutex actions.
- Epoch increments.


We use the `Printf` function provided by the sanitizer library in order to output log messages to `std::err`. The normal output of the program is written to `std::out`.

### Why we can only use the build in `Printf`

TSan uses instrumentation to monitor memory accesses and detect data races by injecting its own code into the program being analyzed. However, certain operations such as opening, closing, reading, and writing can cause interleaved execution of the program with the runtime library, which may result in TSan losing information, generating false positives, or crashing.

To address this issue, the developers of TSan have designed a dedicated print method that ensures the output is atomic and consistent. As a result, only the built-in `Printf` function should be used because it is thread-safe and does not interleave with program execution.

### Defining what will be logged

The file `log.h` can be found at `llvm-project/compiler-rt/lib/tsan/rtl/log.h`, and it contains several `#defines` that specify how logging is performed. To disable logging at specific locations, simply comment out the appropriate line.

- `ENABLE_TSAN_DEFAULT_OUTPUT`: Enables the default Report output of Tsan.(implemented in `tsan_report.cpp`). To improve visibility of other logs, it is recommended to comment this out.
- `LOG_THREAD_ON_READ` : Enables logging of read Operations (implemented in `tsan_rtl_access.cpp`).
- `LOG_THREAD_ON_WRITE` : Enables logging of write Operations in threads (implemented in `tsan_rtl_access.cpp`).
- `LOG_MUTEX_LOCK_UNLOCK` : Enables logging for lock/unlock mutex operations (implemented in `tsan_rtl_mutex.cpp`).
- `LOG_THREAD_JOIN` : Enables logging of join operations in threads (implemented in `tsan_rtl_thread.cpp`).
- `LOG_THREAD_FORK` : Enables logging of fork operations in threads (implemented in `tsan_rtl_thread.cpp`).
- `LOG_THREAD_FINISH` : Enables logging if a thread is finished (implemented in `tsan_rtl_thread.cpp`).
- `LOG_MUTEX_EPOCH_INCREMENTS`: Enables logging of epoch increments in the mutex (implemented in `tsan_rtl_mutex.cpp`).
- `LOG_MUTEX_ACTIONS`: Enables logging of actions in the mutex (implemented in `tsan_rtl_mutex.cpp`).
- `LOG_THREAD_EPOCH`: Enables logging of the thread epochs on the different events (impelemented in the respective files).
- `LOG_CODE_LINE` : Enables logging of the line of code (impelemented in the respective files).

**Note**: "Due to the output not being thread-safe, there may be print races. Unfortunately, uncommenting LOG_CODE_LINE also leads to faulty or inconsistent output. Therefore, the output should always be considered with caution."

**After making any changes, the LLVM compiler must also be recompiled !!!**

### Output format

- `Read`: Thread ThreadID | r(memory address) | line of code | epoch
- `Write`: Thread ThreadID | wr(memory address) | line of code | epoch 
- `Lock`: Thread ThreadID | l(memory address) | line of code | epoch
- `Unlock`: Thread ThreadID | u(memory address) | line of code | epoch
- `Join`: Thread ThreadID | j(terminated Thread) | epoch
- `Fork`: Thread ThreadID | f(started Thread) | epoch
- `Finished`: Thread ThreadID: Finished

### Printing function

In order to print output without data races in TSan itself and to avoid repeating code, we wrote a function deep down in the stack of Tsan. It is called `PrintFileAndLine` (defined in `tsan_rtl_report.cpp`) and calls `PrintFileAndLineOfStack` (defined in `tsan_report.cpp`). Its arguments are the thread state, the address of the variable/lock being acted upon and a string representing the event (e.g. "rd" for reading a variable) and it logs the event in the output format from above.

### Print statement locations

1. `Read` and `Write` (implemented in `tsan_rtl_access.cpp`)
```cpp
ALWAYS_INLINE USED void MemoryAccess(ThreadState* thr, uptr pc, uptr addr,
                                     uptr size, AccessType typ) {
  RawShadow* shadow_mem = MemToShadow(addr);S
 
  if (typ == kAccessWrite) {
    #ifdef LOG_THREAD_ON_WRITE
    PrintFileAndLine(thr, pc, "wr", addr);
    #endif
  } else if (typ == kAccessRead) {
    #ifdef LOG_THREAD_ON_READ
    PrintFileAndLine(thr, pc, "rd", addr);
    #endif
  }
  ...
```
`MemoryAccess()` is a function that performs memory access tracking and race detection for a thread.

2. `Lock` and `Unlock` (implemented in `tsan_rtl_mutex.cpp`)
```cpp
static void RecordMutexLock(ThreadState *thr, uptr pc, uptr addr,
                            StackID stack_id, bool write) {
  auto typ = write ? EventType::kLock : EventType::kRLock;
  #ifdef LOG_MUTEX_LOCK_UNLOCK
    PrintFileAndLine(thr, pc, "l", addr);
  #endif
  // Note: it's important to trace before modifying mutex set
  // because tracing can switch trace part and we write the current
  // mutex set in the beginning of each part.
  // If we do it in the opposite order, we will write already reduced
  // mutex set in the beginning of the part and then trace unlock again.
  TraceMutexLock(thr, typ, pc, addr, stack_id);
  thr->mset.AddAddr(addr, stack_id, write);
}

static void RecordMutexUnlock(ThreadState *thr, uptr addr) {
  // See the comment in RecordMutexLock re order of operations.
  #ifdef LOG_MUTEX_LOCK_UNLOCK
  PrintFileAndLine(thr, pc, "u", addr);
  #endif
  TraceMutexUnlock(thr, addr);
  thr->mset.DelAddr(addr);
}
```
`RecordMutexLock()` and `RecordMutexLock()` are functions that record information about mutex lock and unlock operations.

3. `Join` (implemented in `tsan_rtl_thread.cpp`)
```cpp
void ThreadJoin(ThreadState *thr, uptr pc, Tid tid) {
  CHECK_GT(tid, 0);
  DPrintf("#%d: ThreadJoin tid=%d\n", thr->tid, tid);
  #ifdef LOG_THREAD_JOIN
  Printf("%d | j(%d) \n", thr->tid, tid);
  #endif
  JoinArg arg = {};
  ctx->thread_registry.JoinThread(tid, &arg);
  if (!thr->ignore_sync) {
    SlotLocker locker(thr);
    if (arg.sync_epoch == ctx->global_epoch)
      thr->clock.Acquire(arg.sync);
  }
  Free(arg.sync);
}
```
`ThreadJoin()` is a function that performs a thread join operation.

4. `Fork` (implemented in `tsan_rtl_thread.cpp`)
```cpp
Tid ThreadCreate(ThreadState *thr, uptr pc, uptr uid, bool detached) {
  // The main thread and GCD workers don't have a parent thread.
  Tid parent = kInvalidTid;
  OnCreatedArgs arg = {nullptr, 0, kInvalidStackID};
  if (thr) {
    parent = thr->tid;
    arg.stack = CurrentStackId(thr, pc);
    if (!thr->ignore_sync) {
      SlotLocker locker(thr);
      thr->clock.ReleaseStore(&arg.sync);
      
      arg.sync_epoch = ctx->global_epoch;
      IncrementEpoch(thr);
    }
  }
  Tid tid = ctx->thread_registry.CreateThread(uid, detached, parent, &arg);
  DPrintf("#%d: ThreadCreate tid=%d uid=%zu\n", parent, tid, uid);
  #ifdef LOG_THREAD_FORK
  Printf("%d | f(%d)\n", parent, tid);
  #endif
  return tid;
}
```
`ThreadCreate()` is a function that creates a new thread.

5. `Finished` (implemented in `tsan_rtl_thread.cpp`)
```cpp
void ThreadFinish(ThreadState *thr) {
  DPrintf("#%d: ThreadFinish\n", thr->tid);
  #ifdef LOG_THREAD_FINISHED
  Printf("%d Finished\n", thr->fast_state.sid());
  #endif
  PrintVectorClock(ctx, thr);
  ThreadCheckIgnore(thr);
  if (thr->stk_addr && thr->stk_size)
    DontNeedShadowFor(thr->stk_addr, thr->stk_size);
  if (thr->tls_addr && thr->tls_size)
    DontNeedShadowFor(thr->tls_addr, thr->tls_size);
  thr->is_dead = true;
  ...
```
`ThreadFInish()` is a function that is called when a thread is finished or terminated.

### Logging vector clocks

Like the other `#defines` u can comment in the `PRINT_VECTOR_CLOCK` in the `log.h` file.
Please note that we have only provided the vector clocks here for testing purposes. Therefore, the output may be inaccurate or incomplete.

To log the vector clock of a given thread, the following code is used: (implemented in `log.cpp`)

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
This function prints the state of the vector clock at a certain time.

### Example Output

Example output for the following program (`Abgabe Julian/examples/locking_example.cc`):
```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex m;
int x = 0;
int y = 0;
int z = 0;

void T1() {
    m.lock();
    x = 1;
    m.unlock();
    y = 2;
}

void T2() {
    z = x + y;
    m.lock();
    m.unlock();
}

int main() {
    std::thread t1(T1);
    std::thread t2(T2);
    
    t1.join();
    t2.join();

    std::cout << "z: " << z << std::endl;

    return 0;
}
```

Generated Output:
```
// Main thread gets spawned
Thread -1 | f(0)

// Spawn of thread 1 by main thread
Thread 0 | f(1)

// Spawn of thread 1 by main thread
Thread 0 | f(2)

// Lock of mutex by thread 1
Thread 1 | l(0x00010410c000) | locking_example.cc:11

// Write to variable x by thread 1
Thread 1 | wr(0x00010410c040) | locking_example.cc:12

// Unlock of mutex by thread 1
Thread 1 | u(0x00010410c000) | locking_example.cc:13

// Write to variable y by thread 1
Thread 1 | wr(0x00010410c044) | locking_example.cc:14

// Read of variable x by thread 2
Thread 2 | rd(0x00010410c040) | locking_example.cc:19

// Main thread joins thread 1
Thread 0 | j(1) 

// Read of variable y by thread 2
Thread 2 | rd(0x00010410c044) | locking_example.cc:20

// Write to variably z by thread 2
Thread 2 | wr(0x00010410c048) | locking_example.cc:21

// Main thread joins thread 2
Thread 0 | j(2)

// Main thread reads z to output it in the console
Thread 0 | rd(0x00010410c048) | locking_example.cc:33

// Report of the thread sanitizer, if enabled
ThreadSanitizer: reported 1 warnings
<report>
```

## Common Problems

### Platform issues:

Since LLVM and Clang have been developed and tested for many different platforms, it is possible that certain features may be platform-dependent and may lead to compilation errors or incorrect behavior.

### Configuration issues:

Errors can also occur in the configurations. There are platform-specific bugs that occur when certain features are enabled or disabled. An incorrect configuration can result in certain features being unavailable or the code not compiling correctly.

### Memory errors during compilation:

During compilation, large amounts of code and data are held in memory, especially when multiple processes are running simultaneously. To minimize this problem, you should ensure that you have enough memory to perform the build and close unnecessary processes and applications during the build. You can also reduce the memory requirements of the build process by disabling certain features that you do not need, or by using a less extensive configuration. In some cases, optimized settings for the compiler and linker can also help to reduce the memory requirements and make the build faster and more reliable.

### Using C++ libraries in the project:

As of now, it is unfortunately not possible to use C++ Standard-libraries. However, most C libraries can be used without any problems.
