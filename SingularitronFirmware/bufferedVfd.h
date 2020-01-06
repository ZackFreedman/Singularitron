#ifndef BufferedVfd_h_
#define BufferedVfd_h_

// These libraries have been modified. Use the ones in this repo, not the official ones!
#include <CUU_Interface.h>
#include <CUU_Serial.h>
#include <Noritake_VFD_CUU.h>

class BufferedVfd {
  private:
    CUU_Serial vfdInterface;
    Noritake_VFD_CUU vfd;

    int brightness = 25;

    char previousFramebuffer[4][20];
    char activeFramebuffer[4][20];
    byte cursorRow;
    byte cursorColumn;
    bool framebufferChanged;
    bool justWokeUp;

    char nextCustomCharSlot = 0; // Used to automatically rotate custom char slots. Trust me, it's worth it
    const uint8_t * customChars[8]; // Fucking hate this type syntax

  public:
    BufferedVfd() : vfdInterface(displaySIO, displaySTB, displaySCK) {}

    void setup() {
      // Initialize display
      // Prepare framebuffers
      for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 20; col++) {
          activeFramebuffer[row][col] = ' ';
          previousFramebuffer[row][col] = ' ';
        }
      }

      // Take display off reset
      pinMode(displayReset, OUTPUT);
      digitalWrite(displayReset, HIGH);

      vfd.begin(20, 4);    // 20x4 character module
      vfd.interface(vfdInterface);
      //vfd.brightnessBoost(); // module has brightness boost
      vfd.bcVFD();          // is DS2045G because I a bawse

      pinMode(fiveVoltSupply, OUTPUT);
      pinMode(displaySupply, OUTPUT);
    }

    void powerUp() {
      // Enable +5V high-current rail
      digitalWrite(fiveVoltSupply, HIGH);
      delay(2); // Technically 500uS, but why not be cautious

      // Enable high-voltage filament drive
      digitalWrite(displaySupply, HIGH);

      delay(5);      // wait for device to power up

      vfd.CUU_init();      // initialize module
      vfd.japaneseFont();   // select cyberpunk af font for DS2045G
      setBrightness(brightness);

      justWokeUp = true; // TODO: Replace this with just a forced render?
    }

    void powerDown() {
      for (byte i = 0; i < 8; i++) customChars[i] = NULL;
      nextCustomCharSlot = 0;

      digitalWrite(displaySupply, LOW);
      delay(5);
      digitalWrite(fiveVoltSupply, LOW);
    }

    // Wipe display framebuffer and home cursor
    void clear() {
      for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 20; col++) activeFramebuffer[row][col] = ' ';
      }

      cursorRow = cursorColumn = 0;
      framebufferChanged = true;
    }

    void moveCursorTo(int row, int col) {
      if (row >= 0 || col >= 0) {
        cursorRow = row;
        cursorColumn = col;
      }
    }

    void bufferedPrint(char input, int row, int col) { // See overloaded method with no row/col
      moveCursorTo(row, col);

      /*
            debug_write(input);
            debug_print('\t');
            debug_print(cursorRow);
            debug_print(',');
            debug_println(cursorColumn);
      */

      activeFramebuffer[cursorRow][cursorColumn] = input;

      if (cursorColumn == 19) {
        cursorColumn = 0;
        if (cursorRow == 3) cursorRow = 0;
        else cursorRow++;
      }
      else cursorColumn++;

      if (previousFramebuffer[cursorRow][cursorColumn] != input) framebufferChanged = true;
    }

    void bufferedPrint(char input) {
      bufferedPrint(input, -1, -1);
    }

    void bufferedPrint(const char* input, int row, int col) { // See overloaded method with no row/col
      moveCursorTo(row, col);

      for (unsigned int i = 0; i < strlen(input); i++) {
        if (input[i] == 0x00) break; // Stop printing string at EOF char
        bufferedPrint(input[i]);
      }
    }

    void bufferedPrint(const char* input) {
      bufferedPrint(input, -1, -1);
    }

    void bufferedPrint(String* input, int row, int col) {
      bufferedPrint(input->c_str(), row, col);
    }

    void bufferedPrint(String* input) {
      bufferedPrint(input->c_str());
    }

    void bufferedPrint(float input, int row, int col) {
      static char buffer[10];
      sprintf(buffer, "%.2f", input);
      bufferedPrint(buffer, row, col);
    }

    void bufferedPrint(float input) {
      bufferedPrint(input, -1, -1);
    }

    void bufferedPrint(int input, int row, int col) {
      static char buffer[6];
      sprintf(buffer, "%i", input);
      bufferedPrint(buffer, row, col);
    }

    void bufferedPrint (int input) {
      bufferedPrint(input, -1, -1);
    }

    void render(bool force) { // See overloaded method - force is false
      //      unsigned long startTime = micros();

      if (force) vfd.CUU_clearScreen(); // Clear screen when forced
      else if (!framebufferChanged) return; // Nothing to write

      bool continued = false;

      for (byte row = 0; row < 4; row++) {
        for (byte col = 0; col < 20; col++) {
          if (previousFramebuffer[row][col] != activeFramebuffer[row][col] || force) { // Must write this character
            if (!continued || col == 0) vfd.CUU_setCursor(col, row);
            continued = true;

            vfd.CUU_writeData(activeFramebuffer[row][col]);
          }
          else continued = false; // No need to write this character. We'll need to move the cursor before writing the next char.

          // Now that we're done with this char from the active FB, copy it to the previous FB
          previousFramebuffer[row][col] = activeFramebuffer[row][col];
        }
      }

      // Both framebuffers are now in sync, and write is completed
      framebufferChanged = false;

      //      debug_print("Render time: ");
      //      debug_println(micros() - startTime);
    }

    void render() {
      render(false);
    }

    // Note that this copies previousFramebuffer - that's the one that's currently being displayed
    void copyFramebuffer(char target[4][20]) {
      for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 20; col++) {
          target[row][col] = previousFramebuffer[row][col];
        }
      }
    }

    // Returns index you should use for the custom char
    char createCustomChar(const uint8_t * bitmap) {
      // If this char has already been loaded, return its slot
      for (char i = 0; i < 8; i++) if (customChars[i] == bitmap) return i;

      nextCustomCharSlot = (nextCustomCharSlot + 1) % 8;
      return createCustomChar(bitmap, nextCustomCharSlot);
    }

    char createCustomChar(const uint8_t * bitmap, byte slot) {
      vfd.CUU_createChar(slot, const_cast<uint8_t*>(bitmap));
      customChars[slot] = bitmap;
      return slot;
    }

    void setBrightness(int newBrightness) {
      brightness = newBrightness;
      vfd.CUU_brightness(brightness); // Don't blind the engineer
    }
};

#endif

