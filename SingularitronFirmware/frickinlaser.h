#include "app.h"
#include "cart.h"
#include "pins.h"

#define laserGpioPin 0

class LaserQuickfireApp : public App {
  private:
    byte laserPin;

  public:
    LaserQuickfireApp(Cartridge & owner) : App(owner) {
      laserPin = cartGpioPins[owner.slot][laserGpioPin];
    }
    virtual ~LaserQuickfireApp() {}

    void setup() {
      // Laser pin direction is handled by the Cartridge
      digitalWrite(laserPin, HIGH);
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      display->bufferedPrint("I'MMA QUICKFIRING", 0, 0);
      display->bufferedPrint("MAH LAZOR!!!", 1, 0);
    }

    void teardown(bool ejected) {
      digitalWrite(laserPin, LOW);
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      return 0x00;
    }
};

class LaserMainApp : public App {
  private:
    byte laserPin;
    int laserPower = 255;
    bool laserOn = false;

  public:
    LaserMainApp(Cartridge & owner) : App(owner) {
      laserPin = cartGpioPins[owner.slot][laserGpioPin];
      sleepAllowed = true;
    }
    virtual ~LaserMainApp() {}

    void setup() {
      // Laser pin direction is handled by the Cartridge
//      if (laserOn) analogWrite(laserPin, laserPower);
//      else analogWrite(laserPin, 0);

      digitalWrite(laserPin, laserOn);
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      if (laserOn) display->bufferedPrint("I'MMA FIRING", 0, 0);
      else display->bufferedPrint("I'M NOT-A FIRING", 0, 0);
      display->bufferedPrint("MAH LAZOR!!!", 1, 0);

      display->bufferedPrint("Power level: ", 2, 0);
      static char laserPowerString[6];
      itoa(min(laserPower * 36 + 1, 9001), laserPowerString, 10);
      display->bufferedPrint(laserPowerString);
    }

    void teardown(bool ejected) {
//      analogWrite(laserPin, 0);
      digitalWrite(laserPin, LOW);
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      byte response = 0;

      //      debug_println(clicks, BIN);

      if (clicks & knobMask) {
        debug_println("laser toggle");
        laserOn = !laserOn;
//        if (laserOn) analogWrite(laserPin, laserPower);
//        else analogWrite(laserPin, 0);
        digitalWrite(laserPin, laserOn);
        response |= knobMask;
      }

      if (knobDelta != 0) {
        laserPower = constrain(laserPower + knobDelta * 5, 0, 255); // Can actually go to 1024 but that wouldn't say 9001 on the display
//        if (laserOn) analogWrite(laserPin, laserPower);
        response |= updownMask;
      }

      return response;
    }
};

// Laser dazzler defense app randomly modulates laser in a hopefully disorienting way
class LaserDefenseApp : public App {
  private:
    const int minimumChangeTime = 10;
    const int maximumChangeTime = 50;
    const int minimumPower = 0;
    const int maximumPower = 1024;

    byte laserPin;
    unsigned long changeTime;

  public:
    LaserDefenseApp(Cartridge & owner) : App(owner) {
      laserPin = cartGpioPins[owner.slot][laserGpioPin];
    }
    virtual ~LaserDefenseApp() {}

    void setup() {
      // Laser pin direction is handled by the Cartridge
      analogWrite(laserPin, random(minimumPower, maximumPower));
      changeTime = millis() + random(minimumChangeTime, maximumChangeTime);
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      
      display->bufferedPrint("////////////////////", 0, 0);
      display->bufferedPrint(" LAZOR DAZZLER MODE", 1, 0);
      display->bufferedPrint("AIM DIRECTLY AT EYES", 2, 0);
      display->bufferedPrint("////////////////////", 3, 0);

      if (millis() > changeTime) {
//        analogWrite(laserPin, random(minimumPower, maximumPower));
        if (random(2)) analogWrite(laserPin, minimumPower);
        else analogWrite(laserPin, maximumPower);
        changeTime = millis() + random(minimumChangeTime, maximumChangeTime);
      }
    }

    void teardown(bool ejected) {
      analogWrite(laserPin, 0);
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      return 0x00;
    }
};

class FrickinLaser : public Cartridge {
  private:
    byte laserPin;

  public:
    FrickinLaser(byte slot) : Cartridge(slot) {
      id = frickinLaserModuleId;
      name = "Frickin' Laser";
//      color = 0x880000;
      color = CRGB::Red;
      widgetsAvailable = 0;

      hasQuickfireApp = hasMainApp = hasDefenseApp = true;

      laserPin = cartGpioPins[slot][laserGpioPin];

      debug_println("Frickin' laser constructed");
    }
    virtual ~FrickinLaser() {}

    void onInsert() {
      digitalWrite(laserPin, LOW);
      pinMode(laserPin, OUTPUT);
      debug_println("Frickin' laser inserted");
    }

    void onEject() {
      debug_println("Frickin' laser ejected");
      digitalWrite(laserPin, LOW);
      pinMode(laserPin, INPUT);
    }

    App * generateQuickfireApp() {
      debug_println("Frickin' laser QF generated");
      return new LaserQuickfireApp(*this);
    }

    App * generateMainApp() {
      debug_println("Frickin' laser main generated");
      return new LaserMainApp(*this);
    }

    App * generateDefenseApp() {
      debug_println("Frickin' laser defense generated");
      return new LaserDefenseApp(*this);
    }
};


