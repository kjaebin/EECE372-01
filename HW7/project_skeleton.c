#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize2.h"

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

#define I1_C 1  // Input layer channels
#define I1_H 28 // Input layer height
#define I1_W 28 // Input layer width

#define I2_C 16 // First conv layer output channels
#define I2_H 14 // First conv layer output height
#define I2_W 14 // First conv layer output width

#define I3_C 32 // Second conv layer output channels
#define I3_H 7  // Second conv layer output height
#define I3_W 7  // Second conv layer output width

#define CONV1_KERNAL 3  // First conv layer kernel size
#define CONV1_STRIDE 2  // First conv layer stride

#define CONV2_KERNAL 3  // Second conv layer kernel size
#define CONV2_STRIDE 2  // Second conv layer stride

#define CLASS 10 // Number of classes

typedef struct {
    float conv1_weight[I2_C * I1_C * CONV1_KERNAL * CONV1_KERNAL];
    float conv1_bias[I2_C];
    float conv2_weight[I3_C * I2_C * CONV2_KERNAL * CONV2_KERNAL];
    float conv2_bias[I3_C];
    float fc_weight[CLASS * I3_C * I3_H * I3_W];
    float fc_bias[CLASS];
} model;

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
                out[y][x] = 0;
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

void Padding(float *feature_in, float *feature_out, int C, int H, int W) {
    for (int c = 0; c < C; c++) {
        for (int h = 0; h < H + 2; h++) {
            for (int w = 0; w < W + 2; w++) {
                if (h == 0 || h == H + 1 || w == 0 || w == W + 1) {
                    feature_out[c * (H + 2) * (W + 2) + h * (W + 2) + w] = 0;
                } else {
                    feature_out[c * (H + 2) * (W + 2) + h * (W + 2) + w] = feature_in[c * H * W + (h - 1) * W + (w - 1)];
                }
            }
        }
    }
}

void Conv_2d(float *feature_in, float *feature_out, int in_C, int in_H, int in_W, int out_C, int out_H, int out_W, int K, int S, float *weight, float *bias) {
    for (int oc = 0; oc < out_C; oc++) {
        for (int oh = 0; oh < out_H; oh++) {
            for (int ow = 0; ow < out_W; ow++) {
                float sum = 0;
                for (int ic = 0; ic < in_C; ic++) {
                    for (int kh = 0; kh < K; kh++) {
                        for (int kw = 0; kw < K; kw++) {
                            int ih = oh * S + kh;
                            int iw = ow * S + kw;
                            sum += feature_in[ic * in_H * in_W + ih * in_W + iw] * weight[oc * in_C * K * K + ic * K * K + kh * K + kw];
                        }
                    }
                }
                feature_out[oc * out_H * out_W + oh * out_W + ow] = sum + bias[oc];
            }
        }
    }
}

void ReLU(float *feature_in, int elem_num) {
    for (int i = 0; i < elem_num; i++) {
        if (feature_in[i] < 0) {
            feature_in[i] = 0;
        }
    }
}

void Linear(float *feature_in, float *feature_out, float *weight, float *bias) {
    for (int i = 0; i < CLASS; i++) {
        float sum = 0;
        for (int j = 0; j < I3_C * I3_H * I3_W; j++) {
            sum += feature_in[j] * weight[i * I3_C * I3_H * I3_W + j];
        }
        feature_out[i] = sum + bias[i];
    }
}

int Get_pred(float *activation) {
    int pred = 0;
    float max_val = activation[0];
    for (int i = 1; i < CLASS; i++) {
        if (activation[i] > max_val) {
            max_val = activation[i];
            pred = i;
        }
    }
    return pred;
}

void Get_CAM(float *activation, float *cam, int pred, float *weight) {
    for (int i = 0; i < I3_H * I3_W; i++) {
        cam[i] = 0;
        for (int j = 0; j < I3_C; j++) {
            cam[i] += activation[j * I3_H * I3_W + i] * weight[pred * I3_C * I3_H * I3_W + j * I3_H * I3_W + i];
        }
    }
}

void Log_softmax(float *fc_out) {
    float max = fc_out[0];
    for (int i = 1; i < CLASS; i++) {
        if (fc_out[i] > max) {
            max = fc_out[i];
        }
    }
    float sum = 0.0f;
    for (int i = 0; i < CLASS; i++) {
        fc_out[i] = exp(fc_out[i] - max);
        sum += fc_out[i];
    }
    for (int i = 0; i < CLASS; i++) {
        fc_out[i] = log(fc_out[i] / sum);
    }
}

