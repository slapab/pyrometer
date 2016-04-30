/*
 * I2CDevice.cpp
 *
 *  Created on: 22 mar 2016
 *      Author: scott
 */

#include "stm32f0xx.h"
#include "I2CDevice.h"
#include "TimerCore.h"
#include "HandleTimer.h"

extern uint32_t SystemCoreClock;

/*
 * This implementation will use the I2C1 and PB8 as SCL and PB9 as SDA
 */


I2CDevice::I2CDevice(const uint8_t slaveAddr)
{
    m_slaveAddres = slaveAddr & 0x7F;
}

/*
 * Init IO to use in I2C protocol
 */
void I2CDevice::init()
{
    // CONFIGURE GPIO

    // enable CLK for used GPIO
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    // configure GPIO in right alternative function mode
    GPIOB->MODER &= ~GPIO_MODER_MODER8;     // reset bits to zero
    GPIOB->MODER |=  GPIO_MODER_MODER8_1;   // set alternate function mode


    GPIOB->MODER &= ~GPIO_MODER_MODER9;     // reset bits to zero
    GPIOB->MODER |=  GPIO_MODER_MODER9_1;   // set alternate function mode


    // select alternate function
    GPIOB->AFR[1] &= ~(GPIO_AFRH_AFR8 | GPIO_AFRH_AFR9);    // reset bits to zero
    const uint32_t altfunc = 0x11111111;
    GPIOB->AFR[1] |= (altfunc & GPIO_AFRH_AFR8) | (altfunc & GPIO_AFRH_AFR9); // set alternate function for both pins

    // no pull-up, pull-down
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR8 | GPIO_PUPDR_PUPDR9);

    // Open-drain mode - is used with external pull-up resistors!
    GPIOB->OTYPER |= (GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9);

    // speed -> fast
    GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR8 | GPIO_OSPEEDER_OSPEEDR9);


    // CONFIGURE I2C

    // Init SMBus mode
    initSMBusMode();

}


void I2CDevice::deinit()
{
    //todo
}

void I2CDevice::wakeUp()
{
    // Set mode to GPIO on SPI pins
    GPIOB->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9);     // reset bits
    GPIOB->MODER |=  GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0;   // enable GPIO function

    // Open Drain type -> because of external PULL-UP resistors
    GPIOB->OTYPER |= (GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9);


    // Generate wake up sequence:
    // - SCL high, SDA high for some time
    GPIOB->BSRR |= GPIO_BSRR_BS_8 | GPIO_BSRR_BS_9;
    __asm__("nop"); __asm__("nop"); __asm__("nop");

    // - SDA go to low for at least 14 ms
    // PB9 is SDA
    GPIOB->BSRR |= GPIO_BSRR_BR_9;

    // Delay required during waking up, at least 14ms but additional delay is required before reading the first data
    auto t1 = HandleTimer(SysTickTimerCore);
    t1.Sleep_ms(30);

    // - SDA go to high
    GPIOB->BSRR |= GPIO_BSRR_BS_9;
    __asm__("nop"); __asm__("nop"); __asm__("nop");

    // - enable SMBus mode
    init();
}


void I2CDevice::enable(const bool enable)
{
    if( true == enable )
    {
        // enable I2C module
        I2C1->CR1 |= I2C_CR1_PE ;
    }
    else
    {
        // disable I2C module
        I2C1->CR1 &= ~I2C_CR1_PE ;
    }
}



// 1) doesn't work if size is > 255
// todo it is not finished. Address is sent properly, but need to examine if it works
// with real device
void I2CDevice::send(const uint8_t addr, const uint8_t * const pBuff, const size_t size)
{
    if(nullptr == pBuff)
        return ;


    //set the Slave address
    setSlaveAddr();
    //set number of bytes to be transmitted
    setTransferBytes(size+2); //+1 for PEC byte and +1 for the addr/command

    // enable PEC functionality
    I2C1->CR1 |= I2C_CR1_PECEN ;

    enable(true);

    // this is write transmission
    I2C1->CR2 &= ~I2C_CR2_RD_WRN;

    I2C1->CR2 |= I2C_CR2_AUTOEND    // set autoend
              |  I2C_CR2_PECBYTE    // PEC byte will be transmitted
              |  I2C_CR2_START      // generate start
              ;

    // Transmit the command / address where to store:
    // - wait for TX buffer to be empty
    while( !(I2C1->ISR & I2C_ISR_TXIS) ) { asm(""); }
    // - write the command / address
    I2C1->TXDR = addr;

    // Write Data Bytes
    for(size_t i = 0; i < size ; ++i)
    {
        // wait for NACK flag or TXIS flag
        while(!(I2C1->ISR & I2C_ISR_NACKF) && !(I2C1->ISR & I2C_ISR_TXIS)) { asm(""); }

        // check for NACK flag
        if(I2C1->ISR & I2C_ISR_NACKF)
        {
            // todo report an error
            // This is an error
            // 1) not found address
            // 2) Receiver cannot read byte
            // 3) Receiver has detected error in bytes (CRC)
            return ;
        }
        // TXIS flag - write next byte
        else
        {
            I2C1->TXDR = pBuff[i];
        }
    }


    // wait until STOP symbol will be detected
    while(I2C1->ISR & I2C_ISR_BUSY) { asm(""); }

    // disable peripheral -> clear flags
    enable(false);
}


