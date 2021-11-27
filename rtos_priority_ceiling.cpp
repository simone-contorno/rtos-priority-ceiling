/*
 * Real-Time Operationg Systems
 * 4 threads managed with Semaphores (Priority Ceiling Policy)
 *
 * by Simone Contorno
 */

// Required headers 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>

// The code to be executed by each task 
void task1_1_code();
void task1_2_code();
void task2_1_code();
void task2_2_code();
void task3_code();
void task4_code();

// The characteristic function of each thread 
void *task1(void *);
void *task2(void *);
void *task3(void *);
void *task4(void *);

// Constants
#define NEXEC 30 // Change this value to decrease or increase the number of execution for each task
#define NPERIODICTASKS 4
#define NAPERIODICTASKS 0
#define NTASKS NPERIODICTASKS + NAPERIODICTASKS

// Global variables
long int periods[NTASKS];
struct timespec next_arrival_time[NTASKS];
double WCET[NTASKS];
pthread_attr_t attributes[NTASKS];
pthread_t thread_id[NTASKS];
struct sched_param parameters[NTASKS];
int missed_deadlines[NTASKS];

// Shared resources
int T1T2 = 0;
int T1T4 = 0;
int T2T3 = 0;

// Execution counter
int counter = 1;

// Semaphores 
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;
pthread_mutexattr_t mutexAttr; // Semaphore's attributes 

