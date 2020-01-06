// remap_mlx90614.ino

// #include "i2cmaster.h"
#include <i2c_t3.h>

// New slave address, purposefully left shifted
byte NewMLXAddr = 0x1B;
// 0x5A is the default address - uncomment this to set it back
// byte NewMLXAddr = 0x5A;

void setup() {
  Serial.begin(9600);

  delay(1000);

  Serial.println("Setup...");
  // Initialise the i2c bus, enable pullups and then wait
  Wire1.begin(I2C_MASTER, 0x00, I2C_PINS_29_30, I2C_PULLUP_EXT, I2C_RATE_100);
  delay(5000);
  // Read current address bytes
  ReadAddr(0);
  // Change address to new value (NewMLXAddr);
  ChangeAddr(NewMLXAddr, 0x00);
  // Read address bytes
  ReadAddr(0);
  Serial.print("> Cycle power NOW to set address to: ");
  Serial.print(NewMLXAddr, HEX);
  Serial.println(" - you have 10 seconds");
  // Cycle power to MLX during this 10 second pause
  delay(10000);
  // Read temperature using default address
  ReadTemp(0);
  // Read temperature using new address (note left bit shift for reading)
  ReadTemp(NewMLXAddr << 1);
  Serial.println("**---DONE---**");
}

void loop() {
  delay(5000);
  ReadTemp(NewMLXAddr << 1);
}

word ChangeAddr(byte NewAddr1, byte NewAddr2) {
  Serial.println("> Change address");
  // Send start condition and write bit
  //   Wire1.beginTransmission(0);
  Wire1.beginTransmission(0);
  // Send command for device to return address
  Wire1.write(0x2E);
  // Send low byte zero to erase
  Wire1.write(0x00);
  // Send high byte zero to erase
  Wire1.write(0x00);

  Wire1.write(0x6F);

  if (Wire1.endTransmission(false) == 0) {
    // Release bus, end transaction
    Serial.println("> Data erased.");
  }
  else {
    Serial.println("> Failed to erase data");
    return -1;
  }

  Serial.print("> Writing data: ");
  Serial.print(NewAddr1, HEX);
  Serial.print(", ");
  Serial.println(NewAddr2, HEX);

  for (int a = 0; a != 256; a++) {
    // Send start condition and write bit
    Wire1.beginTransmission(0);
    // Send command for device to return address
    Wire1.write(0x2E);
    // Send low byte zero to erase
    Wire1.write(NewAddr1);
    // Send high byte zero to erase
    Wire1.write(NewAddr2);
    Wire1.write(a);
    if (Wire1.endTransmission() == 0) {
      // Release bus, end transaction then wait 10ms
      delay(100);
      Serial.print("> Found correct CRC: 0x");
      Serial.println(a, HEX);
      return a;
    }
  }

  // Release bus, end transaction
  Wire1.endTransmission();
  Serial.println("> Correct CRC not found");
  return -1;
}

void ReadAddr(byte Address) {
  Serial.println("> Read address");
  // Inform the user
  Serial.print("  MLX address: ");
  Serial.print(Address, HEX);
  Serial.print(", Data: ");

  // Send start condition and write bit
  Wire1.beginTransmission(Address);
  // Send command for device to return address
  Wire1.write(0x2E);
  Wire1.endTransmission(false); 
  
  Wire1.requestFrom(Address, 3);

  // Read 1 byte and then send ack (x2)
  Serial.print(Wire1.read(), HEX);
  Serial.print(", ");
  Serial.print(Wire1.read(), HEX);
  Serial.print(", ");
  Serial.println(Wire1.read(), HEX);
//  Wire1.endTransmission();
}

float ReadTemp(byte Address) {
  int data_low = 0;
  int data_high = 0;
  int pec = 0;

  Serial.println("> Read temperature");
  // Inform the user
  Serial.print("  MLX address: ");
  Serial.print(Address, HEX);
  Serial.print(", ");

  Wire1.beginTransmission(Address);
  // Address of temp bytes
  Wire1.write(0x07);
  // Read - the famous repeat start
  Wire1.endTransmission(); 
  Wire1.requestFrom(Address, 3);
  // Read 1 byte and then send ack (x2)
  data_low = Wire1.read();
  data_high = Wire1.read();
  pec = Wire1.read();
  Wire1.endTransmission();

  // This converts high and low bytes together and processes the temperature
  // MSB is a error bit and is ignored for temperatures
  // Zero out the data
  float temp = 0x0000;
  // This masks off the error bit of the high byte, then moves it left
  // 8 bits and adds the low byte.
  temp = (float)(((data_high & 0x007F) << 8) + data_low);
  temp = (temp * 0.02) - 273.16;
  Serial.print(temp);
  Serial.println(" C");
  return temp;
}
