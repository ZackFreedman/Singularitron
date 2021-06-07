#include <i2c_t3.h>
#include <Encoder.h>
#include <TimeLib.h>
#include <SPI.h>
#include <LowPower_Teensy3.h>
//#include <Snooze.h>

/* The most complicated useless side project of all time.
   Copyright 2017 Zack Freedman
*/

// The following dumb bullshit just lets us turn off serial debug

#define DEBUG

#define GET_MACRO(_1, _2, NAME, ...) NAME
#ifdef DEBUG
#define debug_print_formatted(x, y) Serial.print(x,y)
#define debug_print_noformat(x) Serial.print(x)
#define debug_println_formatted(x, y) Serial.println(x, y)
#define debug_println_noformat(x) Serial.println(x)
#define debug_write(x) Serial.write(x)
#else
#define debug_print_formatted(x, y)
#define debug_print_noformat(x)
#define debug_println_formatted(x, y)
#define debug_println_noformat(x)
#define debug_write(x)
#endif
#define debug_print(...) GET_MACRO(__VA_ARGS__, debug_print_formatted, debug_print_noformat)(__VA_ARGS__)
#define debug_println(...) GET_MACRO(__VA_ARGS__, debug_println_formatted, debug_println_noformat)(__VA_ARGS__)

#define sanity(x) debug_println("Sanity " #x);

// End dumb debugging bullshit

#include "pins.h"
#include "states.h"
#include "flavortext.h"

#include "settings.h"

#include "imu.h"
#include "bufferedVfd.h"
#include "bufferedLeds.h"
#include "cartEeprom.h"

#include "app.h"
#include "cart.h"
#include "systemApps.h"

#include "breathalyzer.h"
#include "enviromodule.h"
#include "fluxgate.h"
#include "frickinlaser.h"
#include "gratuitousflashlight.h"
#include "lightSensor.h"
#include "oledCart.h"
#include "pulseOxCart.h"
#include "sdcardmodule.h"
#include "testModule.h"
#include "thermodule.h"

#define noAnimation 0
#define rightWipe 1
#define leftWipe 2

float headingOffset = -45.0;

IMU imu = IMU();
// This is the entire compass legend. A subset of it is rendered as the compass dial.
// 0xA5 is a dot
char compassLegend[] = {'W', 0xA5, 0xA5, 0xA5, 0xA5, 'N', 0xA5, 0xA5, 0xA5, 0xA5, 'E', 0xA5, 0xA5, 0xA5, 0xA5, 'S', 0xA5, 0xA5, 0xA5, 0xA5};
#define compassLegendSize 20

Encoder knob(encoderB, encoderA);
long oldPosition = 0;

// SnoozeDigital didn't work so good. Maybe try again some time, sleep is pretty jank
//SnoozeDigital snoozeDigital;
//SnoozeTimer snoozeTimer;
//SnoozeUSBSerial snoozeUsb;
//SnoozeSPI snoozeSpi;
//SnoozeBlock snoozeConfigWithBackgroundUpdates(snoozeDigital, snoozeTimer, snoozeUsb);//, snoozeSpi);
//SnoozeBlock snoozeConfigNoBackgroundUpdates(snoozeDigital, snoozeUsb);//, snoozeSpi);
bool justWokeUp;
IntervalTimer sleepingBackgroundUpdateTimer;

#define homeButtonColor 0x38FF38
//#define homeButtonColor CRGB::DarkCyan

char widgetBuffer[10];
BufferedVfd display;
BufferedLeds leds;
CRGB preAnimationLeds[5];
byte activeAnimation = noAnimation;
char preAnimationFramebuffer[4][20];
char flavorTextBuffer[4][21];
elapsedMillis timeSinceAnimationStarted;

#define maxChargeReading 3700 // 12-bit ADC reading of 4.15V divided down to 3.07V-ish
#define minChargeReading 3200 // 12-bit ADC reading of 3.4V divided down to 2.58V-ish
#define chargeReadingRange maxChargeReading - minChargeReading
#define ntcFaultDutyCycleLow 6.25f // %
#define ntcFaultDutyCycleHigh 93.75f // %
#define badBatteryDutyCycleLow 12.5f // %
#define badBatteryDutyCycleHigh 87.5f // %
#define dutyCycleJitter 2.0f // Percentage points
#define chargePinPeriod 28 // 35KHz frequency = 28.57us period
#define chargePinPeriodJitter 2 // us
#define batteryNotCharging 0
#define batteryCharging 1
#define batteryNtcFault 2
#define batteryChargeFault 3
volatile bool chargePinState;
volatile unsigned long chargePinChangeTimestamp;
volatile unsigned long chargePinHighTime;
volatile unsigned long chargePinLowTime;
byte batteryState = batteryNotCharging;
int batteryBars = 10; // Used to stabilize charge indicator
char batteryIconChar = 0xFF; // Used to draw battery icon near charge indicator

bool phoneConnected = false;
char phoneIconChar = 0xFF;

byte firstWidgetToRender = 0; // Used to scroll widgets. This widget's line and the following line of widgets are displayed on the homescreen.

byte state = indeterminateState;
byte previousState = indeterminateState;
byte activeCartSlot = 0xFF;
byte previousCartSlot = 0xFF;

bool homeQuickfire = false; // Used for quick time-checking mode
elapsedMillis timeSinceInterstitial; // Used to display insertion/ejection feedback

TEENSY3_LP power = TEENSY3_LP();
elapsedMillis timeAwake;

volatile bool mustPollButtons, mustPollCarts, mustPollImu;

const byte buttonMasks[4] = {button0Mask, button1Mask, button2Mask, button3Mask}; // Used to create and read bitmasks in App.OnControlEvent()
byte lastButtonStates = 0x00; // Bitmask of previous button states for edge detection, little-endian
unsigned long knobDownTimestamp, homeDownTimestamp;
unsigned long buttonDownTimestamps[4];  // Used to calculate hold
bool knobJustClicked, homeJustClicked;
bool buttonsJustClicked[4];
bool knobIsHeld, homeIsHeld;
bool buttonsHeld[4];
bool homeHoldHandled;

#define cartDetectSettleTime 50 // ms that cart detect pin must remain in same state before it's considered to be inserted or ejected
unsigned long cartDetectTimestamps[4];
Cartridge * carts[4];
App * activeApp;
byte lastCartStates = 0b10101010; // Bitmask of previous carts for edge detection, little-endian. Active low, so this represents "everything ejected".

#define STARTWOKE

