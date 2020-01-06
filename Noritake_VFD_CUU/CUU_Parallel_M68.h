class CUU_Parallel_M68 : public CUU_Interface {
protected:
    unsigned RS_PIN:4;
    unsigned RW_PIN:4;
    unsigned E_PIN:4;
    unsigned D0_PIN:4;
    unsigned D1_PIN:4;
    unsigned D2_PIN:4;
    unsigned D3_PIN:4;
    unsigned D4_PIN:4;
    unsigned D5_PIN:4;
    unsigned D6_PIN:4;
    unsigned D7_PIN:4;

public:
    CUU_Parallel_M68(uint8_t rs, uint8_t wr, uint8_t rd,
      uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
      uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7):
        RS_PIN(rs), RW_PIN(wr), E_PIN(rd),
        D0_PIN(d0), D1_PIN(d1), D2_PIN(d2), D3_PIN(d3),
        D4_PIN(d4), D5_PIN(d5), D6_PIN(d6), D7_PIN(d7)
    {
    }
    
    void init() {
        DIRECTION(RS, 1);
        DIRECTION(RW, 1);
        DIRECTION(E, 1);
    
        DIRECTION(D0, 1);
        DIRECTION(D1, 1);
        DIRECTION(D2, 1);
        DIRECTION(D3, 1);
        DIRECTION(D4, 1);
        DIRECTION(D5, 1);
        DIRECTION(D6, 1);
        DIRECTION(D7, 1);
        
        write(0x38,false);  // 8-bit function set
    	write(0x38,false);  // 8-bit function set
    }
    void write(uint8_t data, bool rs) {
        SETPIN(D0, data & 0x01);
        SETPIN(D1, data & 0x02);
        SETPIN(D2, data & 0x04);
        SETPIN(D3, data & 0x08);
        SETPIN(D4, data & 0x10);
        SETPIN(D5, data & 0x20);
        SETPIN(D6, data & 0x40);
        SETPIN(D7, data & 0x80);
        
        SETPIN(RS, rs);
        LOWER(RW);
        RAISE(E);
        _delay_us(0.5);
        LOWER(E);    
    }
    uint8_t read(bool rs) {
        DIRECTION(D0, 0);
        DIRECTION(D1, 0);
        DIRECTION(D2, 0);
        DIRECTION(D3, 0);
        DIRECTION(D4, 0);
        DIRECTION(D5, 0);
        DIRECTION(D6, 0);
        DIRECTION(D7, 0);
        
        SETPIN(RS, rs);    
        RAISE(RW);
        RAISE(E);
        uint8_t data =
            CHECK(D7) << 7 |
            CHECK(D6) << 6 |
            CHECK(D5) << 5 |
            CHECK(D4) << 4 |
            CHECK(D3) << 3 |
            CHECK(D2) << 2 |
            CHECK(D1) << 1 |
            CHECK(D0);
        LOWER(E);
        
        DIRECTION(D0, 1);
        DIRECTION(D1, 1);
        DIRECTION(D2, 1);
        DIRECTION(D3, 1);
        DIRECTION(D4, 1);
        DIRECTION(D5, 1);
        DIRECTION(D6, 1);
        DIRECTION(D7, 1);
        
        return data;
    }
    bool is8bit() { return true; }
};

class CUU_Parallel_M68_4bit : public CUU_Interface {
protected:
    unsigned RS_PIN:4;
    unsigned RW_PIN:4;
    unsigned E_PIN:4;
    unsigned D4_PIN:4;
    unsigned D5_PIN:4;
    unsigned D6_PIN:4;
    unsigned D7_PIN:4;

public:
    CUU_Parallel_M68_4bit(uint8_t rs, uint8_t wr, uint8_t rd,
      uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7):
        RS_PIN(rs), RW_PIN(wr), E_PIN(rd),
        D4_PIN(d4), D5_PIN(d5), D6_PIN(d6), D7_PIN(d7)
    {
    }
    
    void init() {
        RAISE(RW);
        
        DIRECTION(RS, 1);
        DIRECTION(RW, 1);
        DIRECTION(E, 1);
    
        DIRECTION(D4, 1);
        DIRECTION(D5, 1);
        DIRECTION(D6, 1);
        DIRECTION(D7, 1);    
        
        write(0x33,false);      // 8-bit function set, 8-bit function set
    	write(0x32,false);      // 8-bit function set, 4-bit function set
    }
    void write(uint8_t data, bool rs) {    
        SETPIN(D4, data & 0x10);
        SETPIN(D5, data & 0x20);
        SETPIN(D6, data & 0x40);
        SETPIN(D7, data & 0x80);
        
        SETPIN(RS, rs);
        LOWER(RW);
        
        RAISE(E);
        _delay_us(0.5);
        LOWER(E);
        
        SETPIN(D4, data & 0x01);
        SETPIN(D5, data & 0x02);
        SETPIN(D6, data & 0x04);
        SETPIN(D7, data & 0x08);
        RAISE(E);
        _delay_us(0.5);
        LOWER(E);
    }
    uint8_t read(bool rs) {
        DIRECTION(D4, 0);
        DIRECTION(D5, 0);
        DIRECTION(D6, 0);
        DIRECTION(D7, 0);
        
        SETPIN(RS, rs);    
        RAISE(RW);
        
        RAISE(E);
        _delay_us(5);
        uint8_t data =
            CHECK(D7) << 7 |
            CHECK(D6) << 6 |
            CHECK(D5) << 5 |
            CHECK(D4) << 4;
        LOWER(E);
        _delay_us(5);
        
        RAISE(E);
        _delay_us(5);
        data |=
            CHECK(D7) << 3 |
            CHECK(D6) << 2 |
            CHECK(D5) << 1 |
            CHECK(D4);
        LOWER(E);
        
        DIRECTION(D4, 1);
        DIRECTION(D5, 1);
        DIRECTION(D6, 1);
        DIRECTION(D7, 1);
        
        return data;
    }
    bool is8bit() { return false; }
};