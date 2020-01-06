#include "flavortext.h"
#include "cartEeprom.h"
#include "app.h"
#include "cart.h"
#include "pins.h"
#include "icons.h"
#include "duck.h"
#include "scrollableList.h"

class TestCart : public Cartridge {
  private:
    class TestQuickfireApp;
    class TestMainApp;
    class TestDefenseApp;

    int vibeIntensity = 0;
    int red = 0x33;
    int green = 0;
    int blue = 0x55;

  public:
    TestCart(byte slot) : Cartridge(slot) {
      id = testModuleId;
      name = "Test Cart";
      //      color = 0x330055;
      color = CRGB::Fuchsia;
      widgetsAvailable = 6;
      hasQuickfireApp = hasMainApp = hasDefenseApp = true;
      backgroundUpdateInterval = 0;

      debug_println("Test constructed");
    }

    virtual ~TestCart() {}

    void onInsert() {
      debug_println("Gratuitous Test inserted");
    }

    void onEject() {
      debug_println("Test ejected");
    }

    App * generateQuickfireApp();
    App * generateMainApp();
    App * generateDefenseApp();

    void backgroundUpdate() {
      //      debug_println("hOI!");
    }

    void renderWidget(BufferedVfd * display, byte widgetNumber) {
      switch (widgetNumber) {
        case 0:
          display->bufferedPrint("LUCK: 15");
          break;

        case 1:
          display->bufferedPrint("SKILL: 20");
          break;

        case 2:
          display->bufferedPrint("WILL: 15");
          break;

        case 3:
          display->bufferedPrint("PLEAS: 5");
          break;

        case 4:
          display->bufferedPrint("PAIN: 50");
          break;

        case 5:
          display->bufferedPrint("NAME: 100");
          break;

        default:
          display->bufferedPrint("?????");

      }
    }
};

class TestCart::TestQuickfireApp : public App {
  private:
    char flavorTexts[4][21];
    byte topLine = 0;

    elapsedMillis timeSinceLastLine;
    const unsigned long timeBetweenLines = 300;

  public:
    TestQuickfireApp(Cartridge & owner) : App(owner) {}
    virtual ~TestQuickfireApp() {}

    void setup() {
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 21; j++) flavorTexts[i][j] = 0x00;
      }

      getFullLine(flavorTexts[3], random(2));
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      if (timeSinceLastLine >= timeBetweenLines) {
        timeSinceLastLine -= timeBetweenLines;

        getFullLine(flavorTexts[topLine], random(2));

        topLine = (topLine + 1) % 4;
      }

      for (int i = 0; i < 4; i++) {
        if (i < 3) display->bufferedPrint(flavorTexts[(topLine + i) % 4], i, 0);
        else {
          int progress = constrain(map(long(timeSinceLastLine), 0, timeBetweenLines - 100, 0, 21), 0, 20);
          if (progress > 0) {
            display->bufferedPrint(flavorTexts[(topLine + i) % 4][0], i, 0);
            for (int j = 1; j <= progress; j++) if (flavorTexts[(topLine + i) % 4][j] != 0x00) display->bufferedPrint(flavorTexts[(topLine + i) % 4][j]);
          }

          if (progress < 19) display->bufferedPrint('\x14'); // Black block
        }
      }

      /*
        int foo = random(11) + random(11);
        for (int i = 0; i < foo; i++) display->bufferedPrint(random(0x09, 0xFF) + 1, random(4), random(20));
        display->bufferedPrint("FUZZ DAT SHIT", 0, 0);

        for (int i = 0; i < 5; i++) leds->bufferLed(i, leds->Color(random(55) * 5, random(55) * 5, random(55) * 5));
      */
    }

    void teardown(bool ejected) {}

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      return 0x00;
    }
};


class TestCart::TestMainApp : public App {
  private:
    bool somethingChanged;
    elapsedMillis timeSinceLastChange;

    int caretPosition = 0;
    bool editing = false;

    char flavorText[21];

    char duckString[28];

