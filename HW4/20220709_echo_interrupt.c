#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <wiringPi.h>

#define BAUDRATE B1000000

int fd;
char buf[256];

void callback_function(int status)
{
	int cnt = read(fd, buf, 256);
	buf[cnt] = '\0';
	write(fd, "echo: ", 6);
	write(fd, buf, cnt);
	write(fd, "\r\n", 2);
	printf("Received: %s\r\n", buf);

}

void task()
{
	int i;
	for (i = 0; i < 400000000; i++);
}

int main()
{

	struct termios newtio;
	struct sigaction saio;

	fd = open("/dev/serial0", O_RDWR | O_NOCTTY);
	if (fd < 0) {
		fprintf(stderr, "failed to open port: %s.\r\n", strerror(errno));
		printf("Make sure you are executing in sudo.\r\n");
	}
	usleep(250000);

	memset(&saio, 0, sizeof(saio));
	saio.sa_handler = callback_function;
	saio.sa_restorer = NULL;
	sigaction(SIGIO, &saio, NULL);

	fcntl(fd, F_SETOWN, getpid());
	fcntl(fd, F_SETFL, FASYNC);

	memset(&newtio, 0, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = ICRNL;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 1;

	//	speed_t baudRate = B1000000;      //Use when there is a problem with Baudrate
	//	cfsetispeed(&newtio, baudRate);
	//	cfsetospeed(&newtio, baudRate);

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	write(fd, "interrupt method\r\n", 18);

	int i;
	const int PIN_COUNT = 7;
	const int hex_table[16][7] = {
		{1, 1, 1, 1, 1, 1, 0},  // 0
		{0, 1, 1, 0, 0, 0, 0},  // 1
		{1, 1, 0, 1, 1, 0, 1},  // 2
		{1, 1, 1, 1, 0, 0, 1},  // 3
		{0, 1, 1, 0, 0, 1, 1},  // 4
		{1, 0, 1, 1, 0, 1, 1},  // 5
		{1, 0, 1, 1, 1, 1, 1},  // 6
		{1, 1, 1, 0, 0, 0, 0},  // 7
		{1, 1, 1, 1, 1, 1, 1},  // 8
		{1, 1, 1, 1, 0, 1, 1},  // 9
		{1, 1, 1, 0, 1, 1, 1},  // A
		{0, 0, 1, 1, 1, 1, 1},  // B
		{0, 0, 0, 1, 1, 0, 1},  // C
		{0, 1, 1, 1, 1, 0, 1},  // D
		{1, 0, 0, 1, 1, 1, 1},  // E
		{1, 0, 0, 0, 1, 1, 1}   // F
	};
	int pin_num[] = { 29,28,23,22,21,27,26 };

	wiringPiSetup();

	for (i = 0; i < PIN_COUNT; i++) {
		pinMode(pin_num[i], OUTPUT);
	};

	while (1) {

		task();
		char firstChar = buf[0];


		if (firstChar == '0') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[0][i]);
		}
		else if (firstChar == '1') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[1][i]);
		}
		else if (firstChar == '2') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[2][i]);
		}
		else if (firstChar == '3') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[3][i]);
		}
		else if (firstChar == '4') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[4][i]);
		}
		else if (firstChar == '5') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[5][i]);
		}
		else if (firstChar == '6') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[6][i]);
		}
		else if (firstChar == '7') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[7][i]);
		}
		else if (firstChar == '8') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[8][i]);
		}
		else if (firstChar == '9') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[9][i]);
		}
		else if (firstChar == 'A') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[10][i]);
		}
		else if (firstChar == 'B') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[11][i]);
		}
		else if (firstChar == 'C') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[12][i]);
		}
		else if (firstChar == 'D') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[13][i]);
		}
		else if (firstChar == 'E') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[14][i]);
		}
		else if (firstChar == 'F') {
			for (i = 0; i < PIN_COUNT; i++)
				digitalWrite(pin_num[i], hex_table[15][i]);
		}
		else {
			digitalWrite(29, 0);
			digitalWrite(28, 1);
			digitalWrite(23, 1);
			digitalWrite(22, 0);
			digitalWrite(21, 1);
			digitalWrite(27, 1);
			digitalWrite(26, 1);
		}
	}
	return 0;
}
