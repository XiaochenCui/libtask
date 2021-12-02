#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <task.h>

Channel *c;

void taskmain(int argc, char **argv)
{
    c = chancreate(sizeof(unsigned long), 3);
    for (unsigned long i = 0; i < 10; i++)
    {
        printf("going to send number: %lu\n", i);
        chansendul(c, i);
        printf("send success: %lu\n", i);
    }
    printf("taskmain end\n");
}