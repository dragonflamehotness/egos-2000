#include "app.h"
#include "servers.h"

const int nticks = 10;  /* You may adjust this number. */
int main() {
    printf("Start to sleep for %d ticks\n\r", nticks);
    sleep(nticks);
    printf("Wake up after sleeping for %d ticks\n\r", nticks);
}