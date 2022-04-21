#define BTNLEFT 8
#define BTNRIGHT 9
#define BTNOK 10
#define BTNUP 11
#define BTNDOWN 12
#define MAXWIDTH 48
#define MAXHEIGHT 84
#define SIZE 3

#include "GyverButton.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);
GButton buttLeft(BTNLEFT);
GButton buttRight(BTNRIGHT);
GButton buttOk(BTNOK);
GButton buttUp(BTNUP);
GButton buttDown(BTNDOWN);
int8_t fig = 0, ang = 0, pos = 24, height = 18, prevPos = 24, prevHeight = 0, nextFig, nextAng;
int score = 0;
bool next = true;
bool gameOn = true;
uint32_t myTimer1;

//  0 - I
//  1 - O
//  2 - L
//  3 - J
//  4 - Z
//  5 - S
//  6 - Т
bool field[16][28];
uint8_t buff[MAXWIDTH * MAXHEIGHT / 8];
const int8_t figures[7][12][2] PROGMEM = { // Координати частин фігур
  {
    { -1, 0}, {1, 0}, {2, 0},
    {0, 1}, {0, 2}, {0, 3},
    { -1, 0}, {1, 0}, {2, 0},
    {0, 1}, {0, 2}, {0, 3},
  },
  {
    {0, 1}, {1, 0}, {1, 1},
    {0, 1}, {1, 0}, {1, 1},
    {0, 1}, {1, 0}, {1, 1},
    {0, 1}, {1, 0}, {1, 1},
  },
  {
    { -1, 0}, { -1, 1}, {1, 0},
    {0, 1}, {0, 2}, {1, 2},
    { -2, 1}, { -1, 1}, {0, 1},
    { -1, 0}, {0, 1}, {0, 2},
  },
  {
    { -1, 0}, {1, 0}, {1, 1},
    {0, 1}, {0, 2}, {1, 0},
    {0, 1}, {1, 1}, {2, 1},
    {0, 1}, {0, 2}, { -1, 2},
  },
  {
    { -1, 0}, {0, 1}, {1, 1},
    {0, 1}, { -1, 1}, { -1, 2},
    { -1, 0}, {0, 1}, {1, 1},
    {0, 1}, { -1, 1}, { -1, 2},
  },
  {
    { -1, 1}, {0, 1}, {1, 0},
    {0, 1}, {1, 1}, {1, 2},
    { -1, 1}, {0, 1}, {1, 0},
    {0, 1}, {1, 1}, {1, 2},
  },
  {
    { -1, 0}, {0, 1}, {1, 0},
    {0, 1}, {0, 2}, {1, 1},
    { -1, 1}, {0, 1}, {1, 1},
    { -1, 1}, {0, 1}, {0, 2},
  }
};





