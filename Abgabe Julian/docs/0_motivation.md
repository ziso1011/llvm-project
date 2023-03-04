# Motivation

A data race occurs when there are at least two concurrent accesses to the same memory location where
- at least one of them is a write and
- at least two of them are unsynchronized
[[1]](https://dl.acm.org/doi/abs/10.1145/781498.781529?casa_token=8AmTkNBzhvAAAAAA%3Ajb8pgsNiUyn0ofcbYy01vp8lbZIIF-NwXNh3DxAxHwUXwjkuZ8Y3s44VhcWLlbLTxP5lqQMb7Kh-vQ)

### Example

Consider the following program written in C++:

```cpp
#include <pthread.h>
#include <iostream>

const int iter_count = 100000;

long int Global;
void *Thread1(void *x)
{
  Global = 42;
  return x;
}

int main()
{
  pthread_t t;
  pthread_create(&t, NULL, Thread1, NULL);
  Global = 43;
  std::cout << Global << std::endl;
  pthread_join(t, NULL);
  return 0;
}
```

Here, in the main thread a second thread is spawned. Both threads have access to the global variable `Global`. 
After the spawning of the second thread, the main thread sets the global variable to the value of 43, while the spawned thread attempts to set the same variable to the value 42. As they are trying to do this multiple times (e. g. in a loop), there is a high chance, that the execution times of these threads overlap. Therefore, both threads may attempt to write to the same memory location at the same time. 
This may lead to unintended or undefined behaviour; it is not determined whether the output statement 
```cpp
std::cout << Global << std::endl;
```
will print 42 or 43, as this depends on the order of execution of the two write operations setting the variable to 42 and 43 respectively and the read operation required for printing the value of the variable.

## Data Race Detection Techniques

We want to detect race conditions as the one in the example above in order to debug multithreaded programs and ensure their correct and predictable behaviour. There are several approaches to achieve this.[[1]](https://dl.acm.org/doi/abs/10.1145/781498.781529?casa_token=8AmTkNBzhvAAAAAA%3Ajb8pgsNiUyn0ofcbYy01vp8lbZIIF-NwXNh3DxAxHwUXwjkuZ8Y3s44VhcWLlbLTxP5lqQMb7Kh-vQ)
- **Static**: Analyze the source code of a program at compile time
- **Dynamic**: Work with traces of actual program executions
    - **On-the-fly**: Buffers and analyzes partial trace information, detects races during program execution
    - **Postmortem**: Saves the trace of the program and analyzes it after execution


### Static

Static data race detection techinques analyze the source code of the program at compile-time.

**Advantages**

- Checks the program globally for possible race data races
- Granular analysis of the data involved in possible data races, as they can analyze at the level of variables, structures and classes of the programming language
- As these analysis happens at compile-time, the performance of the program at run-time is not negatively effected at all

**Disadvantages**

- Static analysis is generally slow, as finding data races is a NP-hard problem in the general case ([source](https://www.researchgate.net/publication/2592040_On_the_Complexity_of_Event_Ordering_for_Shared-Memory_Parallel_Program_Executions)).
- Static analysis generates excessive false alarms, masking the important real data races.
- Static analysis may not handle some language features well, such as dynamic class loading in Java.

### Dynamic

Dynamic data race detection is based on actual executions of the program. During the execution, a *trace* of a particular is created (see below), stored and the execution is evaluated based on that trace.

**Advantages**

- Only data races that actually occured during real executions are detected (no false alarms).
- Computationally less complex than static race detection.

**Disadvantages**

- As the behaviour multithreaded programs may change between several executions, dynamic analysis might miss data races based on only one execution. Therefore, multiple runs of the program may be required in order to detect data races.
- Incurs a big performance hit while running the program.

**Trace**

A *trace* $\alpha$ is a sequence of operations performed by variaous threads during the execution of a multithreaded program. In this context, the set of operations a thread $t$ can perform are: [[3]](https://dl.acm.org/doi/abs/10.1145/1543135.1542490?casa_token=Mx7WqwWcF1IAAAAA:n23wZjVnOMFBbqcMWEsuODH-3-JfJMmwJxWGFa9Ihv4vG5IMZ8bajMSkQqRsYxmFi79XG6N5akhd)

- $r(t, x)$: read value from a variable $x$
- $w(t, x)$: write value to a variable $x$
- $acq(t, m)$: acquire a lock $m$
- $rel(t, m)$: release a lock $m$
- $fork(t, u)$: fork a new thread $u$
- $join(t, u)$: block thread $t$ until thread $u$ terminates

We will discuss in later sections how traces can be used to determine (data) race conditions.

## FastTrack and ThreadSanitizer

$\textit{FastTrack}$ is dynamic data race detection technique presented in [[3]](https://dl.acm.org/doi/abs/10.1145/1543135.1542490?casa_token=Mx7WqwWcF1IAAAAA:n23wZjVnOMFBbqcMWEsuODH-3-JfJMmwJxWGFa9Ihv4vG5IMZ8bajMSkQqRsYxmFi79XG6N5akhd). It is the theoretical basis of the [ThreadSanitizer](https://github.com/llvm/llvm-project/tree/main/compiler-rt/lib/tsan) implemented in the [LLVM project](https://llvm.org/).

In order to understand $\textit{FastTrack}$, we will first look at Lamports happens-before relation, Lamport clocks, vector clocks and the  $\textit{DJIT+}$ algorithm FastTrack is based on.