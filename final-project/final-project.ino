#include <Adafruit_NeoPixel.h>
#include <string.h>
#include <MemoryFree.h>

#define RTRIGGER_BTN 8
#define LTRIGGER_BTN 9
#define RIGHT_BTN 10
#define LEFT_BTN 11
#define TOGGLE 12
#define POWER 13


#define PIN 6
#define NUM_LEDS 512
#define NUM_ROWS 32
#define NUM_COLS 16
#define BRIGHTNESS 5 // to reduce current for 256 NeoPixels

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);
int LED_INDEXING[NUM_ROWS][NUM_COLS];
int OCCUPIED[NUM_ROWS][NUM_COLS];
int TESTING = false;
int SHAPE_SIZE = 6; // 2 values for shape and state; 4 values for shape indices
int RIGHT_ROTATE = 1;
int LEFT_ROTATE = 2;

int DROP_TIME = 50;

// commonly used colors
uint32_t RED = strip.Color(255, 0, 0);
uint32_t GREEN = strip.Color(0, 255, 0);
uint32_t BLUE = strip.Color(0, 0, 255);

// NOTE: COLORS[0] is just a filler
uint32_t* COLORS = new uint32_t[8] {strip.Color(0, 0, 0), strip.Color(255, 0 , 0), strip.Color(255, 127, 0), strip.Color(255, 255, 0), strip.Color(0, 255, 0), strip.Color(0, 0, 255), strip.Color(75, 0, 130), strip.Color(148, 0, 211)};
int COLORS_SIZE = 8;

// declaring generating function prototypes
int* genI();
int* genZ();
int* genS();
int* genJ();
int* genL();
int* genT();
int* genO();

int* (*GENERATE_SHAPE[])() = {genI, genZ, genS, genJ, genL, genT, genO};
int GENERATE_SHAPE_SIZE = 7;

void setup() {
  Serial.begin(9600);
  create_led_indexing();
  
  strip.begin();
  strip.setBrightness(BRIGHTNESS);

  pinMode(RTRIGGER_BTN, INPUT_PULLUP);
  pinMode(LTRIGGER_BTN, INPUT_PULLUP);
  pinMode(LEFT_BTN, INPUT_PULLUP);
  pinMode(RIGHT_BTN, INPUT_PULLUP);
  pinMode(TOGGLE, INPUT_PULLUP);
  pinMode(POWER, INPUT_PULLUP);
}

void loop() {
  // chase(strip.Color(255, 0, 0)); // Red
  // strip.setPixelColor(LED_INDEXING[0][6], strip.Color(0, 255, 0));
  // strip.show();

  // int free_memory = freeMemory();
  // Serial.print("Free memory BEFORE: ");
  // Serial.println(free_memory);

  int randIdx = random(GENERATE_SHAPE_SIZE);
  int* shapeArray = GENERATE_SHAPE[randIdx]();

  for (int i = 2; i < 6; i++) {
    shapeArray[i] += 4;
  }
  
  int colorIdx = random(1, COLORS_SIZE);
  drop(shapeArray, SHAPE_SIZE, colorIdx);

  delete[] shapeArray;
  clearRows();

  // int free_memory1 = freeMemory();
  // Serial.print("Free memory after: ");
  // Serial.println(free_memory1);
  
  delay(500);
}

void turn_on_idxs(int* shape_idxs, int idxs_size, uint32_t color) {
  for (int i = 2; i < idxs_size; i++) {
    int* rc = pos_to_idx(shape_idxs[i]);
    int r = rc[0], c = rc[1];
    strip.setPixelColor(LED_INDEXING[r][c], color);
    delete[] rc;
  }
  strip.show();
}

void turn_off_idxs(int* idxs, int idxs_size) {
  for (int i = 2; i < idxs_size; i++) {
    int* rc = pos_to_idx(idxs[i]);
    int r = rc[0], c = rc[1];
    strip.setPixelColor(LED_INDEXING[r][c], 0);
    delete[] rc;
  }
  strip.show();
}

int* getNextDownPos(int* pos, int size) {
  int* newPos = new int[size];
  for (int i = 2; i < size; i++) {
    newPos[i] = pos[i] + 16;
  }
  return newPos;
}

