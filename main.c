#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <mqueue.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "mavlink.h"
#include "mavlink_parser.h"
#include "mavlink_publisher.h"
#include "net.h"
#include "serial.h"

#define SYS_ID 1 /* System ID of this program */

enum { SERIAL_MODE, NET_MODE };

pthread_t thread1, thread2;
sem_t sem_terminate;
mqd_t mqd_mavlink;

int mavlink_fd;
bool verbose = false;

uint8_t get_sys_id(void)
{
    return SYS_ID;
}

void *mavlink_rx_thread(void *arg)
{
    uint8_t c;
    mavlink_status_t status;
    mavlink_message_t recvd_msg;

    while (1) {
        read(mavlink_fd, &c, 1);

        /* Try parsing the message with new income byte */
        if (mavlink_parse_char(MAVLINK_COMM_1, c, &recvd_msg, &status) == 1) {
            /* Notify the tx thread */
            mq_send(mqd_mavlink, (char *) &recvd_msg, sizeof(recvd_msg), 0);
        }
    }
}

static double get_time_sec(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (double) tv.tv_sec + (double) tv.tv_usec * 1e-6;
}

#define MSG_SCHEDULER_INIT(freq)          \
    double timer_##freq = get_time_sec(); \
    double period_##freq = 1.0 / (double) freq;

#define MSG_SEND_HZ(freq, expressions)                     \
    double now_##freq = get_time_sec();                    \
    double elapsed_##freq = (now_##freq) - (timer_##freq); \
    if ((elapsed_##freq) >= (period_##freq)) {             \
        (timer_##freq) = (now_##freq);                     \
        expressions                                        \
    }

void *mavlink_tx_thread(void *arg)
{
    /* Objective of the tx thread:
     * 1. send message periodically (e.g., hearbeat)
     * 2. react to the received message (i.e., protocols)
     */

    MSG_SCHEDULER_INIT(1); /* 1Hz */

    mavlink_message_t recvd_msg;

    while (1) {
        /* clang-format off */
        MSG_SEND_HZ(1,
            mavlink_send_play_tune();
            mavlink_send_heartbeat();
        );
        /* clang-format on */

        /* Trigger the command parser if received new message from the queue */
        if (mq_receive(mqd_mavlink, (char *) &recvd_msg, sizeof(recvd_msg),
                       0) != -1) {
            parse_mavlink_msg(&recvd_msg);
        }

        /* Limit CPU usage of the thread with execution frequency of 100Hz */
        usleep(10000); /* 10000us = 10ms */
    }
}

void mavlink_send_msg(mavlink_message_t *msg)
{
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    size_t len = mavlink_msg_to_send_buffer(buf, msg);

    write(mavlink_fd, buf, len);
}

void sig_handler(int signum)
{
    pthread_cancel(thread1);
    pthread_cancel(thread2);

    sem_post(&sem_terminate);
}

static bool parse_long_from_str(char *str, long *value)
{
    char *end_ptr = NULL;
    errno = 0;
    *value = strtol(str, &end_ptr, 10);
    if (errno != 0 || *end_ptr != '\0') {
        return false;
    } else {
        return true;
    }
}

static void usage(const char *execpath)
{
    printf(
        "Usage:\n"
        "    %s [-v] -n -i ip-address -p port-number\n"
        "or\n"
        "    %s [-v] -s serial-name -b baudrate\n",
        execpath, execpath);
}

static void handle_options(int argc,
                           char **argv,
                           int *mode,
                           char **ip_addr,
                           char **port_num,
                           char **serial_name,
                           char **baudrate)
{
    *mode = SERIAL_MODE;
    *ip_addr = *port_num = *serial_name = *baudrate = NULL;

    int optidx = 0;
    struct option opts[] = {
        {"net", 0, NULL, 'n'},      {"ip", 1, NULL, 'i'},
        {"port", 1, NULL, 'p'},     {"serial", 1, NULL, 's'},
        {"baudrate", 1, NULL, 'b'}, {"verbose", 0, NULL, 'v'},
        {"help", 0, NULL, 'h'},
    };

    int c;
    while ((c = getopt_long(argc, argv, "ni:p:s:b:vh", opts, &optidx)) != -1) {
        switch (c) {
        case 'n':
            *mode = NET_MODE;
            break;
        case 'i':
            *ip_addr = optarg;
            break;
        case 'p':
            *port_num = optarg;
            break;
        case 's':
            *serial_name = optarg;
            break;
        case 'b':
            *baudrate = optarg;
            break;
        case 'v':
            verbose = true;
            break;
        case 'h':
            usage(argv[0]);
            exit(1);
        default:
            break;
        }
    }

    long val;
    if (*mode == SERIAL_MODE) {
        if (!*serial_name) {
            printf("Serial name must be provided via -s option.\n\n");
            usage(argv[0]);
            exit(1);
        }

        if (!*baudrate)
            *baudrate = "115200";

        if (!parse_long_from_str(*baudrate, &val)) {
            printf("Bad serial baudrate value.\n");
            exit(1);
        }
    } else if (*mode == NET_MODE) {
        if (!*ip_addr) {
            printf("IP address must be provided via -i option.\n\n");
            usage(argv[0]);
            exit(1);
        }

        if (!*port_num) {
            printf("Port number must be provided via -p option.\n\n");
            usage(argv[0]);
            exit(1);
        }

        long val;
        if (!parse_long_from_str(*port_num, &val)) {
            printf("Bad port number.\n");
            exit(1);
        }
    }
}

int main(int argc, char **argv)
{
    char *ip_addr;
    char *port_num;
    char *serial_name;
    char *baudrate;
    int mode;

    /* Parse input arguments */
    handle_options(argc, argv, &mode, &ip_addr, &port_num, &serial_name,
                   &baudrate);

    if (mode == SERIAL_MODE) {
        /* Connect via serial */
        mavlink_fd = open_serial(serial_name, atoi(baudrate));
    } else if (mode == NET_MODE) {
        /* Connect via TCP/IP */
        mavlink_fd = open_net(ip_addr, atoi(port_num));
    }

    /* Create a mqueue for passing the received mavlink message from
     * rx thread to the tx thread
     */
    struct mq_attr attr = {.mq_flags = 0,
                           .mq_maxmsg = 10,
                           .mq_msgsize = sizeof(mavlink_message_t),
                           .mq_curmsgs = 0};
    int flags = O_NONBLOCK | O_RDWR | O_CREAT;
    int _mode = S_IRUSR | S_IWUSR;
    mqd_mavlink = mq_open("/mavlink", flags, _mode, &attr);

    if (mqd_mavlink == -1) {
        perror("mq_open");
        exit(1);
    }

    /* Launch mavlink threads */
    pthread_create(&thread1, NULL, mavlink_rx_thread, NULL);
    pthread_create(&thread2, NULL, mavlink_tx_thread, NULL);

    /* Install the signal handlers */
    sem_init(&sem_terminate, 0, 0);
    signal(SIGINT, sig_handler);
    signal(SIGABRT, sig_handler);
    signal(SIGTERM, sig_handler);

    /* Wait until the program is terminated */
    sem_wait(&sem_terminate);

    /* Clean up */
    sem_destroy(&sem_terminate);
    mq_close(mqd_mavlink);
    mq_unlink("/mavlink");
    close(mavlink_fd);

    return 0;
}
