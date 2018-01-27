#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>

//Buffer Size of 120 Items
#define BUFFER_SIZE 120 

//The mutex lock
pthread_mutex_t mutexp;
pthread_mutex_t mutexc;

//The Conditional Variable
pthread_cond_t condp;
pthread_cond_t condc;

//The Two Semaphores full and empty
sem_t full, empty;

// MyShBuff buffer
int MyShBuff[BUFFER_SIZE];

// Buffer counter
int counter;

// producer consumer counter
int prodcount;
int concount;

// Producer Consumer Arrays of BUFFER_SIZE
int prodarray[BUFFER_SIZE];
int conarray[BUFFER_SIZE];

pthread_t tidp, tidc; //Producer and Consumer Thread ID
pthread_attr_t attr; //Set of Thread attributes

void *produce(void *param); // producer thread
void *consume(void *param); // consumer thread

int getMaxCountElement(int *array, int size);

//Compare Function
int compare(const void * a, const void * b) {
   return ( *(int*)b - *(int*)a );
}

// Main Method
int main(int argc, char *argv[]) {
   // Loop Counter 
   int i,j;

   // Verify the correct number of arguments were passed in
   if(argc != 4) {
      fprintf(stderr, "USAGE:./mpmc.out <INT> <INT> <INT>\n");
   }

   int mainSleepTime = atoi(argv[1]); // Time in seconds for main to sleep
   int numProd = atoi(argv[2]); // Number of producer threads
   int numCons = atoi(argv[3]); // Number of consumer threads

   // Init the mutex lock
   pthread_mutex_init(&mutexp, NULL);
   pthread_mutex_init(&mutexc, NULL);

   // Init the conditional Variable
   pthread_cond_init(&condp, NULL);
   pthread_cond_init(&condc, NULL);

   // Init the full semaphore and initialize to 0
   sem_init(&full, 0, 0);

   // Init the empty semaphore and initialize to BUFFER_SIZE
   sem_init(&empty, 0, BUFFER_SIZE);

   // Init the default attributes
   pthread_attr_init(&attr);

   // Init buffer count
   counter = 0;
   
   // Init producer/consumer count
   prodcount = 0;
   concount = 0;
   
   // Filling MyShBuff with -1 as Initial Values
   int buff;
   for (buff=0; buff<BUFFER_SIZE; buff++){
   		MyShBuff[buff]= -1;
   }
   printf("\n\n ********* PRODUCTION & CONSUMPTION *********\n");

   //Create the producer threads
   for(i = 0; i < numProd; i++) {
     pthread_create(&tidp, &attr, produce, NULL);
   }

   //Create the consumer threads
   for(i = 0; i < numCons; i++) {
     pthread_create(&tidc, &attr, consume, NULL);
   }

   // Sleep prod-cons for the specified amount of time in seconds
   sleep(mainSleepTime);

   // Print BUFFER Items
   int bufcount;
   printf("\n\n ********* BUFFER ITEMS ********* \n");
   for (bufcount = 0; bufcount < BUFFER_SIZE; bufcount++){
   		printf("%d ", MyShBuff[bufcount]);
   }

   // Sort and Print Producer Arrays
   qsort(prodarray, sizeof(prodarray) / sizeof(int), sizeof(int), compare);
   printf("\n\n ********* PRODUCERS ************ \n");
   for(i = 0; i < sizeof(prodarray) / sizeof(int); i++) {
      if(prodarray[i] != 0){
         printf("%d ", prodarray[i]);
      }
   }

   // Sort and Print Consumer Arrays
   qsort(conarray, sizeof(conarray) / sizeof(int), sizeof(int), compare);
   printf("\n\n ********* CONSUMERS ************ \n");
   for(i = 0; i < sizeof(conarray) / sizeof(int); i++) {
      if(conarray[i] != 0){
         printf("%d ", conarray[i]);
      }
   }
   
   getMaxCountElement(prodarray, sizeof(prodarray) / sizeof(int));
   getMaxCountElement(conarray, sizeof(conarray) / sizeof(int));
   printf("\n\n**** Winner Thread of the Producers is: %d \n", getMaxCountElement(prodarray, sizeof(prodarray) / sizeof(int)));
   printf("**** Winner Thread of the Consumers is: %d \n", getMaxCountElement(conarray, sizeof(conarray) / sizeof(int)));

//Exit the program
printf("Exit the program\n");
exit(0);
} //End of Main Method

// Produce Function
void *produce(void *param) {

   int item;
   while(1) {
      // Random Sleep Time 
      sleep(rand() / 100000000);

      // Generate a random number between 0 and 10
      item = rand() % 10;
      if(item == 0) {
         item = item + 1;
      }

      // Acquire the empty lock
      sem_wait(&empty);

      // Acquire the mutex lock
      pthread_mutex_lock(&mutexp);

      // Add the item and increment the counter
   	  while (counter >= BUFFER_SIZE || MyShBuff[(counter)] == -2)
      	pthread_cond_wait(&condp, &mutexp);
   	  MyShBuff[counter] = item;
   	  counter++;
      printf("Producer %u produced %d to MyShBuff %d\n", pthread_self(), item, counter);

      // Adding Producer Thread to the Producer Count Array
      prodarray[prodcount] = pthread_self();
      prodcount++;

      // Release the mutex lock
      pthread_mutex_unlock(&mutexp);

      // Notify Consumers
      pthread_mutex_lock(&mutexc);
      pthread_cond_signal(&condc);
      pthread_mutex_unlock(&mutexc);

      // signal full
      sem_post(&full);
   }
} //End of Produce Function

// Consume Function
void *consume(void *param) {

   int item;
   while(1) {
      // Random Sleep Time 
      sleep(rand() / 100000000);

      // Aquire the full lock
      sem_wait(&full);

      // Aquire the mutex lock
      pthread_mutex_lock(&mutexc);

      // Consume the item and update the buffer with -2
	    while (counter == 0 || MyShBuff[(counter-1)] == -2)
		    //pthread_cond_signal(&condp);
       	  pthread_cond_wait(&condc, &mutexc);
	    item = MyShBuff[(counter-1)];
   	  // counter--;

      printf("Consumer %u consumed %d from MyShBuff %d\n", pthread_self(), item, (counter-1));
      MyShBuff[(counter-1)] = -2;
		 
	    // Adding Consumer Thread to the Consumer Count Array
      conarray[concount] = pthread_self();
      concount++;

      // Release the mutex lock
      pthread_mutex_unlock(&mutexc);

      // Notify producer
      pthread_mutex_lock(&mutexp);
      pthread_cond_signal(&condp);
      pthread_mutex_unlock(&mutexp);

      // Signal empty
      sem_post(&empty);
   }
} // End of Consume Function

// Function to get most frequent element of array */
int getMaxCountElement(int *array, int size) {
    int i, j, maxCount, maxElement, count;
    maxCount = INT_MIN;
    /* Count the frequency of every elemenet of array, 
    and check if it is greater than maximum count element 
    we found till now and update it accordingly */
    for(i = 0; i< size; i++){
        count = 1;
        for(j = i+1; j < size; j++){
            if(array[j] == array[i] && array[j] != 0){
                count++;
                /* If count of current element is more than 
                maxCount, uodate maxCount and maxElement */
                if(count > maxCount){
                    maxCount = count;
                    maxElement = array[j];
                }
            }
        }
    }
    //printf("Maximum Repeating Element : %d\nCount : %d", maxElement, maxCount);
    return maxElement;
}