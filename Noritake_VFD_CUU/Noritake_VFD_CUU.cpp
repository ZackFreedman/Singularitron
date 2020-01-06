#include <avr/io.h>
#include <stddef.h>

#include <CUU_Interface.h>
#include <Noritake_VFD_CUU.h>
#include <util/delay.h>

#define MIN_DELAY 1

void Noritake_VFD_CUU::begin(int cols, int lines) {
    this->cols = cols;
    this->lines = lines;
}

void Noritake_VFD_CUU::interface(CUU_Interface &interface) {
    this->io = &interface;
}

// ----------------------------------------------------------------
// Initialize the VFD module. This must be called before any other
// methods.
int
Noritake_VFD_CUU::CUU_init() {
    io->init();
    CUU_brightness(100);
	CUU_displayOn();
	CUU_leftToRight();
	CUU_clearScreen();
	return 0;
}

//----------------------------------------------------------------
//void CUU_command(uint8_t data)
//Send command to the VFD controller.
//data: command to send
void
Noritake_VFD_CUU::CUU_command(uint8_t data) {
	io->write(data, false);
	_delay_us(MIN_DELAY);
}

//----------------------------------------------------------------
//void CUU_writeData(uint8_t data)
//Write data to the VFD controller.
//data: byte to send
void
Noritake_VFD_CUU::CUU_writeData(uint8_t data) {
	io->write(data, true);
	_delay_us(MIN_DELAY);
}

//----------------------------------------------------------------
//uint8_t CUU_readData();
//Read command from the VFD controller.
uint8_t
Noritake_VFD_CUU::CUU_readCommand() {
	uint8_t data = io->read(false);
	_delay_us(MIN_DELAY);
	return data;
}

//----------------------------------------------------------------
//uint8_t CUU_readData();
//Read data from the VFD controller. The cursor advances in the
//same direction as if a write had occurred. The display does not
//shift even if autoscroll is enabled.
uint8_t
Noritake_VFD_CUU::CUU_readData() {
	uint8_t data = io->read(true);
	_delay_us(MIN_DELAY);
	return data;
}

//----------------------------------------------------------------
//uint8_t CUU_readAddress();
//Return the address of either DDRAM (cursor address) or CGRAM.
//Unless you have used CUU_command() to set the CGRAM address and
//you have not done an operation to set the cursor position since,
//then the DDRAM address will be returned. When a CGRAM address
//is read bit6 (0x40) will be set.
uint8_t
Noritake_VFD_CUU::CUU_readAddress() {
	return CUU_readCommand() & ~0x80;
}

//----------------------------------------------------------------
//uint8_t CUU_readRAM()
//Return the data from either DDRAM (cursor address) or CGRAM.
//Unless you have used CUU_command() to set the CGRAM address and
//you have not done an operation to set the cursor position since,
//then the data from DDRAM will be returned. The cursor advances
//in the same direction as if a write had occurred. The display
// does not shift even if autoscroll is enabled.
uint8_t
Noritake_VFD_CUU::CUU_readRAM() {
	return CUU_readData();
}

void
Noritake_VFD_CUU::nextLine() {
	uint8_t x = CUU_readAddress();
    if (is_cu20045_uw4j) {
    	if (x < 0x20)
    		x = 0x20;
    	else if (x<0x40)
    		x = 0x40;
    	else if (x<0x60)
    		x = 0x60;
    	else if (x<0x80)
    		x = 0x00;
    	CUU_setCursor(x);
    } else {
		if (x < 0x14) // line 0
			x = 0x40;
		else if (lines > 2 && 0x40<=x && x<0x54) // line 1
			x = 0x14;
		else if (lines > 3 && 0x14<=x && x<=0x28) // line 2
			x = 0x54;
		else
			x = 0x00;
		CUU_setCursor(x);
    }
	_delay_us(MIN_DELAY);
}

//----------------------------------------------------------------
//Print an arbitrary character (including 0x00).
//The effects of printing past the end of the line depends on the model.
//data:	character to print
void
Noritake_VFD_CUU::print(char data) {
	CUU_writeData(data);
}

//----------------------------------------------------------------
//Print a NULL-terminated string.
//The effects of printing past the end of the line depends on the model.
//str: a null-terminated string
void
Noritake_VFD_CUU::print(const char *str) {
	while (*str)
		print(*str++);
}

//----------------------------------------------------------------
//Print arbitrary characters (including 0x00).
//The effects of printing past the end of the line depends on the model.
//buffer:	characters to print
//size:	number of characters to print
void
Noritake_VFD_CUU::print(const uint8_t *buffer, size_t size) {
	while (size--)
		print((char) *buffer++);
}

