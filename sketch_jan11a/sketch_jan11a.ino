#include <Wire.h>
#include <TimerOne.h>
#include <LiquidCrystal_I2C.h>
const int buttonUp = 25;
const int buttonDown = 24;
const int buttonLeft = 22;
const int buttonRight = 23;
const int buttonRestart = 26;
bool reset = false;
volatile bool restartTriggered = false;  // Indicator pentru restart

#define       NUM_KEYS                       5
#define       ROWS                           2  // number of indicator rows
#define       levels                         4  // difficulty levels
#define       MAX_SNAKE_LENGTH              25  // maximum length of snake


LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16x2 LCD


unsigned long time, timeNow;

int gameSpeed;

int pcont = A3;
int mcr = 10;

boolean skip, gameOver, gameStarted;

int olddir;
int selectedLevel;

byte key = 0;

byte oldkey = 0;

byte Field[8 * ROWS * 16];

struct partdef  {
  int row, column, dir;
  struct partdef *next;
};

typedef partdef part;

part *head, *tail;

int collected;

unsigned int pc, pr;



// display on the screen
void drawMatrix() {
  boolean levelz[levels][4][16] = {

    { {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},

      {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},

      {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},

      {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}
    },

    { {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true},

      {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true},

      {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true},

      {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true}
    },

    { {true, false, false, false, true, false, false, false, false, false, false, true, false, false, false, true},

      {true, false, false, false, false, false, false, false, true, false, false, false, false, false, false, true},

      {true, false, false, false, true, false, false, false, false, false, false, true, false, false, false, true},

      {true, false, false, false, false, false, false, false, true, false, false, false, false, false, false, true}
    },

    { {true, false, true, false, false, false, false, false, false, true, false, false, false, true, false, false},

      {false, false, false, false, true, false, false, true, false, false, false, true, false, false, false, true},

      {true, false, true, false, false, false, false, false, false, true, false, false, false, true, false, false},

      {false, false, false, false, true, false, false, true, false, false, false, true, false, false, false, true}
    }

  };

  byte myChar[8];
  boolean special;
  int cc = 0;

  if (!gameOver)  {
    ChangeDot(pr, pc, true);

    for (int r = 0; r < ROWS; r++) {
      for (int c = 0; c < 16; c++) {
        special = false;
        for (int i = 0; i < 8; i++)  {
          byte b = B00000;

          if ((Field[16 * (r * 8 + i) + (c * 5 + 0) / 5] & (1 << ((c * 5 + 0) % 5))) >> ((c * 5 + 0) % 5)) {
            b += B10000;
            special = true;
          }

          if ((Field[16 * (r * 8 + i) + (c * 5 + 1) / 5] & (1 << ((c * 5 + 1) % 5))) >> ((c * 5 + 1) % 5)) {
            b += B01000;
            special = true;
          }

          if ((Field[16 * (r * 8 + i) + (c * 5 + 2) / 5] & (1 << ((c * 5 + 2) % 5))) >> ((c * 5 + 2) % 5)) {
            b += B00100;
            special = true;
          }

          if ((Field[16 * (r * 8 + i) + (c * 5 + 3) / 5] & (1 << ((c * 5 + 3) % 5))) >> ((c * 5 + 3) % 5)) {
            b += B00010;
            special = true;
          }

          if ((Field[16 * (r * 8 + i) + (c * 5 + 4) / 5] & (1 << ((c * 5 + 4) % 5))) >> ((c * 5 + 4) % 5)) {
            b += B00001;
            special = true;
          }

          myChar[i] = b;
        }

        if (special)  {
          lcd.createChar(cc, myChar);
          lcd.setCursor(c, r);
          lcd.write(byte(cc));

          cc++;

        }

        else  {

          lcd.setCursor(c, r);

          if (levelz[selectedLevel][r][c]) lcd.write(255);

          else lcd.write(128);

        }

      }

    }

  }

}

//----------------------------------

void freeList()

{

  part *p, *q;

  p = tail;

  while (p != NULL)

  {

    q = p;

    p = p->next;

    free(q);

  }

  head = tail = NULL;

}

//----------------------------------

void gameOverFunction()

{

  delay(1000);

  lcd.clear();

  freeList();

  lcd.setCursor(3, 0);

  lcd.print("Game Over!");

  lcd.setCursor(4, 1);

  lcd.print("Score: ");

  lcd.print(collected);

  delay(3000);

}

//----------------------------------

void growSnake()

{

  part *p;

  p = (part*)malloc(sizeof(part));

  p->row = tail->row;

  p->column = tail->column;

  p->dir = tail->dir;

  p->next = tail;

  tail = p;

}