void setup() {
  Serial.begin(9600);
  ble.begin(9600);

  loadSettings();

  analogReadResolution(12);
  //  analogReference(INTERNAL);

  // Buzz buzz
  pinMode(vibes, OUTPUT);

  // Configure interrupts
  pinMode(buttonInterruptPin, INPUT_PULLUP);
  pinMode(hotplugInterruptPin, INPUT_PULLUP);
  pinMode(sentralInterruptPin, INPUT); // Shouldn't need a pullup
  pinMode(chargeStatusPin, INPUT_PULLUP);

  //  snoozeDigital.pinMode(buttonInterruptPin, INPUT_PULLUP, FALLING);
  //  snoozeDigital.pinMode(hotplugInterruptPin, INPUT_PULLUP, FALLING);
  //  snoozeDigital.pinMode(chargeStatusPin, INPUT_PULLUP, FALLING);

  // Configure interfaces
  SPI.begin();
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_400);
  Wire.setDefaultTimeout(2000);
  Wire1.begin(I2C_MASTER, 0x00, I2C_PINS_29_30, I2C_PULLUP_EXT, I2C_RATE_400);
  Wire1.setDefaultTimeout(2000);

  setSyncProvider(getTeensy3Time); // Draw current time from internal RTC
  setSyncInterval(60); // Read internal RTC every 60 seconds. Necessary? Correct interval? Hrmmm?

  randomSeed(now());

  // Initialize IMU
  imu.setup();
  //  attachInterrupt(digitalPinToInterrupt(sentralInterruptPin), onImuEvent, RISING);

  // Initialize display
  display.setup();
  display.setBrightness(displayBrightness);
  leds.setup();
  leds.setBrightness(ledBrightness);

  // Start with display off in case device is recharging empty battery
#ifdef STARTWOKE
  display.powerUp();
#endif

  // Initialize button driver
  for (int i = 0; i < 3; i++) {
    Wire.beginTransmission(buttonDriver);
    Wire.write(0x03); // Select control register
    Wire.write(0x3F); // Assign pins 0-5 to inputs
    byte error = Wire.endTransmission();
    if (error == 0) {
      debug_println("Button driver online");
      break;
    }
    else {
      debug_print("Button driver got error ");
      debug_println(error);
      delay(100);
    }
  }

  // Initialize cartridge IO expander
  for (int i = 0; i < 3; i++) {
    Wire.beginTransmission(cartIOExpander);
    Wire.write(0x03); // control register
    Wire.write(0b10101010); // SS pins are outputs, Detect pins are inputs
    byte error = Wire.endTransmission();
    if (error == 0) {
      debug_println("Cart IO expander online");
      break;
    }
    else {
      debug_print("Cart driver got error ");
      debug_println(error);
      delay(100);
    }
  }

  restoreInterrupts();
  pollButtons();
  pollCarts();

  // Initialize battery shit
  chargePinState = digitalRead(chargeStatusPin);
  attachInterrupt(digitalPinToInterrupt(chargeStatusPin), onBatteryEvent, CHANGE);

  #ifdef STARTWOKE
    switchStateTo(homeScreen);
  #endif
  #ifndef STARTWOKE
    switchStateTo(sleeping);
  #endif

//  switchStateTo(systemApp, byte(1));

//  delay(1000);
//  byte target = 0;
//  int valueToWrite = fluxgateModuleId;
//  debug_print("Programming cart on slot "); debug_println(target);
////  openEeprom(target);
//  debug_print("Value at 0 is 0x"); debug_println(readEeprom(0x00), HEX);
//  writeEeprom(0, valueToWrite, target);
//  debug_print("Wrote ID 0x"); debug_println(valueToWrite, HEX);
//  debug_print("Value at 0 is now 0x"); debug_println(readEeprom(0x00), HEX);
//  closeEeprom();
  //      delay(10000);
}

