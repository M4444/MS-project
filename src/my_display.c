#include "my_display.h"

void MY_DISPLAY_GPIO_Init()
{
    GPIO_InitTypeDef GPIO_InitStruct_B;
    GPIO_InitTypeDef GPIO_InitStruct_C;
    GPIO_InitTypeDef GPIO_InitStruct_D;
    /* GPIO Ports Clock Enable */
    __GPIOD_CLK_ENABLE(); /*For pins PD10/11/13/14: Display-RS,E,DB6,DB7 */
    __GPIOC_CLK_ENABLE(); /*For pin PC7: Display-DB4*/
    __GPIOB_CLK_ENABLE(); /*For pin PB7: Display-DB5*/
    /*Configure GPIO pin : PB7 */
    GPIO_InitStruct_B.Pin = GPIO_PIN_7;
    GPIO_InitStruct_B.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct_B.Pull = GPIO_NOPULL;
    GPIO_InitStruct_B.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_B);
    /*Configure GPIO pin : PC7 */
    GPIO_InitStruct_C.Pin = GPIO_PIN_7;
    GPIO_InitStruct_C.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct_C.Pull = GPIO_NOPULL;
    GPIO_InitStruct_C.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct_C);
    /*Configure GPIO pins : PD10 PD11 PD13 PD14 */
    GPIO_InitStruct_D.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_13 | GPIO_PIN_14;
    GPIO_InitStruct_D.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct_D.Pull = GPIO_NOPULL;
    GPIO_InitStruct_D.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct_D);    
}

void MY_DISPLAY_START(Display *d)
{
    /*Assign pin numbers*/
    d->_rs_pin = GPIO_PIN_10;        /*PD 10*/
    d->_rw_pin = 255; /*GND (always write)*/
    d->_enable_pin = GPIO_PIN_11;   /*PD 11*/
    d->_data_pins[4] = GPIO_PIN_7;  /*PC 7*/
    d->_data_pins[5] = GPIO_PIN_7;  /*PB 7*/
    d->_data_pins[6] = GPIO_PIN_13; /*PD 13*/
    d->_data_pins[7] = GPIO_PIN_14; /*PD 14*/
    
    /*Assign peripherals*/
    d->_rs_GPIO_port = GPIOD;       /*P D 10*/
    d->_enable_GPIO_port = GPIOD;   /*P D 11*/
    d->_data_GPIO_ports[4] = GPIOC; /*P C 7*/
    d->_data_GPIO_ports[5] = GPIOB; /*P B 7*/
    d->_data_GPIO_ports[6] = GPIOD; /*P D 13*/
    d->_data_GPIO_ports[7] = GPIOD; /*P D 14*/ 
  
    /* 4 bit mode */
    d->_displayfunction = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;
  
    MY_DISPLAY_CONNECT(d);
}

