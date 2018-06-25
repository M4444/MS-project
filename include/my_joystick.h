#ifndef _my_joystick_h_
#define _my_joystick_h_

#include <stm32f4xx_hal.h>

/* my constants */
#define JSaddress 0x40
#define TST_BTN_Pin       GPIO_PIN_8
#define TST_BTN_GPIO_Port GPIOE

#define I2CSPEED 100

#define _Control1 0x0F    // Control register 1
#define _Control2 0x2E    // Control register 2
#define _X 0x10
#define _Y 0x11
#define _T_CTRL 0x2D	  // Scale input to fit to the 8 bit result register

#define _READ_BIT 1
#define _WRITE_BIT 0

/* my structures */
typedef struct joystick
{
    uint8_t _address;

    uint16_t _rst_pin;
    uint16_t _tst_pin;
    uint16_t _int_pin;
    uint16_t _sda_pin;
    uint16_t _scl_pin;

    GPIO_TypeDef * _rst_GPIO_port;
    GPIO_TypeDef * _tst_GPIO_port;
    GPIO_TypeDef * _int_GPIO_port;
    GPIO_TypeDef * _sda_GPIO_port;
    GPIO_TypeDef * _scl_GPIO_port;

    uint8_t transBuff;
    uint16_t transSize;
    uint16_t transCnt;

} Joystick;

/* my functions */
void MY_JOYSTICK_GPIO_Init();
HAL_StatusTypeDef MY_JOYSTICK_START(Joystick *j);
HAL_StatusTypeDef MY_JOYSTICK_CONNECT(Joystick *j);

HAL_StatusTypeDef MY_I2C_Init(I2C_HandleTypeDef *hi2c);

void measureJS(Joystick *j);
extern void mainJSdown();
extern void mainJSup();
extern void mainJSleft();
extern void mainJSright();

void EXTI9_5_IRQHandler(void);
char JS_button_is_pressed(void);
GPIO_PinState read_JS_button(void);

__INLINE void I2C_delay(void);
GPIO_PinState read_SDA(Joystick *j); // Set SDA as input and return current level of line, 0 or 1
void set_SCL(Joystick *j);   // Actively drive SCL signal high
void clear_SCL(Joystick *j); // Actively drive SCL signal low
void set_SDA(Joystick *j);   // Actively drive SDA signal high
void clear_SDA(Joystick *j); // Actively drive SDA signal low
void arbitration_lost(void);

void i2c_start_cond(Joystick *j);
void i2c_stop_cond(Joystick *j);
void i2c_write_bit(Joystick *j, uint8_t bit);
GPIO_PinState i2c_read_bit(Joystick *j);
GPIO_PinState i2c_write_byte(Joystick *j, uint8_t send_start, 
                             uint8_t send_stop, uint8_t byte);
uint8_t i2c_read_byte(Joystick *j, uint8_t nack, uint8_t send_stop);

uint8_t readCurReg(Joystick *j);
uint8_t readSpcReg(Joystick *j, uint8_t regNum);
uint8_t writeSpcReg(Joystick *j, uint8_t regNum, uint8_t value);

#endif /* _my_joystick_h_ */
