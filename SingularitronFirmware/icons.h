#ifndef ICONS_H
#define ICONS_H

const uint8_t fileIcon[] = {0x1e, 0x15, 0x17, 0x11, 0x11, 0x11, 0x1f, 0x0};
const uint8_t thermometerIcon[] = {0x4, 0xa, 0xa, 0xa, 0x11, 0x1f, 0xe, 0x0};
const uint8_t co2Icon[] = {0x8, 0x14, 0xe, 0x1f, 0xe, 0x5, 0x2, 0x0};
const uint8_t smokeIcon[] = {0x4, 0x11, 0x4, 0x2, 0x8, 0x4, 0x0, 0x0};
const uint8_t humidityIcon[] = {0x4, 0x4, 0xe, 0xe, 0x1d, 0x1f, 0xe, 0x0}; 
const uint8_t barometerIcon[] = {0xc, 0x12, 0x12, 0x1e, 0x1c, 0x1e, 0xc, 0x0};
const uint8_t wristwatchIcon[] = {0xe, 0xe, 0x15, 0x17, 0x11, 0xe, 0xe, 0x0};
const uint8_t stopwatchIcon[] = {0xe, 0x4, 0xe, 0x15, 0x17, 0x11, 0xe, 0x0};
const uint8_t flagIcon[] = {0x15, 0xb, 0x15, 0xb, 0x1, 0x1, 0x1, 0x0};
const uint8_t altitudeIcon[] = {0x4, 0xe, 0x4, 0xe, 0x4, 0x1f, 0x4, 0x0};
const uint8_t dangerIcon[] = {0xe, 0x15, 0xe, 0x4, 0x11, 0xe, 0x11, 0x0};
const uint8_t batteryIcon[] = {0xe, 0x1f, 0x11, 0x11, 0x11, 0x11, 0x1f, 0x0};
const uint8_t phoneIcon[] = {0x1, 0x1f, 0x11, 0x11, 0x11, 0x11, 0x1f, 0x0};
const uint8_t lightningIcon[] = {0x3, 0xe, 0x1c, 0xe, 0x7, 0xe, 0x18, 0x0};
const uint8_t chipIcon[] = {0x1f, 0xa, 0x1b, 0xa, 0x1b, 0xa, 0x1f, 0x0};
const uint8_t bluetoothIcon[] = {0x4, 0x16, 0xd, 0x6, 0xd, 0x16, 0x4, 0x0};
const uint8_t waypointIcon[] = {0xe, 0x11, 0x11, 0xe, 0x4, 0x4, 0x4, 0x0};

// Note that a one-segment vertical bar is identical to a _ and a seven-segment bar is an 0xFF. Maybe use those instead?
const uint8_t verticalBars[][8] = {
  {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1f, 0x0},
  {0x0, 0x0, 0x0, 0x0, 0x0, 0x1f, 0x1f, 0x0},
  {0x0, 0x0, 0x0, 0x0, 0x1f, 0x1f, 0x1f, 0x0},
  {0x0, 0x0, 0x0, 0x1f, 0x1f, 0x1f, 0x1f, 0x0},
  {0x0, 0x0, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0},
  {0x0, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0},
  {0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0}
};

#endif