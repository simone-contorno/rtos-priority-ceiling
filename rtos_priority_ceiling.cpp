/*
	Real-Time Operationg Systems
	4 threads managed with Semaphores (Priority Ceiling Policy)

	by Simone Contorno
*/

/*
 * NOTE: if you want to try the Priority Ceiling effect on the scheduling:
 * 1) Uncomment the lines 244, 245, 247, 287, 289. (Try to produce a deadlock)
 * 2) Run exec.sh: you will not see notice any deadlock. (Because Priority Ceiling is set)
 * 3) Comment lines from 160 to 170. (Remove Priority Ceiling Policy)
 * 4) Run exec.sh: a deadlock will be produce (if it will not occur try to increase the value of nsleep() at line 245).
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
void task1_code();
void task2_code();
void task3_code();
void task4_code();

// The characteristic function of each thread 
void *task1(void *);
void *task2(void *);
void *task3(void *);
void *task4(void *);

// Constants
#define NEXEC 100 // Change this value to decrease or increase the number of execution for each task
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
			struct timeval timeval1, timeval2;
			struct timezone timezone1, timezone2;

			// Execute the task to estimate its WCET
			gettimeofday(&timeval1, &timezone1);
			if (i == 0) task1_code();
			if (i == 1) task2_code();
			if (i == 2) task3_code();
			if (i == 3) task4_code();
			gettimeofday(&timeval2, &timezone2);
			
			// Compute the Worst Case Execution Time 
			double WCET_temp = ((timeval2.tv_usec - timeval1.tv_usec) * 1000 + (timeval2.tv_usec - timeval1.tv_usec) * 1000);
			if (WCET_temp > WCET[i]) 
				WCET[i] = WCET_temp; 
		}
    }

	// Clear the console
	system("clear");

	// Print all WCET
	for (int i = 0; i < NTASKS; i++)
		printf("Worst Case Execution Time %i = %f [ns]\n", i+1, WCET[i]); fflush(stdout);

	counter = 1; // Reset counter

	// Compute U and check for schedulability
	double U = 0;
	for (int i = 0; i < NTASKS; i++) 
		U += WCET[i] / periods[i];

	double Ulub = NPERIODICTASKS * (pow(2.0,(1.0 / NPERIODICTASKS)) - 1);

	// Check the sufficient conditions: if it is not satisfied, exit  
	if (U > Ulub) {
		printf("\nU = %lf \nUlub = %lf \nTask set not schedulable\n", U, Ulub); fflush(stdout);
		return(-1);
	}
	printf("\nU = %lf \nUlub = %lf \nTask set schedulable\n", U, Ulub); fflush(stdout);

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
		//printf("%i \n", priomin.sched_priority + NTASKS - i);

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
	//for (int i = 0; i < NTASKS; i++)
		//pthread_join(thread_id[i], NULL);

	/*
	 * Join ONLY the first and the second threads
	 * WHY? Because they write into the shared resources, while the other 2
	 * threads just read; so, if the first one and the second one are termined,
	 * the value into the shared resources does not change and continue to 
	 * read the same value has not sense.
	 */
	pthread_join(thread_id[0], NULL);
	pthread_join(thread_id[1], NULL);

  	// Print deadlines values
  	for (int i = 0; i < NTASKS; i++) {
      	printf ("\nMissed Deadlines Task %d = %d", i+1, missed_deadlines[i]); 
		fflush(stdout);
	}
    
	printf ("\n\n"); fflush(stdout);

  	return 0;
}

// Task 1 - Write into T1T2 and T1T4
void task1_code() {
  	printf("\nExecution %i/%i:\n[1] --> ", counter, NEXEC); fflush(stdout);
	T1T2 = rand() % 10;
	T1T4 = rand() % 10;
	printf("%i (W) --> %i (W)\n", T1T2, T1T4); fflush(stdout);
	counter++;
}

