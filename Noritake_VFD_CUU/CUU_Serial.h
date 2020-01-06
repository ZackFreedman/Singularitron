class CUU_Serial : public CUU_Interface {
protected:
    uint8_t SIO_PIN:8;
    uint8_t STB_PIN:8;    
    uint8_t SCK_PIN:8;

public:
    CUU_Serial(uint8_t sio, uint8_t stb, uint8_t sck):
        SIO_PIN(sio), STB_PIN(stb), SCK_PIN(sck)
    {
    }
    
    void init() {
    	// _delay_ms(500);
        _delay_ms(50);
        
        RAISE(SIO);
        RAISE(SCK);
        RAISE(STB);
        
        DIRECTION(SIO, 1);
        DIRECTION(SCK, 1);
        DIRECTION(STB, 1);

        write(0x30,false);  // 8-bit function set
    	write(0x30,false);  // 8-bit function set
    	write(0x30,false);  // 8-bit function set
    	write(0x30,false);  // 8-bit function set
    }
    void write(uint8_t data, bool rs) {
    	uint8_t	x = 0xf8 + 2*rs, i=0x80;

    	LOWER(STB);
    	for ( ; i; i>>=1) {
            LOWER(SCK);
    		if (x & i)
    			RAISE(SIO);
    		else
    			LOWER(SIO);
    		RAISE(SCK);
    	}
    	
    	x = data;
    	for (i=0x80; i; i>>=1) {
    		LOWER(SCK);
    		if (x & i)
    			RAISE(SIO);
    		else
    			LOWER(SIO);
    		RAISE(SCK);
    	}
    	RAISE(STB);
    }
    uint8_t read(bool rs) {
    	uint8_t	i=0x80, data = 0xfc + 2*rs;
    
    	LOWER(STB);
    	for ( ; i; i>>=1) {
    		LOWER(SCK);
    		if (data & i)
    			RAISE(SIO);
    		else
    			LOWER(SIO);
    		RAISE(SCK);
    	}
    
        DIRECTION(SIO, 0);
    	_delay_us(1);
    	
    	for (i=0; i<8; i++) {
    		LOWER(SCK);
    		data <<= 1;
    		RAISE(SCK);
    		if (CHECK(SIO))
    			data |= 1;
    	}
    
    	RAISE(STB);
    	DIRECTION(SIO, 1);
    	return data;
    }
    bool is8bit() { return true; };
};