void I2CDevice::read(const uint8_t addr, uint8_t * const pBuff, const size_t len)
{
    // because this is an SMBus -> need to write command
    // in this case the command defines the EEPROM or RAM memory and cell address

    if(nullptr == pBuff)
        return;
    if(0 == len)
        return;

    // todo is this necessary on each call?
    //set the Slave address
    setSlaveAddr();

    // 1) Transmit command
    // - need to disable the PEC for writing command byte
    I2C1->CR1 &= ~I2C_CR1_PECEN;

    // - set number of bytes to be transmitted: 1 - only command
    setTransferBytes(1);

    enable(true);

    // disable the autoend because need to generate the RESTART signal
    I2C1->CR2 &= ~I2C_CR2_AUTOEND;

    I2C1->CR2 &= ~I2C_CR2_RD_WRN;   // this is a write transmission - to transmit the command
    I2C1->CR2 |= I2C_CR2_START;     // generate start

    // Wait for TX buffer to be empty
    while( !(I2C1->ISR & I2C_ISR_TXIS) );

    // write the command / address
    I2C1->TXDR = addr;

    // Wait for Transfer Completion
    while( !(I2C1->ISR & I2C_ISR_TC) );

    // 2) Read bytes from device

    // - set number of bytes to be received: 'len' data bytes and PEC
    setTransferBytes(len + 1);      // +1 for PEC byte

    // - enable PEC
    I2C1->CR1 |= I2C_CR1_PECEN;

    I2C1->CR2 |= I2C_CR2_AUTOEND    // enable autoend
                | I2C_CR2_RD_WRN    // this will be a read transfer
                | I2C_CR2_START ;   // generate 'restart' signal


    // read data
    for(size_t i = 0 ; i < len ; ++i)
    {
        while(!(I2C1->ISR & I2C_ISR_RXNE));

        pBuff[i] = I2C1->RXDR;
    }


    // wait for the PEC byte and he end transmission
    while( I2C1->ISR & I2C_ISR_BUSY );


    // todo check if transmission was not broken
    if( I2C1->ISR & I2C_ISR_PECERR )
    {
        // return PEC error
        int n = 0 ;
        n ++ ;
    }
    else
    {
        // return OK
        int i = 0 ;
        i++;
    }

    // disable peripheral -> clear flags
    enable(false);

}

void I2CDevice::clockEnable(const bool enable){
    if ( true == enable )
    {
        // Select the SYSCLK as clock source for I2C
        RCC->CFGR3 |= RCC_CFGR3_I2C1SW;
        // enable clock for I2C
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN ;
    }
    else
    {
        // disable clk for I2C
        RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN ;
    }
}


void I2CDevice::initSMBusMode()
{
    // enable clock
    clockEnable(true);

    // disable I2C module
    enable(false);

    //NOTE: followed configuration is SMBus specific configuration

    // timing configuration for: I2CCLK = SYSCLK = 48 MHz
    // and transmission type: 100 kHz
    I2C1->TIMINGR |= ( (I2C_TIMINGR_PRESC & 0xBBBBBBBB)
                    | (I2C_TIMINGR_SCLL & 0x13131313)
                    | (I2C_TIMINGR_SCLH & 0x0F0F0F0F)
                    | (I2C_TIMINGR_SDADEL & 0x22222222)
                    | (I2C_TIMINGR_SCLDEL & 0x44444444));


    // rest configuration
    I2C1->CR1 |= ( I2C_CR1_PECEN    // enable PEC ( CRC8 byte )
//                | I2C_CR1_ERRIE   // enable interrupt on PEC error
//                | I2C_CR1_TCIE    // enable interrupt on Transfer Complete
//                | I2C_CR1_TXIE    // enable interrupt on Transmit buffer empty
//                | I2C_CR1_RXIE    // enable interrupt on Receive buffer Not Empty

                );

    // set the slave address to right register
    setSlaveAddr();

}

void I2CDevice::setSlaveAddr(const uint8_t slaveAddr)
{
    if(0 != slaveAddr)
    {
        m_slaveAddres = slaveAddr & 0x7F;
    }

    uint32_t tmp = I2C1->CR2 ;
    tmp &= ~I2C_CR2_SADD ;

    // set 7-bit address
    tmp |= (static_cast<uint32_t>(m_slaveAddres) << 1);

    I2C1->CR2 = tmp;
}


inline void I2CDevice::setTransferBytes(size_t bytes)
{
    uint32_t tmp = I2C1->CR2 & (~I2C_CR2_NBYTES);
    tmp |= static_cast<uint32_t>((bytes & 0xFF) << 16);
    I2C1->CR2 = tmp;
}
