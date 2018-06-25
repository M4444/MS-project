#include "main.h"
#include "my_menu.h"
#include "my_tim.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MY_MENU_Init();

void _init() {
}

Display d;
Joystick j;
TempSens ts;

/* menu variables*/
uint8_t lockBTN = 0;

int sp = 0, dp = 1;
const char SCALE[3] = {'C', 'F', 'K'};

uint8_t Tprint = 0;

float minTemp = 20, maxTemp = 30;
const float AMOUNT[3] = {0.5, 1, 5};
float amount = 1;
int addSub = 1;
int ap = 1;

uint8_t overMaxTemp = 0, underMinTemp = 0;

int main(void) {    
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    /* Configure the system clock */
    SystemClock_Config();
    /* Initialize all configured peripherals */
    MX_GPIO_Init();

    MY_JOYSTICK_GPIO_Init();
    MY_DISPLAY_GPIO_Init();
    MY_TEMPSENS_GPIO_Init();

    MY_TIM_Init();
    
    /* Start the joystcik */
    MY_JOYSTICK_START(&j);
    /* Start the display */
    MY_DISPLAY_START(&d);
    /* Initialise the temperature sensor*/
    MY_TEMPSENS_Init(&ts);
    readTemp(&ts);

    MY_MENU_Init();

    while (1) {
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    }
}

/*
 * System Clock Configuration
 */

#ifndef FAST_CLOCK
#define FAST_CLOCK 1
#endif

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct;
	#if FAST_CLOCK
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	#endif
    __PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 16;
    #if !FAST_CLOCK
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    #endif
	#if FAST_CLOCK
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 128;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	#endif
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
	#if FAST_CLOCK
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
		|RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
	#endif
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / (100000*10)); /*delay = 1 us*/
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    /* GPIO Ports Clock Enable */
    __GPIOE_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    /*Initialize Display pins*/
    /*Configure GPIO pins : LED1_Pin LED2_Pin */
    GPIO_InitStruct.Pin = LED1_Pin | LED2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    /*Configure GPIO pin : BTN2_Pin */
    GPIO_InitStruct.Pin = BTN2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BTN2_GPIO_Port, &GPIO_InitStruct);
    /*Configure GPIO pin : BTN1_Pin */
    GPIO_InitStruct.Pin = BTN1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(BTN1_GPIO_Port, &GPIO_InitStruct);
    HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void printMaxTemp(Display *d){
    printTemp(d, maxTemp, SCALE[sp], dp);
    printStr(d, "  ");
}

void printMinTemp(Display *d){
    printTemp(d, minTemp, SCALE[sp], dp);
    printStr(d, "  ");
}

Menu m;
MenuWindow mw0;
MenuItem temperature, other;
MenuWindow mw1;
MenuItem showCurr, options;
MenuWindow mw11;
MenuWindow mw12;
MenuItem dpNum, scale, alarm;
MenuWindow mw122;
MenuItem C, F, K;
MenuWindow mw121;
MenuItem zeroDP, oneDP;
MenuWindow mw123;
MenuItem min, max;
MenuWindow mw1231;
MenuItem halfDegMax, oneDegMax, fiveDegMax;
MenuWindow mw1232;
MenuItem halfDegMin, oneDegMin, fiveDegMin;

