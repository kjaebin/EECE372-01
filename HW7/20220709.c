#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>

#include "stb_image.h"
#include "stb_image_resize2.h"
#include "stb_image_write.h"
#include <wiringPi.h>

#define BAUDRATE B1000000

#define CLOCKS_PER_US ((double)CLOCKS_PER_SEC / 1000000)

#define CLASS 10

// Input dim
#define I1_C 1
#define I1_H 28
#define I1_W 28

// Conv1 out dim
#define I2_C 16
#define I2_H 14
#define I2_W 14

// Convw out dim
#define I3_C 1
#define I3_H 14
#define I3_W 14

#define CONV1_KERNAL 3
#define CONV1_STRIDE 2
#define CONV2_KERNAL 3
#define CONV2_STRIDE 1
#define FC_IN (I2_H * I2_W)
#define FC_OUT CLASS

typedef struct _model {
    float conv1_weight[I2_C * I1_C * CONV1_KERNAL * CONV1_KERNAL];
    float conv1_bias[I2_C];

    float conv2_weight[I3_C * I2_C * CONV2_KERNAL * CONV2_KERNAL];
    float conv2_bias[I3_C];

    float fc_weight[FC_OUT * FC_IN];
    float fc_bias[FC_OUT];
} model;

int pin_num[] = { 29, 28, 23, 22, 21, 27, 26 };

void resize_280_to_28(unsigned char* in, unsigned char* out);
void Gray_scale(unsigned char* feature_in, unsigned char* feature_out);
void Normalized(unsigned char* feature_in, float* feature_out);

void Padding(float* feature_in, float* feature_out, int C, int H, int W);
void Conv_2d(float* feature_in, float* feature_out, int in_C, int in_H, int in_W, int out_C, int out_H, int out_W, int K, int S, float* weight, float* bias);
void ReLU(float* feature_in, int elem_num);
void Linear(float* feature_in, float* feature_out, float* weight, float* bias);
void Log_softmax(float* activation);
int Get_pred(float* activation);
void Get_CAM(float* activation, float* cam, int pred, float* weight);
void save_image(float* feature_scaled, float* cam);

void setup_gpio();
void display_number(int number);

void cheeze(const char* filename) {
    char command[256];
    snprintf(command, sizeof(command), "libcamera-still -o %s", filename);
    system(command);
}

