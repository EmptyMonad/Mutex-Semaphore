/*
Project 3 - Producer Consumer Problem

I referenced some code for the printing syntax and the mutex/semaphore utilization.
My apologies if any portion of the code responsible for printing the buffer syntax is marked on any MOSS software for students. :)
*/

#include <iostream>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/select.h>

#ifndef _BUFFER_H_DEFINED_

#define _BUFFER_H_DEFINED_

typedef int buffer_item;

#define BUFFER_SIZE 5

bool buffer_insert_item( buffer_item item );
bool buffer_remove_item( buffer_item *item );

#endif // _BUFFER_H_DEFINED_

using namespace std;

/*Declarations*/

struct thread_t 
{
pthread_t id;
unsigned int num;
};

volatile int stop = 0; //stop is set to volatile because it is changing frequently through our checks and also I saw it in a guide
int producerNum; //whatever producer generates
int consumerNum; //whatever consumer generates
int timer; //how long buffer runs, used for seed generation
int maxTime; //limit to runtime
int fullCount; //counter for full buffer
int emptyCount; //counter for empty buffer
struct thread_t *threads; //thread declaration
void *producer(void *parameter); //producer function
void *consumer(void *parameter); //consumer function
void threadIni(); //thread creation process
void syncIni(); //mutex + semaphore alignment for prod/cons
void iniBuffer(); //buffer function
void display(); //print function for statistics
void sleepMutex(int second); //sleep function with Mutex operations
void printBuffer(); //prints buffer
buffer_item buffer[BUFFER_SIZE]; //the actual buffer
int tail; //num at end of buffer
int head; //num at beginning of buffer
int quantity; //amount in buffer (for counts)
sem_t sem_full; //semaphore thing
sem_t sem_empty; //semaphore thing 2
pthread_mutex_t mutex; //mutex thing
bool isPrime(int p); //checks for prime number
int bufferPrint;

/*End of Declarations*/

int main(int argc, char *argv[])
{
	int i;
	if (argc != 6)
	{
		cout << "This program requires 6 arguments. The execution, the run time, the max run time, the number of producer threads, the number of consumer threads, and 1/0 for buffer printing.";
		exit;
	}
	timer = atoi(argv[1]);
	maxTime = atoi(argv[2]);
	producerNum = atoi(argv[3]);
	consumerNum = atoi(argv[4]);
	bufferPrint = strcmp(argv[5], "1") == 0;
	iniBuffer();
	syncIni();
	if (bufferPrint)
	{
		cout << "Creating Threads.";
		printBuffer();
	}
	threadIni();
	sleepMutex(timer);
	stop = 1;
	for (i = 0; i < producerNum + consumerNum; i++)
	{
		pthread_cancel(threads[i].id);
		pthread_join(threads[i].id, NULL);
	}
	display();
	free(threads);
	return 0;
	
	//cl args
	//init buffer
	//create prod thread
	//create cons thread
	//sleep
	//join threads
	//stat display
	//exit
}



