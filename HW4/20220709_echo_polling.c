#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <wiringPi.h>

#define BAUDRATE B1000000

int fd;
char buf[256];
int pin_num[] = { 29, 28, 23, 22, 21, 27, 26 };

void updateLEDs(char firstChar) {
    const int PIN_COUNT = 7;
    int hex_table[16][7] = {
        {1, 1, 1, 1, 1, 1, 0}, // 0
        {0, 1, 1, 0, 0, 0, 0}, // 1
        // Add the rest of your hex table here...
    };

    int index = (firstChar >= '0' && firstChar <= '9') ? firstChar - '0' :
                (firstChar >= 'A' && firstChar <= 'F') ? firstChar - 'A' + 10 : -1;

    if (index != -1) {
        for (int i = 0; i < PIN_COUNT; i++) {
            digitalWrite(pin_num[i], hex_table[index][i]);
        }
    } else {
        // Display 'X' for invalid input
        digitalWrite(29, 0);
        digitalWrite(28, 1);
        digitalWrite(23, 1);
        digitalWrite(22, 0);
        digitalWrite(21, 1);
        digitalWrite(27, 1);
        digitalWrite(26, 1);
    }
}

void processReceivedData() {
    int cnt = read(fd, buf, sizeof(buf) - 1);
    if (cnt > 0) {
        buf[cnt] = '\0'; // Null-terminate the string
        write(fd, "Echo: ", 6);
        write(fd, buf, cnt);
        write(fd, "\r\n", 2);
        printf("Received: %s\r\n", buf);
        updateLEDs(buf[0]);
    }
}

void task() {
    // Simulate some workload
}

int main() {
    wiringPiSetup();
    for (int i = 0; i < 7; i++) {
        pinMode(pin_num[i], OUTPUT);
    }

    struct termios newtio;
    fd = open("/dev/serial0", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        perror("Failed to open port");
        return -1;
    }

    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;

    tcsetattr(fd, TCSANOW, &newtio);
    tcflush(fd, TCIFLUSH);

    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN;

    while (1) {
        task();
        int ret = poll(fds, 1, -1); // Wait indefinitely until data is available
        if (ret > 0) {
            if (fds[0].revents & POLLIN) {
                processReceivedData();
            }
        }
    }

    close(fd);
    return 0;
}
