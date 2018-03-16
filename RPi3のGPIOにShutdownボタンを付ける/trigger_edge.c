/*
preconditions:
```shell
$ echo 6 > /sys/class/gpio/export 2> /dev/null
$ echo in > /sys/class/gpio/gpio6/direction
$ echo falling > /sys/class/gpio/gpio6/edge
```
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

static char const * PATH = "/sys/class/gpio/gpio6/value";

int main(void) {
    int fd;

    puts("Start Button Trigger");
    fd = open(PATH, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Error: Failed to open the file: %s\n", PATH);
        return -1;
    }

    {
        struct pollfd polls;
        char ch;
        polls.fd = fd;
        polls.events = POLLPRI;

        read(fd, &ch, 1);
        lseek(fd, 0, SEEK_SET);
        poll(&polls, 1, -1);
    }
    puts("Falling Edge Detected");

    close(fd);
    return 0;
}