void loop() {
  // Save power if we're sleeping
  if (state == sleeping &&
      activeAnimation == noAnimation &&
      lastButtonStates == 0b11000000 &&
      cartDetectTimestamps[0] == 0 &&
      cartDetectTimestamps[1] == 0 &&
      cartDetectTimestamps[2] == 0 &&
      cartDetectTimestamps[3] == 0) { // Buttons/carts prevent sleep so time-related changes can be detected

    debug_println("Entering power save");
    display.powerDown();
    //    imu.sleep(); // DO NOT touch mustPollImu - don't want to clear it while the Status register is full
    ble.end();

    unsigned long nextBackgroundUpdate = 0;
    for (byte i = 0; i < 4; i++)
      if (carts[i] != NULL && carts[i]->backgroundUpdateInterval > 0
          && (carts[i]->backgroundUpdateInterval - carts[i]->lastBackgroundUpdateTimestamp) > nextBackgroundUpdate)
        nextBackgroundUpdate = carts[i]->backgroundUpdateInterval - carts[i]->lastBackgroundUpdateTimestamp;
    if (nextBackgroundUpdate > 0) {
      Serial.println(nextBackgroundUpdate);
      //      snoozeTimer.setTimer(nextBackgroundUpdate);
      //      Snooze.sleep(snoozeConfigWithBackgroundUpdates);
      sleepingBackgroundUpdateTimer.begin(wakeToUpdate, nextBackgroundUpdate * 1000); // This function uses microseconds
    }
    //    else Snooze.sleep(snoozeConfigNoBackgroundUpdates);

    //    delay(1000);
    power.Sleep();

    sleepingBackgroundUpdateTimer.end();

    /* ...dreams of enslaving feeble meatsacks... */

    debug_println("Leaving power save");
    ble.begin(9600);
    justWokeUp = mustPollButtons = mustPollCarts = true; // Might not need this
    setSyncProvider(getTeensy3Time); // Need to resync after waking up
    setSyncInterval(60); // Might not be necessary to do this here
  }

  while (ble.available()) {
    char incoming = ble.read();
    debug_print("BLE: ");
    debug_println(incoming);
    if (incoming == 'C') phoneConnected = true;
    else if (incoming == 'D') phoneConnected = false;
  }

  unsigned long startTime = micros();

  // Gentlemen, start your buffers

  // Poll shit
  if (mustPollButtons) pollButtons(); // Seems kinda obvious
  if (mustPollCarts) pollCarts(); // Yup
  //  if (mustPollImu) {
  if (digitalRead(sentralInterruptPin)) {
    imu.poll();
    //    mustPollImu = false;
  }

  // Handle recently inserted/ejected carts
  for (byte slot = 0; slot < 4; slot++) {
    if (cartDetectTimestamps[slot] > 0 && (millis() - cartDetectTimestamps[slot] >= cartDetectSettleTime)) {
      cartDetectTimestamps[slot] = 0;

      if (state == sleeping) switchStateTo(homeScreen); // Changing carts wakes from sleep
      else timeAwake = 0; // Changing carts resets sleep timeout

      if ((lastCartStates & (0b10 << (slot * 2))) == 0) { // True if cart insertion caused the last change
        if (carts[slot] == NULL) {
          debug_print("OK, cart "); debug_print(slot); debug_println(" is really inserted");

          // Read cart's ID from its EEPROM so we can load its drivers
          unsigned int loadedId = readEeprom(0x00, slot);
//          closeEeprom();

          switch (loadedId) {
            case breathalyzerModuleId:
              debug_println("Detected breathalyzer");
              carts[slot] = new BreathalyzerCart(slot);
              break;
            
            case frickinLaserModuleId:
              debug_println("Detected frickin' laser");
              carts[slot] = new FrickinLaser(slot);
              break;

            case flashlightModuleId:
              debug_println("Detected gratuitous flashlight");
              carts[slot] = new GratuitousFlashlight(slot);
              break;

            case thermometerModuleId:
              debug_println("Detected Thermodule");
              carts[slot] = new Thermodule(slot);
              break;

            case enviroModuleId:
              debug_println("Detected EnviroModule");
              carts[slot] = new EnviroModule(slot);
              break;

            case sdCardModuleId:
              debug_println("Detected SD card module");
              carts[slot] = new SDCardModule(slot);
              break;

            case testModuleId:
              debug_println("Detected top secret test module");
              carts[slot] = new TestCart(slot);
              break;

            case fluxgateModuleId:
              debug_println("Detected FLUXGATE MAGNETOMETER");
              carts[slot] = new FluxgateCart(slot);
              break;

            case lightSensorModuleId:
              debug_println("Detected APDS9960 breakout");
              carts[slot] = new LightSensorCart(slot);
              break;

            case oledModuleId:
              debug_println("Detected OLED on a string");
              carts[slot] = new OledCart(slot);
              break;

            case pulseOxModuleId:
              debug_println("Detected pulse ox");
              carts[slot] = new PulseOxCart(slot);
              break;

            default:
              debug_print("Cart ID "); debug_print(loadedId, HEX); debug_println(" not recognized");
              carts[slot] = new UnknownCart(slot);
              break;
          }

          if (carts[slot] != NULL) {
            carts[slot]->onInsert();
            if (carts[slot]->backgroundUpdateInterval > 0) {
              carts[slot]->lastBackgroundUpdateTimestamp = 0;
              carts[slot]->backgroundUpdate();
            }
          }

          switchStateTo(cartInsertInterstitial, slot);
          timeAwake = 0;
        }
      }
      else { // Cart ejection caused the last change
        if (carts[slot] != NULL) {
          debug_print("OK, cart "); debug_print(slot); debug_println(" is really ejected");
          if (activeApp != NULL && activeApp->owner.slot == slot) {
            debug_println("Closing its active app");
            //            switchStateTo(state, slot, true); // Switch to "not installed" screen, tell app it's being torn down because its cart was ejected
            switchStateTo(cartEjectInterstitial, slot, true);
            timeAwake = 0;
          }
          else switchStateTo(cartEjectInterstitial, slot);

          carts[slot]->onEject();
          delete carts[slot];
          carts[slot] = NULL;
        }
      }
    }
  }

  // Interpret battery monitor and CHRG pin
  // Clone volatile variables in case they change
  noInterrupts();
  byte chargeState = chargePinState;
  unsigned long highTime = chargePinHighTime;
  unsigned long lowTime = chargePinLowTime;
  unsigned long period = highTime + lowTime;
  interrupts();

  if (highTime != 0 && lowTime != 0 && (period < chargePinPeriod + chargePinPeriodJitter) && (period > chargePinPeriod - chargePinPeriodJitter)) {
    // A duty cycle of about 35KHz has been detected
    float dutyCycle = float(lowTime) / float(period) * 100.0f;

    // Read "error code" from duty cycle because Linear just has to be so fucking clever
    // ...making the pin both blink and machine-readable is pretty clever though
    if (((dutyCycle < ntcFaultDutyCycleLow + dutyCycleJitter) && (dutyCycle > ntcFaultDutyCycleLow - dutyCycleJitter)) ||
        ((dutyCycle < ntcFaultDutyCycleHigh + dutyCycleJitter) && (dutyCycle > ntcFaultDutyCycleHigh - dutyCycleJitter))) {
      // NTC fault detected
      if (batteryState != batteryNtcFault) {
        debug_println("Battery NTC fault detected?!");
        batteryState = batteryNtcFault;
      }
    }
    else if (((dutyCycle < badBatteryDutyCycleLow + dutyCycleJitter) && (dutyCycle > badBatteryDutyCycleLow - dutyCycleJitter)) ||
             ((dutyCycle < badBatteryDutyCycleHigh + dutyCycleJitter) && (dutyCycle > badBatteryDutyCycleHigh - dutyCycleJitter))) {
      // "Bad Battery" detected (not charging)
      if (batteryState != batteryChargeFault) {
        debug_println("Bad battery - not charging");
        batteryState = batteryChargeFault;
      }
    }

    // If neither fault was detected, something funny is going on. Ignore the error, it will be repeated in a few microseconds
  }
  else { // No duty cycle detected, charge pin indicates charge status
    if (chargeState) { // CHRG is active low
      if (micros() - chargePinChangeTimestamp > 150000 && batteryState != batteryNotCharging) {
        debug_println("Battery stopped charging");
        batteryState = batteryNotCharging;
        timeAwake = 0; // Ending charge wakes device up
        if (state == sleeping) switchStateTo(homeScreen);
      }
    }
    else if (micros() - chargePinChangeTimestamp > 150000 && batteryState != batteryCharging) {
      debug_println("Battery started charging!");
      batteryState = batteryCharging;
      timeAwake = 0; // Plugging in USB wakes device up
      if (state == sleeping) switchStateTo(homeScreen);
    }
  }

  // Update knob
  long newPosition = knob.read();
  int knobDelta = 0;
  if (newPosition != oldPosition && newPosition % 4 == 0) {
    if (newPosition > oldPosition) debug_println("Down!");
    else debug_println("Up!");

    knobDelta = (newPosition - oldPosition) / -4l;
    oldPosition = newPosition;
    timeAwake = 0; // Turning the knob keeps device awake but doesn't wake it up
  }

  // Update controls
  if (holdCutoffTime > 0) {
    if (!knobIsHeld && knobDownTimestamp > 0 && (millis() - knobDownTimestamp > holdCutoffTime)) {
      debug_println("Knob held!");
      knobIsHeld = true;
    }

    if (!homeHoldHandled && !homeIsHeld && homeDownTimestamp > 0 && (millis() - homeDownTimestamp > holdCutoffTime)) {
      debug_println("Home held!");
      homeIsHeld = true;
    }

    if (!buttonsHeld[0] && buttonDownTimestamps[0] > 0 && (millis() - buttonDownTimestamps[0] > holdCutoffTime)) {
      debug_println("Button 1 held!");
      buttonsHeld[0] = true;
    }

    if (!buttonsHeld[1] && buttonDownTimestamps[1] > 0 && (millis() - buttonDownTimestamps[1] > holdCutoffTime)) {
      debug_println("Button 2 held!");
      buttonsHeld[1] = true;
    }

    if (!buttonsHeld[2] && buttonDownTimestamps[2] > 0 && (millis() - buttonDownTimestamps[2] > holdCutoffTime)) {
      debug_println("Button 3 held!");
      buttonsHeld[2] = true;
    }

    if (!buttonsHeld[3] && buttonDownTimestamps[3] > 0 && (millis() - buttonDownTimestamps[3] > holdCutoffTime)) {
      debug_println("Button 4 held!");
      buttonsHeld[3] = true;
    }

    if (knobIsHeld || homeIsHeld || buttonsHeld[0] || buttonsHeld[1] || buttonsHeld[2] || buttonsHeld[3])
      timeAwake = 0; // Do not sleep while a button is held
  }

  // If we have an app active, the app gets first dibs on the controls.
  byte overriddenControls = 0x00; // Controls that have been consumed by the active app
  if (activeApp != NULL) {
    // First, assemble the masks.
    byte controlStates = 0x00; // TODO: Not sure what I intended to do with this. Figure that shit out
    byte clicks =
      knobJustClicked * knobMask +
      buttonsJustClicked[0] * button0Mask +
      buttonsJustClicked[1] * button1Mask +
      buttonsJustClicked[2] * button2Mask +
      buttonsJustClicked[3] * button3Mask +
      homeJustClicked * homeButtonMask;
    byte holds =
      knobIsHeld * knobMask +
      buttonsHeld[0] * button0Mask +
      buttonsHeld[1] * button1Mask +
      buttonsHeld[2] * button2Mask +
      buttonsHeld[3] * button3Mask +
      homeIsHeld * homeButtonMask;

    overriddenControls = activeApp->onControlEvent(controlStates, clicks, holds, knobDelta);
  }

  static bool overriddenButtons[] = {
    overriddenControls & buttonMasks[0],
    overriddenControls & buttonMasks[1],
    overriddenControls & buttonMasks[2],
    overriddenControls & buttonMasks[3]
  };

  // Interpret controls and switch states
  // There is a really long set of if-else blocks here.

  // Highest priority: Defense mode supersedes everything else.

  if (knobIsHeld && homeIsHeld) {
    if (state != defenseArming && state != defenseMode) {
      byte defenseCart = 0xFF; // TODO Prioritize types of defenses
      for (byte i = 0; i < 4; i++) {
        if (carts[i] != NULL && carts[i]->hasDefenseApp) {
          defenseCart = i;
          break;
        }
      }

      if (defenseCart < 4) switchStateTo(defenseArming, defenseCart);
      else {
        debug_println("No cart supports defense mode");
        switchStateTo(defenseArming);
      }
    }
  }

  else if (state == defenseArming) {
    // TODO Verify that defense mode is still available
    if (knobIsHeld) {
      if (!homeIsHeld && carts[activeCartSlot]->hasDefenseApp) switchStateTo(defenseMode, activeCartSlot);
    }
    else switchStateTo(homeScreen); // TODO Return to whatever state you activated defense mode from
  }

  else if (state == defenseMode) {
    if (!knobIsHeld && homeJustClicked) switchStateTo(homeScreen); // TODO return from whence ye came
  }

  // Priority 1: Quickfire apps

  else if (state == cartQuickfire) { // Note that all other buttons are ignored during quickfire
    if (!buttonsHeld[activeCartSlot]) switchStateTo(previousState, previousCartSlot);
  }

  else { // Not currently in quickfire
    for (byte i = 0; i < 4; i++) {
      if (buttonsHeld[i]) {
        switchStateTo(cartQuickfire, i);
        break;
      }
    }

    // Priority 2: Holding/pressing Home has special functionality.

    if (state != cartQuickfire) {
      if (homeIsHeld) {
        if (!homeHoldHandled) {
          if (state == sleeping) {
            debug_println("Super spesh home quickfire");
            homeQuickfire = true;
            switchStateTo(homeScreen);
          }
          else switchStateTo(sleeping);

          homeHoldHandled = true;
        }
      }
      else if (state == homeScreen && homeQuickfire) switchStateTo(sleeping);
      else if (state == homeScreen && knobJustClicked) switchStateTo(systemMenu);
      else if (homeJustClicked) {
        if (!(overriddenControls & homeButtonMask)) {
          if (state == homeScreen) switchStateTo(sleeping);
          else switchStateTo(homeScreen);
        }
      }
      else {
        // Priority 3: Clicking buttons can switch to apps

        if (state == systemMenu) {
          int appMenuSelection = systemAppMenu.control(knobDelta * -1, knobJustClicked);
          if (appMenuSelection == 0) switchStateTo(homeScreen); // "Go Back"
          else if (appMenuSelection > 0) switchStateTo(systemApp, byte(appMenuSelection));
        }

        for (byte i = 0; i < 4; i++) {
          if (buttonsJustClicked[i] && !overriddenButtons[i] && activeCartSlot != i) {
            switchStateTo(cartApp, i);
            break;
          }
        }
      }
    }
  }

  // Update and render stuff
  display.clear();
  leds.clear();

  for (int i = 0; i < 4; i++) {
    if (carts[i] != NULL &&
        carts[i]->backgroundUpdateInterval > 0 &&
        carts[i]->lastBackgroundUpdateTimestamp > carts[i]->backgroundUpdateInterval) {
      carts[i]->lastBackgroundUpdateTimestamp = 0;
      carts[i]->backgroundUpdate();
    }
  }

  if (autoSleepTime > 0 && state != sleeping && (activeApp == NULL || activeApp->sleepAllowed) && timeAwake > autoSleepTime) {
    debug_println("Zzzz");
    switchStateTo(sleeping);
  }

  if (state == cartQuickfire || state == cartApp || state == defenseMode || state == systemApp) {
    if (activeApp != NULL) {
      if (state == defenseMode) for (int i = 0; i < 5; i++) leds.bufferLed(i, carts[activeCartSlot]->color);
      else if (state == systemApp) leds.bufferLed(0, homeButtonColor);
      else leds.bufferLed(activeCartSlot + 1, carts[activeCartSlot]->color);
      activeApp->update(&display, &leds);
      if (activeApp->wantsToQuit) {
        if (state == systemApp) switchStateTo(systemMenu);
        else switchStateTo(homeScreen);
      }
    }
    else {
      if (state == systemApp) {
        display.bufferedPrint("System app ");
        display.bufferedPrint(activeCartSlot);
        display.bufferedPrint(" doesn't exist...");
        leds.bufferLed(0, homeButtonColor);
      }
      else if (activeCartSlot == 0xFF || carts[activeCartSlot] == NULL) {
        display.bufferedPrint("Slot ");
        display.bufferedPrint(activeCartSlot);
        display.bufferedPrint(" is empty!");
      }
      else {
        display.bufferedPrint("The ");
        display.bufferedPrint(carts[activeCartSlot]->name);
        display.bufferedPrint("doesn't have a", 1, 0);
        if (state == cartQuickfire && !carts[activeCartSlot]->hasQuickfireApp) display.bufferedPrint("Quickfire", 2, 0);
        else if (state == cartApp && !carts[activeCartSlot]->hasMainApp) display.bufferedPrint("primary", 2, 0);
        else if (state == defenseMode && !carts[activeCartSlot]->hasDefenseApp) display.bufferedPrint("defense", 2, 0);
        display.bufferedPrint(" app!");

        leds.bufferLed(activeCartSlot + 1, carts[activeCartSlot]->color);
      }
    }
  }

  else if (state == systemMenu) {
    systemAppMenu.render(&display);
    leds.bufferLed(0, homeButtonColor);
  }

  else if (state == cartInsertInterstitial || state == cartEjectInterstitial) {
    int linesToRender = map(long(timeSinceInterstitial), 0, interstitialDisplayTime, 1, 5); // Hang on the last line for a bit
    int progress = constrain(map(long(timeSinceInterstitial) % (interstitialDisplayTime / 5), 0, interstitialDisplayTime / 5, 0, 21), 0, 20);

    if (linesToRender == 5)
      for (int i = 0; i < 4; i++)
        display.bufferedPrint(flavorTextBuffer[i], i, 0);
    else {
      // Lowest line gets "typed out" all cool-like
      display.moveCursorTo(3, 0);
      for (int i = 0; i <= progress; i++) {
        if (flavorTextBuffer[linesToRender - 1][i] == 0x00) break;
        display.bufferedPrint(flavorTextBuffer[linesToRender - 1][i]);
      }
      if (progress < 19) display.bufferedPrint('\x14'); // Full block

      if (linesToRender >= 2) display.bufferedPrint(flavorTextBuffer[linesToRender - 2], 2, 0);
      if (linesToRender >= 3) display.bufferedPrint(flavorTextBuffer[linesToRender - 3], 1, 0);
      if (linesToRender >= 4) display.bufferedPrint(flavorTextBuffer[linesToRender - 4], 0, 0);
    }

    if (timeSinceInterstitial >= interstitialDisplayTime) switchStateTo(previousState, previousCartSlot);
  }

  else if (state == defenseArming) {
    if (activeCartSlot == 0xFF || carts[activeCartSlot] == NULL || !carts[activeCartSlot]->hasDefenseApp) {
      display.bufferedPrint("XXXXXXXXXXXXXXXXXXXX");
      display.bufferedPrint("X No defensive app X", 1, 0);
      display.bufferedPrint("X  is available!   X", 2, 0);
      display.bufferedPrint("XXXXXXXXXXXXXXXXXXXX", 3, 0);

      leds.bufferLed(activeCartSlot + 1, carts[activeCartSlot]->color);
    }
    else {
      display.bufferedPrint("/!");
      display.bufferedPrint('\x8C'); // Backslash
      display.bufferedPrint("Arming defenses!", 1, 0);
      display.bufferedPrint("/!", 2, 0);
      display.bufferedPrint('\x8C');

      for (int i = 0; i < 5; i++) leds.bufferLed(i, CRGB::Gold); //leds.Color(100, 50, 0));
    }
  }

  else if (state == homeScreen) {
    /*
      headingOffset += knobDelta * 5;
      display.bufferedPrint(int(headingOffset), 0, 16);
    */

    // Load icons

    if (batteryState == batteryNotCharging) batteryIconChar = display.createCustomChar(batteryIcon);
    else if (batteryState == batteryCharging) batteryIconChar = display.createCustomChar(lightningIcon);
    else batteryIconChar = display.createCustomChar(dangerIcon);

    phoneIconChar = display.createCustomChar(phoneIcon);

    // Render battery status

    int batteryReading = analogRead(batteryMonitor);

    display.bufferedPrint(batteryIconChar, 0, 0);

    if (batteryState == batteryNotCharging || batteryState == batteryCharging) {
      byte detectedBars = constrain(map(batteryReading, minChargeReading, maxChargeReading, 1, 11), 1, 10);
      if (batteryState == batteryNotCharging) batteryBars = min(batteryBars, detectedBars); // Overrun a little to mitigate truncating fractions
      else batteryBars = detectedBars;

      if (batteryBars <= 5) {
        display.bufferedPrint(char(0x0F + batteryBars)); // 0x10 is one bar, 0x14 is a full block
        if (batteryState != batteryCharging && batteryBars <= 3 && millis() / 500 % 2) display.bufferedPrint('!'); // Blinking low battery warning
      }
      else {
        display.bufferedPrint('\x14'); // Full block
        display.bufferedPrint(char(0x0F + batteryBars - 5)); // 0x10 is one bar, 0x14 is a full block
      }
    }
    //    else if (batteryState == batteryCharging) {
    //      // Sloppy charging animation
    //      // We can't actually measure the battery voltage while charging
    //      // ...I think?
    //      if ((millis() / 500) % 2) display.bufferedPrint('\x1E', 0, 1); // Left-pointing caret
    //      else display.bufferedPrint('\x1e', 0, 2);
    //      // This is the only circumstance in which battery bars can increase
    //      batteryBars = 10;
    //    }
    else { // Battery fault
      display.bufferedPrint("XX");
    }

    // Render compass [--N---E--]

    // Make heading (mostly) positive
    int adjustedHeading = floor(imu.Yaw) + 180 + headingOffset;

    // Wrap overly high values around to other side of the scale.
    // Not sure why these appear at all, but whatevs.
    if (adjustedHeading < 0) adjustedHeading += 360;
    else if (adjustedHeading > 360) adjustedHeading -= 360;

    // Map it onto the compass legend to center compass around heading
    int forwardPosition = map(adjustedHeading, 0, 360, 0, 20);

    // Render the compass
    display.bufferedPrint('[', 0, 4);
    for (int i = forwardPosition - 4; i <= forwardPosition + 5; i++) {
      if (i < 0) display.bufferedPrint(compassLegend[i + compassLegendSize]);
      else if (i > compassLegendSize - 1) display.bufferedPrint(compassLegend[i - compassLegendSize]);
      else display.bufferedPrint(compassLegend[i]);
    }
    display.bufferedPrint(']');

    // Render phone status

    if (phoneConnected) {
      // Phone battery percentage is mocked up for now
      display.bufferedPrint(char(0x17), 0, 17); // Partial bar
      display.bufferedPrint(char(0x14)); // Full bar
    }
    else {
      display.bufferedPrint("--", 0, 17);
    }

    display.bufferedPrint(phoneIconChar, 0, 19);

    // Render the time

    if (hourFormat12() >= 10) {
      display.bufferedPrint('1', 1, 0);
      display.bufferedPrint(char('0' + (hourFormat12() % 10)));
    }
    else display.bufferedPrint(char('0' + hourFormat12()), 1, 0);

    display.bufferedPrint(':');

    if (minute() < 10) display.bufferedPrint('0');
    else display.bufferedPrint(char('0' + (minute() / 10)));
    display.bufferedPrint(char('0' + (minute() % 10)));

    if (isAM()) display.bufferedPrint(" AM ");
    else display.bufferedPrint(" PM ");

    switch (weekday()) {
      case 1: display.bufferedPrint("Sun "); break;
      case 2: display.bufferedPrint("Mon "); break;
      case 3: display.bufferedPrint("Tue "); break;
      case 4: display.bufferedPrint("Wed "); break;
      case 5: display.bufferedPrint("Thu "); break;
      case 6: display.bufferedPrint("Fri "); break;
      case 7: display.bufferedPrint("Sat "); break;
      default: display.bufferedPrint("??? "); break;
    }

    switch (month()) {
      case 1: display.bufferedPrint("Jan "); break;
      case 2: display.bufferedPrint("Feb "); break;
      case 3: display.bufferedPrint("Mar "); break;
      case 4: display.bufferedPrint("Apr "); break;
      case 5: display.bufferedPrint("May "); break;
      case 6: display.bufferedPrint("Jun "); break;
      case 7: display.bufferedPrint("Jul "); break;
      case 8: display.bufferedPrint("Aug "); break;
      case 9: display.bufferedPrint("Sep "); break;
      case 10: display.bufferedPrint("Oct "); break;
      case 11: display.bufferedPrint("Nov "); break;
      case 12: display.bufferedPrint("Dec "); break;
      default: display.bufferedPrint("??? "); break;
    }

    if (day() >= 10) display.bufferedPrint(char('0' + (day() / 10)));
    display.bufferedPrint(char('0' + (day() % 10)));

    // And for the record, the time sucks.

    // Render widgets
    // TODO: Add ranking/ordering for widgets

    byte widgetCol = 2;
    byte widgetRow = 0;
    byte renderedWidgets = 0;
    byte encounteredWidgets = 0;
    byte totalWidgets = 0;

    for (byte slot = 0; slot < 4; slot++) {
      if (carts[slot] != NULL) totalWidgets += carts[slot]->widgetsAvailable;
    }

    if (firstWidgetToRender > max(totalWidgets - 4, 0)) { // Widget count has decreased, so we need to forcibly scroll up to the lowest possible line
      if (totalWidgets % 2 == 0) firstWidgetToRender = totalWidgets - 4;
      else firstWidgetToRender = totalWidgets - 3;
    }

    if (totalWidgets - firstWidgetToRender > 4 && knobDelta == -1) firstWidgetToRender += 2;
    else if (firstWidgetToRender > 1 && knobDelta == 1) firstWidgetToRender -= 2;

    for (byte slot = 0; slot < 4; slot++) {
      if (renderedWidgets < 4 && carts[slot] != NULL && carts[slot]->widgetsAvailable > 0) {
        for (byte widget = 0; widget < carts[slot]->widgetsAvailable; widget++) {
          encounteredWidgets++;
          if (encounteredWidgets < firstWidgetToRender + 1) continue;

          display.moveCursorTo(widgetCol, widgetRow);
          carts[slot]->renderWidget(&display, widget);
          if (widgetRow == 0) widgetRow = 10;
          else {
            widgetRow = 0;
            widgetCol++;
          }

          renderedWidgets++;
          if (renderedWidgets >= min(4, totalWidgets - firstWidgetToRender)) break;
        }
      }
    }

    // Purdy LED's for every active cart
    leds.bufferLed(0, homeButtonColor);
    for (byte i = 0; i < 4; i++) if (carts[i] != NULL) leds.bufferLed(i + 1, carts[i]->color);
  }

  // Overwrite displayed stuff with animations
  // TODO Make this cleaner
  if (timeSinceAnimationStarted > animationTime) activeAnimation = noAnimation;

  const int sweepWidth = 7;
  if (activeAnimation == rightWipe) {
    int animationProgress = constrain(map(long(timeSinceAnimationStarted), 0, animationTime, 0, 20 + sweepWidth + 3), 0, 20 + sweepWidth + 3);
    for (int col = -3; col < 20 + sweepWidth; col++) {
      for (int row = 0; row < 4; row++) {
        int colPlacement = col + row; // Creates noyce 45-degree angle
        if (colPlacement < 0 || colPlacement > 19) continue;

        if (col <= animationProgress) {
          if (animationProgress - col <= sweepWidth) display.bufferedPrint((char)random(0xA6, 0xDE), row, colPlacement);
        }
        else display.bufferedPrint(preAnimationFramebuffer[row][colPlacement], row, colPlacement);
      }
    }

    animationProgress = constrain(map(long(timeSinceAnimationStarted), 0, animationTime, 0, 4), 0, 4);
    for (int i = 0; i < 5; i++) if (i >= animationProgress) leds.bufferLed(i, preAnimationLeds[i]);
  }

  else if (activeAnimation == leftWipe) {
    int animationProgress = constrain(map(long(timeSinceAnimationStarted), 0, animationTime, 20, -sweepWidth - 3), -sweepWidth - 3, 20);
    for (int col = 0; col < 20 + sweepWidth + 3; col++) {
      for (int row = 0; row < 4; row++) {
        int colPlacement = col - row; // Creates noyce 45-degree angle
        if (colPlacement < 0 || colPlacement > 19) continue;

        if (col >= animationProgress) {
          if (col - animationProgress <= sweepWidth) display.bufferedPrint((char)random(0xA6, 0xDE), row, colPlacement);
        }
        else display.bufferedPrint(preAnimationFramebuffer[row][colPlacement], row, colPlacement);
      }
    }

    animationProgress = constrain(map(long(timeSinceAnimationStarted), 0, animationTime, 0, 4), 0, 4);
    animationProgress = 4 - animationProgress;
    for (int i = 0; i < 5; i++) if (i <= animationProgress) leds.bufferLed(i, preAnimationLeds[i]);
  }

  // Render buffered graphics 'n stuff
  leds.render(justWokeUp);
  display.render(justWokeUp);

  // Clean up temporary fields
  justWokeUp = knobJustClicked = homeJustClicked = buttonsJustClicked[0] = buttonsJustClicked[1] = buttonsJustClicked[2] = buttonsJustClicked[3] = false;

  //  debug_print("That took ");
  //  debug_print(micros() - startTime);
  //  debug_println("us");
}

