#include <SdFat.h>

// IMPORTANT: USE_STANDARD_SPI_LIBRARY must be 1 in SdFatConfig. Something funny is going on with the custom library.

#include "app.h"
#include "cart.h"
#include "duck.h"
#include "scrollableList.h"

#define cardSlotEmpty 0
#define cardFaulty 1
#define cardNotFAT 2
#define cardUnknownType 3
#define cardOK 4
#define cantOpenRoot 5

class SDCardModule : public Cartridge {
  private:
    class SDCardApp;

    SdFat sd;
    byte chipSelectPin;
    byte cardDetectPin;

    byte cardStatus = cardSlotEmpty;
    byte cardType = 0;
    byte fatType = 0;
    float volumeSize = 0.0; // GB

  public:
    SDCardModule(byte slot) : Cartridge(slot) {
      id = sdCardModuleId;
      name = "SDCardModule";
      //      color = 0x000088;
      color = CRGB::Blue;
      widgetsAvailable = 1;
      hasMainApp = true;
      backgroundUpdateInterval = 1000;

      debug_println("SDCardModule constructed");
    }

    virtual ~SDCardModule() {}

    void onInsert() {
      debug_println("SDCardModule inserted");
      pinMode(cartGpioPins[slot][0], INPUT_PULLUP); // Chip detect
      chipSelectPin = cartGpioPins[slot][1];
      cardDetectPin = cartGpioPins[slot][0];
      pinMode(10, OUTPUT); // Stupid workaround for stupid issue that requires hardware SS pin to be an output
    }

    void onEject() {
      debug_println("SDCardModule ejected");
      pinMode(cartGpioPins[slot][0], INPUT); // Chip detect
      digitalWrite(cartGpioPins[slot][0], LOW);
      digitalWrite(cartGpioPins[slot][1], LOW);
      pinMode(10, INPUT); // Stupid workaround
    }

    App * generateQuickfireApp();

    App * generateMainApp();

    App * generateDefenseApp();

    void backgroundUpdate();

    void renderWidget(BufferedVfd * display, byte widgetNumber);
};

void SDCardModule::backgroundUpdate() {
  if (cardStatus == cardSlotEmpty && digitalRead(cardDetectPin) == HIGH) {
    // Card has just been plugged in!

    if (!sd.begin(chipSelectPin, SPI_HALF_SPEED)) cardStatus = cardFaulty;
    else {
      cardType = sd.card()->type();
      if (cardType != SD_CARD_TYPE_SD1 && cardType != SD_CARD_TYPE_SD2 && cardType != SD_CARD_TYPE_SDHC) cardStatus = cardUnknownType;
      else {
        if (sd.vol()->fatType() == 0) cardStatus = cardNotFAT;
        else {
          if (!sd.vwd()->isOpen()) cardStatus = cantOpenRoot;
          else {
            cardStatus = cardOK;
            fatType = sd.vol()->fatType();
            volumeSize = float(sd.vol()->blocksPerCluster() * sd.vol()->clusterCount()) * 512.0 / 1024.0 / 1024.0 / 1024.0;

            debug_println("SD card OK!");
          }
        }
      }
    }
  }
  else if (cardStatus != cardSlotEmpty && digitalRead(cardDetectPin) == LOW) {
    // Card has just been ejected!
    cardStatus = cardSlotEmpty;
  }
}

void SDCardModule::renderWidget(BufferedVfd * display, byte widgetNumber) {
  if (widgetNumber != 0) {
    display->bufferedPrint("?????");
    return;
  }

  display->bufferedPrint(display->createCustomChar(fileIcon));
  if (cardStatus == cardOK) {
    display->bufferedPrint(volumeSize);
    display->bufferedPrint("GB");
  }
  else if (cardStatus == cardSlotEmpty) display->bufferedPrint("Empty");
  else display->bufferedPrint("ERR");
}

class SDCardModule::SDCardApp : public App {
  private:
    SDCardModule & sdcm;

    ScrollableList scriptPicker;
    char scriptList[30][20];
    int numberOfScripts = 0;
    bool showScriptPicker = false;

    bool mustDuck = false;
    bool ducking = false;

    SdFile scriptFile;
    String scriptLine;
    String lastScriptLine;

  public:
    SDCardApp(Cartridge & owner) : App(owner), sdcm(static_cast<SDCardModule&>(owner)), scriptList() {
      scriptLine.reserve(1000); // Probably gratuitous but whatevs
      lastScriptLine.reserve(1000);
    }

