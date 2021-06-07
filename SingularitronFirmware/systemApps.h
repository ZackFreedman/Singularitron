#ifndef SYSTEMAPPS_H
#define SYSTEMAPPS_H

#include "scrollableList.h"
#include "app.h"
#include "cart.h" // TODO get rid of this dependency
#include "settings.h"
#include "EEPROM.h"

const char systemAppNames[][19] = {
  "Bootloadest",
  "Set the time",
  "Sleep/Brightness",
  "User Interface"
};

#define numberOfSystemApps 4

ScrollableList systemAppMenu;
bool systemAppMenuPrepared = false;

void prepareSystemAppMenu() {
  if (systemAppMenuPrepared)systemAppMenu.scrollToTop();
  else {
    for (byte i = 0; i < numberOfSystemApps; i++) systemAppMenu.append(const_cast<char*>(systemAppNames[i]));
    systemAppMenuPrepared = true;
  }
}

class PowerSettingsApp : public App {
  private:
    enum State {
      selectingSetting = 0,
      editingDisplayBrightness = 1,
      editingLedBrightness = 2,
      editingAutoSleep = 3
    };

    State state = selectingSetting;
    ScrollableList powerSettingsList;

    bool mustRevertDisplayBrightness = false;
    bool mustRevertLedBrightness = false;

  public:
    PowerSettingsApp(Cartridge & owner): App(owner) {}
    virtual ~PowerSettingsApp() {}

    void setup() {
      powerSettingsList.append("Display brightness");
      powerSettingsList.append("LED brightness");
      powerSettingsList.append("Auto sleep time");
    }

    void teardown(bool ejected) {
      loadSettings(); // Catch-all to discard settings changes if app is clobbered
    }