    char slots[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    ScrollableList scrollyBoi;
    bool scrollyBoiActive = false;

  public:
    TestMainApp(Cartridge & owner) : App(owner), scrollyBoi() {
      sleepAllowed = false;
    }
    virtual ~TestMainApp() {}

    void setup() {
      getFullLine(flavorText, random(2));

      scrollyBoi.append("Eeny");
      scrollyBoi.append("Meeny");
      scrollyBoi.append("Miny");
      scrollyBoi.append("Moe");
      scrollyBoi.append("Grab");
      scrollyBoi.append("A Tiger");
      scrollyBoi.append("By The");
      scrollyBoi.append("Toe");
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      if (scrollyBoiActive) {
        scrollyBoi.render(display);
        return;
      }

      if (slots[0] == 0xFF) slots[0] = display->createCustomChar(lightningIcon);
      if (slots[1] == 0xFF) slots[1] = display->createCustomChar(fileIcon);
      if (slots[2] == 0xFF) slots[2] = display->createCustomChar(thermometerIcon);
      if (slots[3] == 0xFF) slots[3] = display->createCustomChar(co2Icon);
      if (slots[4] == 0xFF) slots[4] = display->createCustomChar(smokeIcon);
      if (slots[5] == 0xFF) slots[5] = display->createCustomChar(humidityIcon);
      if (slots[6] == 0xFF) slots[6] = display->createCustomChar(barometerIcon);
      if (slots[7] == 0xFF) slots[7] = display->createCustomChar(batteryIcon);

      static char tempString[21];

      display->bufferedPrint("THIS SHIT: ", 0, 0);
      for (char i = 0; i < 8; i++) display->bufferedPrint(slots[i]);

      sprintf(tempString, "Vibes: %i%", static_cast<TestCart&>(owner).vibeIntensity);
      display->bufferedPrint(tempString, 1, 1);

      sprintf(tempString, "Buttons: 0x%02X%02X%02X",
              static_cast<TestCart&>(owner).red, static_cast<TestCart&>(owner).green, static_cast<TestCart&>(owner).blue);
      display->bufferedPrint(tempString, 2, 0);

      display->bufferedPrint(flavorText, 3, 0);

      byte caretRow = 0;
      byte caretCol = 0;

      switch (caretPosition) {
        case 0:
          caretRow = 1;
          caretCol = 0;
          break;
        case 1:
          caretRow = 3;
          caretCol = 11;
          break;
        case 2:
          caretRow = 3;
          caretCol = 13;
          break;
        case 3:
          caretRow = 3;
          caretCol = 15;
          break;
        default:
          break;
      }

      //      if (editing) display->bufferedPrint(0x96, caretRow, caretCol); // Filled diamond
      //      else if (caretPosition == 0) display->bufferedPrint(0x1D, caretRow, caretCol); // Right-pointing caret
      //      else display->bufferedPrint(0x1F, caretRow, caretCol); // Up caret

      if (caretPosition == 1 || caretPosition == 2 || caretPosition == 3) {
        if ((editing && millis() % 3) || (!editing && (millis() / 125 % 2))) display->bufferedPrint("  ", 2, 11 + ((caretPosition - 1) * 2));
      }
      else if (caretPosition == 0) {
        if (editing) display->bufferedPrint('\x96', caretRow, caretCol); // Filled diamond
        else if (caretPosition == 0) display->bufferedPrint('\x1D', caretRow, caretCol); // Right-pointing caret
      }

      // Test writing to the button LED's
      for (int i = 0; i < 5; i++) {
        //        leds->bufferLed(i, leds->Color(static_cast<TestCart&>(owner).red,
        //                                       static_cast<TestCart&>(owner).green,
        //                                       static_cast<TestCart&>(owner).blue));
        leds->bufferLed(i, CRGB(
                          static_cast<TestCart&>(owner).red,
                          static_cast<TestCart&>(owner).green,
                          static_cast<TestCart&>(owner).blue));
      }

      /*
        // Write changes after some time to reduce wear on EEPROM
        if (somethingChanged && timeSinceLastChange > changeWriteTime) {
        openEeprom(owner.slot);
        writeEeprom(mainOnAddress, static_cast<TestCart&>(owner).mainOn ? 1 : 0);
        writeEeprom(mainSafetyAddress, static_cast<TestCart&>(owner).safetyOn ? 1 : 0);
        writeEeprom(mainIntensityAddress, static_cast<TestCart&>(owner).mainIntensity);
        writeEeprom(qfIntensityAddress, static_cast<TestCart&>(owner).qfIntensity);
        closeEeprom();

        somethingChanged = false;
        }
      */
    }

    void teardown(bool ejected) {
      analogWrite(vibes, 0);

      /*
        if (!ejected && somethingChanged) {
        openEeprom(owner.slot);
        writeEeprom(mainOnAddress, static_cast<TestCart&>(owner).mainOn ? 1 : 0);
        writeEeprom(mainSafetyAddress, static_cast<TestCart&>(owner).safetyOn ? 1 : 0);
        writeEeprom(mainIntensityAddress, static_cast<TestCart&>(owner).mainIntensity);
        writeEeprom(qfIntensityAddress, static_cast<TestCart&>(owner).qfIntensity);
        closeEeprom();

        somethingChanged = false;
        }
      */
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      if (scrollyBoiActive) {
        int clickedItem = scrollyBoi.control(knobDelta * -1, clicks & knobMask); // Note inverting knob

        if (clickedItem > -1) {
          debug_print("Selected ");
          debug_println(clickedItem);
          scrollyBoiActive = false;
        }

        return knobMask + updownMask;
      }

      byte response = 0x00;

      if (clicks & knobMask) {
        if (caretPosition == 4) {
          //                for (int i = 0; i < 28; i++) duckString[i] = 0x00;
          //
          //                //          strcat(duckString, "STRING ");
          //                //          strcat(duckString, flavorText);
          //
          //                //          strcat(duckString, "DELAY 10000");
          //
          //                strcat(duckString, "NUMLOCK");
          //
          //                debug_print("Ducking ");
          //                debug_println(duckString);
          //                debug_print("Duck response: ");
          //                debug_println(duck(duckString, 28).response);
          scrollyBoiActive = true;
        }
        else {
          editing = !editing;
          response |= knobMask;
        }
      }

      if (knobDelta != 0) {
        if (editing) {
          switch (caretPosition) {
            case 0:
              static_cast<TestCart&>(owner).vibeIntensity = static_cast<TestCart&>(owner).vibeIntensity + knobDelta * 10;
              analogWrite(vibes, static_cast<TestCart&>(owner).vibeIntensity);
              break;
            case 1:
              static_cast<TestCart&>(owner).red = constrain(static_cast<TestCart&>(owner).red + knobDelta * 5, 0, 255);
              break;
            case 2:
              static_cast<TestCart&>(owner).green = constrain(static_cast<TestCart&>(owner).green + knobDelta * 5, 0, 255);
              break;
            case 3:
              static_cast<TestCart&>(owner).blue = constrain(static_cast<TestCart&>(owner).blue + knobDelta * 5, 0, 255);
              break;
            default: // Should not happen
              editing = false;
          }
        }
        else { // Not editing - move the caret
          caretPosition -= knobDelta; // Remember that row 1 is BELOW row 0!
          if (caretPosition > 3) getFullLine(flavorText, random(2));
          caretPosition = constrain(caretPosition, 0, 4);
        }
        response |= updownMask;
      }
      /*
        if (static_cast<TestCart&>(owner).mainOn) {
        if (static_cast<TestCart&>(owner).safetyOn)
          static_cast<TestCart&>(owner).TestWrite(
            min(static_cast<TestCart&>(owner).mainIntensity, TestSafetyLimit));
        else
          static_cast<TestCart&>(owner).TestWrite(static_cast<TestCart&>(owner).mainIntensity);
        }
        else static_cast<TestCart&>(owner).TestWrite(0);

        if (somethingChanged) {
        timeSinceLastChange = 0;
        }
      */

      return response;
    }
};

// Strobes Test in a hopefully disorienting way
class TestCart::TestDefenseApp : public App {
  public:
    TestDefenseApp(Cartridge & owner) : App(owner) {}
    virtual ~TestDefenseApp() {}

    void setup() {}

    void update(BufferedVfd *display, BufferedLeds *leds) {
      display->bufferedPrint("Defense!", 0, 0);
      display->bufferedPrint("clap clap clap", random(1, 4), random(7));
    }

    void teardown(bool ejected) {}

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      return 0x00;
    }
};

App * TestCart::generateQuickfireApp() {
  debug_println("Gratuitous Test QF generated");
  return new TestQuickfireApp(*this);
}

App * TestCart::generateMainApp() {
  debug_println("Gratuitous Test main generated");
  return new TestMainApp(*this);
}

App * TestCart::generateDefenseApp() {
  debug_println("Gratuitous Test defense generated");
  return new TestDefenseApp(*this);
}

