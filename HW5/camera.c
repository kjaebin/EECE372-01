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
	
	fd = open("/dev/ttyAMA0", O_RDWR|O_NOCTTY);
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
		printf("Start!\r\n");
		int input = read(fd, buf, sizeof(buf));
		
		if (input > 0) {
			if (buf[0] == 'c' || buf[0] == 'C') {
			system("raspistill -w 640 -h 480 -t 10 -o output.bmp");
				char filename[20];
				sprintf(filename, "output.bmp");
					FILE* imageFile = fopen(filename, "rb");
					if (imageFile) {
						while (!feof(imageFile)) { 
							int size = 1;
							int Read = fread(fbuf, size, sizeof(fbuf), imageFile);
							int Sent = write(fd, fbuf, Read);
						}
						printf("Success!\r\n");
						fclose(imageFile);
						break;
					}			
			}
		}
	}
	return 0;
}
