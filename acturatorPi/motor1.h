#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define IN 0
#define OUT 1
#define PWM 2
#define LOW 0
#define HIGH 1
#define MVALUE_MAX 256
#define MDIRECTION_MAX 256
//#define MDIRECTION_MAX
// #define PERIOD 20000000
// #define MAX_DRGREE 2400000
// #define MIN_DGREEE 500000

static int PWMExport(int pin)
{
#define BUFFER_MAX 3
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;
    fd = open("/sys/class/pwm/pwmchip0/export", O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open export for writing!\n");
        return (-1);
    }
    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return (0);
}

static int PWMUnexport(int pin)
{
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;
    fd = open("/sys/class/pwm/pwmchip0/unexport", O_WRONLY);

    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open unexport for writing! \n");
        return (-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return (0);
}

static int PWMEnable(int pin, int dir)
{
    static const char s_enable_str[] = "01";
    //#define DIRECTION_MAX 256
    char path[MDIRECTION_MAX];
    int fd;
    snprintf(path, DIRECTION_MAX, "/sys/class/pwm/pwmchip0/pwm%d/enable", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        //fprintf(stderr, "Failed to open gpio direction for writing! \n");
        return (-1);
    }

    if (write(fd, &s_enable_str[dir == 0 ? 0 : 1], 1) == -1)
    {
        fprintf(stderr, "fail write / in PWMEnable ! \n");
    }
    close(fd);
    return (0);
}

static int PWMWritePeriod(int pin, int value)
{
    char period[MVALUE_MAX];
    char path[MVALUE_MAX];
    int fd;
    snprintf(path, MVALUE_MAX, "/sys/class/pwm/pwmchip0/pwm%d/period", pin);
    snprintf(period, MVALUE_MAX, "%d", value);

    fd = open(path, O_WRONLY);

    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open gpio value for writing in period! \n");
        return (-1);
    }

    if (write(fd, period, strlen(period)) == -1)
    {
        fprintf(stderr, "Failed to write period!\n");
        return (-1);
    }
    close(fd);
    return (0);
}

static int PWMWriteDutyCycle(int pin, int value)
{
    char path[MVALUE_MAX];
    char duty_cycle[MVALUE_MAX];
    int fd;
    snprintf(path, MVALUE_MAX, "/sys/class/pwm/pwmchip0/pwm%d/duty_cycle", pin);
    snprintf(duty_cycle, MVALUE_MAX, "%d", value);

    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open gpio value for writing in duty_cycle! \n");
        return (-1);
    }
    if (write(fd, duty_cycle, MVALUE_MAX) == -1)
    {
        fprintf(stderr, "Failed to write duty cycle");
    }

    close(fd);
    return (0);
}
