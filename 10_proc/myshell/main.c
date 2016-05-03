#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_LINE 256
#define PROMPT "# "
#define chop(str) str[strlen(str) -1] = 0x00;

int main(int argc, char **argv)
{
    char buf[MAX_LINE];
    int proc_status;
    pid_t pid;
    printf("My Shell Ver 1.0\n");
    while(1)
    {
        printf("%s", PROMPT);
        memset(buf, 0x00, MAX_LINE);
        fgets(buf, MAX_LINE-1, stdin);
        if(strncmp(buf, "quit\n", 5) == 0)
        {
            break;
        }
        chop(buf);
        pid = fork();
        if(pid == 0)
        {
            if(execl(buf, buf, NULL) == -1)
            {
                printf("Execl failure\n");
                exit(0);
            }
        }
        if(pid > 0)
        {
            printf("child wait\n");
            wait(&proc_status);
            printf("child exit\n");
        }
    }
    return 0;
}
