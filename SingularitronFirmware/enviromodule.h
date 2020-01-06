#include "app.h"
#include "cart.h"
#include "icons.h"

#include <TimeLib.h>
#include <SparkFunBME280.h>
#include <SparkFunCCS811.h>
// Note: CCS811 library has been modified to use Wire1
// TODO: Fork CCS811 library to have selectable I2C interface

#define bme280Address 0x77
#define ccs811Address 0x5A

#define ccsConditioningTime 20 * 60 // 20 minutes between mode 0 and modes 1-4
#define ccsDropRateTime 10 * 60 // 10 minutes of mode 0 between a sampling mode and a lower-sample-rate mode

//#define ccsConditioningTime 10
//#define ccsDropRateTime 5

#define ccsMode0 0
#define ccsMode1 1
#define ccsMode2 2
#define ccsMode3 3
#define ccsMode4 4
#define ccsConditioningToMode1 10 // Omitting the rest for brevity
#define ccsConditioningToMode3 11
#define ccsDroppingToMode3 12

class EnviroModule : public Cartridge {
  private:
    class EnviroApp;

    BME280 bme;
    CCS811 ccs;
    bool bmeAvailable;
    bool ccsAvailable;

    float humidity = 0.0;
    float temperatureC = 0.0;
    float temperatureF = 0.0;
    float pressure = 0.0;
    float altitude = 0.0;
    int co2 = 0;
    int voc = 0;

    time_t ccsChangeTimestamp;
    byte ccsMode = ccsMode0;

  public:
    EnviroModule(byte slot) : Cartridge(slot), ccs(ccs811Address) {
      id = enviroModuleId;
      name = "EnviroModule";
      //      color = 0x116600;
      color = CRGB::Green;
      widgetsAvailable = 6;
      hasMainApp = true;
      backgroundUpdateInterval = 20000; // Update every 20 sec, since that's the lowest rate the BME can go

      debug_println("EnviroModule constructed");
    }

    virtual ~EnviroModule() {}

    void onInsert() {
      debug_println("EnviroModule inserted");

      Wire1.setClock(I2C_RATE_400); // TODO: Make this cart play nice with the Thermodule, which needs 100k rate

      bme.setI2CAddress(bme280Address);

      if (bme.beginI2C(Wire1)) {
        debug_println("BME280 OK!");
        bmeAvailable = true;
        bme.setReferencePressure(102000);
        bme.setFilter(1);
        byte os = 4;
        bme.setTempOverSample(os);
        bme.setPressureOverSample(os);
        bme.setHumidityOverSample(os);

        bme.setMode(MODE_NORMAL); // Take measurements rarely in background, to save power
        bme.setStandbyTime(7); // 20 sec between readings
      }
      else {
        debug_println("BME280 faulty!");
        bmeAvailable = false;
      }

      if (ccs.begin() == CCS811Core::SENSOR_SUCCESS) {
        debug_println("CCS811 OK!");
        ccsAvailable = true;
        ccs.setDriveMode(3); // Low-power run mode
        ccsChangeTimestamp = now();
        ccsMode = ccsConditioningToMode3;
      }
      else {
        debug_println("CCS811 faulty!");
        ccsAvailable = false;
      }
    }

    void onEject() {
      debug_println("EnviroModule ejected");
    }

    App * generateQuickfireApp();

    App * generateMainApp();

    App * generateDefenseApp();

    void backgroundUpdate();
    void renderWidget(BufferedVfd * display, byte widgetNumber);
};

