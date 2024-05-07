#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define SWITCH_PIN 25  // 입력 스위치에 연결된 GPIO 핀
int pin_num[] = { 29, 28, 23, 22, 21, 27, 26 };

int main(){
    int row = 0;
    int btn_state = 0;
    int sev_seg[16][8]={
   {1,1,1,1,1,1,0,0},
   {0,1,1,0,0,0,0,0},
   {1,1,0,1,1,0,1,0},
   {1,1,1,1,0,0,1,0},
   {0,1,1,0,0,1,1,0},
   {1,0,1,1,0,1,1,0},
   {1,0,1,1,1,1,1,0},
   {1,1,1,0,0,1,0,0},
   {1,1,1,1,1,1,1,0},
   {1,1,1,1,0,1,1,0},
   {1,1,1,0,1,1,1,0},
   {0,0,1,1,1,1,1,0},
   {0,0,0,1,1,0,1,0},
   {0,1,1,1,1,0,1,0},
   {1,0,0,1,1,1,1,0},
   {1,0,0,0,1,1,1,0}
    };
    if (wiringPiSetup() == -1){
   return 1;
    }
   
    pinMode(SWITCH_PIN, INPUT);
   for(int i = 0; i < 8; i++){
   pinMode(SEGMENT_PINS[i], OUTPUT);
   }
   
    while(1){
   if(btn_state == 0) {
      if(digitalRead(SWITCH_PIN) == 1){
         row++;
         row = row % 16;
         btn_state = 1;
      }
   }
   else if(btn_state == 1){
      if(digitalRead(SWITCH_PIN) == 0){
         btn_state = 0;
      }
   }
   for(int i = 0; i < 8; i++){
          digitalWrite(SEGMENT_PINS[i], sev_seg[row][i]);
   }
   delay(100);
    }
}
