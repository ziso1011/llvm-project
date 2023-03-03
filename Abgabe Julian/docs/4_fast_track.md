- FastTrack paper: https://users.soe.ucsc.edu/~cormac/papers/pldi09.pdf
- Vector Clocks paper: https://www.vs.inf.ethz.ch/publ/papers/VirtTimeGlobStates.pdf

# Fast Track

FastTrack builds on DJIT+, but improves on it. The key observation made in the paper is that using vector clocks is not necessary to establish happens-beforerelations for most of the operations executed in the investigated program. It achieves better performance by tracking less information and dynamically adapting its representation of the happens-before relation based on memory access patterns.

To achieve that, FastTrack uses a hybrid representation of vector clocks that combines an epoch (a pair of a thread identifier and a logical time) with a full vector clock. An epoch represents a single thread’s view of time, while a full vector clock represents multiple threads’ views of time. FastTrack uses epochs whenever possible to reduce space and time overheads, and switches to full vector clocks only when necessary to maintain precision.

As DJIT+ and other vector clock based race detectors require O(n) storage and O(n) time for comparing, copying and joining vector clocks for n threads, using only an epoch instead of a full vector clock significantly improves performance.

## Epochs and vector clocks

An epoch in FastTrack is defined as the logical clock $c$ of a given thread $t$, denoted as $c@t$. It therefore is equivalent to the $t$'th component of the vector clock for thread $t$ $C_t[t]$ in the DJIT+ algorithm. It can therefore be stored as a single value.

### Comparing epochs and vector clocks

The definition of a happens-before relation between an epoch $c@t$ and a vector clock $C$ can be deduced from the definition of an epoch above:

$c@t \leq C \Leftrightarrow c \leq C[t]$

In contrast to comparing two vector clocks, which requires $O(n)$ time, comparing an epoch and a vector clock only requires $O(1)$ time.

### Usage of epochs and vector clocks in FastTrack

As with DJIT+, full *vector clocks* are used for
- every thread $t$: $C_t$
- every lock $m$: $L_m$

For variables however, FastTrack mainly uses *epochs*.
- When a thread $t$ writes to a variable $x$, $W_x$ just records the epoch $c@t$ of the write:. Hence, the variable $x$ stores only the information about the logical clock of the last thread that has written to it.
- When a thread $t$ reads from a variable $x$, $R_x$ may record the epoch of this read or update its full vector clock, dependent on the circumstances:
  - If all reads are totally ordered, i.e. the reads happen thread-local or lock-protected, $R_x$ is just the epoch of the last read operation
  - If the reads are not totally ordered, i.e. the reads happen from different threads and without locking, $R_x$ is a vector clock that is the join of all reads of $x$

## Detecting race conditions

## Acquiring locks

Acquiring and releasing locks is handled exactly as in the DJIT+ algorithm, as both the threads and locks have vector clocks.

## Read operations

When a thread $t$ reads from a variable $x$, FastTrack distinguishes between four cases. 

- **Subsequent reads from the same epoch**: This happens, when the reads happen within the same and epoch. The reads might be from the same thread or from different threads, as long as their logical clock values are the same. In this case, $R_x$ is an epoch and $R_x$ is not updated, as the read happened in the same epoch as the read before it. No race conditions may occur in this case. In this case, no race condition may occur. 

    These cases make up 78% of all read operations and require nearly no computing resources at all. If this case is true, no further computations or checks need to be done and the handling of the read operation by FastTrack is finished.

After this simple check, FastTrack checks for a write-read race. Let $t$ be the thread reading variable $x$ at epoch $s@t$. If $W_x \leq C_t \Leftrightarrow W_x \leq s$, meaning the last write was in the epoch before or the same as this read, no race condition is detected. Otherwise, a write-read race is raised.

How the read operation is processed from now on depends on whether the variable is shared or not. Whether a variable is shared or not is determined by the algorithm itself. 

