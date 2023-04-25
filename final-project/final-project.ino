#include <Adafruit_NeoPixel.h>

#define PIN 6
#define NUM_LEDS 512
#define NUM_ROWS 32
#define NUM_COLS 16
#define BRIGHTNESS 8 // to reduce current for 256 NeoPixels

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);
int LED_INDEXING[32][16];

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


void setup() {
  Serial.begin(9600);
  create_led_indexing();
  
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
}

void loop() {
  chase(strip.Color(255, 0, 0)); // Red
  // chase(strip.Color(0, 255, 0)); // Green
  // chase(strip.Color(0, 0, 255)); // Blue

  // colorWipe(strip.Color(255, 0, 0), 50);      // Red
//   colorWipe(strip.Color(0, 255, 0), 50);      // Green
//   colorWipe(strip.Color(0, 0, 255), 50);      // Blue

  // Send a theater pixel chase in...
  // theaterChase(strip.Color(255, 255, 255), 50);     // White

  // rainbow(1);
  
  // strip.setPixelColor(0, strip.Color(255, 0, 0));
  // strip.show();
  // Serial.println("showing red.");
  // delay(10000);
}

static void chase(uint32_t c) {
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

// void rainbow(uint8_t wait) {
//   for(int j=0; j<1000; j++) 
//   {
//     for(int i=0; i<strip.numPixels(); i++) 
//     {
//       strip.setPixelColor(i, Wheel((i+j) & 255));
//     }
//     strip.show();
//     delay(wait);
//   }
// }

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

// // Input a value 0 to 255 to get a color value.
// // The colours are a transition r - g - b - back to r.
// uint32_t Wheel(byte WheelPos) {
//   if(WheelPos < 85) {
//    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
//   } else if(WheelPos < 170) {
//    WheelPos -= 85;
//    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
//   } else {
//    WheelPos -= 170;
//    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
//   }
// }