    virtual ~SDCardApp() {}

    void setup() {
      sdcm.backgroundUpdateInterval = 100;
    }

    void update(BufferedVfd *display, BufferedLeds *leds) {
      if (sdcm.cardStatus != cardOK) {
        showScriptPicker = false;
        mustDuck = false;
        ducking = false;
      }

      if (ducking) {
        scriptLine.remove(0);
        lastScriptLine.remove(0);

        debug_println("Ducking!");

        int defaultDelay = 100;
        int lineNumber = 0;

        while (scriptFile.available()) {
          char nextChar = scriptFile.read();
          //        debug_print(nextChar);
          scriptLine += nextChar;

          if (!scriptFile.available() || nextChar == '\r' || nextChar == '\n') {
            scriptLine.trim();
            if (scriptLine.length() > 0) {
              display->moveCursorTo(3, 0);
              for (int i = 0; i < 20; i++) {
                if (i < scriptLine.length()) display->bufferedPrint(scriptLine[i]);
                else display->bufferedPrint(' ');
              }

              display->render(true);
              
              lineNumber++;

              DuckResponse outcome = duck(scriptLine.c_str(), scriptLine.length());

              // We're ignoring errors for now, but change after testing

              switch (outcome.response) {
                case notACommand:
                  debug_print("Line ");
                  debug_print(lineNumber);
                  debug_println(" bad cmd");
                  break;

                case missingToken:
                  debug_print("Line ");
                  debug_print(lineNumber);
                  debug_println(" missing param");
                  break;

                case badToken:
                  debug_print("Line ");
                  debug_print(lineNumber);
                  debug_println(" bad param");
                  break;

                case commandOk:
                  debug_println("Aight");
                  break;

                case changedDefaultDelay:
                  defaultDelay = outcome.otherInformation;
                  debug_print("Delay is now ");
                  debug_print(defaultDelay);
                  debug_println("ms");
                  break;

                case mustDelay:
                  debug_print("Delaying ");
                  debug_print(outcome.otherInformation);
                  debug_println("ms");
                  delay(outcome.otherInformation);
                  break;

                case mustRepeat:
                  debug_print("Repeating cmd \"");
                  debug_print(lastScriptLine);
                  debug_print("\" ");
                  debug_print(outcome.otherInformation);
                  debug_println(" times");
                  // I really need to make this more efficient
                  for (int i = 0; i < outcome.otherInformation; i++)
                    duck(lastScriptLine.c_str(), lastScriptLine.length());
                  break;

                default:
                  debug_print("Line ");
                  debug_print(lineNumber);
                  debug_print(": WTF is response ");
                  debug_print(outcome.response);
                  debug_println('?');
              }

              if (outcome.response != mustDelay) delay(defaultDelay);

              if (outcome.response != mustRepeat) { // I don't actually know what should happen when you repeat a repeat. I'm assuming it does the previous command.
                lastScriptLine.remove(0);
                lastScriptLine.concat(scriptLine); // Can I do this with assignment without sum bull shyit?
              }

              scriptLine.remove(0);
            }
          }
        }

        debug_println("Ducking finally");
        scriptFile.close();

        debug_println("Ducking complete");
        ducking = false;
        mustDuck = false;
      }

      if (showScriptPicker) scriptPicker.render(display);
      else if (sdcm.cardStatus == cardSlotEmpty) {
        display->bufferedPrint("Please insert an");
        display->bufferedPrint("SD card", 1, 0);
      }
      else if (sdcm.cardStatus == cardFaulty) {
        display->bufferedPrint("SD card error ");
        display->bufferedPrint(int(sdcm.sd.card()->errorCode()));
        display->bufferedPrint("Data: ", 2, 0);
        display->bufferedPrint(int(sdcm.sd.card()->errorData()));
      }
      else if (sdcm.cardStatus == cardNotFAT) {
        display->bufferedPrint("The Data Logger cart");
        display->bufferedPrint("can only read and", 1, 0);
        display->bufferedPrint("write FAT-formatted", 2, 0);
        display->bufferedPrint("SD cards!", 3, 0);
      }
      else if (sdcm.cardStatus == cardUnknownType) {
        display->bufferedPrint("Unknown type of SD");
        display->bufferedPrint("card detected!", 1, 0);
      }
      else if (sdcm.cardStatus == cantOpenRoot) {
        display->bufferedPrint("Can't open root!");
      }
      else {
        display->bufferedPrint("Detected ");
        switch (sdcm.cardType) {
          case SD_CARD_TYPE_SD1:
            display->bufferedPrint("SD1");
            break;
          case SD_CARD_TYPE_SD2:
            display->bufferedPrint("SD2");
            break;
          case SD_CARD_TYPE_SDHC:
            display->bufferedPrint("SDHC");
            break;
          default:
            display->bufferedPrint("ERR!");
        }
        display->bufferedPrint(" card");

        display->bufferedPrint("Format: FAT", 1, 0);
        display->bufferedPrint(sdcm.fatType);

        display->bufferedPrint("Size: ", 2, 0);
        display->bufferedPrint(sdcm.volumeSize);
        display->bufferedPrint("GB");

        if (!ducking && mustDuck) {
          display->bufferedPrint('\x96', 3, 0); // Filled diamond
          display->bufferedPrint("HAXX IN PROGRESS...");
          ducking = true;
        }
        else {
          display->bufferedPrint('\x1D', 3, 0); // Right-pointing caret
          display->bufferedPrint("Initiate haxx");
        }
      }
    }

