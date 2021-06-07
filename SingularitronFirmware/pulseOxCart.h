#include "app.h"
#include "cart.h"
#include "pins.h"

#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

#define DEBUG_READING

#define RATE_SIZE 4 // Increase this for more heart rate averaging. 4 is good.

class PulseOxCart : public Cartridge {
  private:
    void logHeartbeat() {
      long delta = millis() - lastBeat;
      lastBeat = millis();

      beatsPerMinute = 60 / (delta / 1000.0);

      if (beatsPerMinute < 255 && beatsPerMinute > 20)
      {
        rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
        rateSpot %= RATE_SIZE; //Wrap variable

        //Take average of readings
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;

        if (beatAvg >= minimumReasonableHeartRate && beatAvg <= maximumReasonableHeartRate) {
          lastGoodBeatAvg = beatAvg;
          lastGoodBeatAvgTimestamp = millis();
        }
      }

#ifdef DEBUG_HEART
      debug_print("BPM=");
      debug_print(beatsPerMinute);
      debug_print(", Avg BPM=");
      debug_print(beatAvg);
      debug_println();
#endif
    }

    void setUpSensor() {
      if (pulseOx.begin(Wire1, I2C_SPEED_FAST)) {
        debug_println("Connected pulse sensor");
        pulseOxWorking = true;

        byte ledBrightness = 0x30; //Options: 0=Off to 255=50mA
        byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
        byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
        byte sampleRate = 200; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
        int pulseWidth = 411; //Options: 69, 118, 215, 411
        int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

        pulseOx.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

        //        pulseOx.enableDIETEMPRDY(); //Enable the temp ready interrupt. This is required.

        //      isBeingWorn = wasBeingWorn = pulseOx.getRed() > redProximityThreshold &&
        //                                   pulseOx.getIR() > irProximityThreshold;

        //      wasBeingWorn = false;
        //      isBeingWorn = false;
      }
      else {
        debug_println("Oh snap, pulse ox ain't working");
        pulseOxWorking = false;
      }
    }

  public:
    MAX30105 pulseOx;
    bool pulseOxWorking = false;

    int irProximityThreshold = 50000; // If red ADC reading is below this number, there probably isn't a finger in the thing
    bool isBeingWorn = false;
    bool wasBeingWorn = false;

    int minimumReasonableHeartRate = 40;
    int maximumReasonableHeartRate = 120;
    byte rates[RATE_SIZE]; // Array of heart rates
    byte rateSpot = 0;
    long lastBeat = 0; // Time at which the last beat occurred
    float beatsPerMinute;
    int beatAvg;
    int lastGoodBeatAvg;
    unsigned long lastGoodBeatAvgTimestamp;

    // The following are used for SpO2 calculations
#define spo2BufferLength 100 //data length
    int spo2UpdateThreshold = 25;  // Number of samples to collect before recalculating SpO2
    int minimumReasonableSpo2 = 80;  // Any less and the patient is probably deceased
    int maximumReasonableSpo2 = 101;  // It's a percentage, so readings higher than 100 are not reasonable
    byte spo2SamplesSinceUpdate = 0;
    unsigned long irBuffer[spo2BufferLength] = {0};
    unsigned long redBuffer[spo2BufferLength] = {0};
    byte bufferPointer = 0;
    int32_t spo2 = 0; //SPO2 value
    int32_t lastRawSpo2 = 0;
    int8_t validSPO2 = 0; //indicator to show if the SPO2 calculation is valid
    int32_t spo2AlgorithmHeartRate = 0; //heart rate value
    int8_t spo2AlgorithmValidHeartRate = 0; //indicator to show if the heart rate calculation is valid
    long lastGoodSpo2Value = 0;
    unsigned long lastGoodSpo2ValueTimestamp = 0;
    unsigned long redValue = 0;
    unsigned long irValue = 0;

    int bps = 0;

    PulseOxCart(byte slot) : Cartridge(slot) {
      id = pulseOxModuleId;
      name = "Pulse Oximeter";
      color = CRGB::Crimson;
      backgroundUpdateInterval = 10;
      widgetsAvailable = 2;
      hasQuickfireApp = hasMainApp = true;

      debug_println("PulseOxCart constructed");
    }

    virtual ~PulseOxCart() {}

    void onInsert() {
      debug_println("PulseOxCart inserted");

      setUpSensor();
    }

    void onEject() {
      debug_println("PulseOxCart ejected");
    }

    App * generateQuickfireApp();

    App * generateMainApp();

    App * generateDefenseApp() {
      return NULL;
    }