// ----------------------------------------------------------------
//Print an arbitrary character (including 0x00) and go to the next line.
//If this was printed on the last line, this will return to the first line.
//The effects of printing past the end of the line depends on the model.
//data:	character to print
void
Noritake_VFD_CUU::println(char data) {
	print(data);
	nextLine();
}

// ----------------------------------------------------------------
//Print a NULL-terminated string and go to the next line.
//If this was printed on the last line, this will return to the first line.
//The effects of printing past the end of the line depends on the model.
//str: a null-terminated string
void
Noritake_VFD_CUU::println(const char *str) {
	print(str);
	nextLine();
}

// ----------------------------------------------------------------
//Print arbitrary characters (including 0x00) and go to the next line.
//If this was printed on the last line, this will return to the first line.
//The effects of printing past the end of the line depends on the model.
//buffer:	characters to print
//size:	number of characters to print
void
Noritake_VFD_CUU::println(const uint8_t *buffer, size_t size) {
	print(buffer, size);
	nextLine();
}

void
Noritake_VFD_CUU::printNumber(unsigned long number, int base) {
	if (number/base)
		printNumber(number/base, base);
	print("0123456789ABCDEF"[number%base]);
}

void
Noritake_VFD_CUU::printNumber(long number, int base) {
	if (number/base)
		printNumber(number/base, base);
	print("0123456789ABCDEF"[number%base]);
}

//----------------------------------------------------------------
//Print a number.
//number:	number to print
//base:	base to print in (2-16)
void
Noritake_VFD_CUU::print(int number, int base) {
	if (number < 0) {
		print('-');
		number = -number;
		printNumber((long)number, base);
	} else
		printNumber((long)number, base);
}

//----------------------------------------------------------------
//Print a number.
//number:	number to print
//base:	base to print in (2-16)
void
Noritake_VFD_CUU::print(unsigned int number, int base) {
	printNumber((unsigned long)number, base);
}

//----------------------------------------------------------------
//Print a number.
//number:	number to print
//base:	base to print in (2-16)
void
Noritake_VFD_CUU::print(long number, int base) {
	if (number < 0) {
		print('-');
		number = -number;
		printNumber((long)number, base);
	} else
		printNumber((long)number, base);
}

//----------------------------------------------------------------
//Print a number.
//number:	number to print
//base:	base to print in (2-16)
void
Noritake_VFD_CUU::print(unsigned long number, int base) {
	printNumber(number, base);
}

// ----------------------------------------------------------------
//Print a number and go to the next line.
//If this was printed on the last line, this will return to the first line.
//The effects of printing past the end of the line depends on the model.
//number:	number to print
//base:	base to print in (2-16)
void
Noritake_VFD_CUU::println(int number, int base) {
	print((long)number, base);
	nextLine();
}

//----------------------------------------------------------------
//Print a number and go to the next line.
//If this was printed on the last line, this will return to the first line.
//The effects of printing past the end of the line depends on the model.
//number:	number to print
//base:	base to print in (2-16)
void
Noritake_VFD_CUU::println(unsigned int number, int base) {
	print((unsigned long)number, base);
	nextLine();
}

//----------------------------------------------------------------
//Print a number and go to the next line.
//If this was printed on the last line, this will return to the first line.
//The effects of printing past the end of the line depends on the model.
//number:	number to print
//base:	base to print in (2-16)
void
Noritake_VFD_CUU::println(long number, int base) {
	print(number, base);
	nextLine();
}

//----------------------------------------------------------------
//Print a number and go to the next line.
//If this was printed on the last line, this will return to the first line.
//The effects of printing past the end of the line depends on the model.
//number:	number to print
//base:	base to print in (2-16)
void
Noritake_VFD_CUU::println(unsigned long number, int base) {
	print(number, base);
	nextLine();
}

// ----------------------------------------------------------------
// Clear the screen. Fill DD RAM with blanks (0x20). Move the
// cursor home. Reset display shift to no shift. Set the entry mode to
// left-to-right.
void
Noritake_VFD_CUU::CUU_clearScreen() {
	CUU_command(0x01);
	_delay_ms(5);
}

// ----------------------------------------------------------------
// Moves the cursor back to the home position (top-left). Display
// shift is reset to no shift.
void
Noritake_VFD_CUU::CUU_home() {
	CUU_command(0x02 + bc_font);
	_delay_us(MIN_DELAY);
}

