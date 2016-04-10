#include "stm32f0xx.h"
#include "UARTDevice.h"

/** This implementation uses the USART1 peripheral with PB6 pin as Tx and
*   PB7 pin as Rx
*/



UARTDevice::UARTDevice()
	: m_TxInProgress(false)
{}

UARTDevice & UARTDevice::get()
{
	static UARTDevice instance;
	return instance ;
}


void UARTDevice::init()
{
    // INIT GPIO

    // - turn on clock for PORTB TX and RX pins ( PORTB, 6 and 7 pins ) STM32F051R8
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN ;
    // - select alternative mode for PB6 and PB7
    // The pins  6 and 7 at PORTB have AF0 as default (USART1 TX and RX)
    GPIOB->MODER |= (GPIO_MODER_MODER6_1);
    GPIOB->MODER |= (GPIO_MODER_MODER7_1);

    // no pull-up, pull-down
    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR6 | GPIO_PUPDR_PUPDR7);


	// speed -> fast
	GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR7 | GPIO_OSPEEDER_OSPEEDR6);

    // INIT UART

    // Turn on clock gating for USART1:
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN ;

    // - set baud rate
    USART1->BRR = UART1_BAUD19200;
    // - default settings: 8bit data, 1 stop bit, non parity check

    USART1->CR1 |= USART_CR1_RE         // turn on the receiver
                | USART_CR1_RXNEIE      // turn on the RXNE interrupt
                ;
    USART1->CR3 |= USART_CR3_EIE;		// turn on Error Frame interrupt

    // - set NVIC for interrupt
    NVIC_ClearPendingIRQ(USART1_IRQn);
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_SetPriority(USART1_IRQn, 1);

}


inline void UARTDevice::enable(const bool enable)
{
    if (true == enable)
    {
        USART1->CR1 |= USART_CR1_UE;
    }
    else
    {
        USART1->CR1 &= ~USART_CR1_UE;
    }
}

inline bool UARTDevice::isReadBufferEmpty()
{
	return m_RcvBuff.isEmpty();
}

bool UARTDevice::send(const uint8_t data)
{
	// append data to the buffer
	bool retval = m_SendBuff.append(data);

	// Initialize the transfer
	if( false == m_TxInProgress )
	{
		// todo refactor this to use within enable(bool) method
		// - enable module
		m_TxInProgress = true;
		enable(true);

		// - enable tx
		// - enable TXNE and TC interrupts
		USART1->CR1 |= USART_CR1_TE | USART_CR1_TXEIE | USART_CR1_TCIE;
	}

	return retval;
}

bool UARTDevice::read(uint8_t & ref)
{
	return m_RcvBuff.get(ref);
}



void USART1_IRQHandler()
{
	static UARTDevice & dev = UARTDevice::get();

	// Transmit data from the ring buffer
	if (USART1->ISR & USART_ISR_TXE)
	{
		uint8_t data;
		if ( true == dev.m_SendBuff.get(data))
		// transmit data
		{
			USART1->TDR = data;
		}
		// Wait for transmission completion if buffer is empty
		else if (USART1->ISR & USART_ISR_TC)
		{
			dev.m_TxInProgress = false;
			// - disable the TXE and TC interrupt
			// - disable tx
			USART1->CR1 &= ~(USART_CR1_TE | USART_CR1_TXEIE | USART_CR1_TCIE);
			// - clear TC flag
			USART1->ICR |= USART_ICR_TCCF;
		}
	}

	// Save just read data to the Ring Buffer
	if (USART1->ISR & USART_ISR_RXNE)
	{
		dev.m_RcvBuff.append(USART1->RDR);
		// clear RXNE if RDR wasn't read
		USART1->RQR |= USART_RQR_RXFRQ;
	}

	// Frame error
	if (USART1->ISR & USART_ISR_FE)
	{
		USART1->ICR |= USART_ICR_FECF;
		USART1->ICR |= USART_ICR_NCF;
	}
}