void *producer(void *parameter)
{	//Initialize the random seed, buffer ID, and struct pointer for P_Thread
	struct thread_t *t = (struct thread_t*) parameter;
	unsigned int id = (unsigned int)t->id;
	unsigned int seed = time(NULL) + id;
	while (!stop)
	{
	int x = rand_r(&seed) %100;
	if (buffer_insert_item(x) == 0)
		{
			break;
		}
	t -> num++;
	t -> num = rand_r(&seed) % maxTime + 1;
	sleepMutex(x);
	}
	return NULL;
}
void *consumer(void *parameter)
{	//starts a thread with the parameter input from arguments, then starts the random seed. Random seed is applied to the stop flag
	//then depending on timing, an item is removed from the buffer. The mutex is utilized here to keep the data from changing during sleep.
	struct thread_t *t = (struct thread_t *) parameter;
	int id = (int)t->id;
	unsigned int seed = time(NULL) + id;

	while (!stop)
	{
	int y;
	if (buffer_remove_item(&y) == 0)
	{
		break;
	}
	t -> num++;
	y = rand_r(&seed) % maxTime + 1;
	sleepMutex(y);
	}
	return NULL;
}
void iniBuffer()
{	//this creates the buffer with a simple for-loop.
	int i;
	tail = 0;
	head = 0;
	quantity = 0;
	for (i = 0; i < BUFFER_SIZE; i++)
	{
		buffer[i] = -1;
	}
}
bool buffer_insert_item(buffer_item item)
{	//creates thread, then makes the semaphore wait for the buffer to have a slot once called from Producer. Error checking from reference code.
	int id = (int)pthread_self();
	if (sem_trywait(&sem_empty) != 0)
	{
		if (errno == EAGAIN)
		{
			if (bufferPrint)
			{
				cout << "Buffer full. Producer waiting... " << id << "\n";
			}
			if (sem_wait(&sem_empty) != 0)
			{
				cout << "Something in the Semaphore broke.\n";
				return 0;
			}
		}
	}
	if (pthread_mutex_lock(&mutex) != 0)
		{
			cout << "Something in the Mutex lock broke. \n";
			return 0;
		}
	buffer[head] = item;
	head = (head + 1) % BUFFER_SIZE;
	quantity++;
	if (quantity == BUFFER_SIZE)
	{
		fullCount++;
	}
	if (bufferPrint)
	{
		cout << item;
		printBuffer();
	}
	if (pthread_mutex_unlock(&mutex) != 0)
		{
			cout << "Something in the Mutex unlock broke.\n";
			return 0;
		}	
	sem_post(&sem_full);
	return 1;
}
bool buffer_remove_item(buffer_item *item)
{	//creates thread, then makes the semaphore wait for the buffer to have a slot once called from Consumer. Error checking from reference code.
	buffer_item z;
	int id = (int)pthread_self();
	z = buffer[tail];
	tail = (tail + 1) % BUFFER_SIZE;
	if (sem_trywait(&sem_full) != 0)
	{
		if (errno == EAGAIN)
		{
			if (bufferPrint)
			{
				cout << "Buffer empty. Consumer waiting... " << id << "\n";
			}
			if (sem_wait(&sem_full) != 0)
			{
				cout << "Something in the Semaphore broke.\n";
				return 0;
			}
		}
	}
	if (pthread_mutex_lock(&mutex) != 0)
		{
			cout << "Something in the Mutex lock broke.\n";
			return 0;
		}
	quantity--;
	if (quantity == 0)
	{
		emptyCount++;
	}
	if (bufferPrint)
	{
		if (isPrime(z))
		{
			cout << "*** PRIME ***\n";
		}
		else 
			{
				cout << "\n";
			}
	}
	printBuffer();
	if (pthread_mutex_unlock(&mutex) != 0)
		{
			cout << "Something in the Mutex unlock broke.\n";
			return 0;
		}	
	sem_post(&sem_empty);
	*item = z;
	buffer[tail-2] = -1;
	return 1;
}
bool isPrime(int p)
{ //simple prime check on value inserted into buffer
	int i;
	for (i = 2; i * i <= p; i++)
	{
		if (p % i == 0)
			return 0;
	}
	return 1;
}
void threadIni()
{ //this creates all of the threads necessary for the program to run
	int i;
	threads = (struct thread_t *)calloc(producerNum + consumerNum, sizeof(struct thread_t));
	if (threads == NULL)
	{
		cout << "Something in the thread broke.\n";
		exit;
	}
	for (i = 0; i < producerNum; i++)
	{
		if (pthread_create(&threads[i].id, NULL, producer, (void *)&threads[i]) != 0)
		{
			cout << "Something in the producer thread broke.\n";
			exit;
		}
	}
	for (i = producerNum; i < producerNum + consumerNum; i++)
	{
		if (pthread_create(&threads[i].id, NULL, consumer, (void *)&threads[i]) != 0)
		{
			cout << "Something in the consumer thread broke.\n";
			exit;
		}
	}
}
void syncIni()
{	//this function is to make sure that the initialization of threads is on the same timing, utilizing the mutex/semaphore to preserve data.
	pthread_mutex_init(&mutex, NULL);
	sem_init(&sem_full, 0, 0);
	sem_init(&sem_empty, 0, BUFFER_SIZE);
}
void sleepMutex(int second)
{	//keeps data in sleep without mutex
	struct timeval timeout;
	timeout.tv_sec = second;
	timeout.tv_usec = 0;
	select(0, NULL, NULL, NULL, &timeout);
}
void printBuffer()
{	//syntax was very hard to get right, utilized reference code for the spacing on certain areas.
	int i;
	cout << "Buffers filled: " << quantity << " ";
	cout << "Buffers: ";
	for (i = 0; i < BUFFER_SIZE; i++)
	{
		cout << buffer[i] << " ";
	}
	cout << "\n";
	for (i = 0; i < BUFFER_SIZE; i++)
	{
		cout << "**";
	}
	cout << "\n";
}
void display()
{	//this function prints the values of everything processed.
	int prodTot = 0;
	int consTot = 0;
	int i;
	char cache[64];
	for (i = 0; i < producerNum; i++)
	{
		prodTot += threads[i].num;
	}
	for (i = producerNum; i < producerNum + consumerNum; i++)
	{
		consTot += threads[i].num;
	}
	cout << "The buffer processing is finished.\n";
	cout << "Runtime:" << timer << "\n";
	cout << "Max Sleep Time: " << maxTime << "\n";
	cout << "# of Producer Threads: " << producerNum << "\n";
	cout << "# of Consumer Threads: " << consumerNum << "\n";
	cout << "Size of Buffer: 5 \n";
	cout << "Total Number of Items Produced: " << prodTot << "\n";
	for (i = 0; i < producerNum; i ++)
	{
		cout << cache << "Thread " << i + 1 << "\n";
		cout << cache << threads[i].num << "\n";
	}
	cout << "Total Number of Items Consumed: " << consTot << "\n";
	for (i = producerNum; i < producerNum + consumerNum; i++)
	{
		cout << cache << "Thread " << i + 1 << "\n";
		cout << cache << threads[i].num << "\n";
	}
	cout << "\n";
	cout << "Number of Items Remaining in Buffer: 0 \n";
	cout << "Number of Times the Buffer was Full: " << fullCount << "\n";
	cout << "Number of Times the Buffer was Empty: " << emptyCount << "\n";
}

/*For Reference*/

/* create the mutex lock 
pthread_mutex_init( &mutex, NULL );
aquire the mutex lock:
pthread_mutex_lock( &mutex );
//critical data here
release the mutex lock:
pthread_mutex_unlock( &mutex );*/

/*semaphore
sem_t sem;
sem_init (&sem, 0, 5);
acquire the semaphore:
sem_wait( &mutex );
//critical data here
release the semaphore :
sem_post( &mutex );
/*For Later*/














