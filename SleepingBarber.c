/*
 * Compilation instruction: cc SleepingBarber.c -o SleepingBarber -lpthread -lm
 * Execution instruction: ./SleepingBarber 
 *
 */

#define _REENTRANT /* Helps using thread-safe. */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

/* The maximum number of customer (threads actually). */
#define MAXIMUM_CUSTOMERS_ALLOWED 16

// Function declaration.
void *customer_function(void *num);
void *barber_function(void *);

void random_wait(int secs);

/* Define the semaphores. */

/* waitingRoom_sempahore Limits the # of customers allowed to enter the waiting room at one time. */
sem_t waitingRoom_sempahore;   

/* barberChair_semaphore ensures mutually exclusive access to the barber chair. */
sem_t barberChair_semaphore;

/* barberSleep_semaphore is used to allow the barber to sleep until a customer arrives. */
sem_t barberSleep_semaphore;

/* customerOnBarberSeat_semaphore is used to make the customer to wait until the barber is done cutting his/her hair. */ 
sem_t customerOnBarberSeat_semaphore;

/* Flag to stop the barber thread when all customers have been serviced. */
int allDone = 0;

int main(int argc, char **argv) {
    pthread_t btid;
    pthread_t tid[MAXIMUM_CUSTOMERS_ALLOWED];
    long Random_Seed;
    int i, numofCustomers, numofChairs;
    int Number[MAXIMUM_CUSTOMERS_ALLOWED];
    
        
    /* Check to make sure there are the right number of command line arguments. */
    if (argc != 4) {
	printf("Usage: SleepingBarber <Num # of Customers> <Num # of Chairs> <Random Seed>\n");
	exit(-1);
    }
    
    /* Get the command line arguments and convert them into integers. */
    numofCustomers = atoi(argv[1]);
    numofChairs = atoi(argv[2]);
    Random_Seed = atol(argv[3]);
    
    // Make sure the number of threads is less than the number of
    // customers we can support.
    if (numofCustomers > MAXIMUM_CUSTOMERS_ALLOWED) {
	printf("The possible number of Customers is %d.\n", MAXIMUM_CUSTOMERS_ALLOWED);
	exit(-1);
    }
    
    printf("\nSleepingBarber.c\n\n");
    printf("A solution to the sleeping barber problem using semaphores.\n");
    
    /* Initialize the random number generator with a new seed. */
    srand48(Random_Seed);

    /* Initialize the numbers array. */
    for (i=0; i<MAXIMUM_CUSTOMERS_ALLOWED; i++) {
	Number[i] = i;
    }
		
    /* Initialize the semaphores with initial values. */
    sem_init(&waitingRoom_sempahore, 0, numofChairs);
    sem_init(&barberChair_semaphore, 0, 1);
    sem_init(&barberSleep_semaphore, 0, 0);
    sem_init(&customerOnBarberSeat_semaphore, 0, 0);
    
    /* Create the barber. */
    pthread_create(&btid, NULL, barber_function, NULL);

    /* Create the customers. */
    for (i=0; i<numofCustomers; i++) {
	pthread_create(&tid[i], NULL, customer_function, (void *)&Number[i]);
    }

    /* Join each of the threads to wait for them to finish. */
    for (i=0; i<numofCustomers; i++) {
	pthread_join(tid[i],NULL);
    }

    /* When all of the customers are finished, kill the barber thread. */
    allDone = 1;
    sem_post(&barberSleep_semaphore);
	/* Wake the barber to exit. */
    pthread_join(btid,NULL);	
}

void *customer_function(void *number) {
    int num = *(int *)number;

    /* Leave for the shop and take some random amount of time to arrive. */
    printf("Customer %d is leaving for barber shop.\n", num);
    random_wait(5);
    printf("Customer %d arrived at barber shop.\n", num);

    /* Wait for space to open up in the waiting room. */
    sem_wait(&waitingRoom_sempahore);
    printf("Customer %d entering waiting room.\n", num);

    /* Wait for the barber chair to become free. */
    sem_wait(&barberChair_semaphore);
    
    /* The chair is free so give up your spot in the waiting room. */
    sem_post(&waitingRoom_sempahore);

    /* Wake up the barber. */
    printf("Customer %d waking the barber.\n", num);
    sem_post(&barberSleep_semaphore);

    /* Wait for the barber to finish cutting your hair. */
    sem_wait(&customerOnBarberSeat_semaphore);
    
    /* Give up the chair. */
    sem_post(&barberChair_semaphore);
    printf("Customer %d leaving barber shop.\n", num);
}

void *barber_function(void *junk) {
    /* While there are still customers to be serviced. */
	/* Our barber will tell if customers are still on the way to his shop. */
    while (!allDone) {

	/* Sleep until someone arrives and wakes you. */
	printf("The barber is sleeping\n");
	sem_wait(&barberSleep_semaphore);

	/* Skip this stuff at the end. */
	if (!allDone) {

	    /* Random amount of time to cut the customer's hair. */
	    printf("The barber is cutting hair\n");
	    random_wait(3);
	    printf("The barber finished cutting hair.\n");

	    /* Release the customer when done cutting. */
	    sem_post(&customerOnBarberSeat_semaphore);
	}
	else {
	    printf("The barber goes to home now.\n");
	}
    }
}

void random_wait(int secs) {
    int len;
	
    /* Generate a random number. */
    len = (int) ((drand48() * secs) + 1);
    sleep(len);
}