void setup() {
  randomSeed(analogRead(A0)); // генератор рандомних чисел
  display.begin();
  display.setContrast(60);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setRotation(3);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.display();
  fig = random(7); // гереруємо першу фігуру
  ang = random(4);
}
void loop() {
  // обробка кнопок
  buttLeft.tick();
  buttRight.tick();
  buttOk.tick();
  buttUp.tick();
  buttDown.tick();
  if (gameOn) {
    game();
  } else {
    gameOver();
  }
  display.display();
  button();
}
void game() {
  if (millis() - myTimer1 >= 300) {   // таймер на millis
    myTimer1 = millis();
    if (next) { // наступна фігура
      nextFig = random(7);
      nextAng = random(4);
      next = !next;
      drawNextFigure();
      display.fillRect(0, 0, 24, 15, 0);
      display.display();
      display.setCursor(4, 4);
      display.print(score);
    }

    if (canMoveFig(3)) {      // перевіряємо чи можна рухатись вниз, якщо так то спускаємо фігуру
      clearFigure();
      height += 3;
      prevPos = pos;
      prevHeight = height - SIZE;
      clearFigure();
      drawFigure();
    } else {   // якщо ні, то прикріплюємо фігуру і малюємо нову
      checkFigureToClear(pos, height + 2);
      fig = nextFig;
      ang = nextAng;
      pos = 24;
      height = 18;
      prevPos = 24;
      prevHeight = 0;
      drawFigure();
      next = !next;
      clearNextFigure();
    }
  }
}
void gameOver() { // game over = гра завершена
  display.clearDisplay();
  display.setCursor(0, 30);
  display.print("Gameover");
  display.setCursor(3, 45);
  display.print("Score:");
  display.print(score);
}
void button() {
  if (buttLeft.isClick()) { // кнопку вліво натиснуто
    if (canMoveFig(2)) {
      clearFigure();
      pos -= SIZE;
      prevPos = pos + SIZE;
      prevHeight = height;
      clearFigure();
      drawFigure();
    }
  }
  if (buttRight.isClick()) { // кнопку вправо натиснуто
    if (canMoveFig(1)) {
      clearFigure();
      pos += SIZE;
      prevPos = pos - SIZE;
      prevHeight = height;
      clearFigure();
      drawFigure();

    }
  }
  if (buttOk.isClick()) { // кнопку ок натиснуто
    if (canMoveFig(4)) {
      clearFigure();
      ang = ++ang % 4;
      drawFigure();
    }
  }
  if (buttUp.isClick()) { // кнопку вверх натиснуто
    if (gameOn == false) {
      gameOn = true;
      display.clearDisplay();
      pos = 24;
      height = 18;
      prevPos = 24;
      prevHeight = 0;
      score = 0;
    }
  }
  if (buttDown.isClick()) { // кнопку вниз натиснуто
    if (canMoveFig(3)) {
      clearFigure();
      height += 3;
      prevPos = pos;
      prevHeight = height - SIZE;
      clearFigure();
      drawFigure();
    } else {
      checkFigureToClear(pos, height + 2);
      fig = nextFig;
      ang = nextAng;
      pos = 24;
      height = 18;
      prevPos = 24;
      prevHeight = 0;
      drawFigure();
      next = !next;
      clearNextFigure();
    }
  }
}
bool canMoveFig(int8_t check_type) { // чи може фігура рухатись
  boolean flag = true;
  int8_t X, Y;
  int8_t this_ang = ang;
  int8_t coordFig[4][2];

  for (byte i = 0; i < 4; i++) {
    // перевіряємо точки фігури
    if (i == 0) {   // стартова точка фигури (початок відліку)
      coordFig[i][0] = pos;
      coordFig[i][1] = height + 2;
    } else {        // решту три точки
      coordFig[i][0] = pos + (int8_t)pgm_read_byte(&figures[fig][this_ang * 3 + i - 1][0]) * SIZE;
      coordFig[i][1] = height - (int8_t)pgm_read_byte(&figures[fig][this_ang * 3 + i - 1][1]) * SIZE + 2;
    }
  }
  if (check_type == 1 || check_type == 2 || check_type == 3) {
    for (byte i = 0; i < 4; i++) {
      X = coordFig[i][0];
      Y = coordFig[i][1];
      if (check_type == 1) { // Чи можна рухатись вправо
        if (X + 1 + SIZE > MAXWIDTH) { // якщо стикнулась зі стінкою
          flag = false;
          return flag;
        } else {
          if (display.getPixel(X + SIZE, Y, buff)) {  // якщо стикнулась з іншою фігурою
            bool checkpoint = false;
            for (byte i = 0; i < 4; i++) {
              int8_t Xx, Yy;
              Xx = coordFig[i][0];
              Yy = coordFig[i][1];
              if (X + SIZE == Xx && Y == Yy) {
                checkpoint = true;
              }
            }
            if (!checkpoint) {
              flag = false;
              return flag;
            }
          }
        }
      }

      if (check_type == 2) { //Чи можна рухатись вліво
        if (X - SIZE + 1 < 0) { // якщо стикнулась зі стінкою
          flag = false;
          return flag;
        } else {
          if (display.getPixel(X - SIZE, Y, buff)) {  // якщо стикнулась з іншою фігурою
            bool checkpoint = false;
            for (byte i = 0; i < 4; i++) {
              int8_t Xx, Yy;
              Xx = coordFig[i][0];
              Yy = coordFig[i][1];
              if (X - SIZE == Xx && Y == Yy) {
                checkpoint = true;
              }
            }
            if (!checkpoint) {
              flag = false;
              return flag;
            }
          }
        }
      }

      if (check_type == 3) { // Чи може фігура рухатись вниз
        if (Y + SIZE > MAXHEIGHT) { // якщо стикнулась з нижнім порогом
          flag = false;
          return flag;
        } else {
          if (display.getPixel(X, Y + SIZE, buff)) { // якщо стикнулась з іншою фігурою
            bool checkpoint = false;
            for (byte i = 0; i < 4; i++) {
              int8_t Xx, Yy;
              Xx = coordFig[i][0];
              Yy = coordFig[i][1];
              if (X == Xx && Y + SIZE == Yy) {
                checkpoint = true;
              }
            }
            if (!checkpoint) {
              flag = false;
              return flag;
            }
          }
        }
      }
    }
  } else { // поворот
    this_ang = ++this_ang % 4;
    int8_t newRotateCoordFig[4][2];
    for (byte i = 0; i < 4; i++) {
      // записуємо точки фігури
      if (i == 0) {
        newRotateCoordFig[i][0] = pos;
        newRotateCoordFig[i][1] = height + 2;
      } else {
        newRotateCoordFig[i][0] = pos + (int8_t)pgm_read_byte(&figures[fig][this_ang * 3 + i - 1][0]) * SIZE;
        newRotateCoordFig[i][1] = height - (int8_t)pgm_read_byte(&figures[fig][this_ang * 3 + i - 1][1]) * SIZE + 2;
      }
    }
    for (byte i = 0; i < 4; i++) { // перевіряємо точки фігури
      int8_t Xx, Yy;
      Xx = newRotateCoordFig[i][0];
      Yy = newRotateCoordFig[i][1];
      if (display.getPixel(Xx, Yy, buff)) { // якщо нова повернута на інший кут фігура буде заходити в іншу фігуру
        bool checkpoint = false;
        for (byte i = 0; i < 4; i++) {
          if (Xx == coordFig[i][0] && Yy == coordFig[i][1]) {
            checkpoint = true;
          }
        }
        if (!checkpoint) {
          flag = false;
          return flag;
        }
      }
      if (Xx + 1 < 0) { // Піксель частини нової фігури виходить за контур зліва
        flag = false;
        return flag;
      }
      if (Xx + 1 + 2 > MAXWIDTH) { // Піксель частини нової фігури виходить за контур зліва
        flag = false;
        return flag;
      }
    }
  }
  return flag;    // повертаємо глобальний прапорець, який відповідає за те стикнеться наша фігура з перешкодою або стінками чи ні
}
void drawFigure() { // малюємо фігуру
  int8_t X, Y;
  for (int i = 0; i < 4; i++) {
    if ( i == 0) {
      X = pos;
      Y = height;
      if ( X == 24 && Y == 18 && display.getPixel(X, Y, buff)) {
        gameOn = false;
      }
      display.fillRect(X, Y, SIZE, SIZE, 1);
      display.display();
    } else {
      X = pos + (int8_t)(pgm_read_byte(&figures[fig][ang * 3 + i - 1][0]) * SIZE);
      Y = height - (int8_t)(pgm_read_byte(&figures[fig][ang * 3 + i - 1][1]) * SIZE);
      display.fillRect(X, Y, SIZE, SIZE, 1);
      display.display();
    }
  }
}
void clearFigure() { // очищуємо фігуру
  int8_t X, Y;
  for (int i = 0; i < 4; i++) {
    if ( i == 0) {
      X = prevPos;
      Y = prevHeight;
      display.fillRect(X, Y, SIZE, SIZE, 0);
      display.display();
    } else {
      X = pos + (int8_t)(pgm_read_byte(&figures[fig][ang * 3 + i - 1][0]) * SIZE);
      Y = height - (int8_t)(pgm_read_byte(&figures[fig][ang * 3 + i - 1][1]) * SIZE);
      display.fillRect(X, Y, SIZE, SIZE, 0);
      display.display();
    }
  }
}

