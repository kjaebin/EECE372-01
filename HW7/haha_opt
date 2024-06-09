#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
#include <arm_neon.h>
#include <omp.h>

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
	
	float32x4_t zero_vector = vdupq_n_f32(0.0f);
	for (int i = 0 ; i <8 ; i ++){
		vst1q_f32(&out[0][i*4], zero_vector);
	}
	for (int i = 1; i < 29; i++) {
		out[i][0] = 0.0f;
		for (int j=0 ; j < 28 ; j+=4){
			float32x4_t neon_in = vld1q_f32(&in[i-1][j]);
			vst1q_f32(&out[i][j+1], neon_in);
		}
		out[i][29] = 0.0f;
	}
	
	for (int i = 0 ; i <8 ; i ++){
		vst1q_f32(&out[29][i*4], zero_vector);
	}  
}


void convolution(float out[][28][28], float in[][30], neural_network_t network) {
		
	float32x4_t zero_vector = vdupq_n_f32(0.0f);
	
    for (int m = 0; m < CHANNEL_NUMBER; m++) {
        for (int i = 0; i < 28; i++) {
			float32_t* out_ptr = &out[m][i][0];
            for (int j = 0; j < 28; j+=4){
                vst1q_f32(out_ptr + j, zero_vector);
            }
        }
    }
	
    for (int m = 0; m < CHANNEL_NUMBER; m++) {
        for (int i = 0; i < 28; i++) {
			float32_t* out_ptr = &out[m][i][0];
			float32_t* in_ptr = &in[i][0];
			for (int j=0 ; j <28; j++){
				float32x4_t partial_sum = zero_vector;
                for (int k = 0; k < CHANNEL_HEIGHT; k++) {
					float32_t* conv_weight_ptr = &network.conv_weight[m][k][0];
					float32x4_t conv_weight = vld1q_f32(conv_weight_ptr);
					float32x4_t in_value = vld1q_f32(in_ptr+30*k + j); 
                    partial_sum = vmlaq_f32(partial_sum, in_value, conv_weight);
                }
                float32_t result = vgetq_lane_f32(partial_sum, 0) + vgetq_lane_f32(partial_sum, 1) + vgetq_lane_f32(partial_sum, 2);
                *(out_ptr+j) += result;
                *(out_ptr+j) += network.conv_bias[m];
            }
		}
		}
		
    }


void relu(float out[], float in[][MNIST_IMAGE_HEIGHT][MNIST_IMAGE_WIDTH]) {
    int index;
    float32x4_t zero_vector = vdupq_n_f32(0.0f);
    
    for (int i = 0; i < CHANNEL_NUMBER; i++) {
        for (int j = 0; j < MNIST_IMAGE_HEIGHT; j++) {
            for (int k = 0; k < MNIST_IMAGE_WIDTH; k+=4) {
                float32x4_t in_vector = vld1q_f32(&in[i][j][k]);
                uint32x4_t condition = vcltq_f32(zero_vector,in_vector);
                
                float32x4_t result = vbslq_f32(condition, in_vector, zero_vector);
                vst1q_f32(&out[k + j * MNIST_IMAGE_WIDTH + i * MNIST_IMAGE_SIZE], result);
            }
        }
    }
}

void fc(float out[], float in[], neural_network_t network) {
    float32x4_t partial_sum = vdupq_n_f32(0.0f);

    for (int i = 0; i < MNIST_LABELS; i++) {
        partial_sum = vdupq_n_f32(0.0f);

        for (int j = 0; j < MNIST_IMAGE_WIDTH * MNIST_IMAGE_HEIGHT * CHANNEL_NUMBER; j += 4) {
            float32x4_t in_vector = vld1q_f32(&in[j]);
            float32x4_t fc_weight_vector = vld1q_f32(&network.fc_weight[i][j]);
            partial_sum = vmlaq_f32(partial_sum, in_vector, fc_weight_vector);
        }
        float32x2_t sum = vadd_f32(vget_low_f32(partial_sum), vget_high_f32(partial_sum));
        sum = vpadd_f32(sum, sum);
        float32_t result = vget_lane_f32(sum, 0);
        out[i] = result + network.fc_bias[i];
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
    clock_t start1, start2, start3, start4, end;

    neural_network_t network;

    FILE* weight;
    weight = fopen("./weight.bin", "rb");
    fread(&network, sizeof(neural_network_t), 1, weight);

    int fd;            //serial ½ÃÀÛ
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
    start1 = clock();
    zero_padding(feature_zeroPadding, feature_scaled);
    //start2 = clock();
    convolution(feature_conv, feature_zeroPadding, network);
    //start3 = clock();
    relu(feature_relu, feature_conv);
    //start4 = clock();
    fc(activations, feature_relu, network);
    end = clock();
    //printf("Execution time: %.3lf ms\n", (double)start2 - start1);
    //printf("Execution time: %.3lf ms\n", (double)start3 - start2);
    //printf("Execution time: %.3lf ms\n", (double)start4 - start3);
    //printf("Execution time: %.3lf ms\n", (double)end - start4);
    printf("Execution time: %.3lf ms\n", (double)end - start1);
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

    if (wiringPiSetup() == -1) { //library include ½ÇÆÐ½Ã Á¾·á
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
