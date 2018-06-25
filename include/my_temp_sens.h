#ifndef _my_temp_sens_h_
#define _my_temp_sens_h_

#include <stm32f4xx_hal.h>
#include "my_tim.h"

/* my constants */
#define DS1820_ID (uint8_t) 0x10

/* my structures */
typedef struct temp_sens
{
    uint16_t _dq_pin;

    GPIO_TypeDef * _dq_GPIO_port;

    float temp;

    uint16_t error_flag;

} TempSens;

/* my functions */
void MY_TEMPSENS_GPIO_Init();
void MY_TEMPSENS_Init(TempSens *ts);
uint8_t getTemp(TempSens *ts);
void storeAndReadTemp(TempSens *ts);
void readTemp(TempSens *ts);

uint8_t reset(TempSens *ts);
void skip(TempSens *ts);
void tempConvert(TempSens *ts);
void readSP(TempSens *ts);
void writeTS(TempSens *ts, uint8_t v);
void write_bit(TempSens *ts, uint8_t v);
uint8_t read(TempSens *ts);
GPIO_PinState read_bit(TempSens *ts);

#endif /* _my_temp_sens_h_ */
