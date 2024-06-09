#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <wiringPi.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>

#define MNIST_IMAGE_WIDTH 28
#define MNIST_IMAGE_HEIGHT 28
#define MNIST_IMAGE_SIZE MNIST_IMAGE_WIDTH * MNIST_IMAGE_HEIGHT
#define MNIST_LABELS 10
#define CHANNEL_NUMBER 32
#define CHANNEL_WIDTH 3
#define CHANNEL_HEIGHT 3
#define PIXEL_SCALE(x) (((float) (x)) / 255.0f)
#define g 24
#define f 23
#define a 22
#define b 21
#define e 29
#define d 25
#define c_ 28
#define DP 27
#define BAUDRATE B1000000

typedef struct neural_network_t_ {
    float conv_bias[CHANNEL_NUMBER];
    float conv_weight[CHANNEL_NUMBER][CHANNEL_HEIGHT][CHANNEL_WIDTH];
    float fc_bias[MNIST_LABELS];
    float fc_weight[MNIST_LABELS][MNIST_IMAGE_WIDTH * MNIST_IMAGE_HEIGHT * CHANNEL_NUMBER];
} neural_network_t;


void resize_280_to_28(unsigned char* out, unsigned char* in) {
    int x, y, c;

    for (y = 0; y < 28; y++) {
        for (x = 0; x < 28; x++) {
            for (c = 0; c < 3; c++) {
                out[y * 28 * 3 + x * 3 + c] = in[y * 10 * 280 * 3 + x * 10 * 3 + c];
            }
        }
    }
}


void RGB_to_Grayscale(uint8_t out[][MNIST_IMAGE_WIDTH], unsigned char* in) {
    int x, y, c;
    int sum = 0;

    for (y = 0; y < 28; y++) {
        for (x = 0; x < 28; x++) {
            sum = 0;
            for (c = 0; c < 3; c++) {
                sum += in[y * 28 * 3 + x * 3 + c];
            }
            for (c = 0; c < 3; c++) {
                in[y * 28 * 3 + x * 3 + c] = 255 - sum / 3;
            }

        }
    }

    for (y = 0; y < 28; y++) {
        for (x = 0; x < 28; x++) {
            sum = 0;
            for (c = 0; c < 3; c++) {
                sum += in[y * 28 * 3 + x * 3 + c];
            }
            if (sum / 3 < 150)
                out[y][x];
            else
                out[y][x] = sum / 3;
        }
    }
}


void pixel_scale(float out[][MNIST_IMAGE_WIDTH], uint8_t in[][MNIST_IMAGE_WIDTH]) {
    int i;

    for (i = 0; i < 28 * 28; i++) {
        out[i / 28][i % 28] = PIXEL_SCALE(in[i / 28][i % 28]);
    }
}


void zero_padding(float out[][30], float in[][28]) {
    for (int i = 0; i < 30; i++) {
        for (int j = 0; j < 30; j++) {
            if (i == 0 || i == 29) {
                out[i][j] = 0;
            }
            else if (j == 0 || j == 29) {
                out[i][j] = 0;
            }
            else {
                out[i][j] = in[i - 1][j - 1];
            }
        }
    }
}


void convolution(float out[][28][28], float in[][30], neural_network_t network) {
    for (int m = 0; m < CHANNEL_NUMBER; m++) {
        for (int i = 0; i < 28; i++) {
            for (int j = 0; j < 28; j++) {
                out[m][i][j] = 0;
            }
        }
    }

    for (int m = 0; m < CHANNEL_NUMBER; m++) {
        for (int i = 0; i < 28; i++) {
            for (int j = 0; j < 28; j++) {
                for (int k = 0; k < CHANNEL_HEIGHT; k++) {
                    for (int l = 0; l < CHANNEL_WIDTH; l++) {
                        out[m][i][j] += in[i + k][j + l] * network.conv_weight[m][k][l];
                    }
                }
                out[m][i][j] += network.conv_bias[m];
            }
        }
    }
}


void relu(float out[], float in[][MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH]) {
    int index;
    for (int i = 0; i < CHANNEL_NUMBER; i++) {
        for (int j = 0; j < MNIST_IMAGE_HEIGHT; j++) {
            for (int k = 0; k < MNIST_IMAGE_WIDTH; k++) {
                index = k + j * MNIST_IMAGE_WIDTH + i * MNIST_IMAGE_SIZE;
                if (in[i][j][k] > 0)
                    out[index] = in[i][j][k];
                else
                    out[index] = 0;
            }
        }
    }

}


