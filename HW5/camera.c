#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#define BAUDRATE B1000000

int main()
{
	int fd;
	struct termios newtio;
	char fbuf[1024];
	char buf[256];

	fd = open("/dev/serial0", O_RDWR|O_NOCTTY);
	if(fd<0) {
		fprintf(stderr, "failed to open port: %s.\r\n", strerror(errno));
		printf("Make sure you are executing in sudo.\r\n");
	}
	usleep(250000);

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = BAUDRATE|CS8|CLOCAL|CREAD;
	newtio.c_iflag = ICRNL;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;
	
//	speed_t baudRate = B1000000;
//	cfsetispeed(&newtio, baudRate);
//	cfsetospeed(&newtio, baudRate);

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	while(1) {

		read(fd, buf, sizeof(buf));
		char first = buf[0];
		if ((first == 'c')||(first == 'C'))[
			system("libcamera-still --width 640 --height 480 -o 20220709_image.bmp");
			FILE* image = fopen("20220709_image.bmp", "rb");
			//printf("cheeze/r/n");
			whlie(fread(fbuf, sizeof(char), sizeof(buf), image) == sizeof(fbuf)) {
				if (feof(image) == 1) {
					break;
				}
				write(fd, fbuf, sizeof(fbuf));
			}
			write(fd, fbuf, sizeof(fbuf));
			fclose(image);
		]

	}
	return 0;
}
