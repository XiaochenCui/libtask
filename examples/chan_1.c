#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <task.h>

Channel *c;

void task_2(void *arg)
{
    printf("task_2 start\n");
    unsigned long v = chanrecvul(c);
    printf("received: %lu\n", v);
}

void taskmain(int argc, char **argv)
{
    c = chancreate(sizeof(unsigned long), 3);
    taskcreate(task_2, c, 32768);
    unsigned long v = 12345;
    printf("going to send number: %lu\n", v);
    chansendul(c, v);
    printf("send success: %lu\n", v);
}