//----------------------------------

// a new dot on the screen

void newPoint()

{

  boolean levelz[levels][4][16] = {

    { {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},

      {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},

      {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},

      {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}
    },


    { {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true},

      {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true},

      {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true},

      {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true}
    },


    { {true, false, false, false, true, false, false, false, false, false, false, true, false, false, false, true},

      {true, false, false, false, false, false, false, false, true, false, false, false, false, false, false, true},

      {true, false, false, false, true, false, false, false, false, false, false, true, false, false, false, true},

      {true, false, false, false, false, false, false, false, true, false, false, false, false, false, false, true}
    },


    { {true, false, true, false, false, false, false, false, false, true, false, false, false, true, false, false},

      {false, false, false, false, true, false, false, true, false, false, false, true, false, false, false, true},

      {true, false, true, false, false, false, false, false, false, true, false, false, false, true, false, false},

      {false, false, false, false, true, false, false, true, false, false, false, true, false, false, false, true}
    }

  };




  part *p;

  p = tail;

  boolean newp = true;

  while (newp)

  {

    pr = random(8 * ROWS);

    pc = random(80);

    newp = false;

    if (levelz[selectedLevel][pr / 8][pc / 5]) newp = true;

    while (p->next != NULL && !newp)

    {

      if (p->row == pr && p->column == pc) newp = true;

      p = p->next;

    }

  }

  if (collected < MAX_SNAKE_LENGTH && gameStarted) growSnake();
}


// head movement

void moveHead()

{

  boolean levelz[levels][4][16] = {

    { {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},

      {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},

      {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},

      {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}
    },


    { {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true},

      {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true},

      {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true},

      {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true}
    },


    { {true, false, false, false, true, false, false, false, false, false, false, true, false, false, false, true},

      {true, false, false, false, false, false, false, false, true, false, false, false, false, false, false, true},

      {true, false, false, false, true, false, false, false, false, false, false, true, false, false, false, true},

      {true, false, false, false, false, false, false, false, true, false, false, false, false, false, false, true}
    },


    { {true, false, true, false, false, false, false, false, false, true, false, false, false, true, false, false},

      {false, false, false, false, true, false, false, true, false, false, false, true, false, false, false, true},

      {true, false, true, false, false, false, false, false, false, true, false, false, false, true, false, false},

      {false, false, false, false, true, false, false, true, false, false, false, true, false, false, false, true}
    }

  };

  switch (head->dir) // 1 step in direction

  {

    case 0: head->row--; break;

    case 1: head->row++; break;

    case 2: head->column++; break;

    case 3: head->column--; break;

    default : break;

  }

  if (head->column >= 80) head->column = 0;

  if (head->column < 0) head->column = 79;

  if (head->row >= (8 * ROWS)) head->row = 0;

  if (head->row < 0) head->row = (8 * ROWS - 1);



  if (levelz[selectedLevel][head->row / 8][head->column / 5]) gameOver = true; // wall collision check



  part *p;

  p = tail;

  while (p != head && !gameOver) // self collision

  {

    if (p->row == head->row && p->column == head->column) gameOver = true;

    p = p->next;

  }

  if (gameOver)

    gameOverFunction();

  else

  {

    ChangeDot(head->row, head->column, true);



    if (head->row == pr && head->column == pc) // point pickup check

    {

      collected++;

      if (gameSpeed < 25) gameSpeed += 3;

      newPoint();

    }

  }

}

//----------------------------------

// funny move

void moveAll()

{

  part *p;

  p = tail;


  ChangeDot(p->row, p->column, false);



  while (p->next != NULL)

  {

    p->row = p->next->row;

    p->column = p->next->column;

    p->dir = p->next->dir;

    p = p->next;

  }

  moveHead();

}

//----------------------------------


// create a snake

void createSnake(int n) // n = size of snake

{

  for (unsigned int i = 0; i < (8 * ROWS * 16); i++)

    Field[i] = 0;



  part *p, *q;

  tail = (part*)malloc(sizeof(part));

  tail->row = 7;

  tail->column = 39 + n / 2;

  tail->dir = 3;

  q = tail;



  ChangeDot(tail->row, tail->column, true);



  for (int i = 0; i < n - 1; i++) // build snake from tail to head

  {

    p = (part*)malloc(sizeof(part));

    p->row = q->row;

    p->column = q->column - 1; //initial snake id placed horizoltally



    ChangeDot(p->row, p->column, true);



    p->dir = q->dir;

    q->next = p;

    q = p;

  }

  if (n > 1)

  {

    p->next = NULL;

    head  = p;

  }

  else

  {

    tail->next = NULL;

    head = tail;

  }

}