int main() {
	printf("\n"); fflush(stdout);

  	/*
	* Set periods of each periodic task in nanoseconds
	* Order them from the least period (highest priority) to the highest period (least priority)
	*/
	periods[0] = 80000000; // 0.08 sec
	periods[1] = 100000000; // 0.1 sec
	periods[2] = 160000000; // 0.16 sec
	periods[3] = 200000000; // 0.2 sec

	// Read maximum and minimum priorities
  	struct sched_param priomax;
  	priomax.sched_priority = sched_get_priority_max(SCHED_FIFO);
  	struct sched_param priomin;
  	priomin.sched_priority = sched_get_priority_min(SCHED_FIFO);

	// Set the maximum priority to the current thread (you are required to be superuser)
	if (getuid() == 0) pthread_setschedparam(pthread_self(), SCHED_FIFO, &priomax);

  	/* 
	* Execute all tasks in standalone modality in order to measure execution times (use gettimeofday).
	* Use the computed values to update the worst case execution time of each task.
	*/
  	for (int i = 0; i < NTASKS; i++) {
		WCET[i] = 0; 
		for (int j = 0; j < NEXEC; j++) {
			// Declare time variables
			struct timespec time_1, time_2;
		
			// Execute the task to estimate its WCET
			clock_gettime(CLOCK_REALTIME, &time_1);
			if (i == 0) {
				task1_1_code();
				task1_2_code();
			}
			if (i == 1) {
				task2_1_code();
				task2_2_code();
			}
			if (i == 2) task3_code();
			if (i == 3) task4_code();
			clock_gettime(CLOCK_REALTIME, &time_2);
			
			// Compute the Worst Case Execution Time 
			double WCET_temp = 1000000000 * (time_2.tv_sec - time_1.tv_sec) + (time_2.tv_nsec-time_1.tv_nsec);
			if (WCET_temp > WCET[i]) 
				WCET[i] = WCET_temp; 
		}
    }

	counter = 1; // Reset counter

	double BWCET[NTASKS];
	double B[NTASKS];

	// Compute the blocking time for thread 1
	for (int i = 0; i < NTASKS; i++) {
		for (int j = 0; j < NEXEC; j++) {
			// Declare time variables
			struct timespec time_1, time_2;
		
			// Execute the task to estimate its WCET
			clock_gettime(CLOCK_REALTIME, &time_1);
			if (i == 0) task2_1_code(); // Thread 1
			if (i == 1 or i == 2) task4_code(); // Thread 1 and Thread 2
			if (i == 3) task3_code(); // Thread 2
			clock_gettime(CLOCK_REALTIME, &time_2);
			
			// Compute the Worst Case Execution Time 
			double BWCET_temp = 1000000000 * (time_2.tv_sec - time_1.tv_sec) + (time_2.tv_nsec-time_1.tv_nsec);
			if (BWCET_temp > BWCET[i]) 
				BWCET[i] = BWCET_temp; 
		}
	}
	// Assign the worst case for each thread
	if (BWCET[0] > BWCET[1])
		B[0] = BWCET[0];
	else 
		B[0] = BWCET[1];
	if (BWCET[2] > BWCET[3])
		B[1] = BWCET[2];
	else 
		B[1] = BWCET[3];
	B[2] = WCET[3];
	B[3] = 0;

	// Clear the console
	system("clear");

	// Print all Worst Case Execution Time
	for (int i = 0; i < NTASKS; i++) {
		printf("Worst Case Execution Time [%i] = %f [ns]\n", i+1, WCET[i]); 
		fflush(stdout);
	}

	printf("\n");
	fflush(stdout);

	// Print all Blocking time
	for (int i = 0; i < NTASKS; i++) {
		printf("Blocking time thread [%i] = %f\n", i+1, B[i]); 
		fflush(stdout);
	}

	printf("\n");
	fflush(stdout);

	// Compute U, Ulub and check for schedulability
	double U, Ulub;
	int flag = 0;

	for (int i = 0; i < NTASKS; i++) {
		// Set to 0 U and Ulub
		U = 0;
		Ulub = 0;

		// Compute U for thread i
		for (int j = 0; j <= i; j++) {
			U += WCET[j] / periods[j];
		}
		U += B[i] / periods[i];

		// Compute Ulub for thread i
		Ulub = (i+1) * (pow(2.0,(1.0 / (i+1))) - 1);

		// Print the result
		printf("[%i] U = %lf ; Ulub = %lf\n", i+1, U, Ulub); 
		fflush(stdout);

		// Check for schedulability
		if (U > Ulub) {
			flag = 1;
			break;
		}
	}
	
	if (flag == 0) {
		printf("--> Task set schedulable.\n"); 
		fflush(stdout);
	}
	else {
		printf("--> Task set not schedulable.\n"); 
		fflush(stdout);
	}

	// Set the minimum priority to the current thread
  	if (getuid() == 0) 
		pthread_setschedparam(pthread_self(), SCHED_FIFO, &priomin);

  	// Set the attributes of each task, including scheduling policy and priority
  	for (int i = 0; i < NPERIODICTASKS; i++) {
		// Initialize the attribute structure of task i
		pthread_attr_init(&(attributes[i]));

		// Set the attributes to tell the kernel that the priorities and policies are explicitly chosen
		pthread_attr_setinheritsched(&(attributes[i]), PTHREAD_EXPLICIT_SCHED);
      
		// Set the attributes to set the SCHED_FIFO policy
		pthread_attr_setschedpolicy(&(attributes[i]), SCHED_FIFO);

		// Properly set the parameters to assign the priority inversely proportional to the period
		parameters[i].sched_priority = priomin.sched_priority + NTASKS - i;

		// Set the attributes and the parameters of the current thread
		pthread_attr_setschedparam(&(attributes[i]), &(parameters[i]));
    }

    // Set attributes to the semaphores (Priority Ceiling)
	pthread_mutexattr_init(&mutexAttr);
	pthread_mutexattr_setprotocol(&mutexAttr, PTHREAD_PRIO_PROTECT);

	pthread_mutexattr_setprioceiling(&mutexAttr, priomin.sched_priority + NTASKS - 0);
	pthread_mutex_init(&mutex1, &mutexAttr);

	pthread_mutexattr_setprioceiling(&mutexAttr, priomin.sched_priority + NTASKS - 0);
	pthread_mutex_init(&mutex2, &mutexAttr);

	pthread_mutexattr_setprioceiling(&mutexAttr, priomin.sched_priority + NTASKS - 1);
	pthread_mutex_init(&mutex3, &mutexAttr);

	// Declare time variables
	struct timespec time_1;
	clock_gettime(CLOCK_REALTIME, &time_1);

  	// Set the next arrival time for each task
  	for (int i = 0; i < NPERIODICTASKS; i++) {
		/*
		* 1. First we encode the current time in nanoseconds and add the period 
		* 2. By assuming that the current time is the beginning of the first period, compute the beginning of the second period
		*/
		long int next_arrival_nanoseconds = time_1.tv_nsec + periods[i];
		next_arrival_time[i].tv_nsec = next_arrival_nanoseconds % 1000000000;
		next_arrival_time[i].tv_sec = time_1.tv_sec + next_arrival_nanoseconds / 1000000000;

       	missed_deadlines[i] = 0;
    }
	
	// Declare the variable to contain the return values of pthread_create	
  	int iret[NTASKS];
	
	// Create all threads
	iret[0] = pthread_create(&(thread_id[0]), &(attributes[0]), task1, NULL);
	iret[1] = pthread_create(&(thread_id[1]), &(attributes[1]), task2, NULL);
	iret[2] = pthread_create(&(thread_id[2]), &(attributes[2]), task3, NULL);
	iret[3] = pthread_create(&(thread_id[3]), &(attributes[3]), task4, NULL);

  	// Join all threads
	for (int i = 0; i < NTASKS; i++)
		pthread_join(thread_id[i], NULL);

  	// Print deadlines values
  	for (int i = 0; i < NTASKS; i++) {
      	printf ("\nMissed Deadlines Task %d = %d", i+1, missed_deadlines[i]); 
		fflush(stdout);
	}
    
	printf ("\n\n"); fflush(stdout);

  	return 0;
}