int main(int argc, char* argv[]) {
    clock_t start1, end1, start2, end2;
    clock_t start_padding, end_padding, start_conv1, end_conv1, start_relu1, end_relu1;
    clock_t start_conv2, end_conv2, start_relu2, end_relu2, start_fc, end_fc;

    model net;
    FILE* weights;
    weights = fopen("./weights.bin", "rb");
    if (weights == NULL) {
        printf("Error opening weights file.\n");
        return -1;
    }
    fread(&net, sizeof(model), 1, weights);
    fclose(weights);

    char* file;
    if (atoi(argv[1]) == 0) {
        /* Camera mode with serial communication */
        int fd;
        struct termios newtio;
        char buf[256];
        char nameofcheezeee[] = "image.bmp";

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

        printf("Waiting for 'c' or 'C' command to take a picture...\n");
        while (1) {
            int n = read(fd, buf, sizeof(buf));
            if (n > 0) {
                buf[n] = '\0';
                if (buf[0] == 'c' || buf[0] == 'C') {
                    cheeze(nameofcheezeee);
                    file = nameofcheezeee;
                    break;
                }
            }
        }
        close(fd);
    }
    else if (atoi(argv[1]) == 1) {
        file = "example_1.bmp";
    }
    else if (atoi(argv[1]) == 2) {
        file = "example_2.bmp";
    }
    //
    else if (atoi(argv[1]) == 3) {
        file = "example_3.bmp";
    }
    else if (atoi(argv[1]) == 4) {
        file = "example_4.bmp";
    }
    else if (atoi(argv[1]) == 5) {
        file = "test.bmp";
    }
    //
    else {
        printf("Wrong Input!\n");
        exit(1);
    }

    unsigned char* feature_in;
    unsigned char* feature_resize;
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
        feature_in = (unsigned char*)malloc(sizeof(unsigned char) * 3 * I1_H * I1_W);
        resize_280_to_28(feature_resize, feature_in);
    }
    else {
        feature_in = stbi_load(file, &width, &height, &channels, 3);
    }

    int pred = 0;
    Gray_scale(feature_in, feature_gray);
    Normalized(feature_gray, feature_scaled);
    /***************      Implement these functions      ********************/
    start1 = clock();
    start_padding = clock();
    Padding(feature_scaled, feature_padding1, I1_C, I1_H, I1_W);
    end_padding = clock();

    start_conv1 = clock();
    Conv_2d(feature_padding1, feature_conv1_out, I1_C, I1_H + 2, I1_W + 2, I2_C, I2_H, I2_W, CONV1_KERNAL, CONV1_STRIDE, net.conv1_weight, net.conv1_bias);
    end_conv1 = clock();

    start_relu1 = clock();
    ReLU(feature_conv1_out, I2_C * I2_H * I2_W);
    end_relu1 = clock();

    start_conv2 = clock();
    Padding(feature_conv1_out, feature_padding2, I2_C, I2_H, I2_W);
    Conv_2d(feature_padding2, feature_conv2_out, I2_C, I2_H + 2, I2_W + 2, I3_C, I3_H, I3_W, CONV2_KERNAL, CONV2_STRIDE, net.conv2_weight, net.conv2_bias);
    end_conv2 = clock();

    start_relu2 = clock();
    ReLU(feature_conv2_out, I3_C * I3_H * I3_W);
    end_relu2 = clock();

    start_fc = clock();
    Linear(feature_conv2_out, fc_out, net.fc_weight, net.fc_bias);
    end_fc = clock();
    end1 = clock() - start1;

    Log_softmax(fc_out);

    start2 = clock();
    pred = Get_pred(fc_out);
    Get_CAM(feature_conv2_out, cam, pred, net.fc_weight);
    end2 = clock() - start2;
    /************************************************************************/
    save_image(feature_scaled, cam);

    setup_gpio();
    display_number(pred);

 /* printf("Zero Padding time: %9.3lf[us]\n", (double)(end_padding - start_padding) / CLOCKS_PER_US);
    printf("Conv1 time: %9.3lf[us]\n", (double)(end_conv1 - start_conv1) / CLOCKS_PER_US);
    printf("Conv2 time: %9.3lf[us]\n", (double)(end_conv2 - start_conv2) / CLOCKS_PER_US);
    printf("ReLU1 time: %9.3lf[us]\n", (double)(end_relu1 - start_relu1) / CLOCKS_PER_US);
    printf("ReLU2 time: %9.3lf[us]\n", (double)(end_relu2 - start_relu2) / CLOCKS_PER_US);
    printf("FC time: %9.3lf[us]\n", (double)(end_fc - start_fc) / CLOCKS_PER_US);
    printf("Total time (excluding Softmax): %9.3lf[us]\n", (double)end1 / CLOCKS_PER_US);
    printf("CAM time: %9.3lf[us]\n", (double)end2 / CLOCKS_PER_US);
    printf("Total time (including CAM): %9.3lf[us]\n", (double)(end1 + end2) / CLOCKS_PER_US);
    printf("\n");  */

    printf("Log softmax value\n");
    for (int i = 0; i < CLASS; i++) {
        printf("%2d: %6.3f\n", i, fc_out[i]);
    }
    printf("Prediction: %d\n", pred);
    printf("Execution time: %9.3lf[us]\n", (double)(end1 + end2) / CLOCKS_PER_US);

    stbi_image_free(feature_in);
    return 0;
}

void resize_280_to_28(unsigned char* in, unsigned char* out) {
    /*            DO NOT MODIFIY            */
    int x, y, c;
    for (y = 0; y < 28; y++) {
        for (x = 0; x < 28; x++) {
            for (c = 0; c < 3; c++) {
                out[y * 28 * 3 + x * 3 + c] = in[y * 10 * 280 * 3 + x * 10 * 3 + c];
            }
        }
    }
    return;
}

void Gray_scale(unsigned char* feature_in, unsigned char* feature_out) {
    /*            DO NOT MODIFIY            */
    for (int h = 0; h < I1_H; h++) {
        for (int w = 0; w < I1_W; w++) {
            int sum = 0;
            for (int c = 0; c < 3; c++) {
                sum += feature_in[I1_H * 3 * h + 3 * w + c];
            }
            feature_out[I1_W * h + w] = sum / 3;
        }
    }

    return;
}

void Normalized(unsigned char* feature_in, float* feature_out) {
    /*            DO NOT MODIFIY            */
    for (int i = 0; i < I1_H * I1_W; i++) {
        feature_out[i] = ((float)feature_in[i]) / 255.0;
    }

    return;
}

void Padding(float* feature_in, float* feature_out, int C, int H, int W) {
    /*          PUT YOUR CODE HERE          */
    // Padding input : float *feature_in
    // Padding output: float *feature_out
    for (int c = 0; c < C; c++) {
        for (int h = 0; h < H + 2; h++) {
            for (int w = 0; w < W + 2; w++) {
                if (h == 0 || h == H + 1 || w == 0 || w == W + 1) {
                    feature_out[c * (H + 2) * (W + 2) + h * (W + 2) + w] = 0;
                }
                else {
                    feature_out[c * (H + 2) * (W + 2) + h * (W + 2) + w] = feature_in[c * H * W + (h - 1) * W + (w - 1)];
                }
            }
        }
    }
}

