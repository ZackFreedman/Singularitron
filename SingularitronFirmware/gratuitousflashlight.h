#include "TeensyDelay.h"

#include "cartEeprom.h"
#include "app.h"
#include "cart.h"
#include "pins.h"

#define changeWriteTime 1000

#define mainSafetyAddress 0x1
#define mainIntensityAddress 0x2
#define qfIntensityAddress 0x3
#define mainOnAddress 0x4

#define flashlightGpio 0

// Very hacky workaround to fake PWM dimming on non-PWM pins.
// I thought all Teensy digital pins had PWM because I am stoopie.

#define pwmPeriod 1500 // Microseconds

volatile byte pwmPin;
volatile bool pwmIsHigh;
volatile unsigned long timeLow = pwmPeriod;
volatile unsigned long timeHigh = 0;
volatile unsigned long timeOfLastFlip;
volatile bool flippingOut = false;

void flip() {
  if (!flippingOut) {
    digitalWriteFast(pwmPin, LOW);
    return;
  }

  if (pwmIsHigh) {
    digitalWriteFast(pwmPin, LOW);
    TeensyDelay::trigger(timeLow);
    pwmIsHigh = false;
  }
  else {
    digitalWriteFast(pwmPin, HIGH);
    TeensyDelay::trigger(timeHigh);
    pwmIsHigh = true;
  }
}

class GratuitousFlashlight : public Cartridge {
  private:
    class FlashlightQuickfireApp;
    class FlashlightMainApp;
    class FlashlightDefenseApp;

    byte flashlightPin;

    bool mainOn = false;
    bool safetyOn = false;
    unsigned int mainIntensity;
    unsigned int qfIntensity;
    unsigned int currentIntensity = 0;

    void flashlightWrite(unsigned int intensity) {
      if (intensity == 0) flippingOut = false;
      else flippingOut = true;

      if (currentIntensity == intensity) return;
      currentIntensity = intensity;

      timeHigh = map(intensity, 0, 1000, 0, pwmPeriod);
      timeLow = pwmPeriod - timeHigh;

      TeensyDelay::trigger(1);
    }

  public:
    GratuitousFlashlight(byte slot) : Cartridge(slot) {
      id = flashlightModuleId;
      name = "Hyper Flashlight";
      //      color = 0x805020;
      color = CRGB::LightYellow;
      widgetsAvailable = 0;
      hasQuickfireApp = hasMainApp = hasDefenseApp = true;

      pwmPin = flashlightPin = cartGpioPins[slot][flashlightGpio];

      debug_println("Gratuitous flashlight constructed");
    }

    virtual ~GratuitousFlashlight() {}

    void onInsert() {
      pinMode(flashlightPin, OUTPUT);
      analogWriteFrequency(flashlightPin, 1000); // TODO Reverse this?
      debug_println("Gratuitous flashlight inserted");

      // Load saved configuration
      unsigned int loadedWord = 0xFFFF;

//      openEeprom(slot);

      loadedWord = readEeprom(mainOnAddress, slot);
      debug_print("On: ");
      debug_println(loadedWord);
      debug_println("(ignored)");
      // mainOn = loadedWord > 0;  // Always start off for safety
      mainOn = false;

      loadedWord = readEeprom(mainSafetyAddress, slot);
      debug_print("Safety: ");
      debug_println(loadedWord);
      safetyOn = loadedWord > 0;

      loadedWord = readEeprom(mainIntensityAddress, slot);
      debug_print("Main: ");
      debug_println(loadedWord);
      if (loadedWord > 1000) mainIntensity = 50; // Default
      else mainIntensity = loadedWord;

      loadedWord = readEeprom(qfIntensityAddress, slot);
      debug_print("QF: ");
      debug_println(loadedWord);
      if (loadedWord > 1000) qfIntensity = 100; // Default
      else qfIntensity = loadedWord;

//      closeEeprom();

      TeensyDelay::begin();
      TeensyDelay::addDelayChannel(flip);
    }

    void onEject() {
      debug_println("Gratuitous flashlight ejected");
      flashlightWrite(0);
      pinMode(flashlightPin, INPUT);
    }

    App * generateQuickfireApp();
    App * generateMainApp();
    App * generateDefenseApp();
};

class GratuitousFlashlight::FlashlightQuickfireApp : public App {
  private:
    unsigned int powerLevel = 0;

  public:
    FlashlightQuickfireApp(Cartridge & owner) : App(owner) {}
    virtual ~FlashlightQuickfireApp() {}

