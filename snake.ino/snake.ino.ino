#include <LinkedList.h>

#include <SoftwareSerial.h>
#include "Goldelox_Serial_4DLib.h"
#include "Goldelox_Const4D.h"


int lcdPin = 7;
int leftPin = 5;
int rightPin = 4;
int upPin = 3;
int downPin = 2;
int colorSelected = 0;
int i = 0;
LinkedList<int> snakeX = LinkedList<int>();
LinkedList<int> snakeY = LinkedList<int>();
int snakeLength = 2;

LinkedList<int> obstaclesX = LinkedList<int>();
LinkedList<int> obstaclesY = LinkedList<int>();
int numObstacles = 0;
int randX = 0;
int randY = 0;
int apple[2] = {0, 0};

int startX = 64;
int startY = 64;
int lastDirection = 2;

int applesEaten = 0;
bool gameStart = true;

int dataX;
int dataY;
SoftwareSerial BT(9, 10);
SoftwareSerial DisplaySerial(11, 12);
Goldelox_Serial_4DLib Display(&DisplaySerial);


void setup() {
  // put your setup code here, to run once:
  //For handling errors

  randomSeed(analogRead(A0));
  snakeX.add(0, startX);
  snakeY.add(0, startY);
  snakeX.add(1, startX - 4);
  snakeY.add(1, startY);
  
  Display.Callback4D = mycallback;

  //5 second timeout on all commands  
  Display.TimeLimit4D = 5000;
  
  Serial.begin(9600);
  DisplaySerial.begin(9600);
  BT.begin(9600);

  // pin modes for joystick inputs
  pinMode(leftPin, INPUT_PULLUP);
  pinMode(rightPin, INPUT_PULLUP);
  pinMode(downPin, INPUT_PULLUP);
  pinMode(upPin, INPUT_PULLUP);

  // reset procedure for lcd
  digitalWrite(lcdPin, 0);
  delay(100);
  digitalWrite(lcdPin, 1);
  //delay(1000);
  DisplaySerial.listen();
  Display.gfx_ScreenMode(LANDSCAPE);
  Display.SSTimeout(0) ;
  Display.SSSpeed(0) ;
  Display.SSMode(0) ;
  Display.gfx_Cls();
  draw_game();
}

void loop() {
  // put your main code here, to run repeatedly:
  BT.listen();
  //data = BT.read();
  while (BT.available() > 0) {
    dataX = BT.read() * 4;
    dataY = BT.read() * 4;
    if (isValidObstacle(dataX, dataY)) {
      placeObstacle(dataX, dataY);
      obstaclesX.add(0, dataX);
      obstaclesY.add(0, dataY);
      numObstacles++;
      if (numObstacles > 8) {
        int obX = obstaclesX.pop();
        int obY = obstaclesY.pop();
        numObstacles--;
        clearObstacle(obX, obY);
      }
    }
  }
  
  int lIn = digitalRead(leftPin);
  int rIn = digitalRead(rightPin);
  int dIn = digitalRead(downPin);
  int uIn = digitalRead(upPin);

  bool up = false;
  bool right = false;
  bool down = false;
  bool left = false;
  if (uIn == 0) {
    up = true;
  }
  if (rIn == 0) {
    right = true;
  }
  if (dIn == 0) {
    down = true;
  }
  if (lIn == 0) {
    left = true;
  }
  handleInputs(up, right, down, left);
  BT.listen();
  delay(100);
  dataX = 0;
  dataY = 0;
}

void draw_game() {
  Display.gfx_RectangleFilled(snakeX.get(0), snakeY.get(0), snakeX.get(0) + 3, snakeY.get(0) + 3, LIGHTGREEN);
  Display.gfx_RectangleFilled(snakeX.get(1), snakeY.get(1), snakeX.get(1) + 3, snakeY.get(1) + 3, LIGHTGREEN);
  BT.listen();
  drawNewApple();
}

void handleInputs(bool up, bool right, bool down, bool left) {
  bool isBTInput = false;
  bool isSnakeMoving = valid_input(up, right, down, left);
  if (isSnakeMoving) {
    if (gameStart) {
      gameStart = false;
    }
    moveSnake(up, right, down, left);
  } else if (!gameStart) {
    if (lastDirection == 1) {
      moveSnake(true, false, false, false);
    } else if (lastDirection == 2) {
      moveSnake(false, true, false, false);
    } else if (lastDirection == 3) {
      moveSnake(false, false, true, false);
    } else if (lastDirection == 4) {
      moveSnake(false, false, false, true);
    }
  }
  BT.listen();
}