// ----------------------------------------------------------------
//Move the cursor to the given linear position. You may use this
//function to move the cursor to areas that are not part of visible
//lines for models that have them. If you attempt to set it to a
//position that is not within one of these ranges, the cursor
//will be set at the address of beginning of the next line. If
//the position is past the last line, then the cursor will be
//set to 0x00. The most highest bit of the position is ignored.
//e.g 0x92 is the same as 0x80+0x12 and 0x80, the highest bit, is ignored
//and the cursor will be set to 0x12.
//pos:	0 through 0x80
//	LINE	POS ON 2-LINE   POS ON 4-LINE	POS ON UW4J
//	0       0x00 - 0x27     0x00 - 0x13     0x00 - 0x13
//	1       0x40 - 0x67     0x40 - 0x53     0x20 - 0x33
//	2                       0x14 - 0x27     0x40 - 0x53
//	3                       0x54 - 0x67     0x60 - 0x73
void
Noritake_VFD_CUU::CUU_setCursor(uint8_t pos) {
	CUU_command(0x80|pos);
	_delay_us(500);
}

// ----------------------------------------------------------------
//Move the cursor to the given position. You cannot use this function
//to move the cursor to areas that are not part of visible lines.
//col: column to move to
//line: line to move to
void
Noritake_VFD_CUU::CUU_setCursor(uint8_t col, uint8_t line) {
	if (col < cols && line < lines) {
        if (is_cu20045_uw4j)
    		switch (line) {
    		case 0:
    			CUU_setCursor(0x00 + col);
    			break;
    		case 1:
    			CUU_setCursor(0x20 + col);
    			break;
    		case 2:
    			CUU_setCursor(0x40 + col);
    			break;
    		case 3:
    			CUU_setCursor(0x60 + col);
    			break;
    		}
        else
    		switch (line) {
    		case 0:
    			CUU_setCursor(0x00 + col);
    			break;
    		case 1:
    			CUU_setCursor(0x40 + col);
    			break;
    		case 2:
    			CUU_setCursor(0x14 + col);
    			break;
    		case 3:
    			CUU_setCursor(0x54 + col);
    			break;
    		}
	}
}

void
Noritake_VFD_CUU::setDisplay() {
	CUU_command(8+display*4+cursor*2+blink);
	_delay_us(MIN_DELAY);
}

// ----------------------------------------------------------------
// Turn the display on.
void
Noritake_VFD_CUU::CUU_displayOn() {
	display = true;
	setDisplay();
}

// ----------------------------------------------------------------
// Turn the display off. This sends the module into a low power
// consumption mode. See the manual for your module for details.
void
Noritake_VFD_CUU::CUU_displayOff() {
	display = false;
	setDisplay();
}

//----------------------------------------------------------------
//Turn the underline cursor on. The cursor will not be visible
//on the following models: CU20045-UW4J, CU20045-UW5J, CU20045-UW5A,
//CU20045-UW7J, CU20049-UW2J, CU20049-UW2A
void
Noritake_VFD_CUU::CUU_cursorOn() {
	cursor = true;
	setDisplay();
}

// ----------------------------------------------------------------
// Turn the underline cursor off.
void
Noritake_VFD_CUU::CUU_cursorOff() {
	cursor = false;
	setDisplay();
}

// ----------------------------------------------------------------
// Enable the full-cell block (blinking) cursor.
void
Noritake_VFD_CUU::CUU_blinkOn() {
	blink = true;
	setDisplay();
}

// ----------------------------------------------------------------
// Disable the full-cell block (blinking) cursor.
void
Noritake_VFD_CUU::CUU_blinkOff() {
	blink = false;
	setDisplay();
}

// ----------------------------------------------------------------
// Scroll the display window to the left. The previous leftmost
// character will no longer be displayed and the previous rightmost
// character will now be the second from the right.
void
Noritake_VFD_CUU::CUU_scrollDisplayLeft() {
	CUU_command(0x18);
	_delay_us(MIN_DELAY);
}

// ----------------------------------------------------------------
// Scroll the display window to the right. The previous rightmost
// character will no longer be displayed and the previous leftmost
// character will now be the second.
void
Noritake_VFD_CUU::CUU_scrollDisplayRight() {
	CUU_command(0x1c);
	_delay_us(MIN_DELAY);
}

void
Noritake_VFD_CUU::setDirection() {
	CUU_command(4+2*(!rightToLeft)+autoscroll);
	_delay_us(MIN_DELAY);
}