    void setup() {
      // Flashlight pin direction is handled by the Cartridge
      powerLevel = static_cast<GratuitousFlashlight&>(owner).qfIntensity;

      static_cast<GratuitousFlashlight&>(owner).flashlightWrite(powerLevel);
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      display->bufferedPrint("Flashlight on!", 0, 0);
      display->bufferedPrint("This isn't even", 1, 0);
      display->bufferedPrint("my final form!", 2, 0);
    }

    void teardown(bool ejected) {
      static_cast<GratuitousFlashlight&>(owner).flashlightWrite(0);
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      return 0x00;
    }
};

class GratuitousFlashlight::FlashlightMainApp : public App {
  private:
    const int flashlightSafetyLimit = 250; //
    const int flashlightHardLimit = 1000; // Current draw limited to 800mA (?) by resistor on the current source

    bool knobWasHeld = false;

    bool somethingChanged;
    elapsedMillis timeSinceLastChange;

    int caretPosition = 0;
    bool editing = false;

  public:
    FlashlightMainApp(Cartridge & owner) : App(owner) {
      sleepAllowed = false;
    }
    virtual ~FlashlightMainApp() {}

    void setup() {
      // Flashlight pin direction is handled by the Cartridge
      //      timeSinceLastFlip = 0;
      
      // if (static_cast<GratuitousFlashlight&>(owner).mainOn) static_cast<GratuitousFlashlight&>(owner).flashlightWrite(static_cast<GratuitousFlashlight&>(owner).mainIntensity);
      
      // Always start flashlight off for safety
      static_cast<GratuitousFlashlight&>(owner).mainOn = false;
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      if (static_cast<GratuitousFlashlight&>(owner).mainOn) display->bufferedPrint("Flashlight on!", 0, 1);
      else display->bufferedPrint("Flashlight off.", 0, 1);

      if (static_cast<GratuitousFlashlight&>(owner).safetyOn) {
        if (static_cast<GratuitousFlashlight&>(owner).mainIntensity > flashlightSafetyLimit)
          display->bufferedPrint("Safety limited!", 1, 1);
        else
          display->bufferedPrint("Safety on", 1, 1);
      }
      else display->bufferedPrint("!!!SAFETY OFF!!!", 1, 1);

      display->bufferedPrint("Power level: ", 2, 1);
      static char powerString[6];
      itoa(static_cast<GratuitousFlashlight&>(owner).mainIntensity * 9 + 1, powerString, 10);
      display->bufferedPrint(powerString);

      display->bufferedPrint("QF power: ", 3, 1);
      itoa(static_cast<GratuitousFlashlight&>(owner).qfIntensity * 9 + 1, powerString, 10);
      display->bufferedPrint(powerString);

      if (editing) display->bufferedPrint('\x96', caretPosition, 0); // Filled diamond
      else display->bufferedPrint('\x1d', caretPosition, 0); // Right-pointing caret

      // Write changes after some time to reduce wear on EEPROM
      if (somethingChanged && timeSinceLastChange > changeWriteTime) {
//        openEeprom(owner.slot);
        writeEeprom(mainOnAddress, static_cast<GratuitousFlashlight&>(owner).mainOn ? 1 : 0, owner.slot);
        writeEeprom(mainSafetyAddress, static_cast<GratuitousFlashlight&>(owner).safetyOn ? 1 : 0, owner.slot);
        writeEeprom(mainIntensityAddress, static_cast<GratuitousFlashlight&>(owner).mainIntensity, owner.slot);
        writeEeprom(qfIntensityAddress, static_cast<GratuitousFlashlight&>(owner).qfIntensity, owner.slot);
//        closeEeprom();

        somethingChanged = false;
      }
    }

