#ifndef _my_tim_h_
#define _my_tim_h_
#include <stm32f4xx_hal.h>

/* my constants */
#define TIM10_Pin	GPIO_PIN_6
#define TIM10_GPIO_Port GPIOF

#define TIM11_Pin	GPIO_PIN_7
#define TIM11_GPIO_Port GPIOF

#define TIM13_Pin	GPIO_PIN_8
#define TIM13_GPIO_Port GPIOF
/* my structures */

/* my functions */
void MY_TIM_Init(void);
void TIM10_IncTick(void);
uint32_t TIM10_GetTick(void);

static __INLINE void TIM10_Delay(__IO uint32_t Delay)
{
  uint32_t tickstart = 0;
  tickstart = TIM10_GetTick();
  while((TIM10_GetTick() - tickstart) < Delay)
  {
  }
}

void every_1s(void);
void every_100ms(void);

#endif /* _my_tim_h_ */