    void update(BufferedVfd * display, BufferedLeds * leds) {
      if (mustRevertDisplayBrightness) {
        display->setBrightness(displayBrightness);
        mustRevertDisplayBrightness = false;
      }

      if (mustRevertLedBrightness) {
        leds->setBrightness(ledBrightness);
        mustRevertLedBrightness = false;
      }

      if (state == selectingSetting) powerSettingsList.render(display);

      else if (state == editingDisplayBrightness) {
        display->setBrightness(displayBrightness);

        display->bufferedPrint("VFD brightness ");
        display->bufferedPrint(displayBrightness);
        display->bufferedPrint("%");

        display->bufferedPrint("Press knob to save", 1, 0);
        display->bufferedPrint("Press Home to undo", 2, 0);
      }

      else if (state == editingLedBrightness) {
        leds->setBrightness(ledBrightness);

        leds->bufferLed(1, 0x0000FF);
        leds->bufferLed(2, 0x00FF00);
        leds->bufferLed(3, 0xFF0000);
        leds->bufferLed(4, 0x888888);

        if (ledBrightness > 0) {
          display->bufferedPrint("LED brightness ");
          display->bufferedPrint(ledBrightness);
          display->bufferedPrint("%");
        }
        else display->bufferedPrint("LED's disabled");

        display->bufferedPrint("Press knob to save", 1, 0);
        display->bufferedPrint("Press Home to undo", 2, 0);
      }

      else if (state == editingAutoSleep) {
        if (autoSleepTime > 0 && autoSleepTime <= 6 * 60 * 60 * 1000) {
          display->bufferedPrint("Sleep timeout: ");

          if (autoSleepTime < 1000) {
            display->bufferedPrint(int(autoSleepTime));
            display->bufferedPrint("ms");
          }
          else if (autoSleepTime < 60 * 1000) {
            display->bufferedPrint(int(autoSleepTime / 1000));
            display->bufferedPrint("sec");
          }
          else if (autoSleepTime < 60 * 60 * 1000) {
            display->bufferedPrint(int(autoSleepTime / 1000 / 60));
            display->bufferedPrint("min");
          }
          else {
            display->bufferedPrint(int(autoSleepTime / 1000 / 60 / 60));
            display->bufferedPrint("hr");
          }
        }
        else display->bufferedPrint("Auto sleep disabled");

        display->bufferedPrint("Press knob to save", 1, 0);
        display->bufferedPrint("Press Home to undo", 2, 0);
      }
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      if (state == selectingSetting) {
        int outcome = powerSettingsList.control(knobDelta * -1, clicks & knobMask);
        if (outcome == 0) wantsToQuit = true;
        else if (outcome > 0) {
          Serial.print("Editing setting ");
          Serial.println(outcome);
          switch (outcome) {
            case 1: state = editingDisplayBrightness; break;
            case 2: state = editingLedBrightness; break;
            case 3: state = editingAutoSleep; break;
            default: break;
          }
        }

        return updownMask + knobMask;
      }

      else if (state == editingDisplayBrightness) {
        displayBrightness = constrain(displayBrightness + knobDelta * 25, 25, 100);
        if (clicks & knobMask) {
          Serial.println("Saved brightness");
          saveSettings();
          state = selectingSetting;
        }
        else if (clicks & homeButtonMask) {
          Serial.println("Reverting brightness");
          mustRevertDisplayBrightness = true;
          loadSettings();
          state = selectingSetting;
        }

        return updownMask + knobMask + homeButtonMask;
      }

      else if (state == editingLedBrightness) {
        ledBrightness = constrain(ledBrightness + knobDelta * 10, 0, 100);
        if (clicks & knobMask) {
          Serial.println("Saved brightness");
          saveSettings();
          state = selectingSetting;
        }
        else if (clicks & homeButtonMask) {
          Serial.println("Reverting brightness");
          mustRevertLedBrightness = true;
          loadSettings();
          state = selectingSetting;
        }

        return updownMask + knobMask + homeButtonMask;
      }

      else if (state == editingAutoSleep) {
        // Use different steps for different time periods for convenience
        const unsigned long millisecondDelta = 500;
        const unsigned long secondDelta = 15 * 1000;
        const unsigned long minuteDelta = 5 * 60 * 1000;
        const unsigned long hourDelta = 60 * 60 * 1000;
        const unsigned long maxTimeout = 6 * 60 * 60 * 1000;

        if (autoSleepTime < 1000) {
          unsigned long possiblyWrappedAutoSleepTime = autoSleepTime + knobDelta * millisecondDelta;
          if (knobDelta < 0 && possiblyWrappedAutoSleepTime > autoSleepTime) autoSleepTime = 0; // Yep, wrapped
          else autoSleepTime = min(possiblyWrappedAutoSleepTime, 1000);
        }
        else if (autoSleepTime < secondDelta) {
          if (knobDelta < 0) autoSleepTime = 1000 - millisecondDelta;
          else if (knobDelta > 0) autoSleepTime = secondDelta;
        }
        else if (autoSleepTime < 60 * 1000)
          autoSleepTime = constrain(autoSleepTime + knobDelta * secondDelta, 1000, 60 * 1000);
        else if (autoSleepTime < minuteDelta) {
          if (knobDelta < 0) autoSleepTime = 60 * 1000 - secondDelta;
          else if (knobDelta > 0) autoSleepTime = minuteDelta;
        }
        else if (autoSleepTime < 60 * 60 * 1000)
          autoSleepTime = constrain(autoSleepTime + knobDelta * minuteDelta, 60 * 1000, 60 * 60 * 1000);
        else if (autoSleepTime <= hourDelta) {
          if (knobDelta < 0) autoSleepTime = 60 * 60 * 1000 - minuteDelta;
          else if (knobDelta > 0) {
            if (autoSleepTime == hourDelta) autoSleepTime = autoSleepTime + hourDelta;
            else autoSleepTime = hourDelta;
          }
        }
        else
          autoSleepTime = constrain(autoSleepTime + knobDelta * hourDelta, 60 * 60 * 1000, maxTimeout);

        //        Serial.print("Auto sleep is now ");
        //        Serial.println(autoSleepTime);

        if (clicks & knobMask) {
          Serial.println("Saved sleep time");
          saveSettings();
          state = selectingSetting;
        }
        else if (clicks & homeButtonMask) {
          Serial.println("Reverting sleep time");
          loadSettings();
          state = selectingSetting;
        }

        return updownMask + knobMask + homeButtonMask;
      }

      return 0x00;
    }
};

