# RTOS-Priority-Ceiling
## Real Time Operating Systems - 4 threads managed with semaphores (Priority Ceiling Policy)
### Author: Simone Contorno

<br>

### Introduction
An overview of this program function.<br>
[Go to Introduction](#intro)

### How it works
A rapid description of how the program works.<br>
[Go to How it works](#how)

### Installation and Execution
How install and run this program in Linux.<br>
[Go to Installation and Execution](#installation)

### Conclusion
Conclusion.<br>
[Go to Conclusion](#con)

<a name="intro"></a>
### Introduction

This program manage 4 threads, using semaphores and Priority Ceiling Policy, in order to:
<ul>
    <li>Task 1_1 write something into the variable T1T2, Task 2_1 read from this one.</li>
    <li>Task 1_2 write something into the variable T1T4, Task 4 read from this one.</li>
    <li>Task 2_2 write something into the variable T2T3, Task 3 read from this one.</li>
</ul>

<a name="how"></a>
### How it works

To do this, the following steps have been followed:
<ul>
    <li>Set the thread with the lowest period as the first one and the
    thread with the highest period as the last one.</li>
    <li>Compute the WCET (Worst Case Execution Time) for each task.</li>
    <li>Compute the Blocking Time for each task.</li>
    <li>Check if the schedulability if feasible or not computing the 
    Utilization factor (U) and the Utilization factor lower upper bound
    (Ulub).</li>
    <li>Set attributes (including scheduling policy and priority) of each task.</li>
    <li>Set attributes (including Priority Ceiling) of each semaphore.</li>
    <li>Compute the next arrival time of each task.</li>
    <li>Create threads and wait for all of them.</li>
    <li>When these terminates, print missed deadlines of each task and terminate the program.</li>
</ul>

Now, let's notice what each Thread does:
<ul>
    <li>Tread 1: 
        <ul>
            <li>Lock the first semaphore, execute Task 1_1 and unlock the semaphore;</li>
            <li>Lock the second semaphore, execute Task 1_2 and unlock the semaphore;</li>
            <li>Sleep until the next arrival time and compute the next one.</li>
        </ul>
    </li>
    <li>Tread 2: 
        <ul>
            <li>Lock the third semaphore, execute Task 2_2 and unlock the semaphore;</li>
            <li>Lock the first semaphore, execute Task 2_1 and unlock the semaphore;</li>
            <li>Sleep until the next arrival time and compute the next one.</li>
        </ul>
    </li>
    <li>Tread 3: 
        <ul>
            <li>Lock the third semaphore, execute Task 3 and unlock the semaphore;</li>
            <li>Sleep until the next arrival time and compute the next one.</li>
        </ul>
    </li>
    <li>Tread 4: 
        <ul>
            <li>Lock the second semaphore, execute Task 4 and unlock the semaphore;</li>
            <li>Sleep until the next arrival time and compute the next one.</li>
        </ul>
    </li>
</ul>

<a name="installation"></a>
### Installation and Execution

Download the repository:

<pre><code>git clone https://github.com/simone-contorno/RTOS-Priority-Ceiling</code></pre>

Now, to run 'rtos_priority_ceiling' type:

<pre><code>./exec.sh</code></pre>

<a name="con"></a>
### Conclusion

Thanks to have read this file, i hope it was clear and interesting.
