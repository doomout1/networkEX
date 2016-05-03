#include <unistd.h>
#include <string.h>
#include <stdio.h>
#define MAXLEN 80

int main(int argc, char **argv)
{
    char buf[MAXLEN];
    printf("Input Your Message : \n");
    memset(buf, 0x00, MAXLEN);
    read(0, buf, MAXLEN);

    printf("Your Message is : \n");
    write(1, buf, MAXLEN);
    return 0;
}
