#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define DEVICE_NODE "/dev/kbd"

/* ham kiem tra entry point open cua vchar driver */
int open_chardev() {
    int fd = open(DEVICE_NODE, O_RDWR);
    if(fd < 0) {
        printf("%s: can not open the device file %s\n", __FUNCTION__, DEVICE_NODE);
        exit(1);
    }
    return fd;
}

/* ham kiem tra entry point release cua vchar driver */
void close_chardev(int fd) {
    close(fd);
}

int main() {
    int ret = 0;
    char option = 'q';
    int fd = -1;
    char buf[1024];
    char *message = "Hello from user space\n";

    memset(buf, '\0', sizeof(buf));
    
    printf("Select below options:\n"
        "\t---- user's method ----\n"
        "\tq: quit the application\n"
        "\tR: Reset user's buffer\n"
        "\ta: append message to user's buffer\n"
        "\td: display user's buffer\n\n"
        "\t---- kernel's method ----\n"
        "\to: open a device node\n"
        "\tc: close the device node\n"
        "\tr: read from kernel buffer\n"
        "\tw: write to kernel buffer\n");

    while (1) {
        printf("Enter your option: ");
        scanf(" %c", &option);

        switch (option) 
        {
            case 'o':
                if (fd < 0)
                    fd = open_chardev();
                else
                    printf("%s has already opened\n", DEVICE_NODE);
                break;
            case 'c':
                if (fd > -1)
                    close_chardev(fd);
                else
                    printf("%s has not opened yet! Can not close\n", DEVICE_NODE);
                fd = -1;
                break;
            case 'q':
                if (fd > -1)
                    close_chardev(fd);
                printf("Quit the application. Good bye!\n");
                return 0;
            case 'r':
                if(fd < 0)
                    break;
                else {
                    while(ret = read(fd, buf, 1024)) {
                      printf("read %d bytes:\n%s", ret, buf);
                    }
                    printf("read %d bytes\n", ret);
                    break;
                }
            /* added by Huynh Trung Tin */
            case 'R':
                memset(buf, '\0', sizeof(buf));
                break;
            case 'a':
                strcat(buf, message);
                break;
            case 'd':
                if(buf[0])
                    printf("user's buffer:\n%s", buf);
                else
                    printf("buffer empty!\n");
                break;
            case 'w':
                if(fd < 0)
                    break;
                else {
                    printf("sizeof(buf)=%ld\tstrlen(buf)=%ld\n", sizeof(buf), strlen(buf));
                    ret = write(fd, buf, strlen(buf));
                    printf("wrote %d bytes to %s\n", ret, DEVICE_NODE);
                    break;
                }
            default:
                printf("invalid option %c\n", option);
        }
    }
}