// Thread 1 - Use mutex1 and mutex2
void *task1(void *ptr) {
	// Set thread affinity
	cpu_set_t cset;
	CPU_ZERO (&cset);
	CPU_SET(0, &cset);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

	for (int i = 0; i < NEXEC; i++) {	
        // Go into the critical section protected by mutex 1 and execute task 1
		pthread_mutex_lock(&mutex1); 
		pthread_mutex_lock(&mutex2);
		//usleep(5);
		//pthread_mutex_lock(&mutex3); 
		task1_code();
		//pthread_mutex_unlock(&mutex3); 
		pthread_mutex_unlock(&mutex2);
		pthread_mutex_unlock(&mutex1); 

		// Declare time variables and get time
        struct timeval hour;
		struct timezone zone;
		gettimeofday(&hour, &zone);

		// After execution, compute the time before the next period starts
		long int timetowait = 1000 * ((next_arrival_time[0].tv_sec - hour.tv_sec) * 1000000 + (next_arrival_time[0].tv_sec - hour.tv_sec) * 1000000);
		if (timetowait < 0) missed_deadlines[0]++;

		// Sleep until the next arrival time and compute the next one
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[0], NULL);
		long int next_arrival_nanoseconds = next_arrival_time[0].tv_nsec + periods[0];
		next_arrival_time[0].tv_nsec = next_arrival_nanoseconds % 1000000000;
		next_arrival_time[0].tv_sec = next_arrival_time[0].tv_sec + next_arrival_nanoseconds / 1000000000;
	}
	return 0;
}

// Task 2 - Read from T1T2 and write into T2T3
void task2_code() {
	printf("[2] --> %i (R) --> ", T1T2); fflush(stdout);
	T2T3 = rand() % 10;
	printf("%i (W)\n", T2T3); fflush(stdout);
}

// Thread 2 - Use mutex3
void *task2(void *ptr ) {
	// Set thread affinity
	cpu_set_t cset;
	CPU_ZERO (&cset);
	CPU_SET(0, &cset);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cset);

  	for (int i = 0; i < NEXEC; i++) {
        // Go into the critical section protected by mutex 2 and execute task 2
		pthread_mutex_lock(&mutex3); 
		//pthread_mutex_lock(&mutex1); 
		task2_code();
		//pthread_mutex_unlock(&mutex1); 
		pthread_mutex_unlock(&mutex3); 

		// Declare time variables and get time
        struct timeval hour;
		struct timezone zone;
		gettimeofday(&hour, &zone);

		// After execution, compute the time before the next period starts
		long int timetowait = 1000 * ((next_arrival_time[1].tv_sec - hour.tv_sec) * 1000000 + (next_arrival_time[1].tv_sec - hour.tv_sec) * 1000000);
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
        struct timeval hour;
		struct timezone zone;
		gettimeofday(&hour, &zone);

		// After execution, compute the time before the next period starts
		long int timetowait = 1000 * ((next_arrival_time[2].tv_sec - hour.tv_sec) * 1000000 + (next_arrival_time[2].tv_sec - hour.tv_sec) * 1000000);
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
		struct timeval hour;
		struct timezone zone;
		gettimeofday(&hour, &zone);

		// After execution, compute the time before the next period starts
		long int timetowait = 1000 * ((next_arrival_time[3].tv_sec - hour.tv_sec) * 1000000 + (next_arrival_time[3].tv_sec - hour.tv_sec) * 1000000);
		if (timetowait < 0) missed_deadlines[3]++;
		
		// Sleep until the next arrival time and compute the next one
		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[3], NULL);
		long int next_arrival_nanoseconds = next_arrival_time[3].tv_nsec + periods[3];
		next_arrival_time[3].tv_nsec = next_arrival_nanoseconds % 1000000000;
		next_arrival_time[3].tv_sec = next_arrival_time[3].tv_sec + next_arrival_nanoseconds / 1000000000;
    }
	return 0;
}