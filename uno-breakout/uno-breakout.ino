#include <SPI.h>
#include "Adafruit_TFTLCD.h"
#include "GFX_Math.h"
#include "object.h"
#include "levels.h"

#define BLACK     0x0000
#define GRAY      0x7BEF
#define LIGHTGRAY 0xC638
#define WHITE     0xFFFF
#define GREEN     0x07E0
#define BLUE      0x3CDF
#define PURPLE    0x99BF
#define RED       0xD083
#define GOLD      0xE5A6

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, NULL);

Block BLOCKS[8][7];

unsigned long t;
unsigned long dt;

uint16_t data;
uint8_t bytesReceived; // 0, 1, or 2

int x;

Ball ball;

uint16_t ColorFromBlock(Block b) {
  switch(b.health) {
    case 0:
      return BLACK;
    case 1: 
      return LIGHTGRAY;
    case 2:
      return GREEN;
    case 3:
      return BLUE;
    case 4:
      return PURPLE;
    case 5: 
      return RED;
    case 6:
      return GOLD;
    default:
      return BLACK;
  }
}

void DrawWalls() {
  Serial.println("Drawing walls...");
  // top wall white
  tft.drawFastHLine(0, 0, 240, WHITE);
  tft.drawFastHLine(0, 1, 240, WHITE);
  tft.drawFastHLine(0, 2, 240, WHITE);
  tft.drawFastHLine(0, 3, 240, WHITE);
  tft.drawFastHLine(0, 4, 240, WHITE);
  tft.drawFastHLine(0, 5, 240, WHITE);
  
  // left wall white
  tft.drawFastVLine(0, 6, 313, WHITE);
  tft.drawFastVLine(1, 6, 313, WHITE);
  tft.drawFastVLine(2, 6, 313, WHITE);
  tft.drawFastVLine(3, 6, 313, WHITE);
  tft.drawFastVLine(4, 6, 313, WHITE);
  tft.drawFastVLine(5, 6, 313, WHITE);
  
  // right wall white
  tft.drawFastVLine(239, 6, 313, WHITE);
  tft.drawFastVLine(238, 6, 313, WHITE);
  tft.drawFastVLine(237, 6, 313, WHITE);
  tft.drawFastVLine(236, 6, 313, WHITE);
  tft.drawFastVLine(235, 6, 313, WHITE);
  tft.drawFastVLine(234, 6, 313, WHITE);
}

const int xdist = 13;
const int ydist = 29;

void GenerateBlocks() {
  Serial.println("Generating blocks...");
  for(int i = 0; i < 8; i++) {
    for(int j = 0; j < 7; j++) {
      BLOCKS[i][j] = Block{vec2i{j*31 + xdist, i*17 + ydist}, 0};
    }
  }
}

void DrawBlocks() {
  Serial.println("Drawing blocks...");
  for(int i = 0; i < 8; i++) {
    for(int j = 0; j < 7; j++) {
      tft.fillRect(BLOCKS[i][j].position.x, BLOCKS[i][j].position.y, 28, 14, ColorFromBlock(BLOCKS[i][j]));
    }
  }
}

void LoadLevel(uint8_t level[][7]) {
  for(int i = 0; i < 8; i++) {
    for(int j = 0; j < 7; j++) {
      BLOCKS[i][j].health = level[i][j];
    }
  }
}

void setup() {
  Serial.begin(9600);
  //pinMode(MISO, OUTPUT);
  //SPCR |= _BV(SPE);
  //SPI.attachInterrupt();

  Serial.println(sizeof(uint16_t));

  tft.reset();
  uint16_t id = tft.readID();
  tft.begin(id);
  tft.setTextColor(GREEN);
  tft.setTextSize(2);
  tft.fillScreen(BLACK);

  // game setup
  Serial.println("test");
  DrawWalls();
  GenerateBlocks();
  LoadLevel(level1);
  DrawBlocks();

  x = 0;
  ball.velocity = vec2i{1, 2};
}

void loop() {
  t = millis();

  tft.setCursor(8, 304);
  tft.fillRect(8, 304, 24, 16, BLACK);
  tft.print(dt);
  Serial.println(dt);

  //update ball
  ball.lastPos = ball.position;
  ball.position += ball.velocity;

  ball.bounds.min.x = ball.position.x - ball.radius;
  ball.bounds.min.y = ball.position.y - ball.radius;
  ball.bounds.max.x = ball.position.x + ball.radius;
  ball.bounds.max.y = ball.position.y + ball.radius;

  if(ball.bounds.max.x >= 233 || ball.bounds.min.x <= 6) {
    ball.velocity.x *= -1;
  }
  if(ball.bounds.max.y >= 319 || ball.bounds.min.y <= 6) {
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
  dt = millis() - t;
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

