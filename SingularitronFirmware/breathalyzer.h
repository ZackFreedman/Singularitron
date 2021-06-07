#include "app.h"
#include "cart.h"
#include "pins.h"
#include "cartEeprom.h"

#define breathalyzerGpio 0

class BreathalyzerApp : public App {
  private:
    byte breathalyzerPin = 0;

    int breathBuffer[10] = {0};
    int highWaterMark = 0;
    elapsedMillis timeSinceHighWaterMark;

  public:
    BreathalyzerApp(Cartridge & owner) : App(owner) {
      breathalyzerPin = cartGpioPins[owner.slot][breathalyzerGpio];
    }
    virtual ~BreathalyzerApp() {}

    void setup() {
      openEeprom(owner.slot);  // The EEPROM's CS pin is also used to power up the breathalyzer
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      for (int i = 1; i < 10; i++) breathBuffer[i - 1] = breathBuffer[i];

      breathBuffer[9] = analogRead(breathalyzerPin);

      int average = 0;
      for (int i = 0; i < 10; i++) average += breathBuffer[i];
      average /= 10;

      if (average >= highWaterMark) {
        highWaterMark = average;
        timeSinceHighWaterMark = 0;
      }

      if (timeSinceHighWaterMark >= 10000) highWaterMark = average;

      byte segments = constrain(map(average, 500, 4096, 0, 85), 0, 85);

      for (int j = 0; j < 2; j++) {
        display->moveCursorTo(j, 0);

        for (int i = 0; i < 20; i++) {
          if (segments >= i * 5) display->bufferedPrint(char(0xFF));
          else {
            display->bufferedPrint(char(0x0F + segments % 5));
            break;
          }
        }
      }

      static char readingString[6];

      display->bufferedPrint("Score: ", 2, 0);
      itoa(highWaterMark > 400 ? highWaterMark : 0, readingString, 10);
      display->bufferedPrint(readingString);

      display->bufferedPrint("Verdict: ", 3, 0);
      if (highWaterMark < 500) display->bufferedPrint("Sober");
      else if (highWaterMark < 600) display->bufferedPrint("Liftoff");
      else if (highWaterMark < 700) display->bufferedPrint("Feelin' it");
      else if (highWaterMark < 800) display->bufferedPrint("Classy");
      else if (highWaterMark < 900) display->bufferedPrint("Lubricated");
      else if (highWaterMark < 1000) display->bufferedPrint("Tipsy");
      else if (highWaterMark < 1200) display->bufferedPrint("Saucy");
      else if (highWaterMark < 1400) display->bufferedPrint("Drunk");
      else if (highWaterMark < 1600) display->bufferedPrint("Omniscient");
      else if (highWaterMark < 1800) display->bufferedPrint("Wizardly");
      else if (highWaterMark < 2000) display->bufferedPrint("Crunk");
      else if (highWaterMark < 2250) display->bufferedPrint("Shrekt");
      else if (highWaterMark < 2500) display->bufferedPrint("Smashed");
      else if (highWaterMark < 2750) display->bufferedPrint("Wasted");
      else if (highWaterMark < 3000) display->bufferedPrint("Hammered");
      else if (highWaterMark < 3250) display->bufferedPrint("Zombified");
      else if (highWaterMark < 3500) display->bufferedPrint("Blackout");
      else if (highWaterMark < 3500) display->bufferedPrint("In Danger");
      else if (highWaterMark < 3750) display->bufferedPrint("Dead");
      else display->bufferedPrint("Godlike");

    }

    void teardown(bool ejected) {
      deselectEeproms();
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      return 0x00;
    }
};

class BreathalyzerCart : public Cartridge {
  public:
    BreathalyzerCart(byte slot) : Cartridge(slot) {
      id = breathalyzerModuleId;
      name = "Breathalyzer";
      color = CRGB::Chartreuse;
      widgetsAvailable = 0;
      hasQuickfireApp = hasMainApp = true;

      debug_println("BreathalyzerCart constructed");
    }

    virtual ~BreathalyzerCart() {}

    void onInsert() {
      debug_println("BreathalyzerCart inserted");

    }

    void onEject() {
      debug_println("BreathalyzerCart ejected");
    }

    App * generateQuickfireApp() {
      debug_println("BreathalyzerCart QF generated");
      return new BreathalyzerApp(*this);
    }

    App * generateMainApp() {
      debug_println("BreathalyzerCart main generated");
      return new BreathalyzerApp(*this);
    }

    App * generateDefenseApp() {
      debug_println("ERROR! BreathalyzerCart has no defense app");
      return NULL;
    }

    void backgroundUpdate() {}
    void renderWidget(char * slot, byte widgetNumber) {
      // TODO here: Render ambient temp widget
    }
};
