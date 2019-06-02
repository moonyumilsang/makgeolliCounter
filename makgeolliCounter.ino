#include <SPI.h>
#include "epd2in9b.h"
#include "epdpaint.h"
#include "imagedata.h"

#include <string.h>
#include <Wire.h>
#include <DS1307.h>
#include <TimeLib.h>

#include <EEPROM.h>

unsigned int differece = 0;

struct DataObject {
  int value[7];
};

int32_t oldHour = -1;
int32_t oldDay = -1;
Epd epd;

int RTCValues[7];

uint32_t getDiff() {  
  DataObject savedData;
  EEPROM.get(0, savedData);
  DS1307.getDate(RTCValues);
  tmElements_t T1;
  tmElements_t T2;

  T1.Year = savedData.value[0] - 1970;
  T1.Month = savedData.value[1];
  T1.Day = savedData.value[2];
  T1.Hour = savedData.value[4];
  T1.Minute = savedData.value[5];
  T1.Second = savedData.value[6];

  T2.Year = RTCValues[0] - 1970;
  T2.Month = RTCValues[1];
  T2.Day = RTCValues[2];
  T2.Hour = RTCValues[4];
  T2.Minute = RTCValues[5];
  T2.Second = RTCValues[6];

  time_t T1sec = makeTime(T1);
  time_t T2sec = makeTime(T2);

  int32_t diff = T2sec - T1sec;
  uint32_t hours = diff/3600;

  return hours;
}

void drawArray(int x, int y, int w, int h, const unsigned char *theArray, int arrayLength, int color = 0) {
  epd.SendCommand(PARTIAL_IN);
  epd.SendCommand(PARTIAL_WINDOW);
  epd.SendData(x & 0xf8);     // x should be the multiple of 8, the last 3 bit will always be ignored
  epd.SendData(((x & 0xf8) + w  - 1) | 0x07);
  epd.SendData(y >> 8);        
  epd.SendData(y & 0xff);
  epd.SendData((y + h - 1) >> 8);        
  epd.SendData((y + h - 1) & 0xff);
  epd.SendData(0x01);         // Gates scan both inside and outside of the partial window. (default) 
  epd.DelayMs(2);
  if (color == 0)
    epd.SendCommand(DATA_START_TRANSMISSION_1);
  else
    epd.SendCommand(DATA_START_TRANSMISSION_2);
  
  for (int i = 0; i < arrayLength; i++) {
    epd.SendData(pgm_read_byte(&theArray[i]));
  }
  epd.SendCommand(PARTIAL_OUT);
}

void drawNumber(int number) {
  int ten = number/10;
  int one = number%10;

  int offset = 194;
  
  if (ten > 0) {
    drawArray(0, 228, 128, 68, N[ten%10], sizeof(N[ten%10]));

    offset -= 34;
  }

  drawArray(0, offset, 128, 68, N[one], sizeof(N[one])); 
}

void drawHours(int hours) {
  epd.Init();
  epd.ClearFrame();
  drawArray(0, 0, 128, 160, HOUR, sizeof(HOUR));
  if (hours > 1)
    drawArray(96, 44, 32, 16, S, sizeof(S));

  drawNumber(hours);
  epd.DisplayFrame();
  epd.Sleep();
}

void drawDays(int days) {
  epd.Init();
  epd.ClearFrame();
  drawArray(0, 88, 80, 72, DAY_KJ, sizeof(DAY_KJ));
  if (days == 1)
    drawArray(80, 25, 48, 135, DAY_EN_ST, sizeof(DAY_EN_ST));
  else if (days == 2)
    drawArray(80, 25, 48, 135, DAY_EN_ND, sizeof(DAY_EN_ND));
  else if (days == 3)
    drawArray(80, 25, 48, 135, DAY_EN_RD, sizeof(DAY_EN_RD));
  else
    drawArray(80, 25, 48, 135, DAY_EN_TH, sizeof(DAY_EN_TH));
  
  drawNumber(days);
  epd.DisplayFrame();
  epd.Sleep();
}

void setup() {
  Serial.begin(9600);
  DS1307.begin();

  pinMode(2, INPUT);
  pinMode(3, INPUT);

  if (digitalRead(2) == LOW && digitalRead(3) == LOW) {
//    Serial.println("Trigger detected! Writing in EEPRON!");
    DS1307.getDate(RTCValues);
    
    DataObject initData = {
      RTCValues[0],
      RTCValues[1],
      RTCValues[2],
      RTCValues[3],
      RTCValues[4],
      RTCValues[5],
      RTCValues[6]
    };

    EEPROM.put(0, initData);
  }
}

void loop() {
  int32_t hours = getDiff();
  //Serial.println(hours);
  if (hours < 48 && (oldHour == -1 || oldHour != hours)) {
    oldHour = hours;
    drawHours(hours);
  } else if (hours >= 48) { 
    int days = hours/24 + 1;
    if (oldDay == -1 || oldDay != days) {
      oldDay = days;
      drawDays(days);
    }
  }
  delay(300000);
}