int* getNextLeftPos(int* pos, int size) {
  int* newPos = new int[size];
  for (int i = 2; i < size; i++) {
    newPos[i] = pos[i] - 1;
  }
  return newPos;
}

int* getNextRightPos(int* pos, int size) {
  int* newPos = new int[size];
  for (int i = 2; i < size; i++) {
    newPos[i] = pos[i] + 1;
  }
  return newPos;
}

bool isNextACollision(int* pos, int size) {
  for (int i = size - 1; i >= 2; i--) {
    int* rc = pos_to_idx(pos[i]);
    int row = rc[0], col = rc[1];
    delete[] rc;
    if (OCCUPIED[row][col] != 0) {
      return true;
    } else if (row >= NUM_ROWS) {
      return true;
    }
  }
  return false;
}

bool wallCollisionLeft(int* pos, int size) {
  for (int i = 2; i < size; i++) {
    int* rc = pos_to_idx(pos[i]);
    int row = rc[0], col = rc[1];
    delete[] rc;
    if (OCCUPIED[row][col] == 1) {
      return true;
    } else if(col % 16 == 15) {
      return true;
    }
  }
  return false;
}

bool wallCollisionRight(int* pos, int size) {
  for (int i = 2; i < size; i++) {
    int* rc = pos_to_idx(pos[i]);
    int row = rc[0], col = rc[1];
    delete[] rc;
    if (OCCUPIED[row][col] == 1) {
      return true;
    } else if(col % 16 == 0) {
      return true;
    }
  }
  return false;
}

// isTogether = true means the rotation is valid
bool isTogether(int* pos, int size) {
  bool together = true;
  int* rc1 = pos_to_idx(pos[2]);
  int* rc2 = pos_to_idx(pos[3]);
  int* rc3 = pos_to_idx(pos[4]);
  int* rc4 = pos_to_idx(pos[5]);
  int row1 = rc1[0], col1 = rc1[1];
  int row2 = rc2[0], col2 = rc2[1];
  int row3 = rc3[0], col3 = rc3[1];
  int row4 = rc4[0], col4 = rc4[1];
  delete[] rc1;
  delete[] rc2;
  delete[] rc3;
  delete[] rc4;
  if (pos[0] == 5) { //t shape
    if (abs((row2-row1) + (col2-col1)) != 1) {
      together = false;
    }
    if (abs((row2-row3) + (col2-col3)) != 1) {
      together = false;
    }
    if (abs((row2-row4) + (col2-col4)) != 1) {
      together = false;
    }
  } else if (pos[0] == 7) { //o shape
    return true;
  } else { //all other shapes
    if (abs((row1-row2) + (col1-col2)) != 1) {
      together = false;
    }
    if (abs((row2-row3) + (col2-col3)) != 1) {
      together = false;
    }
    if (abs((row3-row4) + (col3-col4)) != 1) {
      together = false;
    }
  }
  return together;
}

void clearRows() {
  int counter = 0;
  for (int row = NUM_ROWS - 1; row >= 0; row--) {
    bool isFull = true;
    for (int col = 0; col < NUM_COLS; col++) {
      if (OCCUPIED[row][col] == 0) {
        isFull = false;
        break;
      }
    }
    if (isFull) {
      for (int col = 0; col < NUM_COLS; col++) {
        strip.setPixelColor(LED_INDEXING[row][col], 0);
      }
      strip.show();
      delay(500);
      counter++;
    } else {
      for (int col = 0; col < NUM_COLS; col++) {
        OCCUPIED[row+counter][col] = OCCUPIED[row][col];
        if (OCCUPIED[row][col] != 0 && counter != 0) {
          strip.setPixelColor(LED_INDEXING[row+counter][col], COLORS[OCCUPIED[row][col]]);
          strip.setPixelColor(LED_INDEXING[row][col], 0);
        }
      }
    }
  }
  strip.show();
}

