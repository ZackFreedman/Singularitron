#include "app.h"
#include "cart.h"
#include "pins.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class OledCart : public Cartridge {
  private:
    bool firstTime = true;

  public:
    Adafruit_SSD1306 oled;

    OledCart(byte slot) : Cartridge(slot), oled(128, 32, &Wire1, -1) {
      id = oledModuleId;
      name = "Another display?!";
      color = CRGB::Aquamarine;
      widgetsAvailable = 0;
      backgroundUpdateInterval = 30000;
      hasQuickfireApp = hasMainApp = false;

      oled.begin(SSD1306_SWITCHCAPVCC, 0x3C, 0, 0);  // initialize with the I2C addr 0x3C (for the 128x32)
      oled.clearDisplay();

      oled.setTextSize(2);
      oled.setTextColor(WHITE);
      oled.setCursor(5, 2);
      oled.println("WHAT IS");
      oled.setCursor(5, 18);
      oled.println("MY PURPOSE");

      oled.display();

      debug_println("OledCart constructed");
    }

    virtual ~OledCart() {}

    void onInsert() {
      debug_println("OledCart inserted");
    }

    void onEject() {
      debug_println("OledCart ejected");
    }

    App * generateQuickfireApp() {
      debug_println("ERROR! OledCart has no apps");
      return NULL;
    }

    App * generateMainApp() {
      debug_println("ERROR! OledCart has no apps");
      return NULL;
    }

    App * generateDefenseApp() {
      debug_println("ERROR! OledCart has no apps");
      return NULL;
    }

    void backgroundUpdate() {
      if (firstTime) firstTime = false;
      else {
        oled.clearDisplay();

        oled.setTextSize(4);
        oled.setTextColor(WHITE);
        oled.setCursor(20, 5);
        oled.println("HODL");

        oled.display();
      }
    }

    void renderWidget(char * slot, byte widgetNumber) {

    }
};