void Conv_2d(float* feature_in, float* feature_out, int in_C, int in_H, int in_W, int out_C, int out_H, int out_W, int K, int S, float* weight, float* bias) {
    /*          PUT YOUR CODE HERE          */
    // Conv_2d input : float *feature_in
    // Conv_2d output: float *feature_out
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

void ReLU(float* feature_in, int elem_num) {
    /*          PUT YOUR CODE HERE          */
    // ReLU input : float *feature_in
    // ReLU output: float *feature_in
    for (int i = 0; i < elem_num; i++) {
        if (feature_in[i] < 0) {
            feature_in[i] = 0;
        }
    }
}

void Linear(float* feature_in, float* feature_out, float* weight, float* bias) {
    /*          PUT YOUR CODE HERE          */
    // Linear input : float *feature_in
    // Linear output: float *feature_out
    for (int i = 0; i < CLASS; i++) {
        float sum = 0;
        for (int j = 0; j < I3_C * I3_H * I3_W; j++) {
            sum += feature_in[j] * weight[i * I3_C * I3_H * I3_W + j];
        }
        feature_out[i] = sum + bias[i];
    }
}

void Log_softmax(float* activation) {
    /*            DO NOT MODIFIY            */
    double max = activation[0];
    double sum = 0.0;

    for (int i = 1; i < CLASS; i++) {
        if (activation[i] > max) {
            max = activation[i];
        }
    }

    for (int i = 0; i < CLASS; i++) {
        activation[i] = exp(activation[i] - max);
        sum += activation[i];
    }

    for (int i = 0; i < CLASS; i++) {
        activation[i] = log(activation[i] / sum);
    }

    return;
}

int Get_pred(float* activation) {
    /*          PUT YOUR CODE HERE          */
    // Get_pred input : float *activation
    // Get_pred output: int pred
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

void Get_CAM(float* activation, float* cam, int pred, float* weight) {
    /*          PUT YOUR CODE HERE          */
    // Get_CAM input : float *activation
    // Get_CAM output: float *cam
    for (int i = 0; i < I3_H * I3_W; i++) {
        cam[i] = 0;
        for (int j = 0; j < I3_C; j++) {
            cam[i] += activation[i + j * I3_H * I3_W] * weight[pred * I3_C * I3_H * I3_W + i + j * I3_H * I3_W];
        }
    }
}

void save_image(float* feature_scaled, float* cam) {
    /*            DO NOT MODIFIY            */
    float* output = (float*)malloc(sizeof(float) * 3 * I1_H * I1_W);
    unsigned char* output_bmp = (unsigned char*)malloc(sizeof(unsigned char) * 3 * I1_H * I1_W);
    unsigned char* output_bmp_resized = (unsigned char*)malloc(sizeof(unsigned char) * 3 * I1_H * 14 * I1_W * 14);

    float min = cam[0];
    float max = cam[0];
    for (int i = 1; i < I3_H * I3_W; i++) {
        if (cam[i] < min) {
            min = cam[i];
        }
        if (cam[i] > max) {
            max = cam[i];
        }
    }

    for (int h = 0; h < I1_H; h++) {
        for (int w = 0; w < I1_W; w++) {
            for (int c = 0; c < 3; c++) {
                output[I1_H * I1_W * c + I1_W * h + w] = (cam[I3_W * (h >> 1) + (w >> 1)] - min) / (max - min);
            }
        }
    }

    for (int h = 0; h < I1_H; h++) {
        for (int w = 0; w < I1_W; w++) {
            for (int c = 0; c < 3; c++) {
                output_bmp[I1_H * 3 * h + 3 * w + c] = (output[I1_H * I1_W * c + I1_W * h + w]) * 255;
            }
        }
    }

    stbir_resize_uint8_linear(output_bmp, I1_H, I1_W, 0, output_bmp_resized, I1_H * 14, I1_W * 14, 0, 3);
    stbi_write_bmp("Activation_map.bmp", I1_W * 14, I1_H * 14, 3, output_bmp_resized);

    free(output);
    free(output_bmp);
    return;
}

void setup_gpio() {
    if (wiringPiSetup() == -1) {
        printf("GPIO setup failed\n");
        exit(1);
    }
    for (int i = 0; i < 7; i++) {
        pinMode(pin_num[i], OUTPUT);
    }
}

void display_number(int number) {
    int hex_table[10][7] = {
        {1, 1, 1, 1, 1, 1, 0}, // 0
        {0, 1, 1, 0, 0, 0, 0}, // 1
        {1, 1, 0, 1, 1, 0, 1}, // 2
        {1, 1, 1, 1, 0, 0, 1}, // 3
        {0, 1, 1, 0, 0, 1, 1}, // 4
        {1, 0, 1, 1, 0, 1, 1}, // 5
        {1, 0, 1, 1, 1, 1, 1}, // 6
        {1, 1, 1, 0, 0, 0, 0}, // 7
        {1, 1, 1, 1, 1, 1, 1}, // 8
        {1, 1, 1, 1, 0, 1, 1}  // 9
    };
    for (int i = 0; i < 7; i++) {
        digitalWrite(pin_num[i], hex_table[number][i]);
    }
}