void drop(int* curr_idxs, int size, int colorIdx) {
  int time_since_last_drop = millis();

  uint32_t color = COLORS[colorIdx];

  while (true) {
    if (digitalRead(RTRIGGER_BTN) == LOW) {
      delay(150);
      rotate(curr_idxs, size, color, RIGHT_ROTATE);
    }
    else if (digitalRead(LTRIGGER_BTN) == LOW) {
      delay(150);
      rotate(curr_idxs, size, color, LEFT_ROTATE);
    }
    else if (digitalRead(RIGHT_BTN) == LOW) {
      delay(125);
      moveRight(curr_idxs, size, color);
    }
    else if (digitalRead(LEFT_BTN) == LOW) {
      delay(125);
      moveLeft(curr_idxs, size, color);
    }
    else if (digitalRead(TOGGLE) == LOW) {
      strip.clear();
      for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {
          OCCUPIED[i][j] = 0;
        }
      }
      return;
    }
    else if (digitalRead(POWER) == LOW) {
      while (digitalRead(POWER) == LOW) {
        if (digitalRead(POWER) == HIGH) break;
      }
    }

    // // IF TIME SINCE LAST BLOCK DROP less than DROP_TIME
    if (millis() - time_since_last_drop < DROP_TIME) {
      continue;
    } else {
      time_since_last_drop = millis();
    }

    int* next_idxs = getNextDownPos(curr_idxs, size);

    bool isCollision = isNextACollision(next_idxs, size);

    if (isCollision) { // if moving results in collision, don't move, set OCCUPIED with current locations
      for (int i = 2; i < size; i++) {
        int* rc = pos_to_idx(curr_idxs[i]);
        int row = rc[0], col = rc[1];
        delete[] rc;
        OCCUPIED[row][col] = colorIdx;
      }
      break;
    } else {
      turn_off_idxs(curr_idxs, SHAPE_SIZE);
      turn_on_idxs(next_idxs, SHAPE_SIZE, color);
    }
    
    for (int i = 2; i < SHAPE_SIZE; i++) {
      curr_idxs[i] = next_idxs[i];
    }
    delete[] next_idxs;

    // delay(250); // TODO: CHANGE DELAY TO MILLIS (a non blocking solution so we can still take input)
  }
}

void moveLeft(int* curr_idxs, int size, uint32_t color) {
  int* next_idxs = getNextLeftPos(curr_idxs, size);
  bool isCollision = wallCollisionLeft(next_idxs, size) || isNextACollision(next_idxs, size);
  if (!isCollision) {
    turn_off_idxs(curr_idxs, SHAPE_SIZE);
    turn_on_idxs(next_idxs, SHAPE_SIZE, color);
    for (int i = 2; i < SHAPE_SIZE; i++) {
      curr_idxs[i] = next_idxs[i];
    }
  }
  delete[] next_idxs;
}

void moveRight(int* curr_idxs, int size, uint32_t color) {
  int* next_idxs = getNextRightPos(curr_idxs, size);
  bool isCollision = wallCollisionRight(next_idxs, size) || isNextACollision(next_idxs, size);
  if (!isCollision) {
    turn_off_idxs(curr_idxs, SHAPE_SIZE);
    turn_on_idxs(next_idxs, SHAPE_SIZE, color);
    for (int i = 2; i < SHAPE_SIZE; i++) {
      curr_idxs[i] = next_idxs[i];
    }
  }
  delete[] next_idxs;
}

void rotate(int* curr_idxs, int size, uint32_t color, int dir) {
  int* next_idxs = new int[size];
  if (curr_idxs[0] == 1) {
    next_idxs = getIRotatePos(curr_idxs, size, dir);
  } else if (curr_idxs[0] == 2) {
    next_idxs = getJRotatePos(curr_idxs, size, dir);
  } else if (curr_idxs[0] == 3) {
    next_idxs = getLRotatePos(curr_idxs, size, dir);
  } else if (curr_idxs[0] == 4) {
    next_idxs = getSRotatePos(curr_idxs, size, dir);
  } else if (curr_idxs[0] == 5) {
    next_idxs = getTRotatePos(curr_idxs, size, dir);
  } else if (curr_idxs[0] == 6) {
    next_idxs = getZRotatePos(curr_idxs, size, dir);
  } else {
    next_idxs = getORotatePos(curr_idxs, size, dir);
  }
  bool isCollision = isNextACollision(next_idxs, size) || !isTogether(next_idxs, size);
  if (!isCollision) {
    turn_off_idxs(curr_idxs, SHAPE_SIZE);
    turn_on_idxs(next_idxs, SHAPE_SIZE, color);
    for (int i = 0; i < SHAPE_SIZE; i++) {
      curr_idxs[i] = next_idxs[i];
    }
  }
  delete[] next_idxs;
}