// Task 1_1 - Write into T1T2 
void task1_1_code() {
  	printf("\nExecution %i/%i:\n[1] --> ", counter, NEXEC); fflush(stdout);
	T1T2 = rand() % 10;
	printf("%i (W)\n", T1T2); fflush(stdout);
	counter++;
}

// Task 1_2 - Write into T1T4
void task1_2_code() {
	T1T4 = rand() % 10;
	printf("[1] --> %i (W)\n", T1T4); fflush(stdout);
}

// Thread 1 - Use mutex1 and mutex2
void *task1(void *ptr) {
	// Set thread affinity
	cpu_set_t cset;
	CPU_ZERO (&cset);
	CPU_SET(0, &cset);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

	for (int i = 0; i < NEXEC; i++) {	
        // Go into the critical sections protected by mutex 1 and execute task 1_1
		pthread_mutex_lock(&mutex1); 
		task1_1_code();
		pthread_mutex_unlock(&mutex1); 
		// Go into the critical sections protected by mutex 2 and execute task 1_2
		pthread_mutex_lock(&mutex2);
		task1_2_code();
		pthread_mutex_unlock(&mutex2);
		
		// Declare time variables and get time
        struct timespec time;
		clock_gettime(CLOCK_REALTIME, &time);

		// After execution, compute the time before the next period starts
		long int timetowait = 1000 * ((next_arrival_time[0].tv_sec - time.tv_sec) * 1000000 + (next_arrival_time[0].tv_sec - time.tv_sec) * 1000000);
		if (timetowait < 0) missed_deadlines[0]++;

		// Sleep until the next arrival time and compute the next one
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[0], NULL);
		long int next_arrival_nanoseconds = next_arrival_time[0].tv_nsec + periods[0];
		next_arrival_time[0].tv_nsec = next_arrival_nanoseconds % 1000000000;
		next_arrival_time[0].tv_sec = next_arrival_time[0].tv_sec + next_arrival_nanoseconds / 1000000000;
	}
	return 0;
}

// Task 2_1 - Read from T1T2
void task2_1_code() {
	printf("[2] --> %i (R)\n", T1T2); fflush(stdout);
}

// Task 2_2 - Write into T2T3
void task2_2_code() {
	T2T3 = rand() % 10;
	printf("[2] --> %i (W)\n", T2T3); fflush(stdout);
}