class AnimationSettingsApp : public App {
  private:
    enum State {
      selectingSetting = 0,
      editingAnimation = 1,
      editingInterstitial = 2,
      editingHold = 3
    };

    State state = selectingSetting;
    ScrollableList animationSettingsList;

  public:
    AnimationSettingsApp(Cartridge & owner): App(owner) {}
    virtual ~AnimationSettingsApp() {}

    void setup() {
      animationSettingsList.append("Animation speed");
      animationSettingsList.append("Cart anim. time");
      animationSettingsList.append("Quickfire delay");
    }

    void teardown(bool ejected) {
      loadSettings(); // Catch-all to discard settings changes if app is clobbered
    }

    void update(BufferedVfd * display, BufferedLeds * leds) {
      if (state == selectingSetting) animationSettingsList.render(display);

      else if (state == editingAnimation) {
        if (animationTime > 0 && animationTime <= 10 * 60 * 1000) {
          display->bufferedPrint("Animation time: ");

          if (animationTime < 1000) {
            display->bufferedPrint(int(animationTime));
            display->bufferedPrint("ms");
          }
          else {
            display->bufferedPrint(int(animationTime / 1000));
            display->bufferedPrint("sec");
          }
        }
        else display->bufferedPrint("Animation disabled");

        display->bufferedPrint("Press knob to save", 1, 0);
        display->bufferedPrint("Press Home to undo", 2, 0);
      }

      else if (state == editingInterstitial) {
        if (interstitialDisplayTime > 0 && interstitialDisplayTime <= 10 * 60 * 1000) {
          display->bufferedPrint("Cart animation: ");

          if (interstitialDisplayTime < 1000) {
            display->bufferedPrint(int(interstitialDisplayTime));
            display->bufferedPrint("ms");
          }
          else {
            display->bufferedPrint(int(interstitialDisplayTime / 1000));
            display->bufferedPrint("sec");
          }
        }
        else display->bufferedPrint("Cart anim. disabled");

        display->bufferedPrint("Press knob to save", 1, 0);
        display->bufferedPrint("Press Home to undo", 2, 0);
      }

      else if (state == editingHold) {
        if (holdCutoffTime > 0 && holdCutoffTime <= 10 * 60 * 1000) {
          display->bufferedPrint("QF threshold: ");

          if (holdCutoffTime < 1000) {
            display->bufferedPrint(int(holdCutoffTime));
            display->bufferedPrint("ms");
          }
          else {
            display->bufferedPrint(int(holdCutoffTime / 1000));
            display->bufferedPrint("sec");
          }
        }
        else display->bufferedPrint("Quickfire disabled");

        display->bufferedPrint("Press knob to save", 1, 0);
        display->bufferedPrint("Press Home to undo", 2, 0);
      }
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      if (state == selectingSetting) {
        int outcome = animationSettingsList.control(knobDelta * -1, clicks & knobMask);
        if (outcome == 0) wantsToQuit = true;
        else if (outcome > 0) {
          Serial.print("Editing setting ");
          Serial.println(outcome);
          switch (outcome) {
            case 1: state = editingAnimation; break;
            case 2: state = editingInterstitial; break;
            case 3: state = editingHold; break;
            default: break;
          }
        }

        return updownMask + knobMask;
      }

      else if (state == editingAnimation) {
        // Use different steps for different time periods for convenience
        const unsigned long millisecondDelta = 50;
        const unsigned long secondDelta = 1 * 1000;
        const unsigned long maxTime = 10 * 1000;

        if (animationTime < 1000) {
          unsigned long possiblyWrappedTime = animationTime + knobDelta * millisecondDelta;
          if (knobDelta < 0 && possiblyWrappedTime > animationTime) animationTime = 0; // Yep, wrapped
          else animationTime = min(possiblyWrappedTime, 1000);
        }
        else if (animationTime <= secondDelta) {
          if (knobDelta < 0) animationTime = 1000 - millisecondDelta;
          else if (knobDelta > 0) {
            if (animationTime == secondDelta) animationTime += secondDelta;
            else animationTime = secondDelta;
          }
        }
        else
          animationTime = constrain(animationTime + knobDelta * secondDelta, 1000, maxTime);

        //        Serial.print("Auto sleep is now ");
        //        Serial.println(autoSleepTime);

        if (clicks & knobMask) {
          Serial.println("Saved animation time");
          saveSettings();
          state = selectingSetting;
        }
        else if (clicks & homeButtonMask) {
          Serial.println("Reverting animation time");
          loadSettings();
          state = selectingSetting;
        }

        return updownMask + knobMask + homeButtonMask;
      }

      else if (state == editingInterstitial) {
        // Use different steps for different time periods for convenience
        const unsigned long millisecondDelta = 50;
        const unsigned long secondDelta = 1 * 1000;
        const unsigned long maxTime = 10 * 1000;

        if (interstitialDisplayTime < 1000) {
          unsigned long possiblyWrappedTime = interstitialDisplayTime + knobDelta * millisecondDelta;
          if (knobDelta < 0 && possiblyWrappedTime > interstitialDisplayTime) interstitialDisplayTime = 0; // Yep, wrapped
          else interstitialDisplayTime = min(possiblyWrappedTime, 1000);
        }
        else if (interstitialDisplayTime <= secondDelta) {
          if (knobDelta < 0) interstitialDisplayTime = 1000 - millisecondDelta;
          else if (knobDelta > 0) {
            if (interstitialDisplayTime == secondDelta) interstitialDisplayTime += secondDelta;
            else interstitialDisplayTime = secondDelta;
          }
        }
        else
          interstitialDisplayTime = constrain(interstitialDisplayTime + knobDelta * secondDelta, 1000, maxTime);

        //        Serial.print("Auto sleep is now ");
        //        Serial.println(autoSleepTime);

        if (clicks & knobMask) {
          Serial.println("Saved interstitial time");
          saveSettings();
          state = selectingSetting;
        }
        else if (clicks & homeButtonMask) {
          Serial.println("Reverting interstitial time");
          loadSettings();
          state = selectingSetting;
        }

        return updownMask + knobMask + homeButtonMask;
      }

      else if (state == editingHold) {
        // Use different steps for different time periods for convenience
        const unsigned long millisecondDelta = 50;
        const unsigned long secondDelta = 1 * 1000;
        const unsigned long maxTime = 10 * 1000;

        if (holdCutoffTime < 1000) {
          unsigned long possiblyWrappedTime = holdCutoffTime + knobDelta * millisecondDelta;
          if (knobDelta < 0 && possiblyWrappedTime > holdCutoffTime) holdCutoffTime = 0; // Yep, wrapped
          else holdCutoffTime = min(possiblyWrappedTime, 1000);
        }
        else if (holdCutoffTime <= secondDelta) {
          if (knobDelta < 0) holdCutoffTime = 1000 - millisecondDelta;
          else if (knobDelta > 0) {
            if (holdCutoffTime == secondDelta) holdCutoffTime += secondDelta;
            else holdCutoffTime = secondDelta;
          }
        }
        else
          holdCutoffTime = constrain(holdCutoffTime + knobDelta * secondDelta, 1000, maxTime);

        //        Serial.print("Auto sleep is now ");
        //        Serial.println(autoSleepTime);

        if (clicks & knobMask) {
          Serial.println("Saved QF threshold");
          saveSettings();
          state = selectingSetting;
        }
        else if (clicks & homeButtonMask) {
          Serial.println("Reverting QF threshold");
          loadSettings();
          state = selectingSetting;
        }

        return updownMask + knobMask + homeButtonMask;
      }

      return 0x00;
    }
};