void fc(float out[], float in[], neural_network_t network) {
    float partial_sum = 0, sum = 0;

    for (int i = 0; i < MNIST_LABELS; i++) {
        for (int j = 0; j < MNIST_IMAGE_WIDTH * MNIST_IMAGE_HEIGHT * CHANNEL_NUMBER; j++) {
            partial_sum += in[j] * network.fc_weight[i][j];
        }
        sum = partial_sum + network.fc_bias[i];
        out[i] = sum;
    }

}


void softmax(float activations[], int length) {
    int i;
    float sum, max;

    for (i = 1, max = activations[0]; i < length; i++) {
        if (activations[i] > max) {
            max = activations[i];
        }
    }

    for (i = 0, sum = 0; i < length; i++) {
        activations[i] = exp(activations[i] - max);
        sum += activations[i];
    }

    for (i = 0; i < length; i++) {
        activations[i] /= sum;
    }
}


int main(int argc, char* argv[])
{

    int height;
    int width;
    int channels;
    int x, y, c, i, j;
    int sum;
    int mode = atoi(argv[1]);
    clock_t start, end;

    neural_network_t network;

    FILE* weight;
    weight = fopen("./weight.bin", "rb");
    fread(&network, sizeof(neural_network_t), 1, weight);

    int fd;            //serial 시작
    struct termios newtio;
    char fbuf[1024];
    char buf[256];

    fd = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr, "failed to open port: %s.\r\n", strerror(errno));
        printf("Make sure you are executing in sudo.\r\n");
    }

    memset(&newtio, 0, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = ICRNL;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);



    /*          PUT YOUR CODE HERE                      */
    if (mode == 0) {
        while (1) {
            int cnt = read(fd, buf, 256);
            if (cnt > 0) {
                buf[cnt] = '\0';
                printf("Received: %s\r\n", buf);
                if (strcmp(buf, "c") == 0 || strcmp(buf, "C") == 0) {
                    system("raspistill -w 280 -h 280 -t 1 -o test.bmp");
                    FILE* file = fopen("test.bmp", "rb");
                    FILE* file2 = fopen("project.bmp", "wb");
                    while (!feof(file)) {
                        int cntbp = fread(fbuf, 1, 1024, file);
                        fwrite(fbuf, 1, cntbp, file2);
                    }
                  
                    fclose(file);
                    fclose(file2);
                    break;
                }
            }
        }
    }
    else if (mode == 1) {
        
            FILE* file = fopen("example_1.bmp", "rb");
            FILE* file2 = fopen("project.bmp", "wb");
            while (!feof(file)) {
                int cntbp = fread(fbuf, 1, 1024, file);
                fwrite(fbuf, 1, cntbp, file2);
            }
            fclose(file);
            fclose(file2);
        
    }
    else if (mode == 2) {
        
            FILE* file = fopen("example_2.bmp", "rb");
            FILE* file2 = fopen("project.bmp", "wb");
            while (!feof(file)) {
                int cntbp = fread(fbuf, 1, 1024, file);
                fwrite(fbuf, 1, cntbp, file2);
            }
            fclose(file);
            fclose(file2);
       
    }


    /*          Capture image(project.bmp)              */


    unsigned char* feature_in = stbi_load("project.bmp", &width, &height, &channels, 3);
    unsigned char* feature_resized = (unsigned char*)malloc(sizeof(unsigned char) * 28 * 28 * 3);
    unsigned char feature_gray[MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH];
    float feature_scaled[MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH];
    float feature_zeroPadding[30][30];
    float feature_conv[CHANNEL_NUMBER][MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH];
    float feature_relu[MNIST_IMAGE_SIZE * CHANNEL_NUMBER];
    float activations[MNIST_LABELS];

    resize_280_to_28(feature_resized, feature_in);
    RGB_to_Grayscale(feature_gray, feature_resized);
    pixel_scale(feature_scaled, feature_gray);

    /***************    Implement these functions       ********************/
    start = clock();
    zero_padding(feature_zeroPadding, feature_scaled);
    convolution(feature_conv, feature_zeroPadding, network);
    relu(feature_relu, feature_conv);
    fc(activations, feature_relu, network);
    printf("Execution time: %.3lf ms\n", (double)clock() - start);
    /***********************************************************************/

    softmax(activations, MNIST_LABELS);

    int maxi = 0;
    float maxact = 0;
    printf("\n");
    printf("softmax value\n");
    for (int i = 0; i < 10; i++) {
        printf("%d : %f\n", i, activations[i]);
        if (activations[i] > maxact) {
            maxi = i;
            maxact = activations[i];
        }
    }

    /*          PUT YOUR CODE HERE                      */

    if (wiringPiSetup() == -1) { //library include 실패시 종료
        return 1;
    }

    pinMode(g, OUTPUT);
    pinMode(f, OUTPUT);
    pinMode(a, OUTPUT);
    pinMode(b, OUTPUT);
    pinMode(e, OUTPUT);
    pinMode(d, OUTPUT);
    pinMode(c_, OUTPUT);
    pinMode(DP, OUTPUT);

    digitalWrite(a, 0);
    digitalWrite(b, 0);
    digitalWrite(c_, 0);
    digitalWrite(d, 0);
    digitalWrite(e, 0);
    digitalWrite(f, 0);
    digitalWrite(g, 0);
    digitalWrite(DP, 0);

    switch (maxi) {
    case 0:
        digitalWrite(a, 1);
        digitalWrite(b, 1);
        digitalWrite(c_, 1);
        digitalWrite(d, 1);
        digitalWrite(e, 1);
        digitalWrite(f, 1);
        digitalWrite(g, 0);
        digitalWrite(DP, 0);
        break;
    case 1:
        digitalWrite(a, 0);
        digitalWrite(b, 1);
        digitalWrite(c_, 1);
        digitalWrite(d, 0);
        digitalWrite(e, 0);
        digitalWrite(f, 0);
        digitalWrite(g, 0);
        digitalWrite(DP, 0);
        break;
    case 2:
        digitalWrite(a, 1);
        digitalWrite(b, 1);
        digitalWrite(c_, 0);
        digitalWrite(d, 1);
        digitalWrite(e, 1);
        digitalWrite(f, 0);
        digitalWrite(g, 1);
        digitalWrite(DP, 0);
        break;
    case 3:
        digitalWrite(a, 1);
        digitalWrite(b, 1);
        digitalWrite(c_, 1);
        digitalWrite(d, 1);
        digitalWrite(e, 0);
        digitalWrite(f, 0);
        digitalWrite(g, 1);
        digitalWrite(DP, 0);
        break;
    case 4:
        digitalWrite(a, 0);
        digitalWrite(b, 1);
        digitalWrite(c_, 1);
        digitalWrite(d, 0);
        digitalWrite(e, 0);
        digitalWrite(f, 1);
        digitalWrite(g, 1);
        digitalWrite(DP, 0);
        break;
    case 5:
        digitalWrite(a, 1);
        digitalWrite(b, 0);
        digitalWrite(c_, 1);
        digitalWrite(d, 1);
        digitalWrite(e, 0);
        digitalWrite(f, 1);
        digitalWrite(g, 1);
        digitalWrite(DP, 0);
        break;
    case 6:
        digitalWrite(a, 1);
        digitalWrite(b, 0);
        digitalWrite(c_, 1);
        digitalWrite(d, 1);
        digitalWrite(e, 1);
        digitalWrite(f, 1);
        digitalWrite(g, 1);
        digitalWrite(DP, 0);
        break;
    case 7:
        digitalWrite(a, 1);
        digitalWrite(b, 1);
        digitalWrite(c_, 1);
        digitalWrite(d, 0);
        digitalWrite(e, 0);
        digitalWrite(f, 1);
        digitalWrite(g, 0);
        digitalWrite(DP, 0);
        break;
    case 8:
        digitalWrite(a, 1);
        digitalWrite(b, 1);
        digitalWrite(c_, 1);
        digitalWrite(d, 1);
        digitalWrite(e, 1);
        digitalWrite(f, 1);
        digitalWrite(g, 1);
        digitalWrite(DP, 0);
        break;
    case 9:
        digitalWrite(a, 1);
        digitalWrite(b, 1);
        digitalWrite(c_, 1);
        digitalWrite(d, 1);
        digitalWrite(e, 0);
        digitalWrite(f, 1);
        digitalWrite(g, 1);
        digitalWrite(DP, 0);
        break;
        /*          7-segment                               */


        fclose(weight);
        stbi_image_free(feature_in);
        free(feature_resized);

        return 0;
    }
}
