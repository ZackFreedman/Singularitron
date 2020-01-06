#ifndef BUFFEREDLEDS_H
#define BUFFEREDLEDS_H

#define FASTLED_ALLOW_INTERRUPTS 1
#include <FastLED.h>

class BufferedLeds {
  private:
    // Fields for efficient LED write
    CRGB previousLedBuffer[5];
    CRGB activeLedBuffer[5];
    bool ledBufferChanged;

    int brightness = 255;

    CRGB leds[5];

    const int gamma[256] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6,
      6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11, 11,
      12, 12, 13, 13, 14, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19,
      19, 20, 20, 21, 22, 22, 23, 23, 24, 25, 25, 26, 26, 27, 28, 28,
      29, 30, 30, 31, 32, 33, 33, 34, 35, 35, 36, 37, 38, 39, 39, 40,
      41, 42, 43, 43, 44, 45, 46, 47, 48, 49, 50, 50, 51, 52, 53, 54,
      55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 71,
      72, 73, 74, 75, 76, 77, 78, 80, 81, 82, 83, 84, 86, 87, 88, 89,
      91, 92, 93, 94, 96, 97, 98, 100, 101, 102, 104, 105, 106, 108, 109, 110,
      112, 113, 115, 116, 118, 119, 121, 122, 123, 125, 126, 128, 130, 131, 133, 134,
      136, 137, 139, 140, 142, 144, 145, 147, 149, 150, 152, 154, 155, 157, 159, 160,
      162, 164, 166, 167, 169, 171, 173, 175, 176, 178, 180, 182, 184, 186, 187, 189,
      191, 193, 195, 197, 199, 201, 203, 205, 207, 209, 211, 213, 215, 217, 219, 221,
      223, 225, 227, 229, 231, 233, 235, 238, 240, 242, 244, 246, 248, 251, 253, 255
    };

  public:
    BufferedLeds() {}

    void setup() {
      FastLED.addLeds<WS2812, ledsPin, GRB>(leds, 5).setCorrection(TypicalLEDStrip);
      FastLED.setBrightness(brightness);
      FastLED.show();
    }

    void clear() {
      for (byte i = 0; i < 5; i++) activeLedBuffer[i] = CRGB::Black;
      ledBufferChanged = true;
    }

    void bufferLed(byte index, CRGB color) {
      activeLedBuffer[index] = color;
      if (previousLedBuffer[index] != color) ledBufferChanged = true;
    }

    void render(bool force) { // See overloaded method - force is false
//      if (!force && !ledBufferChanged) return; // Nothing to change

      for (int i = 0; i < 5; i++) {
        leds[i] = activeLedBuffer[i];
        
//        int blue = activeLedBuffer[i] & 0xFF;
//        int green = activeLedBuffer[i] >> 8 & 0xFF;
//        int red = activeLedBuffer[i] >> 16 & 0xFF;
//
//        red = red * brightness / 100;
//        green = green * brightness / 100;
//        blue = blue * brightness / 100;
//
//        red = gamma[red];
//        green = gamma[green];
//        blue = gamma[blue];
//
//        leds.setPixelColor(i, Color(red, green, blue));
        previousLedBuffer[i] = activeLedBuffer[i];
      }

      FastLED.show();
      ledBufferChanged = false;
    }

    void render() {
      render(false);
    }

    // Note that this copies previousLedBuffer - that's the one that's currently being displayed
    void copyLedBuffer(CRGB target[5]) {
      for (int i = 0; i < 5; i++) target[i] = previousLedBuffer[i];
    }

//    uint32_t Color(int r, int g, int b) {
//      return leds.Color(r, g, b);
//    }

    void setBrightness(int newBrightness) {
      brightness = map(newBrightness, 0, 100, 0, 254);
      FastLED.setBrightness(brightness);
    }
};

#endif
