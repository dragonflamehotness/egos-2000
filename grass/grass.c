/*
 * (C) 2022, Cornell University
 * All rights reserved.
 */

/* Author: Yunhao Zhang
 * Description: initialize the process control block and 
 * spawns some kernel processes, including file system and shell
 */

#include "egos.h"
#include "grass.h"

struct earth *earth = (void*)EARTH_ADDR;

int global_var1;
int global_var2;
int global_var3 = 1;

int main() {
    global_var1++;
    global_var2++;
    global_var3++;

    char* buf = malloc(512);
    free(buf);
    SUCCESS("Within grass kernel with stack variable @0x%.8x", &buf);
    return 0;
}
