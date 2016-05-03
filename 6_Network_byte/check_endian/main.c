#include <stdio.h>
#define BIG 0
#define LITTLE 1

int check_endian(void);

int main(int argc, char **argv)
{
    int endian;
    endian = check_endian();
    if(endian == LITTLE)
        printf("little endian\n");
    else
        printf("big endian\n");
    return 0;
}

int check_endian(void)
{
    int i = 0x00000001;
    return((char *)&i)[0] ? LITTLE : BIG;
}