// ----------------------------------------------------------------
// Set the entry mode to move the cursor to the right after a
// character has been inserted.
void
Noritake_VFD_CUU::CUU_leftToRight() {
	rightToLeft = false;
	setDirection();
}

// ----------------------------------------------------------------
// Set the entry mode to move the cursor to the left after a
// character has been inserted.
void
Noritake_VFD_CUU::CUU_rightToLeft() {
	rightToLeft = true;
	setDirection();
}

// ----------------------------------------------------------------
// Automatically scroll the display whenever a character is printed.
void
Noritake_VFD_CUU::CUU_autoscroll() {
	autoscroll = true;
	setDirection();
}

// ----------------------------------------------------------------
// Do not automatically scroll the display whenever a character is
// printed.
void
Noritake_VFD_CUU::CUU_noAutoscroll() {
	autoscroll = false;
	setDirection();
}

// ----------------------------------------------------------------
// Create a character in CG RAM.
// num:	character number may be 0 through 7.
// data:	bitmap data in HD44780 format.
// 	Each byte represents a line.
// 	The five least significant bits are the pixel values.
// 	The most significant bit is the leftmost.
// 	Seven bytes are used for bitmap data.
// 	The fifth bit of the eighth byte sets the whole row if set.
// 	Other bits in the eighth byte are ignored.
//	The eighth row will not be visible on the following models:
//	CU20045-UW4J, CU20045-UW5J, CU20045-UW5A,
//	CU20045-UW7J, CU20049-UW2J, CU20049-UW2A
void
Noritake_VFD_CUU::CUU_createChar(uint8_t num, uint8_t *bits) {
	if (num >= 8)
		return;
	uint8_t	addr = CUU_readAddress();
	bool oldDir = rightToLeft;
	
	if (rightToLeft)
		CUU_leftToRight();
	
	CUU_command(0x40+num*8);	// set CGRAM address
	for (uint8_t i=0; i<8; i++)
		CUU_writeData(bits[i]);
	
	if (oldDir)
		CUU_rightToLeft();
	CUU_command(0x80|addr);	// restore DDRAM address
	_delay_us(MIN_DELAY);
}

//----------------------------------------------------------------
//void CUU_readChar(uint8_t *data, uint8_t num);
//Read a character (8 bytes) from CGRAM
//num:	character number may be 0 through 7.
//data:	bitmap data in HD44780 format.
//	Each byte represents a line.
//	The five least significant bits are the pixel values.
//	The most significant bit is the leftmost.
//	Seven bytes are used for bitmap data.
//	The fifth bit of the eighth byte sets the whole row if set.
//	Other bits in the eighth byte are ignored.
void
Noritake_VFD_CUU::CUU_readChar(uint8_t *data, uint8_t num) {
	if (num >= 8)
		return;
		
	uint8_t	addr = CUU_readAddress();
	bool oldDir = rightToLeft;
	
	if (rightToLeft)
		CUU_leftToRight();

	CUU_command(0x40+num*8);	// set CGRAM address
	for (uint8_t i = 0; i < 8; i++)
		data[i] = CUU_readData();	

	if (oldDir)
		CUU_rightToLeft();
	CUU_command(0x80|addr);	// restore DDRAM address
	_delay_us(MIN_DELAY);
}

//----------------------------------------------------------------
// uint8_t CUU_readBusy();
//Return the busy flag. 1 indicates the device is busy.
uint8_t
Noritake_VFD_CUU::CUU_readBusy() {
	return (CUU_readCommand() & 0x80) != 0;
}

// ----------------------------------------------------------------
// Set the brightness of the VFD module.
// brightness: brightness value in percent: 25, 50, 75, or 100.
// 	Models with brightness boost (CU-UX models) can accept
//	values: 50, 100, 150, or 200.
//	The following models support brightness boost:
//		CU16025-UX6J
//		CU16025-UX6A
//		CU20025-UX1J
void
Noritake_VFD_CUU::CUU_brightness(int brightness) {
    if (bc_vfd) {
        if (brightness <= 0 || brightness > 100) return;
        CUU_command(0x30 + (10000-brightness*100)/625);
    	_delay_us(MIN_DELAY);
    } else {
        const register int m = hasBrightnessBoost? 2: 1;
        if (brightness <= 0 || brightness > 100 * m) return;
        CUU_command(io->is8bit()? 0x30: 0x20);
    	_delay_us(MIN_DELAY);
    	CUU_writeData((100 * m - brightness) / (25 * m));
    	_delay_us(MIN_DELAY);
    }
}
