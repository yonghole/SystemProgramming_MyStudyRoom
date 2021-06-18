#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ARRAY_SIZE(array) sizeof(array) / sizeof(array[0])

static uint8_t MODE = SPI_MODE_0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static int DELAY = 5;

static int prepareSpi(int fd) {
    if (ioctl(fd, SPI_IOC_WR_MODE, &MODE) == -1) {
        perror("can't set mode");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_MODE, &MODE) == -1) {
        perror("can't set mode");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &BITS) == -1) {
        perror("can't set number of bits");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &CLOCK) == -1) {
        perror("can't set write clock");
        return -1;
    }

    if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &CLOCK) == -1) {
        perror("can't set resd clock");
        return -1;
    }

    return 0;
}


/*
 *  (SGL/DIF = 0, D2=D1=D0=0)
 */
uint8_t control_bits_differential(uint8_t channel) {
    return (channel & 7) << 4;
}

/*
 * (SGL/DIF = 1, D2=D1=D0=0)
 */
uint8_t control_bits(uint8_t channel) {
    return 0x8 | control_bits_differential(channel);
}

/*
 * Given a prep'd descriptor, and an ADC channel, fetch the
 * raw ADC value for the given channel.
 */
int readadc(int fd, uint8_t channel) {
    uint8_t tx[] = { 1, control_bits(channel), 0 };
    uint8_t rx[3];

    struct spi_ioc_transfer tr = {
       .tx_buf = (unsigned long)tx,
       .rx_buf = (unsigned long)rx,
       .len = ARRAY_SIZE(tx),
       .delay_usecs = DELAY,
       .speed_hz = CLOCK,
       .bits_per_word = BITS,
    };

    if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) == 1) {
        perror("IO Error");
        abort();
    }
    return ((rx[1] << 8) & 0x300) | (rx[2] & 0xFF);
}
