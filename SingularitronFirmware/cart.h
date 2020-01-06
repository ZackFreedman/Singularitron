#ifndef Cart_h_
#define Cart_h_

#include <FastLED.h>

#include "app.h"
#include "bufferedVfd.h"

#define enviroModuleId 0xC0FF
#define frickinLaserModuleId 0x1A5E
#define flashlightModuleId 0x117E
#define thermometerModuleId 0xFEE1
#define sdCardModuleId 0xDA7A
#define testModuleId 0x7E57

class App;

class Cartridge {
  public:
    Cartridge(byte slot) : slot(slot), id(0x00), name("UNIDENTIFIED MODULE"), color(0x00), widgetsAvailable(0) {}
    virtual ~Cartridge() {}

    byte slot;

    unsigned int id;
    char * name;
    CRGB color;
    byte widgetsAvailable;

    bool hasQuickfireApp = false;
    virtual App * generateQuickfireApp() = 0;

    bool hasMainApp = false;
    virtual App * generateMainApp() = 0;

    bool hasDefenseApp = false;
    virtual App * generateDefenseApp() = 0;

    virtual void onInsert() = 0;
    virtual void onEject() = 0;

    elapsedMillis lastBackgroundUpdateTimestamp;
    unsigned long backgroundUpdateInterval; // 0 means never update. Intervals are mandatory.

    virtual void backgroundUpdate() {}
    virtual void renderWidget(BufferedVfd * display, byte widgetNumber) {} // Home screen will set cursor for you. Widgets must be < 10 chars.
};

#endif

