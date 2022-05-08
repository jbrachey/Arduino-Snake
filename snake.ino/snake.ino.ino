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
int randX = 0;
int randY = 0;
int apple[2] = {0, 0};

int startX = 64;
int startY = 64;
int lastDirection = 2;

int applesEaten = 0;

int data;
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
    data = BT.read();
    /*if (BT.available() > 0) {
      handleInputs(data, 1, 1, 1, 1);
    }*/
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
  if (data > 0 || uIn == 0 || rIn == 0 || dIn == 0 || lIn == 0) {
    handleInputs(data, up, right, down, left);
  }
  data = 0;
}

void draw_game() {
  Display.gfx_RectangleFilled(snakeX.get(0), snakeY.get(0), snakeX.get(0) + 3, snakeY.get(0) + 3, GREEN);
  Display.gfx_RectangleFilled(snakeX.get(1), snakeY.get(1), snakeX.get(1) + 3, snakeY.get(1) + 3, GREEN);
  drawNewApple();
  BT.listen();
}

void handleInputs(int data, bool up, bool right, bool down, bool left) {
  //bool isBTMoving = data > 0 && data < 5;
  bool isBTInput = false;
  //bool isBTColorChanging = data >= 5;
  bool isSnakeMoving = up || right || down || left;
  if (isSnakeMoving) {
    moveSnake(up, right, down, left);
  }
  /*if (isBTInput && isSnakeMoving) {
    // Two squares moving at same time
    moveSquare(colorSelected, data - 1);
    handleJS(up, right, down, left);
  } else if (isBTInput && isJSMoving) {
    colorSelected = data - 5;
    //handleJS(up, right, down, left);
  } else if (isBTMoving && !isJSMoving) {
    moveSquare(colorSelected, data - 1);
  } else if (isBTColorChanging && !isJSMoving){
    colorSelected = data - 5;
  } else if (isJSMoving){
    handleJS(up, right, down, left);
  }*/
  BT.listen();
}

void moveSnake(bool up, bool right, bool down, bool left) {
  if (!valid_input(up, right, down, left)) {
    return;
  }
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
    if (apple_collision(snakeX.get(0), snakeY.get(0), apple[0], apple[1])) {
      snakeLength++;
      applesEaten++;
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
    numInputs++;
  }
  if (right) {
    numInputs++;
  }
  if (down) {
    numInputs++;
  }
  if (left) {
    numInputs++;
  }
  return numInputs == 1;
}

void updateSnake(bool ateApple) {
  DisplaySerial.listen();
  Display.gfx_RectangleFilled(snakeX.get(0), snakeY.get(0), snakeX.get(0) + 3, snakeY.get(0) + 3, GREEN);
  if (!ateApple) {
    Display.gfx_RectangleFilled(snakeX.get(snakeLength), snakeY.get(snakeLength), snakeX.get(snakeLength) + 3, snakeY.get(snakeLength) + 3, BLACK);
  }
  BT.listen();
}
/*
void drawSnakeUp() {
  DisplaySerial.listen();
  Display.gfx_RectangleFilled(snakeX.get(0), snakeY.get(0) - 4, snakeX.get(0) + 3, snakeY.get(0) - 1, GREEN);
  Display.gfx_RectangleFilled(snakeX.get(snakeLength - 1), snakeY.get(snakeLength - 1), snakeX.get(snakeLength - 1) + 3, snakeY.get(snakeLength - 1) + 3, BLACK);
  BT.listen();
}
void drawSnakeRight() {
  DisplaySerial.listen();
  Display.gfx_RectangleFilled(snakeX.get(0) + 4, snakeY.get(0), snakeX.get(0) + 7, snakeY.get(0) + 3, GREEN);
  Display.gfx_RectangleFilled(snakeX.get(snakeLength - 1), snakeY.get(snakeLength - 1), snakeX.get(snakeLength - 1) + 3, snakeY.get(snakeLength - 1) + 3, BLACK);
  BT.listen();
}
void drawSnakeDown() {
  DisplaySerial.listen();
  Display.gfx_RectangleFilled(snakeX.get(0), snakeY.get(0) + 4, snakeX.get(0) + 3, snakeY.get(0) + 7, GREEN);
  Display.gfx_RectangleFilled(snakeX.get(snakeLength - 1), snakeY.get(snakeLength - 1), snakeX.get(snakeLength - 1) + 3, snakeY.get(snakeLength - 1) + 3, BLACK);
  BT.listen();
}
void drawSnakeLeft() {
  DisplaySerial.listen();
  Display.gfx_RectangleFilled(snakeX.get(0) - 4, snakeY.get(0), snakeX.get(0) - 1, snakeY.get(0) + 3, GREEN);
  Display.gfx_RectangleFilled(snakeX.get(snakeLength - 1), snakeY.get(snakeLength - 1), snakeX.get(snakeLength - 1) + 3, snakeY.get(snakeLength - 1) + 3, BLACK);
  BT.listen();
}
*/
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
  if (has_collision(newX, newY)) {
      return false;
  }
  return true;
}

bool has_collision(int x, int y) {
  if (x < 0 || x > 124 || y < 0 || y > 124) {
    return true;
  }
  for (int i = 1; i < snakeLength; i++) {
    if (snakeX.get(i) == x && snakeY.get(i) == y) {
      return true;
    }
  }
  return false;
}

