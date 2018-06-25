#ifndef __MAIN_H_
#define __MAIN_H_
#include <stm32f4xx_hal.h>

#include "my_display.h"
#include "my_joystick.h"
#include "my_temp_sens.h"

#define interrupts() __enable_irq()	
#define noInterrupts() __disable_irq()

typedef enum {
    LED1 = 0,
    LED2 = 1
} Led_TypeDef;

typedef enum {
    BUTTON1 = 0,
    BUTTON2 = 1
} Button_TypeDef;

extern Display d;
extern Joystick j;
extern TempSens ts;

extern uint8_t Tprint;
extern uint8_t lockBTN;
extern int sp, dp;
extern const char SCALE[3];

extern float minTemp, maxTemp;
extern const float AMOUNT[3];
extern float amount;
extern int addSub;
extern int ap;
extern uint8_t overMaxTemp, underMinTemp;

void GPIO_SetPinAsInput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);  /*Sets pin as input*/
void GPIO_SetPinAsOutput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin); /*sets pin as output*/

void disableInput(void);
void enableInput(void);

void menuEnter(void);

void perTprint(void);

/* Reads the pin state of the given button. */
GPIO_PinState read_the_button(Button_TypeDef button);

/**
 * This function will return de-bounced state of the button.
 * Algorithm is very simple, we just wait some time and then read the appropriate bit.
 */
char button_is_pressed(Button_TypeDef button);

void led_on(Led_TypeDef Led);
void led_off(Led_TypeDef Led);
void led_toggle(Led_TypeDef Led);

#endif /* __MAIN_H_ */
