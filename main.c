/*************************************************************************************
 * File     :   main.c
 * Author   :   MinhHieuEC
 * Date     :   31/07/2020
 * Email    :   minhhieuec@gmail.com
**************************************************************************************/

#include "stm8s.h"
#include "main.h"

volatile uint16_t start = 0;
volatile uint16_t stop = 0;

void delay_us(uint16_t x)
{
    while (x--)
    {
        // nop();
        // nop();
        nop();
        nop();
        nop();
        nop();
        nop();
        nop();
        nop();
        nop();
        nop();
    }
} //  end delay func

void delay_ms(uint16_t ms)
{
    while (ms--)
    {
        delay_us(1000);
    }
}

/**
  * @addtogroup UART1_Printf
  * @{
  */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#ifdef _RAISONANCE_
#define PUTCHAR_PROTOTYPE int putchar(char c)
#define GETCHAR_PROTOTYPE int getchar(void)
#elif defined(_COSMIC_)
#define PUTCHAR_PROTOTYPE char putchar(char c)
#define GETCHAR_PROTOTYPE char getchar(void)
#else /* _IAR_ */
#define PUTCHAR_PROTOTYPE int putchar(int c)
#define GETCHAR_PROTOTYPE int getchar(void)
#endif /* _RAISONANCE_ */

/**
  * @brief Retargets the C library printf function to the UART.
  * @param c Character to send
  * @retval char Character sent
  */
PUTCHAR_PROTOTYPE
{
    /* Write a character to the UART1 */
    UART1_SendData8(c);
    /* Loop until the end of transmission */
    while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET)
        ;

    return (c);
}

/**
  * @brief Retargets the C library scanf function to the USART.
  * @param None
  * @retval char Character to Read
  */
GETCHAR_PROTOTYPE
{
#ifdef _COSMIC_
    char c = 0;
#else
    int c = 0;
#endif
    /* Loop until the Read data register flag is SET */
    while (UART1_GetFlagStatus(UART1_FLAG_RXNE) == RESET)
        ;
    c = UART1_ReceiveData8();
    return (c);
}

void uart_hardware_init(void)
{
    UART1_DeInit();

    UART1_Init((uint32_t)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
               UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);
    printf("\n\rTimer 2 Capture mode at pin PD3\n\r");
}

#define TIM_IER_CC1IE (1 << 1)
#define TIM_CR1_APRE (1 << 7)
#define TIM_IER_CC1IE (1 << 1)
#define TIM_CR1_URS (1 << 2)

void timer2_capture_init()
{
    // freq = 1MHz: psc=16 -> TIM2_PSCR = 4
    TIM2->PSCR = 4;
    // AIN: PD3 / TIM2_CH2 -> CC1 will be input on TI2FP1 & CC2 will be output
    // configure pin CC2 (PD3): open-drain output
    // PORT(PD, DDR) |= GPIO_PIN3; // output & pseudo open-drain
    // PORT(PD, CR2) |= GPIO_PIN3; // fast
    GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_OUT_OD_LOW_FAST);
    // out: PWM mode 1 (active -> inactive), preload enable
    // CCMR2: | 0 | 110 | 1 | 0 | 00 |
    TIM2->CCMR2 = 0x68;
    // in: TI2FP1
    TIM2->CCMR1 = 0x02;
    // polarity: out - active LOW, in - capture on rising edge, enable
    TIM2->CCER1 = 0x31;
    // interrupts: CC1IE
    TIM2->IER = TIM_IER_CC1IE;
    // enable preload for registers to refresh their values only by UEV
    TIM2->CR1 = TIM_CR1_APRE | TIM_CR1_URS;
    // ARR values: 1000 for reset, 70 for data in/out
    // CCR2 values: 500 for reset, 60 for sending 0 or reading, <15 for sending 1
    // CCR1 values: >550 if there's devices on line (on reset), >12 (typ.15) - read 0, < 12 (typ.1) - read 1
    // WARN! on reset there would be two CC events generated
}

void main(void)
{
    CLK_DeInit();
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

    uart_hardware_init();

    timer2_capture_init();

    enableInterrupts();


    TIM2->CR1 |= TIM2_CR1_CEN;

    while (1)
    {
    }

} //  end main

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *   where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line
     number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif