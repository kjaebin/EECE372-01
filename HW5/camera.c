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
		int cnt = read(fd, buf, 256);
		if (cnt > 0){
		buf[cnt] = '\0';
		printf("Received: %s\r\n",buf);
		if (strcmp(buf, "c") == 0 || strcmp(buf, "C") == 0 ) {
			system("raspistill -w 280 -h 280 -t 1 -o test.bmp");
			FILE* file = fopen("test.bmp", "rb");
			while(!feof(file)){
			int cntbp = fread(fbuf,1 ,1024, file);
			write(fd, fbuf, cntbp);
			}
			fclose(file);
		}
		}
	}
	return 0;
}

