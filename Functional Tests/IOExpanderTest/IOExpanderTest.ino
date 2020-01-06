#include <i2c_t3.h>

#define IOE_ID 0x24 
uint8_t data;
 
                     // IOE as in Input/Output Expander
void setup()
{
  // Configure interrupt
  pinMode(25, INPUT);
  
  // Set the PCA9534 pins to inputs
  Serial.begin(9600);

  delay(1000);
  Serial.println("Sanity 1");

  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_400);
  Wire.beginTransmission(IOE_ID);
  Wire.write(0x03);            // control register
  Wire.write(0x3f);            // Pins 0-5 are inputs
  Serial.println(Wire.endTransmission());

  Serial.println("Sanity 2");
}
 
void loop()
{
  // put your main code here, to run repeatedly:

  if (digitalRead(25)) return;
  
  Wire.beginTransmission(IOE_ID);
  Wire.write(0x00);                // go to the input port register
  Wire.endTransmission();
  Wire.requestFrom(IOE_ID, 1);     // start read mode
  if(Wire.available()){
      data = Wire.read();  // fetch one byte
      Wire.endTransmission();
      Serial.print(data, HEX);
      Serial.print(", ");
      Serial.print(data, BIN);
      Serial.print(", ");
      masking();
  } else {
    Serial.println("No connection?");
    Serial.println(Wire.getError());
  }

  delay(100); // Let interrupt clear
}
 
void masking(){
  if ((data & 0x08)){  // Bit 4
    Serial.print("Button 3");
  }
  if ((data & 0x20)){  // Bit 6
    Serial.print("Button 4");
  }
  if ((data & 0x04)){  // Bit 3
    Serial.print("Button 2");
  }
  if ((data & 0x10)){  // Bit 5
    Serial.print("Button 5");
  }
  if ((data & 0x02)){  // Bit 2
    Serial.print("Button 1");
  }
  if ((data & 0x01)){  // Bit 1
    Serial.println("Knob");
  }
  Serial.println();
}