void MY_MENU_Init(void){
    /* Menu Window 0 */
    InitMenuItem(&temperature, &mw1, 0, 0, "Temperature");
    InitMenuItem(&other, NULL, 0, 1, "Other");
    InitMenuWindow(&mw0, 0, NULL, 1);
    InitMenu(&m, &mw0, &d);
   
    addMenuItem(&mw0, &temperature);
    addMenuItem(&mw0, &other);

    /* Menu Window 1 */
    InitMenuItem(&showCurr, &mw11, 0, 0, "Show current");
    InitMenuItem(&options, &mw12, 0, 1, "Options");
    InitMenuWindow(&mw1, 1, &mw0, 0);
  
    addMenuItem(&mw1, &showCurr);
    addMenuItem(&mw1, &options);

    addMenuWindow(&m, &mw1);

    /* Menu Window 11 */
    InitMenuWindow(&mw11, 11, &mw1, 0);
    setWindowText(&mw11, 1, 0, "Temperature:");
    setWindowText(&mw11, 3, 1, " ");

    addMenuWindow(&m, &mw11);

    /* Menu Window 12 */
    InitMenuItem(&dpNum, &mw121, 0, 0, "Dec. place num.");
    InitMenuItem(&scale, &mw122, 0, 1, "Scale");
    InitMenuItem(&alarm, &mw123, 7, 1, "Alarm");
    InitMenuWindow(&mw12, 12, &mw1, 0);
  
    addMenuItem(&mw12, &dpNum);
    addMenuItem(&mw12, &scale);
    addMenuItem(&mw12, &alarm);

    addMenuWindow(&m, &mw12);

    /* Menu Window 122 */
    char c[3] = {0xB2,'C','\0'};
    char f[3] = {0xB2,'F','\0'};
    InitMenuItem(&C, NULL, 0, 0, c);
    InitMenuItem(&F, NULL, 8, 0, f);
    InitMenuItem(&K, NULL, 0, 1, " K");
    InitMenuWindow(&mw122, 122, &mw12, 0);
    setAsRadioWindow(&mw122, sp);
  
    addMenuItem(&mw122, &C);
    addMenuItem(&mw122, &F);
    addMenuItem(&mw122, &K);

    addMenuWindow(&m, &mw122);

    /* Menu Window 121 */
    InitMenuItem(&zeroDP, NULL, 0, 0, "0 dec. places");
    InitMenuItem(&oneDP, NULL, 0, 1, "1 dec. place");
    InitMenuWindow(&mw121, 121, &mw12, 0);
    setAsRadioWindow(&mw121, dp);

    addMenuItem(&mw121, &zeroDP);
    addMenuItem(&mw121, &oneDP);

    addMenuWindow(&m, &mw121);

    /* Menu Window 123 */
    InitMenuItem(&max, &mw1231, 0, 0, "MAX: ");
    InitMenuItem(&min, &mw1232, 0, 1, "MIN: ");

    InitMenuWindow(&mw123, 123, &mw12, 0);
    setCustomPrint(&mw123, 6, 0, &printMaxTemp);
    setCustomPrint(&mw123, 6, 1, &printMinTemp);

    addMenuItem(&mw123, &max);
    addMenuItem(&mw123, &min);

    addMenuWindow(&m, &mw123);

    /* Menu Window 1231 */
    InitMenuItem(&halfDegMax, NULL, 1, 1, "0.5");
    InitMenuItem(&oneDegMax, NULL, 6, 1, "1");
    InitMenuItem(&fiveDegMax, NULL, 9, 1, "5");

    InitMenuWindow(&mw1231, 1231, &mw123, 0);
    setWindowText(&mw1231, 1, 0, "MAX: ");
    setCustomPrint(&mw1231, 6, 0, &printMaxTemp);
    setAsRadioWindow(&mw1231, 1);
  
    addMenuItem(&mw1231, &halfDegMax);
    addMenuItem(&mw1231, &oneDegMax);
    addMenuItem(&mw1231, &fiveDegMax);

    addMenuWindow(&m, &mw1231);

    /* Menu Window 1232 */
    InitMenuItem(&halfDegMin, NULL, 1, 1, "0.5");
    InitMenuItem(&oneDegMin, NULL, 6, 1, "1");
    InitMenuItem(&fiveDegMin, NULL, 9, 1, "5");

    InitMenuWindow(&mw1232, 1232, &mw123, 0);
    setWindowText(&mw1232, 1, 0, "MIN: ");
    setCustomPrint(&mw1232, 6, 0, &printMinTemp);
    setAsRadioWindow(&mw1232, 1);
  
    addMenuItem(&mw1232, &halfDegMin);
    addMenuItem(&mw1232, &oneDegMin);
    addMenuItem(&mw1232, &fiveDegMin);

    addMenuWindow(&m, &mw1232);

    /* Start Menu */
    startMenu(&m);
}

void GPIO_SetPinAsInput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    uint8_t i;
    /* Go through all pins */
    for (i = 0x00; i < 0x10; i++) {
        /* Pin is set */
        if (GPIO_Pin & (1 << i)) {
            /* Set 00 bits combination for input */
            GPIOx->MODER &= ~(0x03 << (2 * i));
        }
    }
}

void GPIO_SetPinAsOutput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
    uint8_t i;
    /* Go through all pins */
    for (i = 0x00; i < 0x10; i++) {
        /* Pin is set */
        if (GPIO_Pin & (1 << i)) {
            /* Set 01 bits combination for output */
            GPIOx->MODER = (GPIOx->MODER & ~(0x03 << (2 * i))) | (0x01 << (2 * i));
        }
    }
}

void led_on(Led_TypeDef Led) {
    HAL_GPIO_WritePin(GPIOE, (Led == LED1 ? LED1_Pin : LED2_Pin), GPIO_PIN_SET);
}

