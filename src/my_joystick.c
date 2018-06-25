#include "my_joystick.h"
#include "main.h"

void MY_JOYSTICK_GPIO_Init()
{
    GPIO_InitTypeDef GPIO_InitStruct_A;
    GPIO_InitTypeDef GPIO_InitStruct_C;
    GPIO_InitTypeDef GPIO_InitStruct_E;
    /* GPIO Ports Clock Enable */
    __GPIOE_CLK_ENABLE(); /*For pins PE7/8/10: Joystick-RST,TST,INT */
    __GPIOC_CLK_ENABLE(); /*For pin PC9: Joystick-SDA*/
    __GPIOA_CLK_ENABLE(); /*For pin PA8: Joystick-SCL*/
    /*Configure GPIO pin : PA8 */
    GPIO_InitStruct_A.Pin = GPIO_PIN_8;
    GPIO_InitStruct_A.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct_A.Pull = GPIO_NOPULL; /*pull-ups on joystick*/
    GPIO_InitStruct_A.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct_A);
    /*Configure GPIO pin : PC9 */
    GPIO_InitStruct_C.Pin = GPIO_PIN_9;
    GPIO_InitStruct_C.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct_C.Pull = GPIO_NOPULL; /*pull-ups on joystick*/
    GPIO_InitStruct_C.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct_C);
    /*Configure GPIO pins : PE7 */
    GPIO_InitStruct_E.Pin = GPIO_PIN_7;
    GPIO_InitStruct_E.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct_E.Pull = GPIO_NOPULL;
    GPIO_InitStruct_E.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct_E);
    /*Configure GPIO pin : TST_BTN */
    GPIO_InitStruct_E.Pin = TST_BTN_Pin;
    GPIO_InitStruct_E.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct_E.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(TST_BTN_GPIO_Port, &GPIO_InitStruct_E);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

HAL_StatusTypeDef MY_JOYSTICK_START(Joystick *j)
{
    j->_address = JSaddress;

    /*Assign pin numbers*/
    j->_rst_pin = GPIO_PIN_7; /*PE 7*/
    j->_tst_pin = GPIO_PIN_8; /*PE 8*/
    j->_int_pin = GPIO_PIN_10;/*PE 10*/
    j->_sda_pin = GPIO_PIN_9; /*PC 9*/
    j->_scl_pin = GPIO_PIN_8; /*PA 8*/

    /*Assign peripherals*/
    j->_rst_GPIO_port = GPIOE; /*P E 7*/
    j->_tst_GPIO_port = GPIOE; /*P E 8*/
    j->_int_GPIO_port = GPIOE; /*P E 10*/
    j->_sda_GPIO_port = GPIOC; /*P C 9*/
    j->_scl_GPIO_port = GPIOA; /*P A 8*/
     
    return MY_JOYSTICK_CONNECT(j);
}

