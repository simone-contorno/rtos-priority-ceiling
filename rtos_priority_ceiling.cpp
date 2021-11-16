/*
	Real-Time Operationg Systems
	4 threads managed with Semaphores (Priority Ceiling Policy)

	by Simone Contorno
*/

// Required headers 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>

// The code to be executed by each task (application dependent)
void task1_code();
void task2_code();
void task3_code();
void task4_code();

// The characteristic function of each thread (used for temporization, application independent)
void *task1(void *);
void *task2(void *);
void *task3(void *);
void *task4(void *);

// Additional information required for each task
//#define INNERLOOP 1000
//#define OUTERLOOP 750
#define NEXEC 1000
#define SLEEPTIME 100000000 // 0.1 seconds

#define NPERIODICTASKS 4
#define NAPERIODICTASKS 0
#define NTASKS NPERIODICTASKS + NAPERIODICTASKS

long int periods[NTASKS];
struct timespec next_arrival_time[NTASKS];
double WCET[NTASKS];
pthread_attr_t attributes[NTASKS];
pthread_t thread_id[NTASKS];
struct sched_param parameters[NTASKS];
int missed_deadlines[NTASKS];

int T1T2 = 0;
int T1T4 = 0;
int T2T3 = 0;

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_3 = PTHREAD_COND_INITIALIZER;

int counter = 1;

int main() {
	printf("\n"); fflush(stdout);

  	/*
	* Set periods of each periodic task in nanoseconds.
	* Order them from the least period (highest priority) to the highest period (least priority)
	*/
	periods[0] = 80000000; // 0.08 sec
	periods[1] = 100000000; // 0.1 sec
	periods[2] = 160000000; // 0.16 sec
	periods[3] = 200000000; // 0.2 sec

	// Read maximum and minimum priorities.
  	struct sched_param priomax;
  	priomax.sched_priority = sched_get_priority_max(SCHED_FIFO);
  	struct sched_param priomin;
  	priomin.sched_priority = sched_get_priority_min(SCHED_FIFO);

	// Set the maximum priority to the current thread (you are required to be superuser).
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
	
	system("clear");

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

		// Set the attributes and the parameters of the current thread
		pthread_attr_setschedparam(&(attributes[i]), &(parameters[i]));
    }

	// Declare time variables
	struct timeval hour;
	struct timezone zone;
	gettimeofday(&hour, &zone);
	struct timespec time_1;
	clock_gettime(CLOCK_REALTIME, &time_1);

  	// Set the next arrival time for each task. 
  	for (int i = 0; i < NPERIODICTASKS; i++) {
		/*
		* 1. First we encode the current time in nanoseconds and add the period 
		* 2. By assuming that the current time is the beginning of the first period, compute the beginning of the second period
		* 3. Then we compute the end of the first period and beginning of the next one
		*/
		//long int next_arrival_nano = hour.tv_usec * 1000 + periods[i];
		long int next_arrival_nanoseconds = time_1.tv_nsec + periods[i];
		//next_arrival_time[i].tv_nsec = next_arrival_nano;
		next_arrival_time[i].tv_nsec= next_arrival_nanoseconds % 1000000000;
		//next_arrival_time[i].tv_sec = next_arrival_nano / 1000000000;
		next_arrival_time[i].tv_sec= time_1.tv_sec + next_arrival_nanoseconds / 1000000000;
       	missed_deadlines[i] = 0;
    }
	
	// Delare the variable to contain the return values of pthread_create	
  	int iret[NTASKS];
	
	// Create all threads
	iret[0] = pthread_create(&(thread_id[0]), &(attributes[0]), task1, NULL);
	iret[1] = pthread_create(&(thread_id[1]), &(attributes[1]), task2, NULL);
	iret[2] = pthread_create(&(thread_id[2]), &(attributes[2]), task3, NULL);
	iret[3] = pthread_create(&(thread_id[3]), &(attributes[3]), task4, NULL);

  	// Join all threads
	//for (int i = 0; i < NTASKS; i++)
		//pthread_join(thread_id[i], NULL);

	// Join ONLY the first thread
	pthread_join(thread_id[0], NULL);

  	// Print deadlines values
  	for (int i = 0; i < NTASKS; i++) {
      	printf ("\nMissed Deadlines Task %d = %d", i, missed_deadlines[i]); 
		fflush(stdout);
	}
    
	printf ("\n\n"); fflush(stdout);

  	return 0;
}