void led_off(Led_TypeDef Led) {
    HAL_GPIO_WritePin(GPIOE, (Led == LED1 ? LED1_Pin : LED2_Pin), GPIO_PIN_RESET);
}

void led_toggle(Led_TypeDef Led) {
    HAL_GPIO_TogglePin(GPIOE, (Led == LED1 ? LED1_Pin : LED2_Pin));
}

void EXTI0_IRQHandler(void) {
    /* No need to disable interrupts here as HAL does that for us. */
    if (button_is_pressed(BUTTON1)) {
        HAL_GPIO_EXTI_IRQHandler(BTN1_Pin);
    }
}

void EXTI15_10_IRQHandler(void) {
    /* No need to disable interrupts here as HAL does that for us. */
    if (button_is_pressed(BUTTON2)) {
        HAL_GPIO_EXTI_IRQHandler(BTN2_Pin);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == BTN1_Pin) {
        menuEnter();
    } else if (GPIO_Pin == BTN2_Pin) {
        next(&m);
    } else if (GPIO_Pin == TST_BTN_Pin) {
        menuEnter();
    } else {
        led_toggle(LED1);
        led_toggle(LED2);
    }
}

char c_fullUpArrow = 4;
char c_emptyUpArrow = 5;
char c_fullDownArrow = 0xF5;
char c_emptyDownArrow = 6;

void menuEnter(){
    noInterrupts();
    if(lockBTN == 1) return;
    else Tprint = 0;
    interrupts();
    int s, oldS;
    oldS = getState(&m);
    s = enter(&m);
    if(s<0){
        s = getState(&m);
        if (s==122) sp = getSelectNum(&m);
        else if (s==121) dp = getSelectNum(&m);
        else if (s==1231) { 
            if(ap == getSelectNum(&m)){
                maxTemp += amount * addSub;
                if(maxTemp > 125) maxTemp = 125;
                if(maxTemp < -55) maxTemp = -55;
                reprintCustom(&m);
            }
            else{
                ap = getSelectNum(&m);
                amount = AMOUNT[ap];
            }
        }
        else if (s==1232) { 
            if(ap == getSelectNum(&m)){
                minTemp += amount * addSub;
                if(minTemp > 125) minTemp = 125;
                if(minTemp < -55) minTemp = -55;
                reprintCustom(&m);
            }
            else{
                ap = getSelectNum(&m);
                amount = AMOUNT[ap];
            }
        }
    }
    if((s==1231 && oldS!=1231) || (s==1232 && oldS!=1232)){ // first window entrance
        setCursor(&d, 0, 0);
        write(&d, c_fullUpArrow);
        setCursor(&d, 0, 1);
        write(&d, c_emptyDownArrow);
        addSub = 1;
        ap = getSelectNum(&m);
        amount = AMOUNT[ap];
    }
    if(s!=11){
        Tprint = 0; 
    }
    if(s==11){
        Tprint = 1;
        perTprint();
    }
}

void mainJSdown(){
    down(&m);
    int s = getState(&m);
    if (s==1231 || s==1232) {
        setCursor(&d, 0, 0);
        write(&d, c_emptyUpArrow);
        setCursor(&d, 0, 1);
        write(&d, c_fullDownArrow);
        addSub = -1;
    }
}

void mainJSup(){
    up(&m);
    int s = getState(&m);
    if (s==1231 || s==1232) {
        setCursor(&d, 0, 0);
        write(&d, c_fullUpArrow);
        setCursor(&d, 0, 1);
        write(&d, c_emptyDownArrow);
        addSub = 1;
    }
}

void mainJSleft(){
    left(&m);
}

void mainJSright(){
    right(&m);
}

uint8_t up_Ndown = 0;

void perTprint(void){
    disableInput();
    setCursor(&d, 3, 1);
    if (ts.error_flag == 0){
        printTemp(&d, ts.temp, SCALE[sp], dp);
    }
    else {
        printStr(&d, "Error     ");
    }

    up_Ndown = !up_Ndown;
    setCursor(&d, 1, 1);
    write(&d, up_Ndown?2:3);
    enableInput();
}

void disableInput(){
    lockBTN = 1;
}

void enableInput(){
    lockBTN = 0;
}

GPIO_PinState read_the_button(Button_TypeDef button) {
    if (button == BUTTON1) {
        return HAL_GPIO_ReadPin(GPIOE, BTN1_Pin);
    } else if (button == BUTTON2) {
        return HAL_GPIO_ReadPin(GPIOA, BTN2_Pin);
    } else {
        return GPIO_PIN_RESET;
    }
}

char button_is_pressed(Button_TypeDef button) {
    HAL_Delay(200 *1000);
    return read_the_button(button);
}