bool apple_collision(int x1, int y1, int x2, int y2) {
  /*if ((y1 < (y2 + 4 - 1) && (y1 + 4 - 1) > y2) && (x1 < (x2 + 4 - 1) && (x1 + 4 - 1) > x2)) {
    if (dir == 1) {
      snakeX.add(0, snakeX.get(0));
      snakeY.add(0, snakeY.get(0) - 1);
    } else if (dir == 2) {
      snakeX.add(0, snakeX.get(0) + 1);
      snakeY.add(0, snakeY.get(0));
    } else if (dir == 3) {
      snakeX.add(0, snakeX.get(0));
      snakeY.add(0, snakeY.get(0) + 1);
    } else if (dir == 4) {
      snakeX.add(0, snakeX.get(0) - 1);
      snakeY.add(0, snakeY.get(0));
    }
    snakeLength++;
    return true;
  }*/
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

/*void refillApple() {
  // code does not display the new red apple when the line below is included, but does when it is commented out
  // player cannot move once collision occurs
  
  
  //Display.gfx_RectangleFilled(apple[0], apple[1], apple[0] + 3, apple[1] + 3, BLACK);
  appleEaten = false;
  int randX = random(128);
  int randY = random(128); 

  for (int i = 0; i < snakeLength; i++) {
    while ((randY < (snakeY.get(i) + 4 - 1) && (randY + 4 - 1) > snakeY.get(i)) && (randX < (snakeX.get(i) + 4 - 1) && (randX + 4 - 1) > snakeX.get(i))) {
      randX = random(128);
      randY = random(128);
      i = 0;
    }
  }
  apple[0] = randX;
  apple[1] = randY;
  Display.gfx_RectangleFilled(apple[0], apple[1], apple[0] + 3, apple[1] + 3, RED);
}*/

bool pixelCollision(int x1, int y1, int x2, int y2) {
  if ((y1 < (y2) && (y1) > y2) && (x1 < (x2) && (x1) > x2)) {
    return true;
  }
  return false;
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
/*void handleInputs(int data, bool up, bool right, bool down, bool left) {
  bool isBTMoving = data > 0 && data < 5;
  bool isBTColorChanging = data >= 5;
  bool isJSMoving = up || right || down || left;
  Serial.print("isBTMoving, isBTColorChanging, isJSMoving: ");
  Serial.print(isBTMoving);
  Serial.print(", ");
  Serial.print(isBTColorChanging);
  Serial.print(", ");
  Serial.println(isJSMoving);
  if (isBTMoving) {
    moveSquare(colorSelected, data - 1);
  } else if (isBTColorChanging) {
    colorSelected = data - 5;
  }
  if (isJSMoving) {
    handleJS(up, right, down, left);
  }
}

void handleJS(bool up, bool right, bool down, bool left) {
  if (up) {
      moveSquare(3, 0);
    }
    if (right) {
      moveSquare(3, 1);
    }
    if (down) {
      moveSquare(3, 2);
    }
    if (left) {
      moveSquare(3, 3);
    }
    // This delay slows down the frame rate and gives the BT a larger chance of being heard.
    delay(100);
}

void moveSquare(int color, int dir) {
  if (!is_valid(color, dir)) {
    return;
  }
  clear_square(color);
  if (dir == 0) { // up
    coords[color][1]-=2;
  } else if (dir == 1) { // right
    coords[color][0]+=2;
  } else if (dir == 2) { // down
    coords[color][1]+=2;
  } else if (dir == 3) { // left
    coords[color][0]-=2;
  }
  draw_square(color);
}

void draw_squares() {
  draw_square(2);
}

void draw_square(int color) {
  DisplaySerial.listen();
  if (color == 0) { // red
    Display.gfx_RectangleFilled(coords[0][0], coords[0][1], coords[0][0] + 5, coords[0][1] + 5, RED);
  } else if (color == 1) { // blue
    Display.gfx_RectangleFilled(coords[1][0], coords[1][1], coords[1][0] + 5, coords[1][1] + 5, BLUE);
  } else if (color == 3) { // orange
    Display.gfx_RectangleFilled(coords[2][0], coords[2][1], coords[2][0] + 5, coords[2][1] + 5, GREEN);
  } else if (color == 2) { // green
    Display.gfx_RectangleFilled(coords[3][0], coords[3][1], coords[3][0] + 5, coords[3][1] + 5, ORANGE);
  }
  BT.listen();
  // This delay gives the BT a bit more time to get inputs read.
  //delay(100);
}

void clear_square(int color) {
  DisplaySerial.listen();
  Display.gfx_RectangleFilled(coords[color][0], coords[color][1], coords[color][0] + 5, coords[color][1] + 5, BLACK);
  BT.listen();
}

bool is_valid(int color, int dir) {
  int newX = 0;
  int newY = 0; 
  if (dir == 0) { // up
    if (coords[color][1] == 0) {
      return false;
    }
    newX = coords[color][0];
    newY = coords[color][1] - 1;
  } else if (dir == 1) { // right
    if (coords[color][0] == 122) {
      return false;
    }
    newX = coords[color][0] + 1;
    newY = coords[color][1];
  } else if (dir == 2) { // down
    if (coords[color][1] == 122) {
      return false;
    }
    newX = coords[color][0];
    newY = coords[color][1] + 1;
  } else if (dir == 3) { // left
    if (coords[color][0] == 0) {
      return false;
    }
    newX = coords[color][0] - 1;
    newY = coords[color][1];
  }
  if (has_collision(color, newX, newY)) {
      return false;
  }
  return true;
}

bool has_collision(int color, int x, int y) {
  for (int i = 0; i < 4; i++) {
    if (i == color) {
      continue;
    }
    if (abs(coords[i][0] - x) < 6 && abs(coords[i][1] - y) < 6) {
      return true;
    }
  }
  return false;
}*/

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
