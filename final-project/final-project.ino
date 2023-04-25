#include <Adafruit_NeoPixel.h>

#define PIN 6
#define NUM_LEDS 512
#define NUM_ROWS 32
#define NUM_COLS 16
#define BRIGHTNESS 8 // to reduce current for 256 NeoPixels

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);
int LED_INDEXING[32][16];
int occupied[33][16];

// commonly used colors
uint32_t RED = strip.Color(255, 0, 0);
uint32_t GREEN = strip.Color(0, 255, 0);
uint32_t BLUE = strip.Color(0, 0, 255);

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
// convert the value to the actual position to light up (following array indexing)
int* pos_to_idx(int pos) {
  int row = (pos % NUM_LEDS) / NUM_COLS;
  int col = (pos % NUM_LEDS) % NUM_COLS;
  return new int[2]{row, col};  
}

void setup() {
  Serial.begin(9600);
  create_led_indexing();
  
  strip.begin();
  strip.setBrightness(BRIGHTNESS);

  for (int i = 0; i < 16; i++) {
    occupied[32][i] = 1;
  }
}

void loop() {
  // chase(strip.Color(255, 0, 0)); // Red
  // strip.setPixelColor(LED_INDEXING[0][6], strip.Color(0, 255, 0));

  int* shapeArray = generateLine(GREEN);
  drop(shapeArray, 4, GREEN);


  // shapeArray = generateZ(BLUE);
  // shapeArray = generateZ_reverse(BLUE);
  // shapeArray = generateL(BLUE);
  // shapeArray = generateL_reverse(BLUE);
  // shapeArray = generateT(BLUE);
  // shapeArray = generateBox(BLUE);
  // int shapeArrayLen = sizeof(shapeArray) / sizeof(int);
  // int arrSize = 4;
  // for (int i = 0; i < arrSize; i++) {
  //   Serial.print(shapeArray[i]);
  //   Serial.print(" ");
  // }
  // Serial.println();

  // for (int i = 0; i < 30; i++) {
  //   shapeArray = drop(shapeArray, 4, strip.Color(0, 255, 0));
  //   delay(1000);
  // }
  
  
  
  
  
  
  
  // chase(strip.Color(0, 255, 0)); // Green
  // chase(strip.Color(0, 0, 255)); // Blue
  // colorWipe(strip.Color(255, 0, 0), 50);      // Red
  // colorWipe(strip.Color(0, 255, 0), 50);      // Green
  // colorWipe(strip.Color(0, 0, 255), 50);      // Blue

  // Send a theater pixel chase in...
  // theaterChase(strip.Color(255, 255, 255), 50);     // White

  // rainbow(1);
  
  delay(500);
}

void turn_on_idxs(int* idxs, int idxs_size, uint32_t color) {
  for (int i = 0; i < idxs_size; i++) {
    int* rc = pos_to_idx(idxs[i]);
    int r = rc[0], c = rc[1];
    strip.setPixelColor(LED_INDEXING[r][c], color);
  }
  strip.show();
}

void turn_off_idxs(int* idxs, int idxs_size) {
  for (int i = 0; i < idxs_size; i++) {
    int* rc = pos_to_idx(idxs[i]);
    int r = rc[0], c = rc[1];
    strip.setPixelColor(LED_INDEXING[r][c], 0);
  }
  strip.show();
}

int* generateLine(uint32_t color) {
  int* idxs = new int[4]{6, 7, 8, 9};
  int idxs_size = 4;
  
  turn_on_idxs(idxs, idxs_size, color);
  delay(1000);
  turn_off_idxs(idxs, idxs_size);
  
  return idxs;
}

int* generateZ(uint32_t color) {
  int* idxs = new int[4] {6, 7, 23, 24};
  int idxs_size = 4;
  
  turn_on_idxs(idxs, idxs_size, color);
  delay(1000);
  turn_off_idxs(idxs, idxs_size);

  return idxs;
}

int* generateZ_reverse(uint32_t color) {
  int* idxs = new int[4] {22, 23, 7, 8};
  int idxs_size = 4;
  
  turn_on_idxs(idxs, idxs_size, color);
  delay(1000);
  turn_off_idxs(idxs, idxs_size);

  return idxs;
}

int* generateL(uint32_t color) {
  int* idxs = new int[4] {5, 21, 22, 23};
  int idxs_size = 4;
  
  turn_on_idxs(idxs, idxs_size, color);
  delay(1000);
  turn_off_idxs(idxs, idxs_size);

  return idxs;
}

int* generateL_reverse(uint32_t color) {
  int* idxs = new int[4] {20, 21, 22, 6};
  int idxs_size = 4;
  
  turn_on_idxs(idxs, idxs_size, color);
  delay(1000);
  turn_off_idxs(idxs, idxs_size);

  return idxs;
}

int* generateT(uint32_t color) {
  int* idxs = new int[4] {7, 22, 23, 24};
  int idxs_size = 4;
  
  turn_on_idxs(idxs, idxs_size, color);
  delay(1000);
  turn_off_idxs(idxs, idxs_size);

  return idxs;
}

int* generateBox(uint32_t color) {
  int* idxs = new int[4] {7, 8, 23, 24};
  int idxs_size = 4;
  
  turn_on_idxs(idxs, idxs_size, color);
  delay(1000);
  turn_off_idxs(idxs, idxs_size);

  return idxs;
}

int* getNextPos(int* pos, int size) {
  int* newPos = new int[size];
  for (int i = 0; i < size; i++) {
    newPos[i] = pos[i] + 16;
  }
  return newPos;
}

bool isCollision(int* pos, int size) {
  bool collide = false;
  for (int i = 0; i < size; i++) {
    if (pos[i] >= 512) {
      collide = true;
      break;
    }
    int* rc = pos_to_idx(pos[i]);
    int row = rc[0], col = rc[1];
    if (occupied[row][col] == 1) {
      collide = true;
    }
    else if (row >= NUM_ROWS) {
      collide = true;
    }
  }
  return collide;
}

void drop(int* shape, int size, uint32_t c) {
  // for (int i = 0; i < 32; i++) {
  //   for (int j = 0; j < 16; i++) {
  //     Serial.print(occupied[i][j]);
  //     Serial.print(" ");
  //   }
  //   Serial.println(" ");
  // }  
  while (true) {
    int* nextPos = getNextPos(shape, size);
    // bool collide = 
    if (isCollision(nextPos, size)) {
      for (int i = 0; i < size; i++) {
        int* rc = pos_to_idx(shape[i]);
        int row = rc[0], col = rc[1];
        int pr = row - 1;
        strip.setPixelColor(LED_INDEXING[row][col], c);
        strip.setPixelColor(LED_INDEXING[pr][col], 0);
        strip.show();
        occupied[row][col] = 1;
      }
      break;
    } else {
      for (int i = 0; i < size; i++) {
        int* rc = pos_to_idx(shape[i]);
        int row = rc[0], col = rc[1];
        int pr = row - 1;
        strip.setPixelColor(LED_INDEXING[row][col], c);
        strip.setPixelColor(LED_INDEXING[pr][col], 0);
        strip.show();
        // occupied[row][col] = 1;
        shape[i] += 16;
      }      
    }
  }
  // Serial.println("out of loop");

  // for (int i = 0; i < 33; i++) {
  //   for (int j = 0; j < 16; j++) {
  //     Serial.print(occupied[i][j]);
  //     Serial.print(",");
  //   }
  //   Serial.println();
}



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

void colorWipe(uint32_t c, uint8_t wait) {
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  for(int j=0; j<1000; j++) 
  {
    for(int i=0; i<strip.numPixels(); i++) 
    {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) { //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}