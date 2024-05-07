#include <stdio.h>
#include <wiringPi.h>

#define SWITCH_PIN 25
#define SEGMENT_A 29
#define SEGMENT_B 28
#define SEGMENT_C 23
#define SEGMENT_D 22
#define SEGMENT_E 21
#define SEGMENT_F 27
#define SEGMENT_G 26

int main(void) {

    int count = 0;
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
    int pin_num[] = {SEGMENT_A, SEGMENT_B, SEGMENT_C, SEGMENT_D, SEGMENT_E, SEGMENT_F, SEGMENT_G};

    wiringPiSetup();
    pinMode(SWITCH_PIN, INPUT);
    pullUpDnControl(SWITCH_PIN, PUD_DOWN); // Set pull-down resistor as the switch is connected to 3.3V

    for (i = 0; i < PIN_COUNT; i++) {
        pinMode(pin_num[i], OUTPUT);
    };

    int lastButtonState = LOW;  // the previous reading from the input pin
    int buttonState;            // the current reading from the input pin
    unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
    unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

    while (1) {
        int reading = digitalRead(SWITCH_PIN);

        if (reading != lastButtonState) {
            lastDebounceTime = millis();
        }

        if ((millis() - lastDebounceTime) > debounceDelay) {
            // if the button state has changed:
            if (reading != buttonState) {
                buttonState = reading;

                // only toggle the count if the new button state is HIGH
                if (buttonState == HIGH) {
                    count++;
                    if (count >= 16) count = 0;
                    for (i = 0; i < PIN_COUNT; i++) {
                        digitalWrite(pin_num[i], hex_table[count][i]);
                    }
                }
            }
        }

        lastButtonState = reading;

        delay(100); // Small delay to reduce CPU usage
    }
    return 0;
}