Let's first look at the two cases where the variable is not shared. We assume, that $R_x = q@u$, i.e. that the last read of variable $x$ was by thread $u$ in epoch $q@u$ and that the current read is done by thread $t$ in epoch $s@t$.

- **Last read happened in epoch before current read epoch**: This is the case when $R_x \leq C_t \Leftrightarrow u \leq C_t[u]$, i.e. if the last read on the variable happened in an epoch *before* the epoch of the current read (the same epoch is handled in the first case above), either by the same thread or another thread. In this case, FastTrack simply updates the read epoch of $x$ to the epoch of the currently reading thread: $R_x \leftarrow s@t$.

- **Last read happened in epoch after current read epoch**: This is the case if the last read on the variable happened in an epoch *after* the epoch of the current read. This means that the current read may be concurrent to the previous read in another epoch. In this case, the variable $x$ is marked as *shared* and the $R_x$ becomes a full vector clock instead of an epoch. This new vector clock has value 0 for all elements except the clock elements for the thread $u$ of the previous read, which is set to $q$, and for the thread $t$, which is set to $s$. This is importang as either thread $u$ or $t$ could subsequently participate in a read-write race, and we need to track the epochs of their last reads on $x$ in order to check for those races when writing to variable (see below). This incurs a higher memory usage in order to store the vector clock for the variable.

When a variable is marked as read shared by the third rule above, i.e. when its $R_x$ is a vector clock, the following case comes into play:

- **Read on a shared variable**: The vector clock element $R_x[t]$ corresponding to the reading thread $t$ is set to $t$'s current logical clock value $s$: $R_x[t] = s$. This incurs a higher memory usage in order to store the vector clock for the variable.

### Write operations

We assume that the previous write on variable $x$ is $W_x = q@u$, i.e. was done by thread $u$ in epoch $q$, and that the current write operation is done by thread $t$ in epoch $s@t$. 

FastTrack distinguishes between three cases and checks for several possible race conditions.

- **Writing in the same epoch**: When $W_x = s@t$, i.e. when the same threads subsequently writes to the same variable in the same epoch, no race condition is possible and nothing has to be done. 

    This case makes up roughly 71% of write operations and require nearly no computing resources at all. If this case is true, no further computations or checks need to be done and the handling of the read operation by FastTrack is finished.

When this simple case is not true, FastTrack checks for a write-write race: If $W_x \gt C_t \Leftrightarrow q \gt C_t[u]$, i.e. the last write on the variable happened in an epoch *after* the epoch of the current write, a **write-write race condition is raised**, as this means that two subsequent writes happened without synchronization.

After that, FastTrack distinguishes between read-exclusive and read-shared variables (see above). 

- **Write to a read-exclusive variable**: When writing to a read-exclusive variable , $R_x$ is an epoch. FastTrack first checks for a read-write race: If $R_x \gt C_t$, i.e. if the last read of variable $x$ happened by another thread in an epoch *after* the current write epoch $s@t$, this means that the read may return unexpected values, as the variable may have been changed before the read by another thread without proper synchronization and **read-write race is raised**.
- **Write to a read-shared variable**: When writing to a read-shared variable $x$, $R_x$ is a full vector clock. FastTrack first checks for a shared-write race: If either $R_x \gt C_t \Leftrightarrow q > C_t[u]$ or $W_x \gt C_t$, i.e. if the last read or write of the variable $x$ happened in an epoch *after* the current write epoch $s@t$, the current write is not properly synchronized and may interfere with writes and/or reads from other threads, altering values that are not expected to be altered by the other thread, therefore leading to an overwrite of those values by the other thread or to unexpected values for the other thread when reading the variable. Therefore a **shared-write data race is raised**. 

If in neither of those two cases a data race is detected, $W_x$ is simply updated to the current epoch of the write operation $W_x = s@t$.

## Example 

Let’s consider the same program as in the DJIT+ example, T1 and T2, that access two shared memory locations, x and y. The program is as follows:

