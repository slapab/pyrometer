
#include "stm32f0xx.h"

//#include "I2CDevice.h"
#include "MLX90614Sensor.h"

#include "RingBuffer.hpp"
#include "UARTDevice.h"
#include "I2CDevice.h"

#include <array>

//Interrupt handler declaration in C/C++
#ifdef __cplusplus
 extern "C" {
#endif
 	 void SysTick_Handler();
#ifdef __cplusplus
}
#endif

void SysTick_Handler()
{
	volatile int i = 0;
	++i;
}

int main()
{
    // enable clk for GPIO port C
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN ;

    // init the GPIO for leds
    GPIOC->MODER |= (GPIO_MODER_MODER8_0 | GPIO_MODER_MODER9_0) ; // PC9 as output

    GPIOC->OTYPER &= ~(GPIO_OTYPER_OT_8 | GPIO_OTYPER_OT_9) ;
    // Ensure push pull mode selected--default

    GPIOC->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR8|GPIO_OSPEEDER_OSPEEDR9);
    //Ensure maximum speed setting (even though it is unnecessary)

    GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPDR8|GPIO_PUPDR_PUPDR9);


    SystemCoreClockUpdate();
    SysTick_Config( SystemCoreClock / 1000 ) ;




    I2CDevice i2cDev(0x5A);
    MLX90614Sensor irSensor(i2cDev);

    UARTDevice & UART1dev = UARTDevice::get();
    UART1dev.init();


    for (auto i = 1 ; i <= 3 ; ++i)
    	UART1dev.send(i);

    int a = 1 ;
    while(a-- != 0)
    {
          // toggle bit?
          GPIOC->ODR |= GPIO_ODR_8;
          GPIOC->ODR &= ~GPIO_ODR_9;

          for ( volatile int i = 0 ; i < 4800000 ; ++i) {}

          irSensor.writeEmissivity(58982);

          constexpr const uint8_t cmd_emissivity = MLX90614_CMD_EEPROM | MLX90614_EPPROM_EMISSIVITY;
          uint16_t em = 0;
          em = irSensor.readEEPROM(cmd_emissivity);

//          uint16_t tempAmb = irSensor.readAmbiendTemp();
//          uint16_t tempObj = irSensor.readObjectTemp();

          GPIOC->ODR &= ~GPIO_ODR_8;
          GPIOC->ODR |= GPIO_ODR_9;

          for ( volatile int i = 0 ; i < 480000 ; ++i) {}
    }
    uint8_t rdata = 0;
    UART1dev.enable(true);
    while(1) {

    	if ( true == UART1dev.read(rdata) )
    	{
    		uint16_t tempAmb = irSensor.readAmbiendTemp();
    		uint16_t tempObj = irSensor.readObjectTemp();
    		UART1dev.send(tempAmb>>8);
    		UART1dev.send(tempAmb);
    		UART1dev.send('\n');
    		UART1dev.send(tempObj>>8);
			UART1dev.send(tempObj);
			UART1dev.send('\n');
    	}
    }
    return 0;
}
