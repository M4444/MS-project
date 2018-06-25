#include "my_tim.h"
#include "main.h"
//#include "my_temp_sens.h"

TIM_HandleTypeDef TIM10_HandleStruct;
TIM_HandleTypeDef TIM11_HandleStruct;
TIM_HandleTypeDef TIM13_HandleStruct;

void MY_TIM_Init(void){
    /*Initialize TIM 10*/
    __TIM10_CLK_ENABLE();
    __GPIOF_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct_F;

    TIM10_HandleStruct.Instance = TIM10;
    TIM10_HandleStruct.Init.Prescaler = 1000-1;
    TIM10_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
    TIM10_HandleStruct.Init.Period = 64-1;
    TIM10_HandleStruct.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    TIM10_HandleStruct.Init.RepetitionCounter = 0;

    HAL_TIM_Base_Init(&TIM10_HandleStruct);

    /*Configure GPIO pin : TIM10_Pin */
    GPIO_InitStruct_F.Pin = TIM10_Pin;
    GPIO_InitStruct_F.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct_F.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(TIM10_GPIO_Port, &GPIO_InitStruct_F);
    
    HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

    HAL_TIM_Base_Start_IT(&TIM10_HandleStruct);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    if (htim->Instance == TIM10){
        TIM10_IncTick();
    }
}
void TIM1_UP_TIM10_IRQHandler(void){
    HAL_TIM_IRQHandler(&TIM10_HandleStruct);
}
/************************************/
static __IO uint32_t tim10Tick;
static __IO uint32_t tick_1s = 0;
static __IO uint32_t tick_100ms = 0;

void TIM10_IncTick(void)
{
    tim10Tick++;
    tick_1s++;
    tick_100ms++;
    if(tick_1s>=1000){
        every_1s();
        tick_1s = 0;
    }
    if(tick_100ms>=100){
        every_100ms();
        tick_100ms = 0;
    }
}

uint32_t TIM10_GetTick(void)
{
    return tim10Tick;
}

void every_1s(void){
    storeAndReadTemp(&ts);
    if (Tprint == 1) perTprint();
}

void every_100ms(void){
    measureJS(&j);
}