void EnviroModule::backgroundUpdate() {
  if (bmeAvailable) {
    humidity = bme.readFloatHumidity();
    temperatureC = bme.readTempC();
    temperatureF = temperatureC * 1.8 + 32.0;
    pressure = bme.readFloatPressure() / 100.0; // Base units are Pa, so divide by 100 for hPa
    altitude = bme.readFloatAltitudeFeet();
  }
  else if (bme.beginI2C(Wire1)) {
    debug_println("BME280 OK!");
    bmeAvailable = true;
    bme.setReferencePressure(102000);
    bme.setFilter(1);
    byte os = 4;
    bme.setTempOverSample(os);
    bme.setPressureOverSample(os);
    bme.setHumidityOverSample(os);

    bme.setMode(MODE_NORMAL); // Take measurements rarely in background, to save power
    bme.setStandbyTime(7); // 20 sec between reading
  }

  if (ccsAvailable) {
    if ((ccsMode == ccsConditioningToMode1 || ccsMode == ccsConditioningToMode3) && (now() - ccsChangeTimestamp) > ccsConditioningTime) {
      debug_println("CCS conditioning done!");
      if (ccsMode == ccsConditioningToMode1) ccsMode = ccsMode1;
      else ccsMode = ccsMode3;
      ccsChangeTimestamp = now();
    }
    else if (ccsMode == ccsDroppingToMode3 && (now() - ccsChangeTimestamp) > ccsDropRateTime) {
      debug_println("CCS rate drop done!");
      ccs.setDriveMode(3);
      ccsMode = ccsMode3;
      ccsChangeTimestamp = now();
    }

    if (ccsMode == ccsMode0 || ccsMode == ccsDroppingToMode3) {
      co2 = 0;
      voc = 0;
    }
    else {
      if (bmeAvailable) ccs.setEnvironmentalData(humidity, temperatureC); // Forward some data to CCS811 for self-calibration
      if (ccs.dataAvailable()) {
        ccs.readAlgorithmResults();
        co2 = ccs.getCO2();
        voc = ccs.getTVOC();
      }
    }
  }
  else if (ccs.begin() == CCS811Core::SENSOR_SUCCESS) {
    debug_println("CCS811 OK!");
    ccsAvailable = true;
    ccs.setDriveMode(3); // Low-power run mode
    ccsChangeTimestamp = now();
    ccsMode = ccsConditioningToMode3;
  }
}

void EnviroModule::renderWidget(BufferedVfd * display, byte widgetNumber) {
  switch (widgetNumber) {
    case 0:
      display->bufferedPrint(display->createCustomChar(humidityIcon));
      if (bmeAvailable) {
        //display->bufferedPrint(' ');
        display->bufferedPrint((int)humidity);
        display->bufferedPrint('%');
      }
      else display->bufferedPrint(" N/A");
      break;

    case 1:
      display->bufferedPrint(display->createCustomChar(thermometerIcon));
      if (bmeAvailable) {
        //display->bufferedPrint(' ');
        display->bufferedPrint((int)temperatureF);
        display->bufferedPrint('\x1b'); // DegF icon
      }
      else display->bufferedPrint(" N/A");
      break;

    case 2:
      display->bufferedPrint(display->createCustomChar(co2Icon));

      // Disabling altogether when inaccurate is too austere
      //      if (ccsAvailable && ccsMode != ccsMode0 && ccsMode != ccsDroppingToMode3) {
      //display->bufferedPrint(' ');
      if ((ccsAvailable && ccsMode != ccsMode0 && ccsMode != ccsDroppingToMode3) ||
          ccsMode == ccsConditioningToMode1 || ccsMode == ccsConditioningToMode3)
        display->bufferedPrint('\x8e'); // Tilde
      display->bufferedPrint(co2);
      if (co2 < 100000) display->bufferedPrint("ppm");
      //
      //  }
      //      else display->bufferedPrint(" N/A");
      break;

    case 3:
      display->bufferedPrint(display->createCustomChar(smokeIcon));

      // Disabling altogether when inaccurate is too austere
      //      if (ccsAvailable && ccsMode != ccsMode0 && ccsMode != ccsDroppingToMode3) {
      //display->bufferedPrint(' ');
      if ((ccsAvailable && ccsMode != ccsMode0 && ccsMode != ccsDroppingToMode3) ||
          ccsMode == ccsConditioningToMode1 || ccsMode == ccsConditioningToMode3)
        display->bufferedPrint('\x8e'); // Tilde
      display->bufferedPrint(voc);
      if (voc < 100000) display->bufferedPrint("ppb");
      //      }
      //      else display->bufferedPrint(" N/A");
      break;

    case 4:
      display->bufferedPrint(display->createCustomChar(altitudeIcon));
      if (bmeAvailable) {
        //display->bufferedPrint(' ');
        display->bufferedPrint(int(altitude));
        display->bufferedPrint("ft");
      }
      else display->bufferedPrint(" N/A");
      break;

    case 5:
      display->bufferedPrint(display->createCustomChar(barometerIcon));
      if (bmeAvailable) {
        //display->bufferedPrint(' ');
        display->bufferedPrint(int(pressure));
      }
      else display->bufferedPrint(" N/A");
      break;

    default:
      display->bufferedPrint("?????");
  }
}

class EnviroModule::EnviroApp : public App {
  private:
    EnviroModule & em;

  public:
    EnviroApp(Cartridge & owner) : App(owner), em(static_cast<EnviroModule&>(owner)) {}
    virtual ~EnviroApp() {}