void save_image(float *feature_scaled, float *cam) {
    unsigned char output[28 * 28 * 3];
    for (int i = 0; i < 28 * 28; i++) {
        int val = (int)(cam[i] * 255);
        if (val < 0) val = 0;
        if (val > 255) val = 255;
        output[3 * i] = val;
        output[3 * i + 1] = val;
        output[3 * i + 2] = val;
    }
    stbi_write_bmp("output.bmp", 28, 28, 3, output);
}

int main(int argc, char *argv[]) {
    clock_t start1, end1, start2, end2;

    model net;
    FILE *weights;
    weights = fopen("./weights.bin", "rb");
    fread(&net, sizeof(model), 1, weights);

    char *file;
    if (atoi(argv[1]) == 0) {
        // 카메라 모드: libcamera-still 명령어를 사용하여 BMP 이미지 캡처
        system("libcamera-still -e bmp --width 280 --height 280 -t 20000 -o image.bmp");
        file = "image.bmp";
    } else if (atoi(argv[1]) == 1) {
        file = "example_1.bmp";
    } else if (atoi(argv[1]) == 2) {
        file = "example_2.bmp";
    } else {
        printf("Wrong Input!\n");
        exit(1);
    }

    unsigned char *feature_in;
    unsigned char *feature_resize;
    unsigned char feature_gray[I1_C * I1_H * I1_W];
    float feature_scaled[I1_C * I1_H * I1_W];
    float feature_padding1[I1_C * (I1_H + 2) * (I1_W + 2)];
    float feature_conv1_out[I2_C * I2_H * I2_W];
    float feature_padding2[I2_C * (I2_H + 2) * (I2_W + 2)];
    float feature_conv2_out[I3_C * I3_H * I3_W];
    float fc_out[1 * CLASS];
    float cam[1 * I3_H * I3_W];
    int channels, height, width;

    if (atoi(argv[1]) == 0) {
        feature_resize = stbi_load(file, &width, &height, &channels, 3);
        feature_in = (unsigned char *)malloc(sizeof(unsigned char) * 3 * I1_H * I1_W);
        resize_280_to_28(feature_resize, feature_in);
    } else {
        feature_in = stbi_load(file, &width, &height, &channels, 3);
    }

    int pred = 0;
    Gray_scale(feature_in, feature_gray);
    Normalized(feature_gray, feature_scaled);

    // 신경망 추론 수행
    start1 = clock();
    Padding(feature_scaled, feature_padding1, I1_C, I1_H, I1_W);
    Conv_2d(feature_padding1, feature_conv1_out, I1_C, I1_H + 2, I1_W + 2, I2_C, I2_H, I2_W, CONV1_KERNAL, CONV1_STRIDE, net.conv1_weight, net.conv1_bias);
    ReLU(feature_conv1_out, I2_C * I2_H * I2_W);

    Padding(feature_conv1_out, feature_padding2, I2_C, I2_H, I2_W);
    Conv_2d(feature_padding2, feature_conv2_out, I2_C, I2_H + 2, I2_W + 2, I3_C, I3_H, I3_W, CONV2_KERNAL, CONV2_STRIDE, net.conv2_weight, net.conv2_bias);
    ReLU(feature_conv2_out, I3_C * I3_H * I3_W);

    Linear(feature_conv2_out, fc_out, net.fc_weight, net.fc_bias);
    end1 = clock() - start1;

    Log_softmax(fc_out);

    start2 = clock();
    pred = Get_pred(fc_out);
    Get_CAM(feature_conv2_out, cam, pred, net.fc_weight);
    end2 = clock() - start2;

    save_image(feature_scaled, cam);

    // 7-세그먼트 디스플레이
    if (wiringPiSetup() == -1) {
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

    switch (pred) {
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
    }

    printf("Log softmax value\n");
    for (int i = 0; i < CLASS; i++) {
        printf("%2d: %6.3f\n", i, fc_out[i]);
    }
    printf("Prediction: %d\n", pred);
    printf("Execution time: %9.3lf[us]\n", (double)(end1 + end2) / CLOCKS_PER_SEC);

    fclose(weights);
    if (atoi(argv[1]) == 0) {
        free(feature_in);
        stbi_image_free(feature_resize);
    } else {
        stbi_image_free(feature_in);
    }
    return 0;
}