int* getIRotatePos(int* pos, int size, int dir) {
  int* nPos = new int[size];
  nPos[0] = pos[0];
  if (pos[1] == 1) {
    nPos[1] = 2; //change state variable    
    nPos[2] = pos[2] - 30;
    nPos[3] = pos[3] - 15;
    nPos[4] = pos[4];
    nPos[5] = pos[5] + 15;
  } else {
    nPos[1] = 1; //change state variable
    nPos[2] = pos[2] + 30;
    nPos[3] = pos[3] + 15;
    nPos[4] = pos[4];
    nPos[5] = pos[5] - 15;
  }
  return nPos;
}

int* getJRotatePos(int* pos, int size, int dir) {
  int* nPos = new int[size];
  nPos[0] = pos[0];
  if (dir == RIGHT_ROTATE) {
    if (pos[1] == 1) {
      nPos[1] = 2;
      nPos[2] = pos[2] - 15;
      nPos[3] = pos[3];
      nPos[4] = pos[4] + 15;
      nPos[5] = pos[5] - 2;
    } else if (pos[1] == 2) {
      nPos[1] = 3;
      nPos[2] = pos[2] + 17;
      nPos[3] = pos[3];
      nPos[4] = pos[4] - 17;
      nPos[5] = pos[5] - 32;
    } else if (pos[1] == 3) {
      nPos[1] = 4;
      nPos[2] = pos[2] + 15;
      nPos[3] = pos[3];
      nPos[4] = pos[4] - 15;
      nPos[5] = pos[5] + 2;
    } else {
      nPos[1] = 1;
      nPos[2] = pos[2] - 17;
      nPos[3] = pos[3];
      nPos[4] = pos[4] + 17;
      nPos[5] = pos[5] + 32;
    }
  } else {
    if (pos[1] == 1) {
      nPos[1] = 4;
      nPos[2] = pos[2] + 17;
      nPos[3] = pos[3];
      nPos[4] = pos[4] - 17;
      nPos[5] = pos[5] - 32;
    } else if (pos[1] == 2) {
      nPos[1] = 1;
      nPos[2] = pos[2] + 15;
      nPos[3] = pos[3];
      nPos[4] = pos[4] - 15;
      nPos[5] = pos[5] + 2;
    } else if (pos[1] == 3) {
      nPos[1] = 2;
      nPos[2] = pos[2] - 17;
      nPos[3] = pos[3];
      nPos[4] = pos[4] + 17;
      nPos[5] = pos[5] + 32;
    } else {
      nPos[1] = 3;
      nPos[2] = pos[2] - 15;
      nPos[3] = pos[3];
      nPos[4] = pos[4] + 15;
      nPos[5] = pos[5] - 2;
    }
  }
  return nPos;
}

int* getLRotatePos(int* pos, int size, int dir) {
  int* nPos = new int[size];
  nPos[0] = pos[0];
  if (dir == RIGHT_ROTATE) {
    if (pos[1] == 1) {
      nPos[1] = 2;
      nPos[2] = pos[2] + 15;
      nPos[3] = pos[3];
      nPos[4] = pos[4] - 15;
      nPos[5] = pos[5] - 32;
    } else if (pos[1] == 2) {
      nPos[1] = 3;
      nPos[2] = pos[2] - 17;
      nPos[3] = pos[3];
      nPos[4] = pos[4] + 17;
      nPos[5] = pos[5] + 2;
    } else if (pos[1] == 3) {
      nPos[1] = 4;
      nPos[2] = pos[2] - 15;
      nPos[3] = pos[3];
      nPos[4] = pos[4] + 15;
      nPos[5] = pos[5] + 32;
    } else {
      nPos[1] = 1;
      nPos[2] = pos[2] + 17;
      nPos[3] = pos[3];
      nPos[4] = pos[4] - 17;
      nPos[5] = pos[5] - 2;
    }
  } else {
    if (pos[1] == 1) {
      nPos[1] = 4;
      nPos[2] = pos[2] - 17;
      nPos[3] = pos[3];
      nPos[4] = pos[4] + 17;
      nPos[5] = pos[5] + 2;
    } else if (pos[1] == 2) {
      nPos[1] = 1;
      nPos[2] = pos[2] - 15;
      nPos[3] = pos[3];
      nPos[4] = pos[4] + 15;
      nPos[5] = pos[5] + 32;
    } else if (pos[1] == 3) {
      nPos[1] = 2;
      nPos[2] = pos[2] + 17;
      nPos[3] = pos[3];
      nPos[4] = pos[4] - 17;
      nPos[5] = pos[5] - 2;
    } else {
      nPos[1] = 3;
      nPos[2] = pos[2] + 15;
      nPos[3] = pos[3];
      nPos[4] = pos[4] - 15;
      nPos[5] = pos[5] - 32;
    }
  }
  return nPos;
}

