#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int shmid;
    int semid;

    int *cal_num;
    void *shared_memory = NULL;

    struct sembuf semopen = {0, -1, SEM_UNDO};
    struct sembuf semclose = {0, 1, SEM_UNDO};

    shmid = shmget((key_t)1234, sizeof(int), 0666);
    if(shmid == -1)
    {
        perror("shmget failed : ");
        exit(0);
    }

    semid = semget((key_t)3477, 0, 0666);
    if(semid == -1)
    {
        perror("semget failed : ");
        return 1;
    }

    shared_memory = shmat(shmid, NULL, 0);
    if(shared_memory == (void*)-1)
    {
        perror("shmat failed : ");
        exit(0);
    }

    cal_num = (int *)shared_memory;

    while(1)
    {
        int local_var = 0;
        if(semop(semid, &semopen, 1) == -1)
        {
            perror("semop error : ");
        }
        local_var = *cal_num + 1;
        sleep(2);
        *cal_num = local_var;
        printf("count : %d\n", *cal_num);
        semop(semid, & semclose, 1);
    }
    return 1;
}