// Application specific task_1 code
void task1_code() {
  	printf("\nExecution %i/%i:\n[1] --> ", counter, NEXEC); fflush(stdout);
	T1T2 = rand() % 10;
	T1T4 = rand() % 10;
	printf("%i (W) --> %i (W)\n", T1T2, T1T4); fflush(stdout);
	counter++;
}

// Thread code for task_1 (used only for temporization)
void *task1(void *ptr) {
	struct timespec waittime;
	waittime.tv_sec = 0;
	waittime.tv_nsec = periods[0];

	pthread_mutex_lock(&mutex1); 
	pthread_mutex_lock(&mutex2); 

  	for (int i = 0; i < NEXEC; i++) {	
		task1_code();
		pthread_cond_wait(&cond_1, &mutex1); 
		pthread_cond_wait(&cond_2, &mutex2); 

		// The thread is ready and can compute the end of the current period for the next iteration
		struct timeval hour;
		struct timezone zone;
		gettimeofday(&hour, &zone);

		/* TODO: MISSED DEADLINES
		long int timetowait = next_arrival_time[0].tv_nsec - hour.tv_usec * 1000;
		waittime.tv_sec = timetowait / 1000000000;
		waittime.tv_nsec = timetowait;

		// It would be nice to check if we missed a deadline here
		if (timetowait < 0) {
			missed_deadlines[0]++; 
			// printf("\n 1: %li\n", timetowait);
		}
		
		// Sleep until the end of the current period (which is also the start of the new one)
		//nanosleep(&waittime, NULL);

		// Compute the beginning of the subsequent period
		long int next_arrival_nano = next_arrival_time[0].tv_nsec + periods[0];
		next_arrival_time[i].tv_nsec = next_arrival_nano;
		next_arrival_time[i].tv_sec = next_arrival_nano / 1000000000;
		*/

		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[0], NULL);
		long int next_arrival_nanoseconds = next_arrival_time[0].tv_nsec + periods[0];
		next_arrival_time[0].tv_nsec = next_arrival_nanoseconds % 1000000000;
		next_arrival_time[0].tv_sec = next_arrival_time[0].tv_sec + next_arrival_nanoseconds / 1000000000;

		waittime.tv_sec = SLEEPTIME / 1000000000;
		waittime.tv_nsec = SLEEPTIME;
		nanosleep(&waittime, NULL);
    }

	pthread_mutex_unlock(&mutex1); 
	pthread_mutex_unlock(&mutex2); 

	printf("\n[1] Termined\n"); fflush(stdout);

	return 0;
}

void task2_code() {
	printf("[2] --> %i (R) --> ", T1T2); fflush(stdout);

	T2T3 = rand() % 10;
	
	printf("%i (R)\n", T2T3); fflush(stdout);
}


void *task2(void *ptr ) {
	struct timespec waittime;
	waittime.tv_sec = 0;
	waittime.tv_nsec = periods[1];

	pthread_mutex_lock(&mutex3); 

  	for (int i = 0; i < NEXEC; i++) {
		pthread_mutex_lock(&mutex1);
		task2_code();
		
		// The thread is ready and can compute the end of the current period for the next iteration
		struct timeval hour;
		struct timezone zone;
		gettimeofday(&hour, &zone);
		
		/* TODO: MISSED DEADLINES
		long int timetowait = next_arrival_time[1].tv_nsec - hour.tv_usec * 1000;
		waittime.tv_sec = timetowait / 1000000000;
		waittime.tv_nsec = timetowait;
		
		// It would be nice to check if we missed a deadline here
		if (timetowait < 0) {
			missed_deadlines[1]++; 
			// printf("\n 2: %li\n", timetowait);
		}
		
		// Sleep until the end of the current period (which is also the start of the new one)
		//nanosleep(&waittime, NULL);

		// Compute the beginning of the subsequent period
		
		long int next_arrival_nano = next_arrival_time[1].tv_nsec + periods[1];
		next_arrival_time[i].tv_nsec = next_arrival_nano;
		next_arrival_time[i].tv_sec = next_arrival_nano / 1000000000;
		*/

		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[1], NULL);
		long int next_arrival_nanoseconds = next_arrival_time[1].tv_nsec + periods[1];
		next_arrival_time[1].tv_nsec = next_arrival_nanoseconds % 1000000000;
		next_arrival_time[1].tv_sec = next_arrival_time[1].tv_sec + next_arrival_nanoseconds / 1000000000;

		pthread_cond_signal(&cond_1);
		pthread_mutex_unlock(&mutex1);
		pthread_cond_wait(&cond_3, &mutex3); 
		
		waittime.tv_sec = SLEEPTIME / 1000000000;
		waittime.tv_nsec = SLEEPTIME;
		nanosleep(&waittime, NULL);
    }
	
	pthread_mutex_unlock(&mutex3); 

	printf("\n[2] Termined\n"); fflush(stdout);

	return 0;
}