int* getSRotatePos(int* pos, int size, int dir) {
  int* nPos = new int[size];
  nPos[0] = pos[0];
  if (pos[1] == 1) {
    nPos[1] = 2; //change state variable    
    nPos[2] = pos[2] - 31;
    nPos[3] = pos[3] - 16;
    nPos[4] = pos[4] + 1;
    nPos[5] = pos[5] + 16;
  } else {
    nPos[1] = 1; //change state variable
    nPos[2] = pos[2] + 31;
    nPos[3] = pos[3] + 16;
    nPos[4] = pos[4] - 1;
    nPos[5] = pos[5] - 16;
  }  
  return nPos;
}

int* getTRotatePos(int* pos, int size, int dir) {
  int* nPos = new int[size];
  nPos[0] = pos[0];
  if (dir == RIGHT_ROTATE) {
    if (pos[1] == 1) {
      nPos[1] = 2;
      nPos[2] = pos[2] - 15;
      nPos[3] = pos[3];
      nPos[4] = pos[4] + 15;
      nPos[5] = pos[5] - 17;
    } else if (pos[1] == 2) {
      nPos[1] = 3;
      nPos[2] = pos[2] + 17;
      nPos[3] = pos[3];
      nPos[4] = pos[4] - 17;
      nPos[5] = pos[5] - 15;
    } else if (pos[1] == 3) {
      nPos[1] = 4;
      nPos[2] = pos[2] + 15;
      nPos[3] = pos[3];
      nPos[4] = pos[4] - 15;
      nPos[5] = pos[5] + 17;
    } else {
      nPos[1] = 1;
      nPos[2] = pos[2] - 17;
      nPos[3] = pos[3];
      nPos[4] = pos[4] + 17;
      nPos[5] = pos[5] + 15;
    }
  } else {
    if (pos[1] == 1) {
      nPos[1] = 4;
      nPos[2] = pos[2] + 17;
      nPos[3] = pos[3];
      nPos[4] = pos[4] - 17;
      nPos[5] = pos[5] - 15;
    } else if (pos[1] == 2) {
      nPos[1] = 1;
      nPos[2] = pos[2] + 15;
      nPos[3] = pos[3];
      nPos[4] = pos[4] - 15;
      nPos[5] = pos[5] + 17;
    } else if (pos[1] == 3) {
      nPos[1] = 2;
      nPos[2] = pos[2] - 17;
      nPos[3] = pos[3];
      nPos[4] = pos[4] + 17;
      nPos[5] = pos[5] + 15;
    } else {
      nPos[1] = 3;
      nPos[2] = pos[2] - 15;
      nPos[3] = pos[3];
      nPos[4] = pos[4] + 15;
      nPos[5] = pos[5] - 17;
    }
  }
  return nPos;
}

int* getZRotatePos(int* pos, int size, int dir) {
  int* nPos = new int[size];
  nPos[0] = pos[0];
  if (pos[1] == 1) {
    nPos[1] = 2; //change state variable    
    nPos[2] = pos[2] - 14;
    nPos[3] = pos[3] + 1;
    nPos[4] = pos[4] - 16;
    nPos[5] = pos[5] - 1;
  } else {
    nPos[1] = 1; //change state variable
    nPos[2] = pos[2] + 14;
    nPos[3] = pos[3] - 1;
    nPos[4] = pos[4] + 16;
    nPos[5] = pos[5] + 1;
  }
  return nPos;
}

int* getORotatePos(int* pos, int size, int dir) {
  int* nPos = new int[size];
  for (int i = 0; i < SHAPE_SIZE; i++) {
    nPos[i] = pos[i];
  }
  return nPos;
}

