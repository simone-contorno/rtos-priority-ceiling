# Reat-Time Operating Systems
## 4 threads managed with semaphores (Priority Ceiling Policy)

<br><br>
Robotics Engineering - Simone Contorno<br>

### Introduction
An overview of this program function.<br>
[Go to Introduction](#intro)

### How it works
A rapid description of how the program works.<br>
[Go to How it works](#how)

### Installation and Execution
How install and run this program on Linux.<br>
[Go to Installation and Execution](#installation)

### Conclusion
Conclusion.<br>
[Go to Counclusion](#conclusion)

<a name="intro"></a>
### Introduction

This program manage 4 threads, using semaphores and Priority Ceiling Policy, in order to:
<ul>
    <li>Task 1 write something into a variable, Task 2 read from this one.</li>
    <li>Task 1 write something into a variable, Task 4 read from this one.</li>
    <li>Task 2 write something into a variable, Task 3 read from this one.</li>
</ul>

<a name="how"></a>
### How it works

To do this, the following steps have been followed:
<ul>
    <li>Set the thread with the lowest period as the first one and the
    thread with the highest period as the last one.</li>
    <li>Compute the WCET (Worst Case Execution Time) for each task.</li>
    <li>Check if the schedulability if feasible or not computing the 
    Utilization factor (U) and the Utilization factor lower upper bound
    (Ulub).</li>
    <li>Set attributes (incliding scheduling policy and priority) of each
    task.</li>
    <li>Compute the next arrival time of each task.</li>
    <li>Create thread and wait for the first one.</li>
    <li>When the first thread terminated, print missed deadlines of each
    task and terminate.</li>
</ul>

Now, let's notice what each Thread do:
<ul>
    <li>Tread 1: executes the first task that take the first and the second semaphore for all the time; write into the variables; wait for the second and the fourth threads. When all the executions are finished, it unlock the two semaphores and terminates.</li>
    <li>Thread 2: executes the second task that take the third sempahore for all the time; take the first semaphore when the first thread are waiting a signal to take again the control; read the variable e write into another one; send the signal to the first thread; wait for the third thread. When all the execution are finished, it unlock the
    third semaphore and terminates.</li>
    <li>Thread 3: executes the third task that take the third semaphore when the second thread are waiting a signal to take again the control;
    read the variable; send the signal to the second thread. When all the
    executions are finished, it terminates.</li>
    <li>Thread 4: executes the fourth task that take the second semaphore when the first thread are waiting a signal to take again the control;
    read the variable; send the signal to the first thread. When all the
    executions are finished, it terminates.</li>
</ul>
All threads sleep until the next arrival time and compute the next one.

<a name="installation"></a>
### Installation and Execution

Download the repository:

<pre>
    <code>
        git clone https://github.com/simone-contorno/rtos_priority_ceiling
    </code>
</pre>

Now, to run 'rtos_priority_ceiling' type:

<pre>
    <code>
        ./exec.sh
    </code>
</pre>

<a name="conclusion"></a>
### Conclusion

Thanks to have read this file, i hope it was clear and interesting.