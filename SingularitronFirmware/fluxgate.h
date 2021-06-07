#include "app.h"
#include "cart.h"
#include "pins.h"
#include "icons.h"

#define fluxgateGpio 0

class FluxgateApp : public App {
  private:
    byte fluxgatePin = 0;

    unsigned int maxReading = 1000;
    unsigned int timeDelta = 70;
    unsigned int window = 2000;
    unsigned int stability = 150;
    unsigned int historyLength = window / timeDelta;
    elapsedMillis timeSinceLastUpdate;

    int readingHistory[30] = {0};  // Size should be more than window / timedelta
    byte historyPointer;
    int zeroPoint = 0;
    int lastKnownAverage = 0;
    int compensatedValue = 0;
    float displayHistory[20];

    byte hasCreatedBarChars = false;

  public:
    FluxgateApp(Cartridge & owner) : App(owner) {
      fluxgatePin = cartGpioPins[owner.slot][fluxgateGpio];
    }
    virtual ~FluxgateApp() {}

    void setup() {
      for (int i = 0; i < historyLength; i++) {
        readingHistory[i] = analogRead(fluxgatePin);
      }

      zeroPoint = readingHistory[0];

      timeSinceLastUpdate = 0;
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      if (!hasCreatedBarChars) {
        for (int i = 0; i < 7; i++) {
          display->createCustomChar(verticalBars[i], i);
        }

        hasCreatedBarChars = true;
      }

      unsigned long theTime = timeSinceLastUpdate;

      int sensorValue = analogRead(fluxgatePin);

      while (theTime >= timeDelta) {
        for (int i = 0; i < 19; i++) {
          displayHistory[i] = displayHistory[i + 1];
        }

        sensorValue = analogRead(fluxgatePin);
        readingHistory[historyPointer] = sensorValue;
        historyPointer = (historyPointer + 1) % historyLength;

        int total = 0;
        for (int i = 0; i < historyLength; i++) {
          total += readingHistory[i];
        }

        lastKnownAverage = total / historyLength;

        compensatedValue = abs(sensorValue - zeroPoint);

        //  compensatedValue += sin(double(millis()) / 300.0) * 1000.0 + 800.0;

//        Serial.print("Reading: ");
//        Serial.print(sensorValue);
//        Serial.print("\tCompensated: ");
//        Serial.println(compensatedValue);

        displayHistory[19] = min(float(compensatedValue) / float(maxReading) * 29.0, 28);

        theTime -= timeDelta;
        timeSinceLastUpdate -= timeDelta;
      }

      digitalWrite(vibes, compensatedValue >= maxReading /2);

      for (int row = 0; row < 4; row++) {
        display->moveCursorTo(row, 0);
        int minValueForThisRow = (3 - row) * 7;

        for (int col = 0; col < 20; col++) {
          if (displayHistory[col] >= minValueForThisRow) {
            display->bufferedPrint(char(min(6, displayHistory[col] - minValueForThisRow)));
          }
          else {
            display->bufferedPrint(' ');
          }
        }
      }

      static char readingString[6];

      display->bufferedPrint("Reading: ", 0, 0);
      itoa(compensatedValue, readingString, 10);
      display->bufferedPrint(readingString);
    }

    void teardown(bool ejected) {
      digitalWrite(vibes, LOW);
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      if (clicks & knobMask) {
        zeroPoint = lastKnownAverage;
      }
    }
};

class FluxgateCart : public Cartridge {
  public:
    FluxgateCart(byte slot) : Cartridge(slot) {
      id = fluxgateModuleId;
      name = "THUNDERHAND";
      color = CRGB::BlueViolet;
      widgetsAvailable = 0;
      hasQuickfireApp = hasMainApp = true;

      debug_println("FluxgateCart constructed");
    }

    virtual ~FluxgateCart() {}

    void onInsert() {
      debug_println("FluxgateCart inserted");
    }

    void onEject() {
      debug_println("FluxgateCart ejected");
    }

    App * generateQuickfireApp() {
      debug_println("FluxgateCart QF generated");
      return new FluxgateApp(*this);
    }

    App * generateMainApp() {
      debug_println("FluxgateCart main generated");
      return new FluxgateApp(*this);
    }

    App * generateDefenseApp() {
      debug_println("ERROR! FluxgateCart has no defense app");
      return NULL;
    }

    void backgroundUpdate() {}
    void renderWidget(char * slot, byte widgetNumber) {
      // TODO here: Render ambient temp widget
    }
};