void create_led_indexing() {
  int leds_per_strip = 256;
  int step_size = 8;

  int counter = 0;
  int led_idx_i = 0;
  int led_idx_j = 0;

  // column 1
  for (int i = leds_per_strip - step_size; i >= 0; i -= step_size) {
    if (counter % 2 == 1) {
      for (int ii = i; ii < i + 8; ii++) {
        LED_INDEXING[led_idx_i][led_idx_j] = ii;
        led_idx_j++;
      }
      led_idx_i++;
      led_idx_j = 0;
    } else {
      for (int ii = i + 7; ii >= i; ii--) {
        LED_INDEXING[led_idx_i][led_idx_j] = ii;
        led_idx_j++;
      }
      led_idx_i++;
      led_idx_j = 0;
    }
    counter++;
  }

  counter = 0;
  led_idx_i = 0;
  led_idx_j = 8;

  // column 2
  for (int i = leds_per_strip; i < leds_per_strip * 2; i += step_size) {
    if (counter % 2 == 1) {
      for (int ii = i; ii < i + 8; ii++) {
        LED_INDEXING[led_idx_i][led_idx_j] = ii;
        led_idx_j++;
      }
      led_idx_i++;
      led_idx_j = 8;
    } else {
      for (int ii = i + 7; ii >= i; ii--) {
        LED_INDEXING[led_idx_i][led_idx_j] = ii;
        led_idx_j++;
      }
      led_idx_i++;
      led_idx_j = 8;
    }
    counter++;
  }
}

void print_led_indexing() {
  // Determine the number of rows and columns in the array
  int numRows = sizeof(LED_INDEXING) / sizeof(LED_INDEXING[0]);
  int numCols = sizeof(LED_INDEXING[0]) / sizeof(LED_INDEXING[0][0]);

  // Iterate over the array using the determined ranges
  for (int i = 0; i < numRows; i++) {
    for (int j = 0; j < numCols; j++) {
      // Access the element at row i and column j
      int element = LED_INDEXING[i][j];
      Serial.print(element);
      Serial.print(",");
    }
    Serial.println();
  }
}

// given the wack position in array (ex. value from 0 to 511)
// convert the value to the actual position (row, col) to light up using array indexing
int* pos_to_idx(int pos) {
  // int row = (pos % NUM_LEDS) / NUM_COLS;
  // int col = (pos % NUM_LEDS) % NUM_COLS;
  int row = pos / NUM_COLS;
  int col = pos % NUM_COLS;
  return new int[2] {row, col};
}

int* genI() {
  int* idxs = new int[6] {1, 1, 0, 1, 2, 3};
  return idxs;
}

int* genJ() {
  int* idxs = new int[6] {2, 3, 18, 17, 16, 0};
  return idxs;
}

int* genL() {
  int* idxs = new int[6] {3, 3, 16, 17, 18, 2};
  return idxs;
}

int* genS() {
  int* idxs = new int[6] {4, 1, 16, 17, 1, 2};
  return idxs;
}

int* genT() {
  int* idxs = new int[6] {5, 3, 18, 17, 16, 1};
  return idxs;
}

int* genZ() {
  int* idxs = new int[6] {6, 1, 0, 1, 17, 18};
  return idxs;
}

int* genO() {
  int* idxs = new int[6] {7, 1, 0, 1, 16, 17};
  return idxs;
}

// void copyArray(int* source, int* destination, int size) {
//   memcpy(destination, source, size * sizeof(int));
// }

// other methods
void chase(uint32_t c) {
  Serial.println("chasing!");
  for(uint16_t i = 0; i < strip.numPixels(); i++) {
    int row = (i % NUM_LEDS) / NUM_COLS;
    int col = (i % NUM_LEDS) % NUM_COLS;
    int eraseRow = (i - 4) / NUM_COLS;
    int eraseCol = (i - 4) % NUM_COLS;

    if (i < 4) {
      eraseRow = (i + NUM_LEDS - 4) / NUM_COLS;
      eraseCol = (i + NUM_LEDS - 4) % NUM_COLS;      
    }

    strip.setPixelColor(LED_INDEXING[row][col], c); // Draw new pixel
    strip.setPixelColor(LED_INDEXING[eraseRow][eraseCol], 0); // Erase pixel a few steps back
    strip.show();
    delay(5);
  }
}