void task3_code() {
	printf("[3] --> %d (R)\n", T2T3); fflush(stdout);
}

void *task3(void *ptr) {
	struct timespec waittime;
	waittime.tv_sec = 0;
	waittime.tv_nsec = periods[2];

  	for (int i = 0; i < NEXEC; i++) {
		pthread_mutex_lock(&mutex3); 
		task3_code();
	
		// The thread is ready and can compute the end of the current period for the next iteration
		struct timeval hour;
		struct timezone zone;
		gettimeofday(&hour, &zone);

		/* TODO: MISSED DEADLINES
		long int timetowait = next_arrival_time[2].tv_nsec - hour.tv_usec * 1000;
		waittime.tv_sec = timetowait / 1000000000;
		waittime.tv_nsec = timetowait;

		// It would be nice to check if we missed a deadline here
		if (timetowait < 0) {
			missed_deadlines[2]++; 
			// printf("\n 3: %li\n", timetowait);
		}

		// Sleep until the end of the current period (which is also the start of the new one)
		//nanosleep(&waittime, NULL);

		// Compute the beginning of the subsequent period
		long int next_arrival_nano = next_arrival_time[2].tv_nsec + periods[2];
		next_arrival_time[i].tv_nsec = next_arrival_nano;
		next_arrival_time[i].tv_sec = next_arrival_nano / 1000000000;
		*/

		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[2], NULL);
		long int next_arrival_nanoseconds = next_arrival_time[2].tv_nsec + periods[2];
		next_arrival_time[2].tv_nsec = next_arrival_nanoseconds % 1000000000;
		next_arrival_time[2].tv_sec = next_arrival_time[2].tv_sec + next_arrival_nanoseconds / 1000000000;

		pthread_cond_signal(&cond_3); 
		pthread_mutex_unlock(&mutex3); 

		waittime.tv_sec = SLEEPTIME / 1000000000;
		waittime.tv_nsec = SLEEPTIME;
		nanosleep(&waittime, NULL);
    }

	printf("\n[3] Termined\n"); fflush(stdout);

	return 0;
}

void task4_code() {
	printf("[4] --> %i (R)\n", T1T4); fflush(stdout);
}

void *task4(void *ptr) {
	struct timespec waittime;
	waittime.tv_sec = 0;
	waittime.tv_nsec = periods[3];

  	for (int i = 0; i < NEXEC; i++) {
		pthread_mutex_lock(&mutex2); 
		task4_code();

		// The thread is ready and can compute the end of the current period for the next iteration
		struct timeval hour;
		struct timezone zone;
		gettimeofday(&hour, &zone);

		/* TODO: MISSED DEADLINES
		long int timetowait = next_arrival_time[3].tv_nsec - hour.tv_usec * 1000;
		waittime.tv_sec = timetowait / 1000000000;
		waittime.tv_nsec = timetowait;

		// It would be nice to check if we missed a deadline here
		if (timetowait < 0) {
			missed_deadlines[3]++; 
			// printf("\n 4: %li\n", timetowait);
		}

		// Sleep until the end of the current period (which is also the start of the new one)
		//nanosleep(&waittime, NULL);

		// Compute the beginning of the subsequent period
		long int next_arrival_nano = next_arrival_time[3].tv_nsec + periods[3];
		next_arrival_time[i].tv_nsec = next_arrival_nano;
		next_arrival_time[i].tv_sec = next_arrival_nano / 1000000000;
		*/

		clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &next_arrival_time[3], NULL);
		long int next_arrival_nanoseconds = next_arrival_time[3].tv_nsec + periods[3];
		next_arrival_time[3].tv_nsec = next_arrival_nanoseconds % 1000000000;
		next_arrival_time[3].tv_sec = next_arrival_time[3].tv_sec + next_arrival_nanoseconds / 1000000000;

		pthread_cond_signal(&cond_2); 
		pthread_mutex_unlock(&mutex2);

		waittime.tv_sec = SLEEPTIME / 1000000000;
		waittime.tv_nsec = SLEEPTIME;
		nanosleep(&waittime, NULL);
    }

	printf("\n[4] Termined\n"); fflush(stdout);

	return 0;
}