class SetTimeApp : public App {
#define SETTING_HOUR 0
#define SETTING_MINUTE 1
#define SETTING_AM 2
#define SETTING_MONTH 3
#define SETTING_DAY 4
#define SETTING_YEAR 5
#define SETTING_SAVE 6

  private:
    bool editing = false;
    int currentlySetting = SETTING_HOUR;
    int setHour;
    int setMinute;
    boolean setIsAm;
    int setMonth;
    int setDay;
    int setYear;

    const byte daysOfMonth[12] = {
      31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    void commitNewTime() {
      TimeElements tm;

      tm.Second = 0;
      tm.Minute = setMinute;
      tm.Hour = setIsAm ? setHour : setHour + 12;
      tm.Day = setDay;
      tm.Month = setMonth;
      tm.Year = setYear - 1970;

      Teensy3Clock.set(makeTime(tm));
      setTime(makeTime(tm));
    }

  public:
    SetTimeApp(Cartridge & owner): App(owner) {}
    virtual ~SetTimeApp() {}

    void setup() {
      setHour = hourFormat12();
      setMinute = minute();
      setIsAm = isAM();
      setMonth = month();
      setDay = day();
      setYear = year();
    }

    void update(BufferedVfd * display, BufferedLeds * leds) {
      if (setHour >= 10) {
        display->bufferedPrint('1', 0, 0);
        display->bufferedPrint(char('0' + (setHour % 10)));
      }
      else display->bufferedPrint(char('0' + setHour), 0, 1);

      display->bufferedPrint(':');

      if (setMinute < 10) display->bufferedPrint('0');
      else display->bufferedPrint(char('0' + (setMinute / 10)));
      display->bufferedPrint(char('0' + (setMinute % 10)));

      if (setIsAm) display->bufferedPrint(" AM ");
      else display->bufferedPrint(" PM ");

      if (setMonth < 10) display->bufferedPrint('0');
      display->bufferedPrint(setMonth);

      display->bufferedPrint('/');

      if (setDay < 10) display->bufferedPrint('0');
      display->bufferedPrint(setDay);

      display->bufferedPrint('/');

      display->bufferedPrint(setYear);

      if (currentlySetting == SETTING_SAVE) display->bufferedPrint('\x1d', 2, 0); // Right-pointing caret
      else {
        int caretColumn = 0;
        switch (currentlySetting) {
          case SETTING_HOUR: caretColumn = 1; break;
          case SETTING_MINUTE: caretColumn = 4; break;
          case SETTING_AM: caretColumn = 7; break;
          case SETTING_MONTH: caretColumn = 10; break;
          case SETTING_DAY: caretColumn = 13; break;
          case SETTING_YEAR: caretColumn = 18; break;
          default: caretColumn = 0;
        }

        display->bufferedPrint(editing ? '\x96' : '\x1F', 1, caretColumn);
      }

      display->bufferedPrint("Save and exit", 2, 1);

      display->bufferedPrint("(The Time sucks)", 3, 0);
    }

    void teardown(bool ejected) {}

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      if (clicks & knobMask) {
        if (currentlySetting == SETTING_SAVE) {
          commitNewTime();
          wantsToQuit = true;
        }
        else editing = !editing;
      }

      if (knobDelta != 0) {
        if (editing) {
          switch (currentlySetting) {
            case SETTING_HOUR:
              setHour = (setHour + knobDelta) % 12;
              if (setHour < 1) setHour = 12;
              break;
            case SETTING_MINUTE:
              setMinute = (setMinute + knobDelta) % 60;
              if (setMinute < 0) setMinute = 59;
              break;
            case SETTING_AM:
              setIsAm = !setIsAm;
              break;
            case SETTING_MONTH:
              setMonth = (setMonth + knobDelta) % 12;
              if (setMonth < 1) setMonth = 12;
              break;
            case SETTING_DAY:
              setDay = (setDay + knobDelta) % daysOfMonth[setMonth - 1];
              if (setDay < 1) setDay = daysOfMonth[setMonth - 1];
              // TODO: Handle leap years
              break;
            case SETTING_YEAR:
              setYear += knobDelta;
              setYear = max(setYear, 1970);
              break;
            case SETTING_SAVE:
              break;
            default: // Should not happen
              editing = false;
          }

          if (currentlySetting - 1 == SETTING_MONTH) setDay = min(setDay, daysOfMonth[setMonth - 1]);
        }
        else { // Not editing - move the caret
          currentlySetting -= knobDelta; // Remember that row 1 is BELOW row 0!
          currentlySetting = constrain(currentlySetting, 0, 6);
        }
      }
      return ~homeButtonMask; // Consume all controls except the home button
    }
};

