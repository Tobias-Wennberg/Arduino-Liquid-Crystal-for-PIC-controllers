#include "lq.h"

#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

char _displayfunction = 0x00;
char _displaycontrol = 0x00;
char _displaymode = 0x00;
char _numlines = 0;
char _row_offsets[3];

void setRowOffsets(int row0, int row1, int row2, int row3);
void clear();
void home();
void setCursor(char col, char row);
void noDisplay();
void display();
void noCursor();
void cursor();
void noBlink();
void blink();
void scrollDisplayLeft(void);
void scrollDisplayRight(void);
void leftToRight(void);
void rightToLeft(void);
void autoscroll(void);
void noAutoscroll(void);
void createChar(char location, char charmap[]);
void command(char value);
void write(char value);
void send(char value, bool mode);
void pulseEnable(void);
void write4bits(char value);
void begin(uint8_t cols, uint8_t lines);
void init();

void setRowOffsets(int row0, int row1, int row2, int row3)
{
        _row_offsets[0] = row0;
        _row_offsets[1] = row1;
        _row_offsets[2] = row2;
        _row_offsets[3] = row3;
}

void clear()
{
        command(LCD_CLEARDISPLAY);      // clear display, set cursor position to zero
        __delay_us(2000);       // this command takes a long time!
}

void home()
{
        command(LCD_RETURNHOME);        // set cursor position to zero
        __delay_us(2000);       // this command takes a long time!
}

void setCursor(char col, char row)
{
        const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
        if ( row >= max_lines ) {
                row = max_lines - 1;            // we count rows starting w/ 0
        }
        if ( row >= _numlines ) {
                row = _numlines - 1;            // we count rows starting w/ 0
        }

        command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

// Turn the display on/off (quickly)
void noDisplay() {
        _displaycontrol &= ~LCD_DISPLAYON;
        command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void display() {
        _displaycontrol |= LCD_DISPLAYON;
        command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void noCursor() {
        _displaycontrol &= ~LCD_CURSORON;
        command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void cursor() {
        _displaycontrol |= LCD_CURSORON;
        command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void noBlink() {
        _displaycontrol &= ~LCD_BLINKON;
        command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void blink() {
        _displaycontrol |= LCD_BLINKON;
        command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void scrollDisplayLeft(void) {
        command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void scrollDisplayRight(void) {
        command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void leftToRight(void) {
        _displaymode |= LCD_ENTRYLEFT;
        command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void rightToLeft(void) {
        _displaymode &= ~LCD_ENTRYLEFT;
        command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void autoscroll(void) {
        _displaymode |= LCD_ENTRYSHIFTINCREMENT;
        command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void noAutoscroll(void) {
        _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
        command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void createChar(char location, char charmap[]) {
        location &= 0x7; // we only have 8 locations 0-7
        command(LCD_SETCGRAMADDR | (location << 3));
        for (int i=0; i<8; i++) {
                write(charmap[i]);
        }
}

/*********** mid level commands, for sending data/cmds */

void command(char value) {
        send(value, 0);
}

void write(char value) {
        send(value, 1);
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void send(char value, bool mode) {
        RS = mode;
        write4bits(value>>4);
        write4bits(value);
}

void pulseEnable(void) {
        EN = 0;
        __delay_us(1);
        EN = 1;
        __delay_us(1);          // enable pulse must be >450 ns
        EN = 0;
        __delay_us(100);         // commands need >37 us to settle
}

void write4bits(char x) {
    D4 = (bool)(x&0x1);
    D5 = (bool)(x&0x2);
    D6 = (bool)(x&0x4);
    D7 = (bool)(x&0x8);
    pulseEnable();
}
void begin(uint8_t cols, uint8_t lines) {
        _displayfunction |= LCD_2LINE;
        _numlines = 2;

        setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);

        // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
        // according to datasheet, we need at least 40 ms after power rises above 2.7 V
        // before sending commands. Arduino can turn on way before 4.5 V so we'll wait 50
        __delay_us(50000);
        // Now we pull both RS and R/W low to begin commands
        RS = 0;
        EN = 1;

        // this is according to the Hitachi HD44780 datasheet
        // figure 24, pg 46

        // we start in 8bit mode, try to set 4 bit mode
        write4bits(0x03);
        __delay_us(4500); // wait min 4.1ms

        // second try
        write4bits(0x03);
        __delay_us(4500); // wait min 4.1ms

        // third go!
        write4bits(0x03);
        __delay_us(150);

        // finally, set to 4-bit interface
        write4bits(0x02);

        // finally, set # lines, font size, etc.
        command(LCD_FUNCTIONSET | _displayfunction);

        // turn the display on with no cursor or blinking default
        _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
        display();

        // clear it off
        clear();

        // Initialize to default text direction (for romance languages)
        _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
        // set the entry mode
        command(LCD_ENTRYMODESET | _displaymode);
}
void init()
{
        _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

        begin(16, 1);
}
