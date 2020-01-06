#include "app.h"
#include "cart.h"
#include "pins.h"

#define thermoduleLaserGpio 1

// Lots of code in here ripped off from Adafruit_MLX90614 library

//#define MLX90614_I2CADDR 0x5A
#define MLX90614_I2CADDR 0x1B // Changed address to avoid conflict with CSS811
// RAM
#define MLX90614_RAWIR1 0x04
#define MLX90614_RAWIR2 0x05
#define MLX90614_TA 0x06
#define MLX90614_TOBJ1 0x07
#define MLX90614_TOBJ2 0x08
// EEPROM
#define MLX90614_TOMAX 0x20
#define MLX90614_TOMIN 0x21
#define MLX90614_PWMCTRL 0x22
#define MLX90614_TARANGE 0x23
#define MLX90614_EMISS 0x24
#define MLX90614_CONFIG 0x25
#define MLX90614_ADDR 0x0E
#define MLX90614_ID1 0x3C
#define MLX90614_ID2 0x3D
#define MLX90614_ID3 0x3E
#define MLX90614_ID4 0x3F

uint16_t read16(uint8_t a) {
  uint16_t ret;

  Wire1.beginTransmission(MLX90614_I2CADDR); // start transmission to device
  Wire1.write(a); // sends register address to read from
  Wire1.endTransmission(false); // end transmission

  Wire1.requestFrom(MLX90614_I2CADDR, (uint8_t)3);// send data n-bytes read
  ret = Wire1.read(); // receive DATA
  ret |= Wire1.read() << 8; // receive DATA

  Wire1.read(); // Not sure what this (labeled 'pec' in code I ripped off) is for

  return ret;
}

float readTemp(uint8_t reg) {
  float temp;

  temp = read16(reg);
  temp *= .02;
  temp  -= 273.15;
  return temp;
}

double readObjectTempF(void) {
  return (readTemp(MLX90614_TOBJ1) * 9 / 5) + 32;
}


double readAmbientTempF(void) {
  return (readTemp(MLX90614_TA) * 9 / 5) + 32;
}

class IrThermometerApp : public App {
  private:
    byte laserPin;

  public:
    IrThermometerApp(Cartridge & owner) : App(owner) {
      laserPin = cartGpioPins[owner.slot][thermoduleLaserGpio];
    }
    virtual ~IrThermometerApp() {}

    void setup() {
      // Laser pin direction is handled by the Cartridge
      // DO NOT CALL Wire1.begin() - I2C has already been configured by main code

      digitalWrite(laserPin, HIGH);
      Wire1.setClock(I2C_RATE_100);
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      static char tempString[6];

      display->bufferedPrint("Object temp: ", 0, 0);
      itoa(readObjectTempF(), tempString, 10);
      display->bufferedPrint(tempString);
      display->bufferedPrint('\x1B'); // DegF symbol

      display->bufferedPrint("Ambient temp: ", 1, 0);
      itoa(readAmbientTempF(), tempString, 10);
      display->bufferedPrint(tempString);
      display->bufferedPrint('\x1B'); // DegF symbol
    }

    void teardown(bool ejected) {
      digitalWrite(laserPin, LOW);
      Wire1.setClock(I2C_RATE_400);
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      return 0x00;
    }
};

class Thermodule : public Cartridge {
  private:
    byte laserPin;
  
  public:
    Thermodule(byte slot) : Cartridge(slot) {
      id = thermometerModuleId;
      name = "Thermodule";
//      color = 0x661100;
      color = CRGB::OrangeRed;
      widgetsAvailable = 0;
      hasQuickfireApp = hasMainApp = true;

      laserPin = cartGpioPins[slot][thermoduleLaserGpio];

      debug_println("Thermodule constructed");
    }

    virtual ~Thermodule() {}

    void onInsert() {
      digitalWrite(laserPin, LOW);
      pinMode(laserPin, OUTPUT);
      debug_println("Thermodule inserted");
    }

    void onEject() {
      debug_println("Thermodule ejected");
      digitalWrite(laserPin, LOW);
      pinMode(laserPin, INPUT);
    }

    App * generateQuickfireApp() {
      debug_println("Thermodule QF generated");
      return new IrThermometerApp(*this);
    }

    App * generateMainApp() {
      debug_println("Thermodule main generated");
      return new IrThermometerApp(*this);
    }

    App * generateDefenseApp() {
      debug_println("ERROR! Thermodule has no defense app");
      return NULL;
    }

    void backgroundUpdate() {}
    void renderWidget(char * slot, byte widgetNumber) {
      // TODO here: Render ambient temp widget
    }
};


