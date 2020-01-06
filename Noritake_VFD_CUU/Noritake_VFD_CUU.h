#include <stdint.h>
#include <stddef.h>

class Noritake_VFD_CUU {
	protected:
	bool	display;	// display is on
	bool	cursor;		// cursor is displayed
	bool	blink;		// cursor is blinking
	bool	rightToLeft;	// text direction is right-to-left
	bool	autoscroll;	// autoscroll on input
    bool    hasBrightnessBoost; // module has brightness boost
    bool    is_cu20045_uw4j; // module is a CU20045-UW4J
    CUU_Interface *io; // interface
	
	void setDisplay();	// Set display according to display/cursor/blink members
	void setDirection();	// Set text direction based on rightToLeft/autoscroll
	void printNumber(long number, int base);
	void printNumber(unsigned long number, int base);
	void nextLine();
	
	public:
    
    int     cols; // Number of columns on the display
    int     lines; // Number of lines on the display.
    bool    bc_font;// true=Japanese (default); false=European
    bool    bc_vfd; // true=DS2045G; false=CU-U
    
    void begin(int cols, int lines);
    void interface(CUU_Interface &interface);
    void brightnessBoost() { hasBrightnessBoost = true; }
    void cu20045_uw4j() {  is_cu20045_uw4j = true; };
    
    void bcVFD() { bc_vfd = true; }
    void japaneseFont() { bc_font = true; CUU_home(); }
    void europeanFont() { bc_font = false; CUU_home(); }
    
	Noritake_VFD_CUU () {
        display = true;
		cursor = false;
		blink = false;
		rightToLeft = false;
		autoscroll = false;
        hasBrightnessBoost = false;
        is_cu20045_uw4j = false;
	}
	
    int CUU_init();
	void CUU_clearScreen();
	void CUU_home();
	void CUU_setCursor(uint8_t pos);
	void CUU_setCursor(uint8_t col, uint8_t line);
	void CUU_createChar(uint8_t num, uint8_t *data);
	void CUU_readChar(uint8_t *data, uint8_t num);
	
	void CUU_command(uint8_t data);
	void CUU_writeData(uint8_t data);
	uint8_t CUU_readData();
	uint8_t CUU_readCommand();
	uint8_t CUU_readAddress();
	uint8_t CUU_readRAM();
	uint8_t CUU_readBusy();
	
	void print(char data);
	void print(const char *str);
	void print(const uint8_t *buffer, size_t size);
	void print(int number, int base);
	void print(unsigned int number, int base);
	void print(long number, int base);
	void print(unsigned long number, int base);
	void println(char data);
	void println(const char *str);
	void println(const uint8_t *buffer, size_t size);
	void println(int number, int base);
	void println(unsigned int number, int base);
	void println(long number, int base);
	void println(unsigned long number, int base);
	
	void CUU_displayOn();
	void CUU_displayOff();
	void CUU_cursorOn();
	void CUU_cursorOff();
	void CUU_blinkOn();
	void CUU_blinkOff();
	void CUU_scrollDisplayLeft();
	void CUU_scrollDisplayRight();
	void CUU_leftToRight();
	void CUU_rightToLeft();
	void CUU_autoscroll();
	void CUU_noAutoscroll();
	void CUU_brightness(int brightness);
};
