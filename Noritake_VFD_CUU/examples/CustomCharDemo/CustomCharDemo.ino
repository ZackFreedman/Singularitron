#include <CUU_Interface.h>
#include <CUU_Parallel_I80.h>
#include <CUU_Parallel_M68.h>
#include <CUU_Serial.h>
#include <Noritake_VFD_CUU.h>
#include <util/delay.h>

// Uncomment one of the communication interfaces below.
//CUU_Serial interface(3, 5, 6); // SIO, STB, SCK
//CUU_Parallel_I80 interface(9,10,11, 0,1,2,3,4,5,6,7); //RS,RW,E,D0-D7
//CUU_Parallel_I80_4bit interface(9,10,11, 4,5,6,7); //RS,RW,E,D4-D7
//CUU_Parallel_M68 interface(9,10,11, 0,1,2,3,4,5,6,7);//RS,WR,RD,D0-D7
//CUU_Parallel_M68_4bit interface(9,10,11, 4,5,6,7);//RS,WR,RD,D4-D7

Noritake_VFD_CUU vfd;

uint8_t cone[] = {
  0b00000,
  0b00001,
  0b00011,
  0b00111,
  0b00111,
  0b00011,
  0b00001,
  0b00000
};

uint8_t wave[] = {
  0b01000,
  0b00100,
  0b10010,
  0b01010,
  0b01010,
  0b10100,
  0b01000,
  0b00000
};

void setup() {
  _delay_ms(500);      // wait for device to power up
  vfd.begin(20, 2);    // 20x2 character module
  vfd.interface(interface); // select which interface to use
  
  // Uncomment if the target module supports brightness boost
  //vfd.brightnessBoost(); // module has brightness boost
  
  // Uncomment if model is DS2045G
  //vfd.bcVFD();          // is DS2045G

  vfd.CUU_init();      // initialize module
  
  //vfd.japaneseFont();   // select Japanese font for DS2045G
  //vfd.europeanFont();   // select European font for DS2045G

  vfd.CUU_createChar(0x00, cone); // define cone as char 0
  vfd.CUU_createChar(0x01, wave); // define wave as char 1
  vfd.print('\x00'); // print cone
  vfd.print('\x01'); // print wave
}

void loop() {
}