    void setup() {
      em.backgroundUpdateInterval = 100; // What do we want? Responsiveness! When do we want it? Every 200ms or less!

      if (em.bmeAvailable) em.bme.setStandbyTime(1);

      if (em.ccsAvailable) {
        if (em.ccsMode == ccsMode3 || em.ccsMode == ccsMode2) {
          em.ccs.setDriveMode(1); // We can increase the rate freely
          em.ccsMode = ccsMode1;
        }
        else if (em.ccsMode == ccsDroppingToMode3) {
          em.ccs.setDriveMode(1); // Should already be conditioned for mode 1
          em.ccsMode = ccsMode1;
        }
        else if (em.ccsMode == ccsConditioningToMode3) {
          em.ccs.setDriveMode(1); // Condition into mode 1 instead
          em.ccsMode = ccsConditioningToMode1;
          // DO NOT change timer!
        }
        else if (em.ccsMode == ccsMode0) {
          em.ccs.setDriveMode(1); // Not sure how this could happen, but whatevs, condition into mode 1
          em.ccsMode = ccsConditioningToMode1;
          em.ccsChangeTimestamp = now();
        }
      }
    }

    void teardown(bool ejected) {
      if (ejected) return;

      em.backgroundUpdateInterval = 20000; // Responsiveness doesn't matter when you're not looking at it

      if (em.bmeAvailable) em.bme.setStandbyTime(7);

      if (em.ccsAvailable) {
        if (em.ccsMode == ccsMode1) {
          em.ccs.setDriveMode(0);
          em.ccsMode = ccsDroppingToMode3;
          em.ccsChangeTimestamp = now();
        }
        else if (em.ccsMode == ccsConditioningToMode1) {
          em.ccs.setDriveMode(3); // Condition into mode 3 instead
          em.ccsMode = ccsConditioningToMode3;
          // DO NOT change timer!
        }
      }
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      static char tempString[10];

      if (em.bmeAvailable) {
        sprintf(tempString, "Hum: %.0f%%", em.humidity);
        display->bufferedPrint(tempString, 0, 0);

        sprintf(tempString, "Prs: %.0f", em.pressure);
        display->bufferedPrint(tempString, 0, 10);

        sprintf(tempString, "Alt: %.0f", floor(em.altitude));
        display->bufferedPrint(tempString, 1, 0);

        sprintf(tempString, "Tmp: %.0f", em.temperatureF);
        display->bufferedPrint(tempString, 1, 10);
        display->bufferedPrint('\x1b'); // DegF symbol
      }
      else {
        display->bufferedPrint("Hum: N/A", 0, 0);
        display->bufferedPrint("Prs: N/A", 0, 10);
        display->bufferedPrint("Alt: N/A", 1, 0);
        display->bufferedPrint("Tmp: N/A", 1, 10);
      }

      if (em.ccsAvailable && em.ccsMode != ccsMode0 && em.ccsMode != ccsDroppingToMode3) {
        if (em.ccsMode == ccsConditioningToMode1 || em.ccsMode == ccsConditioningToMode3) {
          sprintf(tempString, "CO2: \x8e%i", em.co2);
          display->bufferedPrint(tempString, 2, 0);

          sprintf(tempString, "VOC: \x8e%i", em.voc);
          display->bufferedPrint(tempString, 2, 10);
        }
        else {
          sprintf(tempString, "CO2: %i", em.co2);
          display->bufferedPrint(tempString, 2, 0);

          sprintf(tempString, "VOC: %i", em.voc);
          display->bufferedPrint(tempString, 2, 10);
        }
      }
      else {
        // Completely disabling imprecise readings is too austere for demo purposes
        //        display->bufferedPrint("CO2: N/A", 2, 0);
        //        display->bufferedPrint("VOC: N/A", 2, 10);

        sprintf(tempString, "CO2: \x8e%i", em.co2);
        display->bufferedPrint(tempString, 2, 0);

        sprintf(tempString, "VOC: \x8e%i", em.voc);
        display->bufferedPrint(tempString, 2, 10);
      }

      display->bufferedPrint("> Zero altimeter", 3, 0);
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      if (clicks & knobMask) {
        if (em.bmeAvailable) em.bme.setReferencePressure(em.bme.readFloatPressure());
        return knobMask;
      }

      return 0x00;
    }
};

App * EnviroModule::generateQuickfireApp() {
  debug_println("ERROR! EnviroModule has no QF app");
  return NULL;
}

App * EnviroModule::generateMainApp() {
  debug_println("EnviroModule main generated");

  return new EnviroApp(*this);
}

App * EnviroModule::generateDefenseApp() {
  debug_println("ERROR! EnviroModule has no defense app");
  return NULL;
}
