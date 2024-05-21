#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <linux/videodev2.h>

#define BAUDRATE B1000000

void capture_image(const char* filename) {
    char command[256];
    snprintf(command, sizeof(command), "libcamera-still -o %s", filename);
    system(command);
}

int main()
{
    int fd;
    struct termios newtio;
    char buf[256];
    char image_filename[] = "captured_image.jpg";

    fd = open("/dev/serial0", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr, "failed to open port: %s.\r\n", strerror(errno));
        printf("Make sure you are executing in sudo.\r\n");
        return -1;
    }
    usleep(250000);

    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);

    while (1) {
        int n = read(fd, buf, sizeof(buf));
        if (n > 0) {
            buf[n] = '\0';
            if (buf[0] == 'c' || buf[0] == 'C') {
                capture_image(image_filename);
                FILE* fp = fopen(image_filename, "rb");
                if (fp) {
                    fseek(fp, 0, SEEK_END);
                    long filesize = ftell(fp);
                    fseek(fp, 0, SEEK_SET);
                    fread(buf, 1, filesize, fp);
                    write(fd, buf, filesize);
                    fclose(fp);
                }
                else {
                    fprintf(stderr, "failed to open image file: %s.\r\n", strerror(errno));
                }
            }
        }
    }
    close(fd);
    return 0;
}