void onButtonChange() {
  mustPollButtons = true;
}

void onHotplug() {
  mustPollCarts = true;
}

void onImuEvent() {
  mustPollImu = true;
}

void onBatteryEvent() {
  chargePinState = digitalReadFast(chargeStatusPin);

  if (!chargePinState) { // CHRG has just fallen
    chargePinHighTime = micros() - chargePinChangeTimestamp;
  }
  else { // CHRG has just risen
    chargePinLowTime = micros() - chargePinChangeTimestamp;
    chargePinHighTime = 0; // We only care about full up-down cycles
  }

  chargePinChangeTimestamp = micros();
}

#define knobPollMask 0b00000001
#define homePollMask 0b00000010
#define button0PollMask 0b00000100
#define button1PollMask 0b00001000
#define button2PollMask 0b00100000
#define button3PollMask 0b00010000

void pollButtons() {
  debug_println("Polling buttons");

  mustPollButtons = false;
  Wire.beginTransmission(buttonDriver);
  Wire.write(0x00);                // go to the input port register
  Wire.endTransmission();
  Wire.requestFrom(buttonDriver, 1);     // start read mode
  if (Wire.available()) {
    byte buttonStates = Wire.read();  // fetch one byte
    Wire.endTransmission();

    if ((buttonStates & knobPollMask) && !(lastButtonStates & knobPollMask)) {
      debug_println("Knob down");
      knobDownTimestamp = millis();
    }
    else if (!(buttonStates & knobPollMask) && (lastButtonStates & knobPollMask)) {
      debug_println("Knob up");

      if (!knobIsHeld) {
        debug_println("Knob click!");
        knobJustClicked = true;
      }

      knobIsHeld = false;
      knobDownTimestamp = 0;
    }

    if ((buttonStates & homePollMask) && !(lastButtonStates & homePollMask)) {
      debug_println("Home down");
      homeDownTimestamp = millis();
    }
    else if (!(buttonStates & homePollMask) && (lastButtonStates & homePollMask)) {

      debug_println("Home up");

      if (!homeIsHeld) {
        debug_println("Home click!");
        homeJustClicked = true;
      }

      homeIsHeld = false;
      homeHoldHandled = false;
      homeDownTimestamp = 0;
    }

    if ((buttonStates & button0PollMask) && !(lastButtonStates & button0PollMask)) {
      debug_println("B0 down");
      buttonDownTimestamps[0] = millis();
    }
    else if (!(buttonStates & button0PollMask) && (lastButtonStates & button0PollMask)) {
      debug_println("B0 up");

      if (!buttonsHeld[0]) {
        debug_println("B0 click!");
        buttonsJustClicked[0] = true;
      }

      buttonsHeld[0] = false;
      buttonDownTimestamps[0] = 0;
    }

    if ((buttonStates & button1PollMask) && !(lastButtonStates & button1PollMask)) {
      debug_println("B1 down");
      buttonDownTimestamps[1] = millis();
    }
    else if (!(buttonStates & button1PollMask) && (lastButtonStates & button1PollMask)) {
      debug_println("B1 up");

      if (!buttonsHeld[1]) {
        debug_println("B1 click!");
        buttonsJustClicked[1] = true;
      }

      buttonsHeld[1] = false;
      buttonDownTimestamps[1] = 0;
    }

    if ((buttonStates & button2PollMask) && !(lastButtonStates & button2PollMask)) {
      debug_println("B2 down");
      buttonDownTimestamps[2] = millis();
    }
    else if (!(buttonStates & button2PollMask) && (lastButtonStates & button2PollMask)) {
      debug_println("B2 up");

      if (!buttonsHeld[2]) {
        debug_println("B2 click!");
        buttonsJustClicked[2] = true;
      }

      buttonsHeld[2] = false;
      buttonDownTimestamps[2] = 0;
    }

    if ((buttonStates & button3PollMask) && !(lastButtonStates & button3PollMask)) {
      debug_println("B3 down");
      buttonDownTimestamps[3] = millis();
    }
    else if (!(buttonStates & button3PollMask) && (lastButtonStates & button3PollMask)) {
      debug_println("B3 up");

      if (!buttonsHeld[3]) {
        debug_println("B3 click!");
        buttonsJustClicked[3] = true;
      }

      buttonsHeld[3] = false;
      buttonDownTimestamps[3] = 0;
    }

    lastButtonStates = buttonStates;
  } else {
    debug_print("Buttons error ");
    debug_println(Wire.getError());
  }

  timeAwake = 0; // Refresh sleep timer when we press a button
}