void drawNextFigure() { // малюємо іконку наступної фігури
  int8_t X, Y;
  for (int i = 0; i < 4; i++) {
    if ( i == 0) {
      X = 40;
      Y = 8;
      display.fillRect(X, Y, SIZE - 1, SIZE - 1, 1);
      display.display();
    } else {
      X = 40 + (int8_t)(pgm_read_byte(&figures[nextFig][nextAng * 3 + i - 1][0]) * (SIZE - 1));
      Y = 8 - (int8_t)(pgm_read_byte(&figures[nextFig][nextAng * 3 + i - 1][1]) * (SIZE - 1));
      display.fillRect(X, Y, SIZE - 1, SIZE - 1, 1);
      display.display();
    }
  }
}
void clearNextFigure() { // очищаємо іконку наступної фігури
  int8_t X, Y;
  for (int i = 0; i < 4; i++) {
    if ( i == 0) {
      X = 40;
      Y = 8;
      display.fillRect(X, Y, SIZE - 1, SIZE - 1, 0);
      display.display();
    } else {
      X = 40 + (int8_t)(pgm_read_byte(&figures[nextFig][nextAng * 3 + i - 1][0]) * (SIZE - 1));
      Y = 8 - (int8_t)(pgm_read_byte(&figures[nextFig][nextAng * 3 + i - 1][1]) * (SIZE - 1));
      display.fillRect(X, Y, SIZE - 1, SIZE - 1, 0);
      display.display();
    }
  }
}


void clearLine(int8_t Y) { // очищаємо заповнену лінію
  display.fillRect(0, Y - 2, 48, 3, 0);
  score++;
  display.display();
}
bool needClearLine(int8_t X, int8_t Y) { // перевіряємо чи вся лінія заповнена, чи її слід очистити
  bool flag = true;
  for (int k = 0; k < MAXWIDTH; k += 3) {
    if (!display.getPixel(k, Y, buff)) {
      flag = false;
      return flag;
    }
  }
  return flag;
}
void moveAllFiguresDown(int8_t Y) { // коли лінію очищено, спускаємо вниз верхні лінії з фігурами
  for (int i = Y; i > 60; i -= SIZE) {
    for (int k = 0; k < MAXWIDTH; k += SIZE) {
      display.fillRect(k, i - 2, SIZE, SIZE, display.getPixel(k, i - SIZE, buff));
    }
  }
}
void checkFigureToClear(int8_t X, int8_t Y) { // перевірка після приєднання фігури чи заповнила вона якусь лінію ( використана рекурсія )
  bool flag = false;
  if (needClearLine(X, Y)) {
    clearLine(Y);
    moveAllFiguresDown(Y);
    checkFigureToClear(X, Y);
  } else {
    Y -= 3;
    if (needClearLine(X, Y)) {
      clearLine(Y);
      moveAllFiguresDown(Y);
      checkFigureToClear(X, Y);
    }
  }
}