// Thread 2 - Use mutex3 and mutex1
void *task2(void *ptr ) {
	// Set thread affinity
	cpu_set_t cset;
	CPU_ZERO (&cset);
	CPU_SET(0, &cset);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

  	for (int i = 0; i < NEXEC; i++) {
		// Go into the critical sections protected by mutex 3 and execute task 2_2
		pthread_mutex_lock(&mutex3); 
		task2_2_code();
		pthread_mutex_unlock(&mutex3); 
        // Go into the critical sections protected by mutex 1 and execute task 2_1
		pthread_mutex_lock(&mutex1); 
		task2_1_code();
		pthread_mutex_unlock(&mutex1); 
		
		// Declare time variables and get time
        struct timespec time;
		clock_gettime(CLOCK_REALTIME, &time);

		// After execution, compute the time before the next period starts
		long int timetowait = 1000 * ((next_arrival_time[1].tv_sec - time.tv_sec) * 1000000 + (next_arrival_time[1].tv_sec - time.tv_sec) * 1000000);
		if (timetowait < 0) missed_deadlines[1]++;

		// Sleep until the next arrival time and compute the next one
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[1], NULL);
		long int next_arrival_nanoseconds = next_arrival_time[1].tv_nsec + periods[1];
		next_arrival_time[1].tv_nsec = next_arrival_nanoseconds % 1000000000;
		next_arrival_time[1].tv_sec = next_arrival_time[1].tv_sec + next_arrival_nanoseconds / 1000000000;
	}
	return 0;
}

// Task 3 - Read from T2T3
void task3_code() {
	printf("[3] --> %d (R)\n", T2T3); fflush(stdout);
}

// Thread 3 - Use mutex3
void *task3(void *ptr) {
	// Set thread affinity
	cpu_set_t cset;
	CPU_ZERO (&cset);
	CPU_SET(0, &cset);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

  	for (int i = 0; i < NEXEC; i++) {
        // Go into the critical section protected by mutex 2 and execute task 3
		pthread_mutex_lock(&mutex3); 
		task3_code();
        pthread_mutex_unlock(&mutex3);

		// Declare time variables and get time
        struct timespec time;
		clock_gettime(CLOCK_REALTIME, &time);

		// After execution, compute the time before the next period starts
		long int timetowait = 1000 * ((next_arrival_time[2].tv_sec - time.tv_sec) * 1000000 + (next_arrival_time[2].tv_sec - time.tv_sec) * 1000000);
		if (timetowait < 0) missed_deadlines[2]++;

		// Sleep until the next arrival time and compute the next one
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[2], NULL);
		long int next_arrival_nanoseconds = next_arrival_time[2].tv_nsec + periods[2];
		next_arrival_time[2].tv_nsec = next_arrival_nanoseconds % 1000000000;
		next_arrival_time[2].tv_sec = next_arrival_time[2].tv_sec + next_arrival_nanoseconds / 1000000000;
    }
	return 0;
}

// Task 4 - Read from T1T4
void task4_code() {
	printf("[4] --> %i (R)\n", T1T4); fflush(stdout);
}

// Thread 4 - Use mutex2
void *task4(void *ptr) {
	// Set thread affinity
	cpu_set_t cset;
	CPU_ZERO (&cset);
	CPU_SET(0, &cset);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

  	for (int i = 0; i < NEXEC; i++) {
        // Go into the critical section protected by mutex 1 and execute task 4
		pthread_mutex_lock(&mutex2); 
		task4_code();
		pthread_mutex_unlock(&mutex2); 

		// Declare time variables and get time
		struct timespec time;
		clock_gettime(CLOCK_REALTIME, &time);

		// After execution, compute the time before the next period starts
		long int timetowait = 1000 * ((next_arrival_time[3].tv_sec - time.tv_sec) * 1000000 + (next_arrival_time[3].tv_sec - time.tv_sec) * 1000000);
		if (timetowait < 0) missed_deadlines[3]++;
		
		// Sleep until the next arrival time and compute the next one
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[3], NULL);
		long int next_arrival_nanoseconds = next_arrival_time[3].tv_nsec + periods[3];
		next_arrival_time[3].tv_nsec = next_arrival_nanoseconds % 1000000000;
		next_arrival_time[3].tv_sec = next_arrival_time[3].tv_sec + next_arrival_nanoseconds / 1000000000;
    }
	return 0;
}