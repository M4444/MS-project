#ifndef _my_display_h_
#define _my_display_h_

#include <stm32f4xx_hal.h>
#include "my_temp_sens.h"
#include "my_tim.h"

/* my constants */
/*flags for function set*/
//#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
//#define LCD_1LINE 0x00
//#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

/*commands*/
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

/*flags for display on/off control*/
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
//#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

/*flags for display entry mode*/
//#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

/* my structures */
typedef struct display
{
  uint16_t _rs_pin; // LOW: command.  HIGH: character.
  uint16_t _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
  uint16_t _enable_pin; // activated by a HIGH pulse.
  uint16_t _data_pins[8];

  GPIO_TypeDef * _rs_GPIO_port;
  GPIO_TypeDef * _rw_GPIO_port;
  GPIO_TypeDef * _enable_GPIO_port;
  GPIO_TypeDef * _data_GPIO_ports[8];

  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _cursor_address;
  uint8_t _ddram_mode;
  uint8_t _incNdec;

  uint8_t _numlines,_currline;
} Display;

/* my functions */
void MY_DISPLAY_GPIO_Init();
void MY_DISPLAY_START(Display *d);
void MY_DISPLAY_CONNECT(Display *d);
void clear(Display *d);
void scrollDisplayLeft(Display *d);
void scrollDisplayRight(Display *d);

void command(Display *d, uint8_t value);
void home(Display *d);
void write(Display *d, uint8_t value);
void send(Display *d, uint8_t value, GPIO_PinState PinState);
void write4bits(Display *d, uint8_t value);
void pulseEnable(Display *d);
void setCursor(Display *d, uint8_t col, uint8_t row);
void createChar(Display *d, uint8_t location, uint8_t charmap[]);
void printTemp(Display *d, float temp, char scale, uint8_t digits);
void printStr(Display *d, const char[]);
void printFloat(Display *d, double number, uint8_t digits);
void printNumber(Display *d, unsigned long n);
void printHex(Display *d, uint8_t num);
void printSigByte(Display *d, uint8_t byte);

uint8_t enterD(Display *d);
void nextD(Display *d);

#endif /* _my_display_h_ */