HAL_StatusTypeDef MY_JOYSTICK_CONNECT(Joystick *j){
    /*Hardware reset: low pulse after power-up*/
    /*alternative: software reset*/
    HAL_GPIO_WritePin(j->_rst_GPIO_port, j->_rst_pin, GPIO_PIN_RESET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(j->_rst_GPIO_port, j->_rst_pin, GPIO_PIN_SET);

    HAL_Delay(1100); /*Tstartup >1000us*/
   
    // check communication
    GPIO_PinState status;
    status = i2c_write_byte(j, 1, 1, (j->_address << 1) | _WRITE_BIT); // check device

    writeSpcReg(j, _Control1, 0xf8);

    if (status == GPIO_PIN_RESET) return HAL_OK;
    else return HAL_ERROR;
}

uint8_t downPush = 0;
uint8_t upPush = 0;
uint8_t leftPush = 0;
uint8_t rightPush = 0;

void measureJS(Joystick *j){
    uint8_t X = readSpcReg(j, _X);
    uint8_t Y = readSpcReg(j, _Y);

    /* X < -50 */
    if (X<0xCE && X>0x99){
        if (downPush == 0){
            downPush = 1;
            mainJSdown();
        }
    }
    else downPush = 0;

    /* X > 50 */
    if (X>0x32 && X<0x99){
        if (upPush == 0){
            upPush = 1;
            mainJSup();
        }
    }
    else upPush = 0;

    /* Y < -50 */
    if (Y<0xCE && Y>0x99){
        if (leftPush == 0){
            leftPush = 1;
            mainJSleft();
        }
    }
    else leftPush = 0;

    /* Y > 50 */
    if (Y>0x32 && Y<0x99){
        if (rightPush == 0){
            rightPush = 1;
            mainJSright();
        }
    }
    else rightPush = 0;
}

void EXTI9_5_IRQHandler(void) {
    if (JS_button_is_pressed()) {
        HAL_GPIO_EXTI_IRQHandler(TST_BTN_Pin);
    }
}

char JS_button_is_pressed(void) {
    HAL_Delay(200 *100 *10);
    return read_JS_button();
}

GPIO_PinState read_JS_button(void) {
    return HAL_GPIO_ReadPin(TST_BTN_GPIO_Port, TST_BTN_Pin);
}

uint8_t readCurReg(Joystick *j){
    if (i2c_write_byte(j, 1, 0, (j->_address << 1) | _READ_BIT) != GPIO_PIN_RESET) return '2';
    return i2c_read_byte(j, 1, 1);
}

uint8_t readSpcReg(Joystick *j, uint8_t regNum){
    if (i2c_write_byte(j, 1, 0, (j->_address << 1) | _WRITE_BIT) != GPIO_PIN_RESET) return '1';
    if (i2c_write_byte(j, 0, 0, regNum) != GPIO_PIN_RESET) return '2';
    if (i2c_write_byte(j, 1, 0, (j->_address << 1) | _READ_BIT) != GPIO_PIN_RESET) return '3';
    return i2c_read_byte(j, 1, 1);
}

uint8_t writeSpcReg(Joystick *j, uint8_t regNum, uint8_t value){
    if (i2c_write_byte(j, 1, 0, (j->_address << 1) | _WRITE_BIT) != GPIO_PIN_RESET) return '1';
    if (i2c_write_byte(j, 0, 0, regNum) != GPIO_PIN_RESET) return '2';
    if (i2c_write_byte(j, 0, 1, value) != GPIO_PIN_RESET) return '3';
    return 'M';
}

uint8_t started = 0; // global data

void i2c_start_cond(Joystick *j) 
{
    set_SDA(j);
    set_SCL(j);
    I2C_delay();
    // SCL is high, set SDA from 1 to 0.
    clear_SDA(j);
    I2C_delay();
    clear_SCL(j);
    I2C_delay();
    started = 1;
}

void i2c_stop_cond(Joystick *j)
{
    set_SCL(j);
    I2C_delay();
    // SCL is high, set SDA from 0 to 1
    set_SDA(j);
    I2C_delay();
    started = 0;
}

// Write a bit to I2C bus
void i2c_write_bit(Joystick *j, uint8_t bit) 
{
    if(bit == 0){
        clear_SDA(j);
    } 
    else{
        set_SDA(j);
    }

    set_SCL(j);

    I2C_delay();

    clear_SCL(j);
    I2C_delay();

    if(bit != 0){
        clear_SDA(j);
    } 
    I2C_delay();
}

// Read a bit from I2C bus
GPIO_PinState i2c_read_bit(Joystick *j) 
{
    GPIO_PinState bit;

    set_SDA(j);
    set_SCL(j);
    I2C_delay();

    bit = read_SDA(j);
    
    clear_SCL(j);
    I2C_delay();

    return bit;
}

// Write a byte to I2C bus. Return 0 if ack by the slave.
GPIO_PinState i2c_write_byte(Joystick *j, uint8_t send_start, 
                             uint8_t send_stop, uint8_t byte) 
{
    uint8_t bit;
    GPIO_PinState nack;

    if(send_start){
        i2c_start_cond(j);
    }

    for(bit = 0; bit < 8; bit++){
        i2c_write_bit(j, ( byte & 0x80 ) != 0 );
        byte <<= 1;
    }

    nack = i2c_read_bit(j);

    if (send_stop){
        i2c_stop_cond(j);
    }

    return nack;
}

// Read a byte from I2C bus
uint8_t i2c_read_byte(Joystick *j, uint8_t nack, uint8_t send_stop)
{
    uint8_t byte = 0;
    uint8_t bit;

    for(bit = 0; bit < 8; bit++){
        byte = (byte << 1) | i2c_read_bit(j);
    }

    i2c_write_bit(j, nack);

    if(send_stop){
        i2c_stop_cond(j);
    }
   
    I2C_delay();

    return byte;
}

GPIO_PinState read_SDA(Joystick *j){
    GPIO_SetPinAsInput(j->_sda_GPIO_port, j->_sda_pin);	// let pin float, pull up will raise
    HAL_Delay(10);

    return HAL_GPIO_ReadPin(j->_sda_GPIO_port, j->_sda_pin);
}

void set_SCL(Joystick *j){
    GPIO_SetPinAsOutput(j->_scl_GPIO_port, j->_scl_pin);
    HAL_GPIO_WritePin(j->_scl_GPIO_port, j->_scl_pin, GPIO_PIN_SET);	// drive output high
}

void clear_SCL(Joystick *j){
    HAL_GPIO_WritePin(j->_scl_GPIO_port, j->_scl_pin, GPIO_PIN_RESET);
    GPIO_SetPinAsOutput(j->_scl_GPIO_port, j->_scl_pin);	// drive output low
}

void set_SDA(Joystick *j){
    GPIO_SetPinAsOutput(j->_sda_GPIO_port, j->_sda_pin);
    HAL_GPIO_WritePin(j->_sda_GPIO_port, j->_sda_pin, GPIO_PIN_SET);	// drive output high
}

void clear_SDA(Joystick *j){
    HAL_GPIO_WritePin(j->_sda_GPIO_port, j->_sda_pin, GPIO_PIN_RESET);
    GPIO_SetPinAsOutput(j->_sda_GPIO_port, j->_sda_pin);	// drive output low   
}

void I2C_delay(void){ 
    HAL_Delay(50);
}
