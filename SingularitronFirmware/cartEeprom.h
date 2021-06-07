#ifndef slot_EEPROM_H
#define slot_EEPROM_H

byte activeEeprom = 0xFF; // slot with an open EEPROM connection

void selectActiveEeprom() {
  // Assert CS of target slot's EEPROM
  Wire.beginTransmission(cartIOExpander);
  Wire.write(0x01);          // go to the output port register
  Wire.write(1 << (activeEeprom * 2)); // CS lines are index 0, 2, 4, and 6
  Wire.endTransmission();
}

void deselectEeproms() {
  // Deassert SS
  Wire.beginTransmission(cartIOExpander);
  Wire.write(0x01);  // go to the output port register
  Wire.write(0x00);  // Set all lines low. Note that this has no effect on input lines.
  Wire.endTransmission();
}

void openEeprom(byte slot) {
  if (slot > 3) {
    debug_print("slot ");
    debug_print(slot);
    debug_println(" invalid");

    return;
  }
  // Check if slot is plugged in?

  // Initialize SPI pins (should be done before CS is asserted)
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // May need 2MHz

  activeEeprom = slot;
  selectActiveEeprom();
}

unsigned int readEeprom(unsigned int address, byte slot) {
  // This all assumes X16 (16-bit word) organization.
  // Address must be below 128

  byte previousEeprom = 255;

  if (activeEeprom != slot) {
    previousEeprom = activeEeprom;
    openEeprom(slot);
  }

  // Clear that shit!
  //  for (int i = 0; i < eepromBufferLength; i++) eepromBuffer[i] = 0x00;

  // Command that shit!
  unsigned int readCommand = 0b00000110 << 8; // Read command [5 bits garbo][start bit 1][opcode 10]
  SPI.transfer16(readCommand + address);

  //  digitalWrite(spiSCK, HIGH);
  //  delay(1);
  //  digitalWrite(spiSCK, LOW);
  //  delay(1);

  // Download that shit!
  unsigned int high = SPI.transfer16(0x0000);
  unsigned int low = SPI.transfer16(0x0000); // Garbage 0 pushes everything down into the next word

  // Toggle CS to end sequential read - otherwise next command would be ignored
  deselectEeproms();
  delayMicroseconds(1);
  selectActiveEeprom();

  if (previousEeprom != 255) {
    openEeprom(previousEeprom);
  }

  return (high << 1) + (low >> 15);
}

unsigned int readEeprom(unsigned int address) {
  return readEeprom(address, activeEeprom);
}

void writeEeprom(unsigned int address, unsigned int data, byte slot) {
  if (readEeprom(address, slot) == data) {
    debug_print("Addr 0x");
    debug_print(address, HEX);
    debug_print(" is already 0x");
    debug_println(data, HEX);
    return; // No redundant writes please
  }

  byte previousEeprom = 255;

  if (activeEeprom != slot) {
    previousEeprom = activeEeprom;
    openEeprom(slot);
  }

  // Issue erase/write enable
  SPI.transfer16((0b00000100 << 8) + 0b11000000);
  // EWEN byte 1 - [5 bytes garbo][start bit 1][opcode 00]
  // EWEN byte 2 - [1][A7 set][6 bytes garbo]

  // Flip CS to finish command
  deselectEeproms();
  delayMicroseconds(1);
  selectActiveEeprom();

  unsigned int writeCommand = 0b00000101 << 8; // Command byte - [5 bytes garbo][SB 1][opcode 01]

  // Be careful that address is less than 128 on the x8 device!
  SPI.transfer16(writeCommand + address);
  SPI.transfer16(data);

  // Commit erase and programming on the LC series by dropping the CS pin for a little bit
  deselectEeproms();
  delayMicroseconds(1); // Nominal minimum 250ns
  selectActiveEeprom();

  // Wait for device to complete the write
  delay(10); // Nominal maximum 6ms

  // Issue erase/write disable
  SPI.transfer16((0b00000100 << 8) + 0b00000000);
  // EWDS byte 1 - [5 bytes garbo][start bit 1][opcode 00]
  // EWDS byte 2 - [0][A7 cleared][6 bytes garbo]

  // Flip CS to finish command
  deselectEeproms();
  delayMicroseconds(1);

  if (previousEeprom != 255) {
    openEeprom(previousEeprom);
  }
  else {
    selectActiveEeprom();
  }
}

void writeEeprom(unsigned int address, unsigned int data) {
  writeEeprom(address, data, activeEeprom);
}

//void closeEeprom() {
//  activeEeprom = 0xFF;
//
//  SPI.endTransaction();
//
//  deselectEeproms();
//}

#endif