void pollCarts() {
  mustPollCarts = false;
  Wire.beginTransmission(cartIOExpander);
  Wire.write(0x00);                // go to the input port register
  Wire.endTransmission();
  Wire.requestFrom(cartIOExpander, 1);     // start read mode
  if (Wire.available()) {
    byte cartStates = Wire.read();  // fetch one byte
    Wire.endTransmission();

    for (byte i = 0; i < 4; i++) {
      byte mask = 0b10 << (i * 2); // Bits 1, 3, 5, and 7 are cart detect lines
      if ((cartStates & mask) && !(lastCartStates & mask)) {
        debug_print("Cart "); debug_print(i); debug_println(" ejected");
        cartDetectTimestamps[i] = millis();
      }
      else if (!(cartStates & mask) && (lastCartStates & mask)) {
        debug_print("Cart "); debug_print(i); debug_println(" inserted");
        cartDetectTimestamps[i] = millis();
      }
    }

    lastCartStates = cartStates;
  } else {
    debug_print("Cart error ");
    debug_println(Wire.getError());
  }
}

void switchStateTo(byte newState, byte slot, bool ejected) {
  debug_print("Change state from ");
  debug_print(state);
  debug_print(" to ");
  debug_print(newState);

  if (slot == 0xFF) debug_println();
  else {
    debug_print(" cart ");
    debug_println(slot);
  }

  // Record display and leds for animations
  display.copyFramebuffer(preAnimationFramebuffer);
  leds.copyLedBuffer(preAnimationLeds);

  // Tear down current state

  if (state == sleeping) {
    display.powerUp();
    display.clear();
    display.render();
    leds.clear();
    leds.render(true);
    //    imu.wake(); // DO NOT touch mustPollImu - don't want to clear it while the Status register is full
  }
  else if (state == homeScreen) {
    homeQuickfire = false;
    leds.render(true);
  }
  else if (activeApp != NULL && (state == cartApp || state == cartQuickfire || state == defenseMode || state == systemApp)) {
    activeApp->teardown(ejected);
    delete activeApp;
    activeApp = NULL;
  }

  // Set up new state

  if (newState == sleeping) {
    // Do sleep shutdown stuff after finishing the animation
    activeAnimation = rightWipe;
  }

  else if (newState == homeScreen) {
    if (state == sleeping || state == indeterminateState) activeAnimation = leftWipe; // When "going forward" from sleep to home screen, wipe left
    else activeAnimation = rightWipe; // When "going back" from an app to the home screen, wipe right
  }

  else if (newState == cartQuickfire) {
    if (!ejected && carts[slot] != NULL && carts[slot]->hasQuickfireApp) {
      activeApp = carts[slot]->generateQuickfireApp();
      activeApp->setup();
    }

    // Use left wipe if new cart is to the left, right for right.
    if (activeCartSlot > 3) activeAnimation = leftWipe;
    else if (slot < activeCartSlot) activeAnimation = rightWipe;
    else if (slot > activeCartSlot) activeAnimation = leftWipe;
    else {
      // When switching from a cart's app to its own QF, wipe right
      activeAnimation = rightWipe;
    }
    //    activeAnimation = leftWipe;
  }

  else if (newState == cartApp) {
    if (!ejected && carts[slot] != NULL && carts[slot]->hasMainApp) {
      activeApp = carts[slot]->generateMainApp();
      activeApp->setup();
    }

    // Use left wipe if new cart is to the left, right for right.
    if (activeCartSlot > 3) activeAnimation = leftWipe;
    else if (slot < activeCartSlot) activeAnimation = rightWipe;
    else if (slot > activeCartSlot) activeAnimation = leftWipe;
    else {
      // When switching from a cart's QF to its own app, wipe left
      activeAnimation = leftWipe;
    }

    //    if (state == cartQuickfire) activeAnimation = rightWipe;
    //    else activeAnimation = leftWipe;
  }

  else if (newState == cartInsertInterstitial) {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 20; j++) {
        flavorTextBuffer[i][j] = 0x00;
      }
    }

    for (int i = 0; i < 3; i++)
      getFullLine(flavorTextBuffer[i], true);

    String finale = carts[slot]->name;
    finale.toUpperCase();
    finale += " GO!";

    finale.toCharArray(flavorTextBuffer[3], 21);

    activeAnimation = leftWipe;
    timeSinceInterstitial = 0;
  }

  else if (newState == cartEjectInterstitial) {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 20; j++) {
        flavorTextBuffer[i][j] = 0x00;
      }
    }

    for (int i = 0; i < 3; i++)
      getFullLine(flavorTextBuffer[i], false);

    String finale = carts[slot]->name;
    finale.toUpperCase();
    finale += " OUT!";

    finale.toCharArray(flavorTextBuffer[3], 21);

    activeAnimation = leftWipe;
    timeSinceInterstitial = 0;
  }

  else if (newState == systemMenu) {
    prepareSystemAppMenu();

    activeAnimation = rightWipe; // Because knob is to the left of the display
  }

  else if (newState == systemApp) {
    activeApp = generateSystemApp(slot);
    if (activeApp != NULL) activeApp->setup();

    if (state == cartQuickfire) activeAnimation = rightWipe; // When "going back" from quickfire to an app, wipe right
    else activeAnimation = leftWipe;
  }

  else if (newState == defenseMode) {
    // TODO Put these in the update
    for (int i = 0; i < 5; i++) leds.bufferLed(i, carts[slot]->color);

    activeApp = carts[slot]->generateDefenseApp();
    activeApp->setup();
  }

  else if (newState == defenseArming);

  else {
    debug_print("State ");
    debug_print(newState);
    debug_println(" unknown. Ignoring.");
    return;
  }

  // Finally, some bookkeeping
  if (state == sleeping || state == homeScreen || state == cartApp || state == systemApp) {
    // Other states are transitory and shouldn't be returned to
    previousCartSlot = activeCartSlot;
    previousState = state;
  }

  activeCartSlot = slot;
  state = newState;
  if (activeAnimation != noAnimation) timeSinceAnimationStarted = 0;
}

void switchStateTo(byte state, byte slot) {
  switchStateTo(state, slot, false);
}

void switchStateTo(byte state) {
  switchStateTo(state, 0xFF, false);
}

void switchStateTo(byte state, bool ejected) {
  switchStateTo(state, 0xFF, ejected);
}

// Wrapper for TimeLib.setSyncProvider()
time_t getTeensy3Time() {
  return Teensy3Clock.get();
}

void restoreInterrupts() {
  attachInterrupt(digitalPinToInterrupt(buttonInterruptPin), onButtonChange, FALLING);
  mustPollButtons = true;

  attachInterrupt(digitalPinToInterrupt(hotplugInterruptPin), onHotplug, FALLING);
  mustPollCarts = true;
}

void wakeToUpdate() {} // Used for intervaltimer to wake device for background updates