void moveSnake(bool up, bool right, bool down, bool left) {
  bool moved = false;
  if (up && lastDirection != 3) {
    if (!is_valid(0)) {
      end_game();
    }
    moved = true;
    snakeY.add(0, snakeY.get(0) - 4);
    snakeX.add(0, snakeX.get(0));
    lastDirection = 1;
  } else if (right && lastDirection != 4) {
    if (!is_valid(1)) {
      end_game();
    }
    moved = true;
    snakeY.add(0, snakeY.get(0));
    snakeX.add(0, snakeX.get(0) + 4);
    lastDirection = 2;
  } else if (down && lastDirection != 1) {
    if (!is_valid(2)) {
      end_game();
    }
    moved = true;
    snakeY.add(0, snakeY.get(0) + 4);
    snakeX.add(0, snakeX.get(0));
    lastDirection = 3;
  } else if (left && lastDirection != 2) {
    if (!is_valid(3)) {
      end_game();
    }
    moved = true;
    snakeY.add(0, snakeY.get(0));
    snakeX.add(0, snakeX.get(0) - 4);
    lastDirection = 4;
  }
  if (moved) {
    if (hasCollision(snakeX.get(0), snakeY.get(0), apple[0], apple[1])) {
      snakeLength++;
      applesEaten++;
      int sendX = apple[0] / 4;
      int sendY = apple[1] / 4;
      BT.write(sendX);
      BT.write(sendY);
      drawNewApple();
      updateSnake(true);
    } else {
      updateSnake(false);
      snakeX.pop();
      snakeY.pop();
    }
  }
}

bool valid_input(bool up, bool right, bool down, bool left) {
  int numInputs = 0;
  if (up) {
    if (lastDirection == 3) {
      return false;
    }
    numInputs++;
  }
  if (right) {
    if (lastDirection == 4) {
      return false;
    }
    numInputs++;
  }
  if (down) {
    if (lastDirection == 1) {
      return false;
    }
    numInputs++;
  }
  if (left) {
    if (lastDirection == 2) {
      return false;
    }
    numInputs++;
  }
  return numInputs == 1;
}

void updateSnake(bool ateApple) {
  DisplaySerial.listen();
  Display.gfx_RectangleFilled(snakeX.get(0), snakeY.get(0), snakeX.get(0) + 3, snakeY.get(0) + 3, LIGHTGREEN);
  if (!ateApple) {
    Display.gfx_RectangleFilled(snakeX.get(snakeLength), snakeY.get(snakeLength), snakeX.get(snakeLength) + 3, snakeY.get(snakeLength) + 3, BLACK);
  }
  BT.listen();
}

bool isValidObstacle(int x, int y) {
  if (hasCollision(x, y, apple[0], apple[1])) {
    return false;
  }
  for (int i = 0; i < snakeLength; i++) {
    if (hasCollision(x, y, snakeX.get(i), snakeY.get(i))) {
      return false;
    }
  }
  return true;
}

void placeObstacle(int x, int y) {
  DisplaySerial.listen();
  Display.gfx_RectangleFilled(x, y, x + 3, y + 3, WHITE);
  BT.listen();
}

void clearObstacle(int x, int y) {
  DisplaySerial.listen();
  Display.gfx_RectangleFilled(x, y, x + 3, y + 3, BLACK);
  BT.listen();
}

bool is_valid(int dir) {
  int newX = 0;
  int newY = 0;
  if (dir == 0) { // up
    newX = snakeX.get(0);
    newY = snakeY.get(0) - 4;
  } else if (dir == 1) { // right
    newX = snakeX.get(0) + 4;
    newY = snakeY.get(0);
  } else if (dir == 2) { // down
    newX = snakeX.get(0);
    newY = snakeY.get(0) + 4;
  } else if (dir == 3) { // left
    newX = snakeX.get(0) - 4;
    newY = snakeY.get(0);
  }
  if (isSnakeCollision(newX, newY)) {
      return false;
  }
  return true;
}

bool isSnakeCollision(int x, int y) {
  if (x < 0 || x > 124 || y < 0 || y > 124) {
    return true;
  }
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX.get(i) == x && snakeY.get(i) == y) {
      return true;
    }
  }
  for (int i = 0; i < numObstacles; i++) {
    if (obstaclesX.get(i) == x && obstaclesY.get(i) == y) {
      return true;
    }
  }
  return false;
}

bool hasCollision(int x1, int y1, int x2, int y2) {
  if (x1 == x2 && y1 == y2) {
    return true;
  }
  return false;
}

void drawNewApple() {
  bool validApple = false;
  while (!validApple) {
    randX = random(32) * 4;
    randY = random(32) * 4;
    validApple = true;
    for (int i = 0; i < snakeLength; i++) {
      if (snakeX.get(i) == randX && snakeY.get(i) == randY) {
        validApple = false;
        break;
      }
    }
  }
  apple[0] = randX;
  apple[1] = randY;
  drawApple(apple[0], apple[1]);
}

void drawApple(int appleX, int appleY) {
  DisplaySerial.listen();
  Display.gfx_RectangleFilled(appleX, appleY, appleX + 3, appleY + 3, RED);
  BT.listen();
}

void end_game() {
  DisplaySerial.listen();
  Display.gfx_Cls();
  Display.println("Game over :(");
  Display.print("Apples eaten: ");
  Display.print(applesEaten);
  while (true) {
    delay(1000);
  }
}

void mycallback(int ErrCode, unsigned char Errorbyte)
{
  // Pin 13 has an LED connected on most Arduino boards. Just give it a name
  int led = 13;
  pinMode(led, OUTPUT);
  while(1)
  {
    digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(200);                // wait for 200 ms
    digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
    delay(200);                // wait for 200 ms
    
  }
}
