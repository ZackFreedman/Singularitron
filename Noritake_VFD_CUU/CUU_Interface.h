#include "Arduino.h"
#include <stdint.h>
#include <util/delay.h>

#define DIRECTION(X,D)  if (D) pinMode(X##_PIN, OUTPUT); else pinMode(X##_PIN, INPUT)
#define RAISE(X)	digitalWriteFast(X##_PIN, HIGH)
#define LOWER(X)	digitalWriteFast(X##_PIN, LOW)
#define CHECK(X)	digitalReadFast(X##_PIN)
#define SETPIN(X,V) digitalWriteFast(X##_PIN, (V)? HIGH: LOW)

class CUU_Interface {

public:
    virtual void init() = 0;
    virtual void write(uint8_t data, bool rs) = 0;
    virtual uint8_t read(bool rs) = 0;
    virtual bool is8bit() = 0;
};