    void teardown(bool ejected) {
      sdcm.backgroundUpdateInterval = 1000;
      if (scriptFile.isOpen()) scriptFile.close();
    }

    byte onControlEvent(byte controlStates, byte clicks, byte holds, int knobDelta) {
      if (showScriptPicker) {
        if (sdcm.cardStatus != cardOK) showScriptPicker = false;
        else {
          int pickedScriptIndex = scriptPicker.control(knobDelta * -1, clicks & knobMask);
          if (pickedScriptIndex == 0) {
            debug_println("Script selection canceled");
            showScriptPicker = false;
          }
          else if (pickedScriptIndex > 0) {
            // Figure out what the name of the selected script is
            // TODO Combine this with script picker assembly
            SdFile scriptDirectory;
            int currentScriptNumber = 1; // Picker number 0 is taken up by "go back" entry
            char scriptName[13]; // Max FAT file name size

            scriptDirectory.open("duck", O_READ);

            while (scriptFile.openNext(&scriptDirectory, O_READ)) {
              if (!scriptFile.isFile()) {
                scriptFile.close();
                continue;
              }

              if (currentScriptNumber == pickedScriptIndex) break; // Leave script open, we'll use it in duck()

              currentScriptNumber++;
              scriptFile.close();
            }

            if (scriptFile.isOpen()) {
              debug_print("Picked file ");
              debug_print(pickedScriptIndex);
              debug_print(" - ");
              debug_println(scriptName);
              mustDuck = true;
            }
            else {
              debug_print("Script ");
              debug_print(pickedScriptIndex);
              debug_println(" missing..?");
            }
            showScriptPicker = false;
          }
        }
        return updownMask + knobMask;
      }

      if (clicks & knobMask) {
        if (sdcm.cardStatus != cardOK) {
          debug_println("Can't duck - no card");
          showScriptPicker = false;
          return knobMask;
        }

        scriptPicker.reset();
        int numberOfScripts = 0;
        SdFile scriptDirectory;
        SdFile scannedScript;
        char scriptName[13]; // Max FAT file name size

        if (!scriptDirectory.open("duck", O_READ)) {
          debug_println("No script folder?");
          scriptDirectory.close();
          return knobMask;
        }

        while (scannedScript.openNext(&scriptDirectory, O_READ)) {
          if (!scannedScript.isFile()) {
            scannedScript.close();
            continue;
          }
          scannedScript.getName(scriptName, 13);
          scriptPicker.append(scriptName);
          numberOfScripts++;
          scannedScript.close();
        }

        if (numberOfScripts > 0)
          showScriptPicker = true;
        else {
          showScriptPicker = false;
          debug_println("No scripts found!");
        }

        scriptDirectory.close();

        return knobMask;
      }

      return 0x00;
    }
};

App * SDCardModule::generateQuickfireApp() {
  debug_println("ERROR! SDCardModule has no QF app");
  return NULL;
}

App * SDCardModule::generateMainApp() {
  debug_println("SDCardModule main generated");

  return new SDCardApp(*this);
}

App * SDCardModule::generateDefenseApp() {
  debug_println("ERROR! SDCardModule has no defense app");
  return NULL;
}
