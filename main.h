#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <string.h>
#define APB_BUS 0x40010000
#define AHB_BUS 0x40020000
#define RCC_BASE 0x40021000
#define TIM6_BASE 0x40001000
#define GPIOA_BASE 0x50000000
#define EXTI_BASE 0x40021800
#define SYSCFG_BASE 0x40010000 //sysconfig base reg.
#define GPIOA_PU (*(volatile uint32_t*)(GPIOA_BASE+0x0C))
#define RCC_IOPENR (*(volatile uint32_t*)(RCC_BASE + 0x34))  // register for enabling clock for GIPO ports.
#define GPIOA_MODER (*(volatile uint32_t*)(GPIOA_BASE + 0x00)) //GPIOA mode // set as input ofr external interrupt
#define RCC_APBENR2 (*(volatile uint32_t*)(RCC_BASE + 0x40)) // reg to enable clock for SYSCFG.
#define RCC_APB1ENR (*(volatile uint32_t*)(RCC_BASE + 0x3C)) //reg to enable clock for tim6
#define EXTICR1 (*(volatile uint32_t*)(SYSCFG_BASE + 0x060)) //rerouting PORTA to EXTI for interrupt.
#define EXTI_RTSR1 (*(volatile uint32_t*)(EXTI_BASE)) //rising edge reg
#define EXTI_FTSR1 (*(volatile uint32_t*)(EXTI_BASE + 0x004)) //falling edge reg
#define EXTI_RPR1 (*(volatile uint32_t*)(EXTI_BASE + 0x00C)) //used to clear pending status
#define EXTI_FPR1 (*(volatile uint32_t*)(EXTI_BASE + 0x010)) //used to clear pending status EXTI_BASE
#define EXTI_IMR1 (*(volatile uint32_t*)(EXTI_BASE+0x080)) //unmask the interrupt signal
#define NVIC_BASE 0xE000E100
#define NVIC_ISER (*(volatile uint32_t*)NVIC_BASE)


//TIMER REGISTERS
#define TIM6_CR1 (*(volatile uint32_t*)(TIM6_BASE + 0x00)) //TIM6 control register 1
#define TIM6_SR (*(volatile uint32_t*)(TIM6_BASE + 0x10))  // TIM6 status register
#define TIM6_CNT (*(volatile uint32_t*)(TIM6_BASE + 0x24))  // counter value
#define TIM6_ARR (*(volatile uint32_t*)(TIM6_BASE + 0x2C))   // auto reload value
#define TIM6_PSC (*(volatile uint32_t*)(TIM6_BASE + 0x28))  //prescaler for TIM6


//UART
#define USART2_BASE   0x40004400
#define USART2_CR1   (*(volatile uint32_t*)(USART2_BASE + 0x00))
#define USART2_BRR   (*(volatile uint32_t*)(USART2_BASE + 0x0C))
#define USART2_ISR   (*(volatile uint32_t*)(USART2_BASE + 0x1C))
#define USART2_TDR   (*(volatile uint32_t*)(USART2_BASE + 0x28))
#define GPIOA_AFRL   (*(volatile uint32_t*)(GPIOA_BASE + 0x20))

extern const char *morse_table[36];

#endif   // MAIN_H