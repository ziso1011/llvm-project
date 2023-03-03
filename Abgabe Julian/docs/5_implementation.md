# Implementation

The implementation is done in LLVM.
It is split into two parts: the instrumentation module and the runtime-library. 

## Thread State

A thread state is comprised of:
- `FastState`: A bit field storing
  - ignore: Ignore Bit
  - tid: Thread ID
  - unused          : -

  - epoch           : kClkBits

Locations with added logging:

## Logging

**Vector Clock**
`log.h` and `log.cpp` implement the function `PrintVectorClock()`, that given a thread context and a thread state prints the current state of the vector clock associated to the current thread.


### Thread State

Each thread has its own state, which is implemented [here](https://github.com/llvm/llvm-project/blob/b57819e130258b4cb30912dcf2f420af94d43808/compiler-rt/lib/tsan/rtl/tsan_rtl.h#L158). It roughly looks like this:

```cpp
struct ThreadState {
    FastState fast_state;
    VectorClock clock;
    const Tid tid;
    bool is_dead;
}
```

```cpp
struct VectorClock {
    // One logical clock value for every thread
    // Epoch is a 16 bit unsigned integer
    Epoch[] clocks;
    
    // Resets the vector clock to [0, ..., 0]
    reset()

    // Acquires a lock
    // src: The vector clock of the lock acquired
    acquire(VectorClock src)

    release(VectorClock dst)
    releaseStore(VectorClock dst)
    releaseStoreAcquire(VectorClock dst)
    releaseAcquire(VectorClock dst)
}
```

### Epoch
An epoch is a pair $(c, t)$ of
- the clock $c$ of
- a thread $t$

It is denoted as $c@t$.

#### Happens-before relation of epochs
An epoch $c@t$ *happens-before* an epoch $c'@t$ if and only if the clock $c$ is less or equal to the clock $c'$:

$c@t \leq c@t$ iff $c \leq c'$

An epoch $c@t$ *happens-before* a vector clock $V$ if and only if the clock of the epoch is less than or equal to the corresponding clock of thread $t$ in the vector clock: 

$c@t \leq V$ iff $c \leq V_t$

#### Code representation

An epoch is represented in the code by a `u_16`

```cpp
enum class Epoch : u16 {};
```

This is part of the `FastState` [here](https://github.com/llvm/llvm-project/blob/b57819e130258b4cb30912dcf2f420af94d43808/compiler-rt/lib/tsan/rtl/tsan_shadow.h#L32) which in turn is part of the state for each thread ([here](https://github.com/llvm/llvm-project/blob/b57819e130258b4cb30912dcf2f420af94d43808/compiler-rt/lib/tsan/rtl/tsan_rtl.h#L159)). -->