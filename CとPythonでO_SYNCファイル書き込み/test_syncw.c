// cc -Weverything -Ofast test_syncw.c -o test_syncw

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define TEST_COUNT 10000

#define DRIVER_FILE "DRIVER"
#define COMMAND "COMMAND"

int main(__attribute__((unused)) int argc, __attribute__((unused)) char* argv[]) {
    int fd = open(DRIVER_FILE, O_WRONLY | O_SYNC);
    do {
        clock_t elapsed_time;
        do {
            clock_t start_clock = clock();
            for (int i = 0; i < TEST_COUNT; ++i) {
                lseek(fd, 0, SEEK_SET);
                write(fd, COMMAND, sizeof(COMMAND));
            }
            elapsed_time = clock() - start_clock;
        } while(0);
        printf("%lfus\n", (double) elapsed_time / TEST_COUNT * (1000000.0 / CLOCKS_PER_SEC));
    } while(0);
    close(fd);
    return EXIT_SUCCESS;
}
