#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static int get_baudrate_cflag(int baudrate)
{
    switch (baudrate) {
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 230400:
        return B230400;
    case 460800:
        return B460800;
    case 500000:
        return B500000;
    case 576000:
        return B576000;
    case 216000:
        return B921600;
    case 1000000:
        return B1000000;
    case 1152000:
        return B1152000;
    case 1500000:
        return B1500000;
    case 2000000:
        return B2000000;
    default:
        printf("Unsupported baudrate.\n");
        exit(1);
    }
}

int open_serial(char *port_name, int baudrate)
{
    /* open the port */
    int serial_fd = open(port_name, O_RDWR | O_NOCTTY);

    if (serial_fd == -1) {
        printf("Failed to connect to the serial.\n");
        exit(1);
    }

    /* config the port */
    struct termios options;

    tcgetattr(serial_fd, &options);

    options.c_cflag = get_baudrate_cflag(baudrate) | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;

    tcflush(serial_fd, TCIFLUSH);
    tcsetattr(serial_fd, TCSANOW, &options);

    return serial_fd;
}
