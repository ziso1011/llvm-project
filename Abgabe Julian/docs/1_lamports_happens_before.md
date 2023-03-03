# Lamport's Happens-Before Relation

Lamport was originally interested in a consistent (partial) order of all events happening in a distributed system. 
A distributed system is a system with multiple spatially separated processes running simultaniously. These processes communicate with each other via messages.
Due to their spatial separation, the message transmission delay is not negligible compared to the time between events in a single process.
This concept of a distributed system can be generalised and statements regarding distributed systems also apply to the case of a multithreaded program.

In such systems, a notion of an event *happening before* another is important for a multitude of scenarios. For example when two processes want to access an exclusive resource, we need a way to grant access to only one of the processes at a time (mutual exclusion). This however is not possible wihtout a strict ordering on the requests for this resource.

## Happens-before relation
Consider a distributed system with mutliple processes sending between and receiving messages from each other. Sending and receiving a message is an event in a process. Lamport defines the happens-before relation, denoted by $\leq$, as follows [[4]](ttps://lamport.azurewebsites.net/pubs/time-clocks.pdf)

**Definition *happens-before***

The relation $\leq$ on the set of events of a system is the smallest relation satisfying the three conditions:
1. If $a$ and $b$ are events in the same process and $a$ comes before $b$, then $a \leq b$
2. If $a$ is the sending of a message by one process and $b$ is the receipt of the same message by another process, then $a \leq b$
3. If $a \leq b$ and $b \leq c$, then $a \leq c$
4. $a \not\leq a$ for any event $a$

Two events are said to be **concurrent** if $a \not\leq b$ and $b \not\leq a$.


This definition of a happens-before relation allows us to make statements about the causality of two events; If $a \leq b$, it is possible that $a$ casually affected $b$, whereas when $a \not \leq b$ $a$ it is not possible that $a$ casually affected $b$.

### Application in data race detection

We can also make use of the happens-before relation above to detect data races in traces $\alpha$ of program executions. In order to do that, the release and acquisition of the lock now take the role of sending and receiving a message, while threads of the program are now equivalent to processes in distributed systems.

Releasing the lock $m$ by thread $t_1$ 

$rel(t_1, m)$ 

is the equivalent of a process sending a message, while the acquisition the same lock $m$ by thread $t_2$

$acq(t_2, m)$ 

after it has been released by $t_1$ is equivalent to the reception of the same message by another process. Therefore if $rel(t_1, m)$ appears before $acq(t_2, m)$ in the program trac, according to the definition we have $rel(t_1, m) \leq acq(t_2, m)$.

### Sources
- Lamports happens-before relation: https://lamport.azurewebsites.net/pubs/time-clocks.pdf