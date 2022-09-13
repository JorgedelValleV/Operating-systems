#include <stdio.h> 
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define NR_PHILOSOPHERS 5

pthread_t philosophers[NR_PHILOSOPHERS];
pthread_mutex_t forks[NR_PHILOSOPHERS];


void init()
{
    int i;
    for(i=0; i<NR_PHILOSOPHERS; i++)
        pthread_mutex_init(&forks[i],NULL);
    
}

void think(int i) {
    printf("Philosopher %d thinking... \n" , i);
    sleep(random() % 10);
    printf("Philosopher %d stopped thinking!!! \n" , i);

}

void eat(int i) {
    printf("Philosopher %d eating... \n" , i);
    sleep(random() % 5);
    printf("Philosopher %d is not eating anymore!!! \n" , i);

}

void toSleep(int i) {
    printf("Philosopher %d sleeping... \n" , i);
    sleep(random() % 10);
    printf("Philosopher %d is awake!!! \n" , i);
    
}

void* philosopher(void* i)
{
    int nPhilosopher = (int)i;
    int right = nPhilosopher;
    int left = (nPhilosopher - 1 == -1) ? NR_PHILOSOPHERS - 1 : (nPhilosopher - 1);
    while(1)
    {
        
        think(nPhilosopher);
        
        /// TRY TO GRAB BOTH FORKS (right and left)
        // siguiendo el modelo del filosofo zurdo todos toman primero el palilllo de la derecha que tiene menor indice 
        // salvo el ultimo que al tener a la derecha el de mayor indice y a la izquierda el de menor, toma el izquierdo

        // como en este caso el palillo izquierdo tiene menor indice es el problema simetrico donde el zurdo coge 1º el derecho

        if(left<right){//normal
            pthread_mutex_lock(&forks[left]);
            pthread_mutex_lock(&forks[right]);
        }
        else{//filosofo zurdo
            pthread_mutex_lock(&forks[right]);
            pthread_mutex_lock(&forks[left]);
        }

        //
        eat(nPhilosopher);
        
        // PUT FORKS BACK ON THE TABLE
        pthread_mutex_unlock(&forks[left]);
        pthread_mutex_unlock(&forks[right]);
        //
        toSleep(nPhilosopher);
   }

}

int main()
{
    init();
    unsigned long i;
    for(i=0; i<NR_PHILOSOPHERS; i++)
        pthread_create(&philosophers[i], NULL, philosopher, (void*)i);
    
    for(i=0; i<NR_PHILOSOPHERS; i++)
        pthread_join(philosophers[i],NULL);
    return 0;
} 
/*
 * EJEMPLO DE EJECUCION
*/
/*
Philosopher 0 thinking... 
Philosopher 1 thinking... 
Philosopher 2 thinking... 
Philosopher 3 thinking... 
Philosopher 4 thinking... 
Philosopher 0 stopped thinking!!! 
Philosopher 0 eating... 
Philosopher 0 is not eating anymore!!! 
Philosopher 0 sleeping... 
Philosopher 4 stopped thinking!!! 
Philosopher 4 eating... 
Philosopher 3 stopped thinking!!! 
Philosopher 4 is not eating anymore!!! 
Philosopher 3 eating... 
Philosopher 4 sleeping... 
Philosopher 1 stopped thinking!!! 
Philosopher 1 eating... 
Philosopher 4 is awake!!! 
Philosopher 4 thinking... 
Philosopher 2 stopped thinking!!! 
Philosopher 1 is not eating anymore!!! 
Philosopher 1 sleeping... 
Philosopher 1 is awake!!! 
Philosopher 1 thinking... 
Philosopher 0 is awake!!! 
Philosopher 0 thinking... 
Philosopher 3 is not eating anymore!!! 
Philosopher 2 eating... 
Philosopher 3 sleeping... 
Philosopher 3 is awake!!! 
Philosopher 3 thinking... 
Philosopher 2 is not eating anymore!!! 
Philosopher 2 sleeping... 
Philosopher 0 stopped thinking!!! 
Philosopher 0 eating... 
Philosopher 2 is awake!!! 
Philosopher 2 thinking... 
Philosopher 0 is not eating anymore!!! 
Philosopher 0 sleeping... 
Philosopher 2 stopped thinking!!! 
Philosopher 2 eating... 
Philosopher 4 stopped thinking!!! 
Philosopher 4 eating... 
Philosopher 2 is not eating anymore!!! 
Philosopher 2 sleeping... 
Philosopher 3 stopped thinking!!! 
Philosopher 1 stopped thinking!!! 
Philosopher 1 eating... 
Philosopher 1 is not eating anymore!!! 
Philosopher 1 sleeping... 
Philosopher 4 is not eating anymore!!! 
Philosopher 4 sleeping... 
Philosopher 3 eating... 
Philosopher 2 is awake!!! 
Philosopher 2 thinking... 
Philosopher 1 is awake!!! 
Philosopher 1 thinking... 
Philosopher 3 is not eating anymore!!! 
Philosopher 3 sleeping... 
Philosopher 4 is awake!!! 
Philosopher 4 thinking... 
Philosopher 0 is awake!!! 
Philosopher 0 thinking... 
Philosopher 3 is awake!!! 
Philosopher 3 thinking... 
*/