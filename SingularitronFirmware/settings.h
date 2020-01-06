#ifndef SETTINGS_H
#define SETTINGS_H

#include <EEPROM.h>

// Declaring key settings and the saving/loading functionality as globals
// lets the main sketch use them and the system apps change them.
// This isn't really good practice, but the weird Arduino buildpath forced my hand.

#define defaultDisplayBrightness 25
int displayBrightness = defaultDisplayBrightness; // Can be 25, 50, 75, or 100
#define displayBrightnessMemoryLocation 0x10

#define defaultLedBrightness 100
int ledBrightness = defaultLedBrightness; // Can be 0-100
#define ledBrightnessMemoryLocation 0x20

#define defaultHoldCutoffTime 300
unsigned long holdCutoffTime = defaultHoldCutoffTime; // Release button before this many ms and it will count as a press
#define holdCutoffTimeMemoryLocation 0x30

#define defaultAnimationTime 200
unsigned long animationTime = defaultAnimationTime; // Time in ms that transitional animations (wipes) will take
#define animationTimeMemoryLocation 0x40

#define defaultInterstitialTime 2000
unsigned long interstitialDisplayTime = defaultInterstitialTime; // Time in ms that cart insert/remove interstitial will display
#define interstitialDisplayTimeMemoryLocation 0x50

#define defaultAutoSleepTime 180000
unsigned long autoSleepTime = defaultAutoSleepTime; // Automatically snooze after this much inactivity
#define autoSleepTimeMemoryLocation 0x60

void saveSettings() {
  Serial.println("Saving settings");

  EEPROM.put(displayBrightnessMemoryLocation, displayBrightness);
  EEPROM.put(ledBrightnessMemoryLocation, ledBrightness);
  EEPROM.put(holdCutoffTimeMemoryLocation, holdCutoffTime);
  EEPROM.put(animationTimeMemoryLocation, animationTime);
  EEPROM.put(interstitialDisplayTimeMemoryLocation, interstitialDisplayTime);
  EEPROM.put(autoSleepTimeMemoryLocation, autoSleepTime);
}

void loadSettings() {
  boolean mustSaveDefaults = false;
  
  int intPlaceholder = -1; // Twos compliment 0xFFFF

  EEPROM.get(displayBrightnessMemoryLocation, intPlaceholder);
  if (intPlaceholder == -1) {
    displayBrightness = defaultDisplayBrightness;
    Serial.println("No saved VFD brightness");
    mustSaveDefaults = true;
  }
  else {
    displayBrightness = intPlaceholder;
    Serial.print("Loaded VFD brightness ");
    Serial.println(displayBrightness);
  }

  EEPROM.get(ledBrightnessMemoryLocation, intPlaceholder);
  if (intPlaceholder == -1) {
    ledBrightness = defaultLedBrightness;
    Serial.println("No saved LED brightness");
    mustSaveDefaults = true;
  }
  else {
    ledBrightness = intPlaceholder;
    Serial.print("Loaded LED brightness ");
    Serial.println(ledBrightness);
  }

  unsigned long ulPlaceholder = 0xFFFFFFFF;

  EEPROM.get(holdCutoffTimeMemoryLocation, ulPlaceholder);
  if (ulPlaceholder == 0xFFFFFFFF) {
    holdCutoffTime = defaultHoldCutoffTime;
    Serial.println("No saved hold cutoff");
    mustSaveDefaults = true;
  }
  else {
    holdCutoffTime = ulPlaceholder;
    Serial.print("Loaded hold cutoff ");
    Serial.println(holdCutoffTime);
  }

  EEPROM.get(animationTimeMemoryLocation, ulPlaceholder);
  if (ulPlaceholder == 0xFFFFFFFF) {
    animationTime = defaultAnimationTime;
    Serial.println("No saved animation time");
    mustSaveDefaults = true;
  }
  else {
    animationTime = ulPlaceholder;
    Serial.print("Loaded animation time ");
    Serial.println(animationTime);
  }

  EEPROM.get(interstitialDisplayTimeMemoryLocation, ulPlaceholder);
  if (ulPlaceholder == 0xFFFFFFFF) {
    interstitialDisplayTime = defaultInterstitialTime;
    Serial.println("No saved interstitial time");
    mustSaveDefaults = true;
  }
  else {
    interstitialDisplayTime = ulPlaceholder;
    Serial.print("Loaded interstitial time ");
    Serial.println(interstitialDisplayTime);
  }

  EEPROM.get(autoSleepTimeMemoryLocation, ulPlaceholder);
  if (ulPlaceholder == 0xFFFFFFFF) {
    autoSleepTime = defaultAutoSleepTime;
    Serial.println("No saved auto sleep time");
    mustSaveDefaults = true;
  }
  else {
    autoSleepTime = ulPlaceholder;
    Serial.print("Loaded auto sleep time ");
    Serial.println(autoSleepTime);
  }

  if (mustSaveDefaults) {
    saveSettings();
  }
}

#endif
