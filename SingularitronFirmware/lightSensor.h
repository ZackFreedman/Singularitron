#include "app.h"
#include "cart.h"
#include "pins.h"

#include <SparkFun_APDS9960.h>

class LightSensorCart : public Cartridge {
  public:
    SparkFun_APDS9960 apds = SparkFun_APDS9960();

    LightSensorCart(byte slot) : Cartridge(slot) {
      id = thermometerModuleId;
      name = "Light Sensor";
      color = CRGB::FairyLightNCC;
      widgetsAvailable = 0;
      hasQuickfireApp = hasMainApp = true;

      apds.init();
      apds.setProximityGain(PGAIN_2X);
      apds.enableProximitySensor(false);
      apds.enableLightSensor(false);
      apds.disablePower();

      debug_println("LightSensorCart constructed");
    }

    virtual ~LightSensorCart() {}

    void onInsert() {
      debug_println("LightSensorCart inserted");
    }

    void onEject() {
      debug_println("LightSensorCart ejected");
    }

    App * generateQuickfireApp();

    App * generateMainApp();

    App * generateDefenseApp() {
      debug_println("ERROR! LightSensorCart has no defense app");
      return NULL;
    }

    void backgroundUpdate() {}
    void renderWidget(char * slot, byte widgetNumber) {
      // TODO here: Render ambient temp widget
    }
};

class LightSensorApp : public App {
  private:
    LightSensorCart & cart;
    uint16_t r, g, b, c;
    uint8_t proximity;

  public:
    LightSensorApp(Cartridge & owner) : App(owner), cart(static_cast<LightSensorCart&>(owner)) {}
    virtual ~LightSensorApp() {}

    void setup() {
      cart.apds.enablePower();
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      cart.apds.readAmbientLight(c);
      cart.apds.readRedLight(r);
      cart.apds.readGreenLight(g);
      cart.apds.readBlueLight(b);

      cart.apds.readProximity(proximity);

      static char tempString[6] = {'0', '0', 0, 0, 0, 0};

      byte segments = constrain(map(r, 0, 37889, 0, 75), 0, 75);

      display->bufferedPrint("  Red", 0, 0);
      for (int i = 0; i < 15; i++) {
        if (segments >= i * 5) display->bufferedPrint(char(0xFF));
        else {
          display->bufferedPrint(char(0x0F + segments % 5));
          break;
        }
      }

      segments = constrain(map(g, 0, 37889, 0, 75), 0, 75);

      display->bufferedPrint("Green", 1, 0);
      for (int i = 0; i < 15; i++) {
        if (segments >= i * 5) display->bufferedPrint(char(0xFF));
        else {
          display->bufferedPrint(char(0x0F + segments % 5));
          break;
        }
      }

      segments = constrain(map(b, 0, 37889, 0, 75), 0, 75);

      display->bufferedPrint(" Blue", 2, 0);
      for (int i = 0; i < 15; i++) {
        if (segments >= i * 5) display->bufferedPrint(char(0xFF));
        else {
          display->bufferedPrint(char(0x0F + segments % 5));
          break;
        }
      }

      display->bufferedPrint("Proximity: ", 3, 0);
      itoa(proximity, tempString, 10);
      display->bufferedPrint(tempString);
    }

    void teardown(bool ejected) {
      cart.apds.disablePower();
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      return 0x00;
    }
};

App * LightSensorCart::generateQuickfireApp() {
  debug_println("LightSensorCart QF generated");
  return new LightSensorApp(*this);
}

App * LightSensorCart::generateMainApp() {
  debug_println("LightSensorCart main generated");
  return new LightSensorApp(*this);
}