void startF()

{

  byte mySnake[8][8] =

  {

    { B00000,

      B00000,

      B00011,

      B00110,

      B01100,

      B11000,

      B00000,

    },

    { B00000,

      B11000,

      B11110,

      B00011,

      B00001,

      B00000,

      B00000,

    },

    { B00000,

      B00000,

      B00000,

      B00000,

      B00000,

      B11111,

      B01110,

    },

    { B00000,

      B00000,

      B00011,

      B01111,

      B11000,

      B00000,

      B00000,

    },

    { B00000,

      B11100,

      B11111,

      B00001,

      B00000,

      B00000,

      B00000,

    },

    { B00000,

      B00000,

      B00000,

      B11000,

      B01101,

      B00111,

      B00000,

    },

    { B00000,

      B00000,

      B01110,

      B11011,

      B11111,

      B01110,

      B00000,

    },

    { B00000,

      B00000,

      B00000,

      B01000,

      B10000,

      B01000,

      B00000,

    }

  };



  gameOver = false;

  gameStarted = false;

  selectedLevel = 1;



  lcd.clear();

 lcd.setCursor(0, 0);
lcd.print("     SNAKE!");  // Display "SNAKE!" on the first line

for (int i = 0; i < 8; i++) {
  lcd.createChar(i, mySnake[i]);     // Create custom characters
  lcd.setCursor(i + 4, 1);          // Position custom characters on the second line
  lcd.write(byte(i));               // Write custom characters
}




  collected = 0;

  gameSpeed = 1;       // increases the speed of the snake in the game  // the game speed right now is 100 but when you start at first make the speed to 10 or 20 and it can up to 1000 // but would advise playing gthe game in slower speed as the speed of the snake gradually increases.
  createSnake(5);         // number of body parts the snake starts with

  time = 0;

  delay(3000);    // delay before the game starts after setting the speed


}

//----------------------------------

void setup()  {
  Serial.begin(9600);

    pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
  pinMode(buttonLeft, INPUT_PULLUP);
  pinMode(buttonRight, INPUT_PULLUP);


   // Timer1.initialize(100000);  // Configurează Timer1 să declanșeze la fiecare 100ms
  //Timer1.attachInterrupt(handleRestart);  // Atribuie funcția de întrerupere

   // attachInterrupt(digitalPinToInterrupt(buttonRestart), onRestartPress, FALLING);


  lcd.init(); // Initialize the LCD
  lcd.backlight(); // Turn on the LCD backlight
  startF();
}


//----------------------------------

void restartGame() {
  // Clear the LCD display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Restarting...");

  delay(2000); // Optional: delay for user feedback

  // Reset game state
  freeList();           // Free the snake's memory
  collected = 0;        // Reset collected points
  gameSpeed = 10;       // Reset speed to initial value
  gameOver = false;     // Clear game over state
  gameStarted = false;  // Reset game start flag

  // Restart the game
  startF();             // Reinitialize the game
}


// making changes to matrix of points on eran

void ChangeDot(unsigned int RowVal, unsigned int ColVal, boolean NewVal)
{
  // Calculating the index of the Field array based on the row and column
  unsigned int index = 16 * RowVal + ColVal / 5;

  // If we want to turn on the dot, we set the corresponding bit to 1
  if (NewVal) {
    Field[index] |= (1 << (ColVal % 5));
  }
  // If we want to turn off the dot, we set the corresponding bit to 0
  else {
    Field[index] &= ~(1 << (ColVal % 5));
  }
}

void loop() {
  static unsigned long lastUpdate = 0;

  // Ensure the game starts properly
  if (!gameStarted) {
    start:
    reset=false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Starting Game...");
    delay(2000);
    gameStarted = true;
    createSnake(5);  // Initialize snake
    newPoint();  // Place the first point
  }

  // Periodically update the game
  if (millis() - lastUpdate > (1000 / (gameSpeed + 1))) {  // Control speed
    lastUpdate = millis();
    moveAll();  // Move the snake
    drawMatrix();  // Refresh the LCD
  }

  // Handle button inputs for direction change
  if (digitalRead(buttonUp) == LOW && head->dir != 1) {
    head->dir = 0;  // Move up
  } else if (digitalRead(buttonDown) == LOW && head->dir != 0) {
    head->dir = 1;  // Move down
  } else if (digitalRead(buttonLeft) == LOW && head->dir != 2) {
    head->dir = 3;  // Move left
  } else if (digitalRead(buttonRight) == LOW && head->dir != 3) {
    head->dir = 2;  // Move right
  }

}





