#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mut2 = PTHREAD_MUTEX_INITIALIZER;

long long a = 0, b = 1, c = 2;


void deadlockCreator1(void* arg)
{
    pthread_mutex_lock(&mut);
    for (a = 0; a < 50000000; a++)
    b = 0;
    pthread_mutex_unlock(&mut);
}

void deadlockCreator2(void* arg)
{
    pthread_mutex_lock(&mut2);
    for (b = 0; b < 50000000; b++)
    a = 0;
    pthread_mutex_unlock(&mut2);
}

int main(int argc, char **argv)
{
    pthread_t th1, th2;
    if (pthread_create(&th1, NULL, deadlockCreator1, NULL) != 0)
    {
        perror("pthread_create");
        exit(1);
    }
    if (pthread_create(&th2, NULL, deadlockCreator2, NULL) != 0)
    {
        perror("pthread_create");
        exit(1);
    }
    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    
    return 0;
}