void MY_DISPLAY_CONNECT(Display *d)
{
    // SEE PAGE 46 OF "HD44780U (LCD-II)" DATASHEET FOR INITIALIZATION SPECIFICATION!
    // According to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. We'll wait 50.

    HAL_Delay(50*1000);

    // Now we pull both RS and R/W low to begin commands
    HAL_GPIO_WritePin(d->_rs_GPIO_port, d->_rs_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(d->_enable_GPIO_port, d->_enable_pin, GPIO_PIN_RESET);
    if (d->_rw_pin != 255) { 
      HAL_GPIO_WritePin(d->_rw_GPIO_port, d->_rw_pin, GPIO_PIN_RESET);
    }
  
    //put the LCD into 4 bit  mode
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    write4bits(d, 0x03);
    HAL_Delay(5*1000); // wait min 4.1ms

    // second try
    write4bits(d, 0x03);
    HAL_Delay(5*1000); // wait min 4.1ms
    
    // third try
    write4bits(d, 0x03); 
    HAL_Delay(1*1000); // wait min 100 us

    // finally, set to 8-bit interface
    write4bits(d, 0x02); 

    // finally, set # lines, font size, etc.
    command(d, LCD_FUNCTIONSET | d->_displayfunction);  

    // turn the display on with a blinking cursor by default
    d->_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
    command(d, LCD_DISPLAYCONTROL | d->_displaycontrol);

    // clear it off
    clear(d);

    d->_ddram_mode = 1;

    // Initialize to default text direction (for romance languages)
    d->_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    command(d, LCD_ENTRYMODESET | d->_displaymode);

    /* custom characters */
    uint8_t charmap0[8] = {32-1, 16+1, 4, 8+4+2, 8+4+2, 4, 16+1, 32-1};
    createChar(d, 0, charmap0);
    uint8_t charmap1[8] = {32-1, 16+1, 0, 0, 0, 0, 16+1, 32-1};
    createChar(d, 1, charmap1); 
    
    uint8_t charmap2[8] = {32-1, 32-1, 32-1, 32-1, 0, 0, 0, 0};
    createChar(d, 2, charmap2);
    uint8_t charmap3[8] = {0, 0, 0, 0, 32-1, 32-1, 32-1, 32-1};
    createChar(d, 3, charmap3);

    /* arrows for alarm menu*/
    uint8_t charmap4[8] = {0, 4, 4, 8+4+2, 8+4+2, 32-1, 32-1, 0};
    createChar(d, 4, charmap4);
    uint8_t charmap5[8] = {0, 4, 4, 8+2, 8+2, 16+1, 32-1, 0};
    createChar(d, 5, charmap5);
    uint8_t charmap6[8] = {0, 32-1, 16+1, 8+2, 8+2, 4, 4, 0};
    createChar(d, 6, charmap6);

}

void printSigByte(Display *d, uint8_t byte)
{
    if (byte & 0x80){
        byte = ~(byte-1);
    }
    printNumber(d, byte);
}

void printHex(Display *d, uint8_t num)
{
    uint8_t numH, numL;

    write(d, '0');
    write(d, 'x');

    numH = (num & 0xF0) >> 4;
    if (numH > 9) numH = numH-10 + 'a';
    else numH += '0';
    write(d, numH);
     
    numL = num & 0x0F;
    if (numL > 9) numL = numL-10 + 'a';
    else numL += '0';
    write(d, numL);
}

void printStr(Display *d, const char str[])
{
    while (*str)
        write(d, *str++);
}

void printFloat(Display *d, double number, uint8_t digits) 
{ 
  if (digits > 1) digits = 1;
  // Handle negative numbers
  if (number < 0.0)
  {
     write(d, '-');
     number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  uint8_t i;
  for (i=0; i<digits; ++i)
    rounding /= 10.0;
  
  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;
  printNumber(d, int_part);

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0)
    write(d, '.'); 

  // Extract digits from the remainder one at a time
  while (digits-- > 0)
  {
    remainder *= 10.0;
    int toPrint = (int) remainder;
    printNumber(d, (unsigned long)toPrint);
    remainder -= toPrint; 
  } 
}

void printNumber(Display *d, unsigned long n)
{
    unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars. 
    unsigned long i = 0;

    if (n == 0) {
      write(d, '0');
      return;
    } 

    while (n > 0) {
      buf[i++] = n % 10;
      n /= 10;
    }

    for (; i > 0; i--)
      write(d, (char) (buf[i - 1] < 10 ?
        '0' + buf[i - 1] :
        'A' + buf[i - 1] - 10));
}

void printTemp(Display *d, float temp, char scale, uint8_t digits){
    switch(scale)
    {
        default: case'c': case'C':
            break;
        case'f': case'F':
            temp = temp * 1.8 + 32;
            break;
        case'k': case'K':
            temp += 273.15; 
            break;
    }

    if (digits > 1) digits = 1;
    printFloat(d, temp, digits);
    write(d, ' ');
    switch(scale)
    {
        default: case'c': case'C': 
            write(d, 0xB2); 
            write(d, 'C'); 
            break;
        case'f': case'F': 
            write(d, 0xB2); 
            write(d, 'F'); 
            break;
        case'k': case'K': 
            write(d, 'K'); 
            break;
    }
}

void clear(Display *d)
{
    command(d, LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
    HAL_Delay(1600);
}

void home(Display *d)
{
    command(d, LCD_RETURNHOME);  // set cursor position to zero
    HAL_Delay(2*1000);
}

void setCursor(Display *d, uint8_t col, uint8_t row)
{
    int row_offsets[] = { 0x00, 0x40 };
  
    command(d, LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// These commands scroll the display without changing the RAM
void scrollDisplayLeft(Display *d) {
    command(d, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void scrollDisplayRight(Display *d) {
    command(d, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void createChar(Display *d, uint8_t location, uint8_t charmap[]) {
    location &= 0x7; // we only have 8 locations 0-7
    command(d, LCD_SETCGRAMADDR | (location << 3));
    int i;
    for (i=0; i<8; i++) {
      write(d, charmap[i]);
    }
    command(d, LCD_SETDDRAMADDR | d->_cursor_address);
}
/*********** mid level commands, for sending data/cmds */
inline void command(Display *d, uint8_t value) {
    send(d, value, GPIO_PIN_RESET);
    if (value == LCD_CLEARDISPLAY) d->_cursor_address = 0;
    if (value == LCD_RETURNHOME) d->_cursor_address = 0;
    if ((value & (LCD_ENTRYMODESET | LCD_ENTRYSHIFTDECREMENT)) != 0) d->_incNdec = 1;
    if ((value & (LCD_ENTRYMODESET | LCD_ENTRYSHIFTDECREMENT)) != 0) d->_incNdec = 0;
    if ((value & LCD_SETCGRAMADDR) != 0) d->_ddram_mode = 0;
    if ((value & LCD_SETDDRAMADDR) != 0) { 
        d->_ddram_mode = 1; 
        d->_cursor_address = value - LCD_SETDDRAMADDR;
    }
}

inline void write(Display *d, uint8_t value) {
    send(d, value, GPIO_PIN_SET);
    if (d->_ddram_mode) (d->_incNdec == 0)? d->_cursor_address++ : d->_cursor_address--;
}
/************ low level data pushing commands **********/
// write either command or data
void send(Display *d, uint8_t value, GPIO_PinState PinState) {
    HAL_GPIO_WritePin(d->_rs_GPIO_port, d->_rs_pin, PinState);

    // if there is a RW pin indicated, set it low to Write
    if (d->_rw_pin != 255) { 
        HAL_GPIO_WritePin(d->_rw_GPIO_port, d->_rw_pin, GPIO_PIN_RESET);
    }
  
    write4bits(d, value>>4);
    write4bits(d, value);
}

void write4bits(Display *d, uint8_t value) {
    int i;
    for (i = 0; i < 4; i++) {
        HAL_GPIO_WritePin(d->_data_GPIO_ports[i+4], d->_data_pins[i+4], (value >> i) & 0x01);
    }

    pulseEnable(d);
}

void pulseEnable(Display *d) {
    HAL_GPIO_WritePin(d->_enable_GPIO_port, d->_enable_pin, GPIO_PIN_RESET);
    HAL_Delay(1*1000);
    HAL_GPIO_WritePin(d->_enable_GPIO_port, d->_enable_pin, GPIO_PIN_SET);
     HAL_Delay(1*1000);    // enable pulse must be >450ns
    HAL_GPIO_WritePin(d->_enable_GPIO_port, d->_enable_pin, GPIO_PIN_RESET);
     HAL_Delay(1*1000);   // commands need > 37us to settle
}