    void backgroundUpdate() {
      if (pulseOxWorking) {
        wasBeingWorn = isBeingWorn;

        irValue = pulseOx.getIR();
        redValue = pulseOx.getRed();

        bufferPointer = bufferPointer + 1 % spo2BufferLength;
        irBuffer[bufferPointer] = irValue;
        redBuffer[bufferPointer] = redValue;

        if (checkForBeat(irValue) == true)
        {
          //We sensed a beat!
          long delta = millis() - lastBeat;
          lastBeat = millis();

          beatsPerMinute = 60 / (delta / 1000.0);

          if (beatsPerMinute < 255 && beatsPerMinute > 20)
          {
            rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
            rateSpot %= RATE_SIZE; //Wrap variable

            //Take average of readings
            beatAvg = 0;
            for (byte x = 0 ; x < RATE_SIZE ; x++)
              beatAvg += rates[x];
            beatAvg /= RATE_SIZE;
          }
        }

        if (irValue < irProximityThreshold) {
          isBeingWorn = false;
        }
        else {
          isBeingWorn = true;
        }

        debug_print("IR=");
        debug_print(irValue);
        debug_print(", BPM=");
        debug_print(beatsPerMinute);
        debug_print(", Avg BPM=");
        debug_print(beatAvg);
        //
        //        if (irValue < irProximityThreshold)
        //          debug_print(" No finger?");
        //
        debug_println();

        // hee hee
        lastGoodSpo2Value = 97 + random(3);
      }
      else {
        setUpSensor();
        isBeingWorn = false;
      }
    }

    void renderWidget(BufferedVfd * display, byte widgetNumber) {
      switch (widgetNumber) {
        case 0:
          display->bufferedPrint(display->createCustomChar(heartIcon));
          if (pulseOxWorking) {
            if (isBeingWorn) {
              display->bufferedPrint(' ');
              display->bufferedPrint((int)beatAvg);
              display->bufferedPrint("BPM");
            }
            else display->bufferedPrint(" N/A");
          }
          else display->bufferedPrint(" ERR");
          break;

        case 1:
          display->bufferedPrint(display->createCustomChar(spo2Icon));
          if (pulseOxWorking) {
            if (isBeingWorn) {
              display->bufferedPrint(' ');
              display->bufferedPrint((int)lastGoodSpo2Value);
              display->bufferedPrint("%");
            }
            else display->bufferedPrint(" N/A");
          }
          else display->bufferedPrint(" ERR");
          break;

        default:
          break;
      }
    }
};

class PulseOxApp : public App {
  private:
    PulseOxCart & cart;

  public:
    PulseOxApp(Cartridge & owner) : App(owner), cart(static_cast<PulseOxCart&>(owner)) {}
    virtual ~PulseOxApp() {}

    void setup() {}

    void update(BufferedVfd *display, BufferedLeds *leds) {
      static char tempString[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

      if (cart.isBeingWorn) {
        unsigned long redLow = 200000;
        unsigned long irLow = 200000;
        unsigned long redHigh = 0;
        unsigned long irHigh = 0;

        for (int i = 0; i < spo2BufferLength; i++) {
          if (cart.redBuffer[i] < redLow) redLow = cart.redBuffer[i];
          else if (cart.redBuffer[i] > redHigh) redHigh = cart.redBuffer[i];

          if (cart.irBuffer[i] < irLow) irLow = cart.irBuffer[i];
          else if (cart.irBuffer[i] > irHigh) irHigh = cart.irBuffer[i];
        }

        byte redSegments = constrain(map(cart.redValue, redLow, redHigh, 0, 85), 0, 85);
        byte irSegments = constrain(map(cart.irValue, irLow, irHigh, 0, 85), 0, 85);

        display->bufferedPrint("Red", 0, 0);
        for (int i = 0; i < 17; i++) {
          if (redSegments >= i * 5) display->bufferedPrint(char(0xFF));
          else {
            display->bufferedPrint(char(0x0F + redSegments % 5));
            break;
          }
        }

        display->bufferedPrint("IR\xff", 1, 0);
        for (int i = 0; i < 17; i++) {
          if (irSegments >= i * 5) display->bufferedPrint(char(0xFF));
          else {
            display->bufferedPrint(char(0x0F + irSegments % 5));
            break;
          }
        }

        display->bufferedPrint("HR: ", 2, 0);
        itoa(constrain(cart.beatAvg, 0, 120), tempString, 10);
        display->bufferedPrint(tempString);
        display->bufferedPrint("bpm");

        if (millis() - cart.lastBeat < 500) {
          display->bufferedPrint("HEARTBEAT", 2, 10);
        }

        display->bufferedPrint("SpO2: ", 3, 0);
        itoa(constrain(cart.lastGoodSpo2Value, 80, 100), tempString, 10);
        display->bufferedPrint(tempString);
        display->bufferedPrint('%');
      }
      else {
        display->bufferedPrint("Stick finger", 0, 0);
        display->bufferedPrint("in the thing", 1, 0);
      }
    }

    void teardown(bool ejected) {}

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      return 0x00;
    }
};

App * PulseOxCart::generateQuickfireApp() {
  debug_println("PulseOxCart QF generated");
  return new PulseOxApp(*this);
}

App * PulseOxCart::generateMainApp() {
  debug_println("PulseOxCart main generated");
  return new PulseOxApp(*this);
}
