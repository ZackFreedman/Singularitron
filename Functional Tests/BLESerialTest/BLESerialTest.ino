// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/*
 * Serial Port over BLE
 * Create UART service compatible with Nordic's *nRF Toolbox* and Adafruit's *Bluefruit LE* iOS/Android apps.
 *
 * BLESerial class implements same protocols as Arduino's built-in Serial class and can be used as it's wireless
 * replacement. Data transfers are routed through a BLE service with TX and RX characteristics. To make the
 * service discoverable all UUIDs are NUS (Nordic UART Service) compatible.
 *
 * Please note that TX and RX characteristics use Notify and WriteWithoutResponse, so there's no guarantee
 * that the data will make it to the other end. However, under normal circumstances and reasonable signal
 * strengths everything works well.
 */


// Import libraries (BLEPeripheral depends on SPI)
#include <SPI.h>
#include <BLEPeripheral.h>
#include "BLESerial.h"

// define pins (varies per shield/board)
//#define BLE_REQ   10
//#define BLE_RDY   2
//#define BLE_RST   9

//#define PIN_SERIAL_RX (28) // HW pin 12, P0.29, labeled INT_BLE
//#define PIN_SERIAL_TX (29) // HW pin 11, P0.28, labeled WAKE_BLE

// create ble serial instance, see pinouts above
BLESerial BLESerial(-1, -1, -1);
bool wasConnected = false;

void setup() {
  // custom services and characteristics can be added as well
  BLESerial.setLocalName("Singularitron");

  Serial.setPins(28, 29);
  Serial.begin(9600);
  BLESerial.begin();

  sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
}

void loop() {
  sd_app_evt_wait(); // Go to sleep. This is linked in from the SoftDevice.

  if (BLESerial) {
    if (!wasConnected) Serial.print("C");
    wasConnected = true;
  }
  else {
    if (wasConnected) Serial.print("D");
    wasConnected = false;
  }
  
  BLESerial.poll();

//   Serial.println("Sup breh");
//   delay(100);
  
//  loopback_hwserial();
  forward();
  //loopback();
//   spam();

}


// forward received from Serial to BLESerial and vice versa
void forward() {
  if (BLESerial && Serial) {
    int byte;
    while ((byte = BLESerial.read()) > 0) Serial.write((char)byte);
    while ((byte = Serial.read()) > 0) BLESerial.write((char)byte);
  }
}

// echo all received data back
void loopback() {
  if (BLESerial) {
    int byte;
    while ((byte = BLESerial.read()) > 0) BLESerial.write(byte);
  }
}

// echo all received data back
void loopback_hwserial() {
  if (Serial) {
    int byte;
    while ((byte = Serial.read()) > 0) Serial.write(byte);
  }
}

// periodically sent time stamps
void spam() {
  if (BLESerial) {
    BLESerial.print(millis());
    BLESerial.println(" tick-tacks!");
    delay(1000);
  }
}