    void teardown(bool ejected) {
      static_cast<GratuitousFlashlight&>(owner).flashlightWrite(0);
      if (!ejected && somethingChanged) {
//        openEeprom(owner.slot);
        writeEeprom(mainOnAddress, static_cast<GratuitousFlashlight&>(owner).mainOn ? 1 : 0, owner.slot);
        writeEeprom(mainSafetyAddress, static_cast<GratuitousFlashlight&>(owner).safetyOn ? 1 : 0, owner.slot);
        writeEeprom(mainIntensityAddress, static_cast<GratuitousFlashlight&>(owner).mainIntensity, owner.slot);
        writeEeprom(qfIntensityAddress, static_cast<GratuitousFlashlight&>(owner).qfIntensity, owner.slot);
//        closeEeprom();

        somethingChanged = false;
      }
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      byte response = 0x00;

      if (clicks & knobMask) {
        switch (caretPosition) {
          case 0:
            static_cast<GratuitousFlashlight&>(owner).mainOn = !static_cast<GratuitousFlashlight&>(owner).mainOn;
            //            timeSinceLastFlip = 0;
            somethingChanged = true;
            break;
          case 1:
            static_cast<GratuitousFlashlight&>(owner).safetyOn = !static_cast<GratuitousFlashlight&>(owner).safetyOn;
            somethingChanged = true;
            break;
          default:
            editing = !editing;
        }
        response |= knobMask;
      }

      if (knobDelta != 0) {
        if (editing) {
          unsigned int newIntensity = 0;

          switch (caretPosition) {
            case 2: // Main intensity
              newIntensity = constrain(static_cast<GratuitousFlashlight&>(owner).mainIntensity + knobDelta * 20, 0, flashlightHardLimit);
              if (knobDelta < 0 && newIntensity > static_cast<GratuitousFlashlight&>(owner).mainIntensity) // We rolled under
                static_cast<GratuitousFlashlight&>(owner).mainIntensity = 0;
              else
                static_cast<GratuitousFlashlight&>(owner).mainIntensity = newIntensity;
              somethingChanged = true;
              break;

            case 3: // QF intensity
              newIntensity = constrain(static_cast<GratuitousFlashlight&>(owner).qfIntensity + knobDelta * 20, 0, flashlightSafetyLimit); // QF is always constrained to safe limit
              if (knobDelta < 0 && newIntensity > static_cast<GratuitousFlashlight&>(owner).qfIntensity) // We rolled over
                static_cast<GratuitousFlashlight&>(owner).qfIntensity = 0;
              else
                static_cast<GratuitousFlashlight&>(owner).qfIntensity = newIntensity;
              somethingChanged = true;
              break;

            default: // Should not happen
              editing = false;
          }
        }
        else { // Not editing - move the caret
          caretPosition -= knobDelta; // Remember that row 1 is BELOW row 0!
          caretPosition = constrain(caretPosition, 0, 3);
        }
        response |= updownMask;
      }

      if (static_cast<GratuitousFlashlight&>(owner).mainOn) {
        if (static_cast<GratuitousFlashlight&>(owner).safetyOn)
          static_cast<GratuitousFlashlight&>(owner).flashlightWrite(
            min(static_cast<GratuitousFlashlight&>(owner).mainIntensity, flashlightSafetyLimit));
        else
          static_cast<GratuitousFlashlight&>(owner).flashlightWrite(static_cast<GratuitousFlashlight&>(owner).mainIntensity);
      }
      else static_cast<GratuitousFlashlight&>(owner).flashlightWrite(0);

      if (somethingChanged) {
        timeSinceLastChange = 0;
      }

      return response;
    }
};

// Strobes flashlight in a hopefully disorienting way
class GratuitousFlashlight::FlashlightDefenseApp : public App {
  private:
    const int strobeOnTime = 10;
    const int strobeOffTime = 50;
    const int strobePower = 1000;

    bool enabled = true;
    bool flashlightOn = true;
    unsigned long changeTime;

  public:
    FlashlightDefenseApp(Cartridge & owner) : App(owner) {}
    virtual ~FlashlightDefenseApp() {}

    void setup() {
      // Flashlight pin direction is handled by the Cartridge
      //      timeSinceLastFlip = 0;
      static_cast<GratuitousFlashlight&>(owner).flashlightWrite(strobePower);
      changeTime = millis() + strobeOnTime;
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      // TODO: Turn display off

      if (!enabled) {
        if (flashlightOn) {
          flashlightOn = false;
          static_cast<GratuitousFlashlight&>(owner).flashlightWrite(0);
        }
      }
      else if (millis() > changeTime) {
        flashlightOn = !flashlightOn;
        if (flashlightOn) {
          static_cast<GratuitousFlashlight&>(owner).flashlightWrite(strobePower);
          changeTime = millis() + strobeOnTime;
        }
        else {
          static_cast<GratuitousFlashlight&>(owner).flashlightWrite(0);
          changeTime = millis() + strobeOffTime;
        }
      }
    }

    void teardown(bool ejected) {
      static_cast<GratuitousFlashlight&>(owner).flashlightWrite(0);
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      if (clicks & knobMask) {
        enabled = !enabled;
        flashlightOn = enabled;
        return knobMask;
      }

      return 0x00;
    }
};

App * GratuitousFlashlight::generateQuickfireApp() {
  debug_println("Gratuitous flashlight QF generated");
  return new FlashlightQuickfireApp(*this);
}

App * GratuitousFlashlight::generateMainApp() {
  debug_println("Gratuitous flashlight main generated");
  return new FlashlightMainApp(*this);
}

App * GratuitousFlashlight::generateDefenseApp() {
  debug_println("Gratuitous flashlight defense generated");
  return new FlashlightDefenseApp(*this);
}
