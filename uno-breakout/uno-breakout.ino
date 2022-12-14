#include <SPI.h>
#include "Adafruit_TFTLCD.h"
#include "GFX_Math.h"
#include "object.h"
#include "levels.h"

#define JOY_Y A4
#define JOY_X A5
#define JOY_BTN 11

#define BLACK     0x0000
#define GRAY      0x7BEF
#define LIGHTGRAY 0xC638
#define WHITE     0xFFFF
#define GREEN     0x07E0
#define BLUE      0x075F
#define PURPLE    0xF81F
#define RED       0xF800
#define GOLD      0xFF80
#define PINK      0xF81F

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, NULL);

#define BLOCK_SIZE_X 28
#define BLOCK_SIZE_Y 14
Block BLOCKS[8][7];

#define PADDLE_W 64
#define PADDLE_H 8
Paddle paddle;

unsigned long t;
unsigned long dt;
int count;
bool ballInBlockZone;
bool holdingButton;
bool showFrametime;

enum GameState {
  TITLE,
  PLAYING,
  PAUSED,
  GAME_OVER,
  WON_LEVEL,
  WON_GAME
};

GameState gameState;

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

// 28 = xblocksize, 3 = gap, 13 = xoffset
// 14 = yblocksize, 3 = gap, 29 = yoffset
vec2i blockPositionFromIndex(int x, int y) {
  return vec2i{x*BLOCK_SIZE_X + x*3 + 13, y*BLOCK_SIZE_Y + y*3 + 29};
}
vec2i indexFromBlockPosition(int x, int y) {
  return vec2i{(x - 13) / 31, (y - 29) / 17};
}

void GenerateBlocks() {
  Serial.println("Generating blocks...");
  for(int i = 0; i < 8; i++) {
    for(int j = 0; j < 7; j++) {
      vec2i position = blockPositionFromIndex(j, i);
      BLOCKS[i][j] = Block{position, 0};
    }
  }
}

