#ifndef Pins_h_
#define Pins_h_

#define ble Serial2

#define buttonDriver 0x24
#define cartIOExpander 0x20

#define ledsPin 0
#define encoderB 1
#define fiveVoltSupply 3
#define sentralInterruptPin 4
#define displaySupply 5
#define vibes 6
#define displaySIO 7
#define displayReset 8
#define spiSCK 13
#define cart2gpio1 14
#define cart1gpio2 15
#define cart0gpio2 16
#define cart0gpio1 17
#define cart3gpio2 20
#define cart3gpio1 21
#define cart1gpio1 22
#define cart2gpio2 23
#define batteryMonitor A12
#define hotplugInterruptPin 24
#define buttonInterruptPin 25
#define displaySTB 26
#define displaySCK 31
#define encoderA 32
#define chargeStatusPin 33

const int cartGpioPins[4][2] = {
  {cart0gpio1, cart0gpio2},
  {cart1gpio1, cart1gpio2},
  {cart2gpio1, cart2gpio2},
  {cart3gpio1, cart3gpio2}
  };

#endif

