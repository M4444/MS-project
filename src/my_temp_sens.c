#include "my_temp_sens.h"
#include "main.h"

void MY_TEMPSENS_GPIO_Init()
{
    GPIO_InitTypeDef GPIO_InitStruct_C;
    /* GPIO Port Clock Enable */
    __GPIOC_CLK_ENABLE(); /*For pin PC13: TempSens-DQ*/
    /*Configure GPIO pin : PC13 */
    GPIO_InitStruct_C.Pin = GPIO_PIN_13;
    GPIO_InitStruct_C.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct_C.Pull = GPIO_PULLUP;
    GPIO_InitStruct_C.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct_C);
}

void MY_TEMPSENS_Init(TempSens *ts)
{
    /*Assign pin number*/
    ts->_dq_pin = GPIO_PIN_13; /*PC 13*/
    
    /*Assign peripheral*/
    ts->_dq_GPIO_port = GPIOC; /*P C 13*/

    ts->temp = 0;

    ts->error_flag = 0;
}

uint8_t getTemp(TempSens *ts){
    int i;
    uint8_t present = 0;
    uint8_t data[12];

    present = reset(ts);
    if (!present) return present;
    skip(ts);
    // Start conversion
    tempConvert(ts);
    
    for(i = 0; i<850*100; i++){
        if (read_bit(ts) == 1) break;
        HAL_Delay(10);
    } 

    present = reset(ts);
    if (!present) return present;
    skip(ts);

    // Issue Read scratchpad command
    readSP(ts);
    for ( i = 0; i < 9; i++) {
    data[i] = read(ts);
    }

    // Calculate temperature value
    if (data[1]==0xFF){ 
        data[0] = ~(data[0]-1);
        ts->temp = data[0] * (-0.5);
    }
    else if (data[1]==0x00){
        ts->temp = data[0] * 0.5;
    }

    return 1;
}

void storeAndReadTemp(TempSens *ts){
    int i;
    uint8_t data[12];

    /*Store previously read*/
    if (reset(ts)) {
        skip(ts);
        // Issue Read scratchpad command
        readSP(ts);
        for (i = 0; i < 9; i++) {
            data[i] = read(ts);
        }
        // Calculate temperature value
        if (data[1]==0xFF){ 
            data[0] = ~(data[0]-1);
            ts->temp = data[0] * (-0.5);
        }
        else if (data[1]==0x00){
            ts->temp = data[0] * 0.5;
        }

    	if (ts->temp >= maxTemp) led_toggle(LED1);
    	else led_off(LED1);
    	if (ts->temp <= minTemp) led_toggle(LED2);
    	else led_off(LED2);

        ts->error_flag = 0;
    }
    else ts->error_flag = 1;
    
    /*Read again*/
    if (reset(ts)) {
        skip(ts);
        // Start conversion
        tempConvert(ts);
    }
}

void readTemp(TempSens *ts){
    if (reset(ts)) {
        skip(ts);
        // Start conversion
        tempConvert(ts);
    }
}

// Perform the onewire reset function.  We will wait up to 250uS for
// the bus to come high, if it doesn't then it is broken or shorted
// and we return a 0;
//
// Returns 1 if a device asserted a presence pulse, 
// GPIO_PIN_RESET otherwise.
uint8_t reset(TempSens *ts)
{
    uint8_t r;
    uint16_t retries = 750;

    noInterrupts();
    GPIO_SetPinAsInput(ts->_dq_GPIO_port, ts->_dq_pin);
    HAL_GPIO_WritePin(ts->_dq_GPIO_port, ts->_dq_pin, GPIO_PIN_SET);	// Enable pull-up
    interrupts();
    // wait until the wire is high (max 1500 ms)
    do {
        if (--retries == 0) return 0;
        HAL_Delay(2 *10);
    } while ( HAL_GPIO_ReadPin(ts->_dq_GPIO_port, ts->_dq_pin) == GPIO_PIN_RESET);

    noInterrupts();
    HAL_GPIO_WritePin(ts->_dq_GPIO_port, ts->_dq_pin, GPIO_PIN_RESET);
    GPIO_SetPinAsOutput(ts->_dq_GPIO_port, ts->_dq_pin);	// drive output low
    interrupts(); HAL_Delay(60 *10); noInterrupts();
    GPIO_SetPinAsInput(ts->_dq_GPIO_port, ts->_dq_pin);	// allow it to float
    HAL_GPIO_WritePin(ts->_dq_GPIO_port, ts->_dq_pin, GPIO_PIN_SET);	// Enable pull-up
    interrupts(); HAL_Delay(8 *10); noInterrupts();
    /*acctually responds after 26 us*/
    r = (HAL_GPIO_ReadPin(ts->_dq_GPIO_port, ts->_dq_pin) == GPIO_PIN_RESET)? 1 : 0;
    interrupts();
    HAL_Delay(42 *10);

    return r;
}