class BootloadestApp : public App {
  private:
    char flavorTexts[4][21];
    byte topLine = 0;

    elapsedMillis timeSinceLastLine;
    unsigned long timeBetweenLines;

  public:
    BootloadestApp(Cartridge & owner) : App(owner) {}
    virtual ~BootloadestApp() {}

    void setup() {
      timeBetweenLines = animationTime;

      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 21; j++) flavorTexts[i][j] = 0x00;
      }

      getFullLine(flavorTexts[3], random(2));
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      if (timeSinceLastLine >= timeBetweenLines) {
        timeSinceLastLine -= timeBetweenLines;

        getFullLine(flavorTexts[topLine], random(2));

        topLine = (topLine + 1) % 4;
      }

      for (int i = 0; i < 4; i++) {
        if (i < 3) display->bufferedPrint(flavorTexts[(topLine + i) % 4], i, 0);
        else {
          int progress = constrain(map(long(timeSinceLastLine), 0, timeBetweenLines - 100, 0, 21), 0, 20);
          if (progress > 0) {
            display->bufferedPrint(flavorTexts[(topLine + i) % 4][0], i, 0);
            for (int j = 1; j <= progress; j++) {
              if (flavorTexts[(topLine + i) % 4][j] == 0x00) {
                break;
              }
              else {
                display->bufferedPrint(flavorTexts[(topLine + i) % 4][j]);
              }
            }
            if (progress < 19) display->bufferedPrint('\x14'); // Black block
          }
        }
      }

      /*
        int foo = random(11) + random(11);
        for (int i = 0; i < foo; i++) display->bufferedPrint(random(0x09, 0xFF) + 1, random(4), random(20));
        display->bufferedPrint("FUZZ DAT SHIT", 0, 0);

        for (int i = 0; i < 5; i++) leds->bufferLed(i, leds->Color(random(55) * 5, random(55) * 5, random(55) * 5));
      */
    }

    void teardown(bool ejected) {}

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      return 0x00;
    }
};

class NullCartridge : public Cartridge { // TODO get rid of this jank ass bullshit
  public:
    NullCartridge(byte slot) : Cartridge(slot) {}
    virtual ~NullCartridge() {}

    App * generateQuickfireApp() {
      return NULL;
    }
    App * generateMainApp() {
      return NULL;
    }
    App * generateDefenseApp() {
      return NULL;
    }

    void onInsert() {}
    void onEject() {}
};

NullCartridge nullCart(0xFF);

App * generateSystemApp(byte index) {
  if (index == 1) return new BootloadestApp(nullCart);
  if (index == 2) return new SetTimeApp(nullCart);
  if (index == 3) return new PowerSettingsApp(nullCart);
  if (index == 4) return new AnimationSettingsApp(nullCart);
  return NULL;
}

#endif
