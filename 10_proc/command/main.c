#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    printf("--START %d--\n", getpid());
    execl("./exec_test","exec_test",NULL);
    printf("--END--\n");
    return 1;
}