// Do a ROM skip
void skip(TempSens *ts)
{
    writeTS(ts, 0xCC);           // Skip ROM
}

// Initiates temperature conversion
void tempConvert(TempSens *ts)
{
    writeTS(ts, 0x44);           // Convert T
}

// Reads bytes from scratchpad and reads CRC byte
void readSP(TempSens *ts)
{
    writeTS(ts, 0xBE);           // Read Scratchpad
}


// Write a byte.
void writeTS(TempSens *ts, uint8_t v) {
    uint8_t i = 8;
    /* Write 8 bits */
    while (i--) {
        /* LSB bit is first */
        write_bit(ts, v & 0x01);
        v >>= 1;
    }
}

// Write a bit.
void write_bit(TempSens *ts, uint8_t v)
{
    if (v & 1) {
        noInterrupts();
        HAL_GPIO_WritePin(ts->_dq_GPIO_port, ts->_dq_pin, GPIO_PIN_RESET);
        GPIO_SetPinAsOutput(ts->_dq_GPIO_port, ts->_dq_pin);	// drive output low
        interrupts(); HAL_Delay(10); noInterrupts();
        HAL_GPIO_WritePin(ts->_dq_GPIO_port, ts->_dq_pin, GPIO_PIN_SET);	// drive output high
        interrupts();
        HAL_Delay(55);
    } else {
        noInterrupts();
        HAL_GPIO_WritePin(ts->_dq_GPIO_port, ts->_dq_pin, GPIO_PIN_RESET);
        GPIO_SetPinAsOutput(ts->_dq_GPIO_port, ts->_dq_pin);	// drive output low
        interrupts(); HAL_Delay(65); noInterrupts();
        HAL_GPIO_WritePin(ts->_dq_GPIO_port, ts->_dq_pin, GPIO_PIN_SET);	// drive output high
        interrupts();
        HAL_Delay(5);
    }
}

// Read a byte
uint8_t read(TempSens *ts) {
    uint8_t bitMask;
    uint8_t r = 0;

    for (bitMask = 0x01; bitMask; bitMask <<= 1) {
	if ( read_bit(ts) == GPIO_PIN_SET ) r |= bitMask;
    }
    return r;
}

// Read a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
GPIO_PinState read_bit(TempSens *ts)
{
    GPIO_PinState r;

    noInterrupts();
    GPIO_SetPinAsOutput(ts->_dq_GPIO_port, ts->_dq_pin);
    HAL_GPIO_WritePin(ts->_dq_GPIO_port, ts->_dq_pin, GPIO_PIN_RESET);
    interrupts(); HAL_Delay(3); noInterrupts();
    GPIO_SetPinAsInput(ts->_dq_GPIO_port, ts->_dq_pin);	// let pin float, pull up will raise
    HAL_GPIO_WritePin(ts->_dq_GPIO_port, ts->_dq_pin, GPIO_PIN_SET);	// Enable pull-up
    /*rises in less than 1us*/
    interrupts(); HAL_Delay(10); noInterrupts();

    r = HAL_GPIO_ReadPin(ts->_dq_GPIO_port, ts->_dq_pin);
    interrupts();
    HAL_Delay(6 *10*10);
    return r;
}