```
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

- The threads 1 and 2 have their own vector clock, $C_{T_1}$ and $C_{T_2}$, which are both initialized with [0, 0].
- The variables/memory locations x and y have epochs $R_x$, $R_y$, $W_x$, $W_y$ for reading and writing initialized with 0.
- The lock has a vector clock that is initialized with [0, 0].

The algorithm will perform the following steps:

1. $T_1$ acquires lock m. As the locks vector clock $L_m$ has the value $[0, 0]$, the vector clock of $T_1$ stays at the value $[0, 0]$.

    New state: 
    - $C_1 = [0, 0]$
    - $C_2 = [0, 0]$
    - $L_m = [0, 0]$
    - $R_x = 0@0$, $W_x = 0@0$
    - $R_y = 0@0$, $W_y = 0@0$
    - $R_z = 0@0$, $W_z = 0@0$

2. $T_1$ writes to x. The algorithm logs this access and updates $W_x$ to $0@1$, indicating a write access of $T_1$ in its first epoch.

    New state: 
    - $C_1 = [0, 0]$
    - $C_2 = [0, 0]$
    - $L_m = [0, 0]$
    - $R_x = 0@0$, $W_x = 0@1$
    - $R_y = 0@0$, $W_y = 0@0$
    - $R_z = 0@0$, $W_z = 0@0$

3. $T_1$ releases lock m. Releasing the lock sets its vector clock to $C_{T_1}$: $L_m \leftarrow C_{T_1} = [1, 0]$. Additionally, the epoch of thread $t$ is incremented and  its vector clock is updated to $[1, 0]$, as a new time frame for thread $T_1$ begins.

    New state: 
    - $C_1 = [1, 0]$
    - $C_2 = [0, 0]$
    - $L_m = [0, 0]$
    - $R_x = 0@0$, $W_x = 0@1$
    - $R_y = 0@0$, $W_y = 0@0$
    - $R_z = 0@0$, $W_z = 0@0$

4. $T_1$ writes to y. The algorithm checks for a read-write race, but as $R_y = 0@0 \leq 2@T_1$, no race is detected. It then logs this access by updating $W_x$ to $2@1$, indicating a write of $T_1$ in time frame $2$.

    New state: 
    - $C_1 = [2, 1]$
    - $C_2 = [1, 1]$
    - $L_m = [1, 0]$
    - $R_x = 0@0$, $W_x = 2@1$
    - $R_y = 0@0$, $W_y = 0@0$
    - $R_z = 0@0$, $W_z = 0@0$

5. $T_2$ reads from x. This happens in epoch 

    New state: 
    - $C_1 = [2, 1]$
    - $C_2 = [1, 1]$
    - $L_m = [1, 0]$
    - $R_x = 0@0$, $W_x = 2@1$
    - $R_y = 0@0$, $W_y = 0@0$
    - $R_z = 0@0$, $W_z = 0@0$

This is the first read access to x in the current time frame of $T_2$. Two things happen:
    - The algorithm logs this access by updating $R_x[T_2] \leftarrow C_{T_2}[T_2] = 1$.
    - The algorithm checks whether thread $T_1$ released a lock before writing to $x$. As $W_{x}[T_1] = 1 \lt C_{T_1}[T_1] = 2$, this is not the case. Hence, no data race is reported.
6. $T_2$ reads from y. This is the first read access to y in the current time frame of $T_2$. Two things happen:
    - The algorithm logs this access by updating $R_y[T_2] \leftarrow C_{T_2}[T_2] = 1$.
    - The algorithm checks whether thread $T_1$ released a lock before writing to $<$. As $W_{<}[T_1] = 2 \geq C_{T_1}[T_1] = 2$, this is the case. 
    
      **Therefore a data race is reported.**
7. $T_2$ acquires lock m. This is an event that updates its vector clock to [2, 1].
8. $T_2$ releases lock m. This is an event that updates its vector clock to [2, 2].
9. ...