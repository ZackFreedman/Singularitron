#ifndef App_h_
#define App_h_

#include "cart.h"
#include "bufferedVfd.h"
#include "bufferedLeds.h"

#define updownMask 0b01000000
#define knobMask 0b00100000
#define button0Mask 0b00010000
#define button1Mask 0b00001000
#define button2Mask 0b00000100
#define button3Mask 0b00000010
#define homeButtonMask 0b00000001

class Cartridge;

class App {
  public:
    App(Cartridge & owner) : owner(owner) {}  // 
    virtual ~App() {}

    bool sleepAllowed = false; // Device can sleep while this app is active
    Cartridge & owner;

    virtual void setup() = 0;
    virtual void update(BufferedVfd *display, BufferedLeds *leds) = 0; // For now, output directly to framebuffer. System will render at end of loop.
    virtual void teardown(bool ejected) = 0;

    // Buttons bitmask: [0][0][knob press][button 0][button 1][button 2][button 3][home button]
    // Knob up = -1, knob down = 1
    // Return a mask of any control events that have been handled by the app and should not be sent to the system
    // Byte 6 of return mask is whether knobDelta has been handled
    virtual byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) = 0;

    boolean wantsToQuit = false;
};

#endif
