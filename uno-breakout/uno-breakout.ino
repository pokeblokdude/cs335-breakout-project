#include <SPI.h>
#include "Adafruit_TFTLCD.h"
#include "GFX_Math.h"

#define BLACK 0x0000
#define GRAY 0x7BEF
#define WHITE 0xFFFF

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, NULL);

uint16_t data;
uint8_t bytesReceived; // 0, 1, or 2

int x;

struct Bounds { 
  vec2i min;
  vec2i max;
};

struct Ball {
  vec2i position = vec2i{120, 160};
  vec2i lastPos;
  vec2i velocity;
  Bounds bounds;
  int radius = 4;
  uint16_t color = 0xFFFF;
};

Ball ball;

void DrawWalls() {
  tft.drawFastHLine(0, 0, 240, WHITE);
  tft.drawFastHLine(0, 1, 240, WHITE);
  tft.drawFastHLine(0, 2, 240, WHITE);
  tft.drawFastHLine(0, 3, 240, WHITE);
  tft.drawFastHLine(0, 4, 240, WHITE);
  tft.drawFastHLine(0, 5, 240, WHITE);
  
  tft.drawFastVLine(0, 6, 313, WHITE);
  tft.drawFastVLine(1, 6, 313, WHITE);
  tft.drawFastVLine(2, 6, 313, WHITE);
  tft.drawFastVLine(3, 6, 313, WHITE);
  tft.drawFastVLine(4, 6, 313, WHITE);
  tft.drawFastVLine(5, 6, 313, WHITE);
  
  tft.drawFastVLine(239, 6, 313, WHITE);
  tft.drawFastVLine(238, 6, 313, WHITE);
  tft.drawFastVLine(237, 6, 313, WHITE);
  tft.drawFastVLine(236, 6, 313, WHITE);
  tft.drawFastVLine(235, 6, 313, WHITE);
  tft.drawFastVLine(234, 6, 313, WHITE);

  tft.drawFastHLine(7, 6, 226, GRAY);
  tft.drawFastHLine(8, 7, 224, GRAY);
  tft.drawFastHLine(9, 8, 222, GRAY);
  
  tft.drawFastVLine(6, 7, 313, GRAY);
  tft.drawFastVLine(7, 8, 312, GRAY);
  tft.drawFastVLine(8, 9, 311, GRAY);
  
  tft.drawFastVLine(233, 7, 313, GRAY);
  tft.drawFastVLine(232, 8, 312, GRAY);
  tft.drawFastVLine(231, 9, 311, GRAY);

  tft.drawPixel(6, 6, WHITE);
  tft.drawPixel(7, 7, WHITE);
  tft.drawPixel(8, 8, WHITE);
  
  tft.drawPixel(233, 6, WHITE);
  tft.drawPixel(232, 7, WHITE);
  tft.drawPixel(231, 8, WHITE);
}

void setup() {
  Serial.begin(9600);
  pinMode(MISO, OUTPUT);
  SPCR |= _BV(SPE);
  //SPI.attachInterrupt();

  tft.reset();
  uint16_t id = tft.readID();
  tft.begin(id);
  tft.fillScreen(BLACK);
  DrawWalls();

  x = 0;
  ball.velocity = vec2i{1, 1};
}

void loop() {
  unsigned long t = millis();
  //update ball
  ball.lastPos = ball.position;
  ball.position += ball.velocity;

  ball.bounds.min.x = ball.position.x - ball.radius;
  ball.bounds.min.y = ball.position.y - ball.radius;
  ball.bounds.max.x = ball.position.x + ball.radius;
  ball.bounds.max.y = ball.position.y + ball.radius;

  if(ball.bounds.max.x >= 230 || ball.bounds.min.x <= 9) {
    ball.velocity.x *= -1;
  }
  if(ball.bounds.max.y >= 319 || ball.bounds.min.y <= 9) {
    ball.velocity.y *= -1;
  }
  tft.fillCircle(ball.lastPos.x, ball.lastPos.y, ball.radius, 0x0000);
  tft.fillCircle(ball.position.x, ball.position.y, ball.radius, ball.color);
  delay(5);

  // if(bytesReceived == 2) {
  //   int lastX = x-1;
  //   if(x > 240) {
  //     x = 0;
  //     lastX = 240;
  //   }
  //   //Serial.println(data);
  //   //tft.fillRect(120, lastX, 32, 32, 0x0000);
  //   //tft.fillRect(120, x, 32, 32, data);
  //   tft.fillCircle(120, lastX, 10, 0x0000);
  //   tft.fillCircle(120, x, 10, data);
  //   bytesReceived = 0;
  //   x++;
  // }
  //delay(100);
  unsigned long dt = millis() - t;
  Serial.println(dt);
}

ISR(SPI_STC_vect) {
  uint16_t i = SPDR;
  if(bytesReceived == 0) {
    bytesReceived++;
    data = i << 8;
  }
  else {
    bytesReceived++;
    data += i;
  }
}

