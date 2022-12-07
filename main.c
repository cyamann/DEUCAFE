#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>


#define CUSTOMER_NUMBER 25
#define CUSTOMER_ARRIVAL_TIME_MIN 1
#define CUSTOMER_ARRIVAL_TIME_MAX 3
#define REGISTER_NUMBER 5
#define COFFEE_TIME_MIN 2
#define COFFEE_TIME_MAX 5

sem_t reg; //register definition for the semaphore
sem_t customer;
sem_t mutex; //for the critical sections

//function signatures
void ActiveRegister(void*);
void ActiveCustomer(void*);

int servedCustomer = 0;
int servedCustomerSeat = 0;
int totalServedCustomer = 0;
int* registers;


int main(int argc, char** args){
   //designating an area dor customers 
   registers = (int*) calloc(CUSTOMER_NUMBER, sizeof(int));


    //declare the threads
    pthread_t register_thread[REGISTER_NUMBER];
    pthread_t customer[CUSTOMER_NUMBER];

    //entry semaphore values
    sem_init(&reg, 0, 0);
    sem_init(&customer, 0, 0);
    sem_init(&mutex, 0, 1);

    printf("\nDEU CAFE is Open.\n");
    for(int i=0; i<REGISTER_NUMBER; i++){
        pthread_create(&register_thread[i], NULL, (void*) ActiveRegister, (void*)&i); //starts the cafe threads
        sleep(1);
    }

    for(int i=0; i<CUSTOMER_NUMBER; i++){
        pthread_create(&customer[i], NULL, (void*) ActiveCustomer, (void*)&i);//starts the customer threads
    }

    //finish the customer thread
    for(int i=0; i<CUSTOMER_NUMBER; i++){
        pthread_join(customer[i], NULL);
    }

    printf("\n\n DEU CAFE is closed!\n");

    return 0;
}

void ActiveRegister(void* counter){
    int t = *(int*)counter + 1; //register id counter
    int nextCustomer, customerId;
    int coffee_time = 0;
    printf("Register %d is opened. \n",t);

    while(1){
        sem_wait(&reg);//waiting for the registers
        sem_wait(&mutex);//for not to have customers at the same register and the same time

        servedCustomer = (++servedCustomer)% CUSTOMER_NUMBER;
        nextCustomer = servedCustomer;
        customerId = registers[nextCustomer];
        registers[nextCustomer] = pthread_self();
        //waiting for critical section thread
        sem_post(&mutex);
        //waiting for customer thread
        sem_post(&customer);

        coffee_time = rand()%COFFEE_TIME_MAX + COFFEE_TIME_MIN;
        printf("Customer %d goes to register %d.\n",customerId,t);
        printf("Customer %d finished buying from register %d after %d seconds.\n", customerId,t,coffee_time);
    }
}

void ActiveCustomer(void* counter){

    int c = *(int*)counter;
    int servedSeat , registerId;
    int arrival_time = 0;

    while(1){
        //waiting for critical section thread
        sem_wait(&mutex);
        arrival_time = rand()%CUSTOMER_ARRIVAL_TIME_MAX + CUSTOMER_ARRIVAL_TIME_MIN;
        sleep(arrival_time);
        printf("Customer %d created after %d seconds.\n",c,arrival_time);

        if(servedCustomer<CUSTOMER_NUMBER){


        servedCustomerSeat = (++servedCustomerSeat) % CUSTOMER_NUMBER;
        servedSeat = servedCustomerSeat;
        registers[servedSeat] = c;

        sem_post(&reg);//for assigning a customer to a register
        sem_post(&mutex);//end oof critical section because the customer has been served

        registerId = registers[servedSeat];

        totalServedCustomer++;
        sem_post(&mutex);

        }
        pthread_exit(0);
    }
}