void DrawBlocks() {
  Serial.println("Drawing blocks...");
  for(int i = 0; i < 8; i++) {
    for(int j = 0; j < 7; j++) {
      tft.fillRect(BLOCKS[i][j].position.x, BLOCKS[i][j].position.y, BLOCK_SIZE_X, BLOCK_SIZE_Y, ColorFromBlock(BLOCKS[i][j]));
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

void DrawPaddle() {
  tft.fillRect(paddle.lastPos.x, paddle.lastPos.y, paddle.w, paddle.h, 0x0000);
  tft.fillRect(paddle.position.x, paddle.position.y, paddle.w, paddle.h, 0xFFFF);
}

void DrawBall() {
  tft.fillRect(ball.lastPos.x-ball.radius, ball.lastPos.y-ball.radius, 2*ball.radius+1, 2*ball.radius+1, 0x0000);
  tft.fillCircle(ball.position.x, ball.position.y, ball.radius, ball.color);
  delay(5);
}

void DrawTitleScreen() {
  tft.fillScreen(BLACK);
  tft.setTextSize(4);
  tft.setTextColor(BLUE);
  tft.setCursor(20, 100);
  tft.print("BREAKOUT!");
  tft.setTextColor(PURPLE);
  tft.setCursor(22, 99);
  tft.print("BREAKOUT!");
  tft.setTextColor(RED);
  tft.setCursor(24, 98);
  tft.print("BREAKOUT!");
  tft.setTextSize(2);
  //tft.setTextColor(PURPLE);
  tft.setCursor(107, 160);
  tft.println("Start");
}

void DrawPauseMenu() {
  tft.fillRect(20, 100, 200, 60, BLACK);
  tft.setTextSize(4);
  tft.setCursor(50, 120);
  tft.println("PAUSED");
  tft.setTextSize(1);
  tft.setCursor(50, 160);
  tft.println("Press button to resume.");
}

void DrawGameoverScreen() {
  tft.fillRect(20, 100, 200, 60, BLACK);
  tft.setTextSize(4);
  tft.setTextColor(RED);
  tft.setCursor(24, 120);
  tft.println("YOU LOST");
  tft.setTextSize(1);
  tft.setCursor(46, 160);
  tft.println("Press button to restart.");
}

void RESTART_GAME() {
  Serial.println("restarting game");
  tft.fillScreen(BLACK);
  DrawWalls();
  GenerateBlocks();
  LoadLevel(test);
  DrawBlocks();

  ball.velocity = vec2i{2, -1};
  ball.position = vec2i{120, 250};
  ball.lastPos = ball.position;
  ball.bounds.min.x = ball.position.x - ball.radius;
  ball.bounds.min.y = ball.position.y - ball.radius;
  ball.bounds.max.x = ball.position.x + ball.radius;
  ball.bounds.max.y = ball.position.y + ball.radius;

  paddle.w = PADDLE_W;
  paddle.h = PADDLE_H;
  paddle.position = vec2i{120 - PADDLE_W/2, 284};
  paddle.lastPos = paddle.position;
  DrawPaddle();

  gameState = PLAYING;
}

// ========================================================== SETUP ==============================================================
void setup() {
  Serial.begin(9600);
  
  pinMode(JOY_BTN, INPUT);

  tft.reset();
  uint16_t id = tft.readID();
  tft.begin(id);
  tft.setTextColor(GREEN);
  tft.setTextSize(2);

  gameState = TITLE;
  holdingButton = false;
  count = 0;
  showFrametime = false;

  // game setup
  //RESTART_GAME();
  ball.position = vec2i{120, 250};
  ball.lastPos = ball.position;
  ball.velocity = vec2i{2, 0};
  DrawTitleScreen();
}
// =================================================================================================================================

// checks if the ball is within a given rect
bool ballBlockCollision(int x, int y, int w, int h) {
  // check if the ball is anywhere other than the block
  if(
    ball.bounds.min.x > x + w ||
    ball.bounds.max.x < x     ||
    ball.bounds.min.y > y + h ||
    ball.bounds.max.y < y
  ) {
    return false;
  }
  return true;
}

// CREDIT: https://youtu.be/AGblHq-ZamI
// checks if a collision should bounce the ball vertically or horizontally
bool ballHorizontalCollision(int x, int y, int w, int h) {
  if(ball.velocity.x == 0) {
    return false;
  }
  else if(ball.velocity.y == 0) {
    return true;
  }
  else {
    float slope = ball.velocity.y / (float)ball.velocity.x;
    if(slope > 0 && ball.velocity.x > 0) {
      // moving down right
      vec2i corner = vec2i{x - ball.position.x, y - ball.position.y};   // distance to nearest corner
      if(corner.x <= 0) {
        return false;
      }
      else if(corner.y/(float)corner.x < slope) {
        return true;
      }
      else {
        return false;
      }
    }
    else if(slope < 0 && ball.velocity.x > 0) {
      //moving up right
      vec2i corner = vec2i{x - ball.position.x, y + h - ball.position.y};   // distance to nearest corner
      if(corner.x <= 0) {
        return false;
      }
      else if(corner.y/(float)corner.x < slope) {
        return false;
      }
      else {
        return true;
      }
    }
    else if(slope > 0 && ball.velocity.x < 0) {
      // moving up left
      vec2i corner = vec2i{x + w - ball.position.x, y + h - ball.position.y};   // distance to nearest corner
      if(corner.x >= 0) {
        return false;
      }
      else if(corner.y/(float)corner.x > slope) {
        return false;
      }
      else {
        return true;
      }
    }
    else {
      // moving left down - WORKING
      vec2i corner = vec2i{x + w - ball.position.x, y - ball.position.y};   // distance to nearest corner
      if(corner.x >= 0) {
        return false;
      }
      else if(corner.y/(float)corner.x < slope) {
        return false;
      }
      else {
        return true;
      }
    }
  }
}

// =================================================================== LOOP ===========================================================
void loop() {
  t = millis();
  Serial.println(gameState);

  if(showFrametime) {
    tft.setCursor(8, 304);
    tft.fillRect(8, 304, 36, 16, BLACK);
    tft.print(dt);
  }
  else {
    delay(5); // this compensates for the reduction in latency when not printing the frametime
  }
  count += dt;
  //Serial.println(dt);
  
  int joyx = (analogRead(JOY_X) - 515) / 50;
  int joyy = (analogRead(JOY_Y) - 515) / 50;
  int btn = 1 - digitalRead(JOY_BTN);

  if(btn == 1 && holdingButton) {
    btn = 0;
  }
  else if(btn == 1 && !holdingButton) {
    holdingButton = true;    
  }
  else {
    holdingButton = false;
  }
  //Serial.println(btn);

  switch(gameState) {
    case PLAYING:
    {
      if(btn == 1) {
        DrawPauseMenu();
        gameState = PAUSED;
        break;
      }

      //update ball
      ball.lastPos = ball.position;

      if(ball.bounds.max.x >= 232) {
        ball.velocity.x = -abs(ball.velocity.x);
      }
      else if(ball.bounds.min.x <= 6) {
        ball.velocity.x = abs(ball.velocity.x);
      }
      if(ball.bounds.min.y <= 6) {
        ball.velocity.y *= -1;
      }
      else if(ball.bounds.max.y >= 319) {
        DrawGameoverScreen();
        gameState = GAME_OVER;
        break;
      }

      // update paddle
      paddle.velocity.x = joyx;
      paddle.lastPos = paddle.position;
      paddle.position += paddle.velocity;
      paddle.position.x = clamp(paddle.position.x, 7, 233 - PADDLE_W);
      // only redraw paddle if it has moved
      if(paddle.lastPos.x != paddle.position.x) {
        DrawPaddle();
      }

      // block collisions
      ballInBlockZone = ball.bounds.min.y < 162;
      if(ballInBlockZone) {
        // collide ball with blocks
        for(int i = 0; i < 8; i++) {
          for(int j = 0; j < 7; j++) {
            if(BLOCKS[i][j].health != 0) {
              bool collision = ballBlockCollision(BLOCKS[i][j].position.x, BLOCKS[i][j].position.y, BLOCK_SIZE_X, BLOCK_SIZE_Y);
              if(collision) {
                BLOCKS[i][j].health -= 1;
                tft.fillRect(BLOCKS[i][j].position.x, BLOCKS[i][j].position.y, BLOCK_SIZE_X, BLOCK_SIZE_Y, ColorFromBlock(BLOCKS[i][j]));
                if(ballHorizontalCollision(BLOCKS[i][j].position.x, BLOCKS[i][j].position.y, BLOCK_SIZE_X, BLOCK_SIZE_Y)) {
                  ball.velocity.x *= -1;
                }
                else {
                  ball.velocity.y *= -1;
                }
                break;
              }
            }
          }
        }
      }
      else {
        // paddle collision
        bool collision = ballBlockCollision(paddle.position.x, paddle.position.y, paddle.w, paddle.h);
        if(collision) {
          if(ballHorizontalCollision(paddle.position.x, paddle.position.y, paddle.w, paddle.h)) {
            ball.velocity.x *= -1;
          }
          else {
            ball.velocity.y *= -1;
          }
        }
      }

      ball.position += ball.velocity;

      ball.bounds.min.x = ball.position.x - ball.radius;
      ball.bounds.min.y = ball.position.y - ball.radius;
      ball.bounds.max.x = ball.position.x + ball.radius;
      ball.bounds.max.y = ball.position.y + ball.radius;

      DrawBall();

      //delay(300);
      break;
    }
    case PAUSED:
      if(btn == 1) {
        tft.fillRect(50, 120, 150, 70, BLACK);
        delay(50);
        DrawBlocks();
        delay(10);
        tft.setTextSize(2);
        gameState = PLAYING;
      }
      break;
    case TITLE:
    {
      if(btn == 1) {
        tft.setTextColor(GREEN);
        gameState = PLAYING;
        RESTART_GAME();
        break;
      }      
      uint16_t col = RED;
      if(count > 1000) {
        col = BLACK;
      }
      if(count > 2000) {
        col = RED;
        count = 0;
      }
      tft.fillTriangle(77, 160, 77, 176, 92, 168, col);
      
      ball.lastPos = ball.position;

      if(ball.bounds.max.x >= 240) {
        ball.velocity.x = -abs(ball.velocity.x);
      }
      else if(ball.bounds.min.x <= 0) {
        ball.velocity.x = abs(ball.velocity.x);
      }

      ball.position += ball.velocity;
      ball.bounds.min.x = ball.position.x - ball.radius;
      ball.bounds.min.y = ball.position.y - ball.radius;
      ball.bounds.max.x = ball.position.x + ball.radius;
      ball.bounds.max.y = ball.position.y + ball.radius;
      DrawBall();
      break;
    }
    case GAME_OVER:
      if(btn == 1) {
        tft.setTextSize(2);
        gameState = PLAYING;
        RESTART_GAME();
        delay(50);
      }
      break;
    default:
      Serial.println("Something went horribly wrong");
  }

  dt = millis() - t;
}
// ==================================================================================================================================

