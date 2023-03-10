#include <stdint.h>

//---------------
// RCC Registers
//---------------

#define REG_RCC_CR     (volatile uint32_t*)(uintptr_t)0x40021000U // Clock Control Register
#define REG_RCC_CFGR   (volatile uint32_t*)(uintptr_t)0x40021004U // PLL Configuration Register
#define REG_RCC_AHBENR (volatile uint32_t*)(uintptr_t)0x40021014U // AHB1 Peripheral Clock Enable Register
#define REG_RCC_CFGR2  (volatile uint32_t*)(uintptr_t)0x4002102CU // Clock configuration register 2

//----------------
// GPIO Registers
//----------------

#define GPIOC_MODER (volatile uint32_t*)(uintptr_t)0x48000800U // GPIO port mode register
#define GPIOC_TYPER (volatile uint32_t*)(uintptr_t)0x48000804U // GPIO port output type register
#define GPIOC_ODR (volatile uint32_t*)(uintptr_t)0x48000814U // GPIOx_ODR, when x offset

#define REG_SET_0(REG, VALUE)  *REG &= ~VALUE
#define REG_SET_1(REG, VALUE)  *REG |= VALUE
#define MODIFY_REG(REG, MODIFYMASK, VALUE)  *REG |= MODIFYMASK << VALUE

//----------------
//board_clocking_init()
//----------------

#define HSE_ON 0x00010000U
#define HSE_READY 0x00020000U

//----------------
//board_gpio_init()
//----------------

#define ON_CLOCKING 0x80000U
#define BIT_8 8U
#define BIT_9 9U
#define SET_BLUE_LED 0b01U
#define SET_GREEN_LED 0b01
#define SET_OUTPUT 0b00

//----------------
//main()
//----------------

#define BLUE_LED 0x000100U
#define GREEN_LED 0x000200U

#define CPU_FREQENCY 48000000U // CPU frequency: 48 MHz
#define ONE_MILLISECOND CPU_FREQENCY/1000U

void board_clocking_init(){
    // (1) Clock HSE and wait for oscillations to setup.
    *REG_RCC_CR = HSE_ON;
    while ((*REG_RCC_CR & HSE_READY) != HSE_READY);

    // (2) Configure PLL:
    // PREDIV output: HSE/2 = 4 MHz
    *REG_RCC_CFGR2 |= 1U;

    // (3) Select PREDIV output as PLL input (4 MHz):
    *REG_RCC_CFGR |= 0x00010000U;

    // (4) Set PLLMUL to 12:
    // SYSCLK frequency = 48 MHz
    *REG_RCC_CFGR |= (12U-2U) << 18U;

    // (5) Enable PLL:
    *REG_RCC_CR |= 0x01000000U;
    while ((*REG_RCC_CR & 0x02000000U) != 0x02000000U);

    // (6) Configure AHB frequency to 48 MHz:
    *REG_RCC_CFGR |= 0b000U << 4U;

    // (7) Select PLL as SYSCLK source:
    *REG_RCC_CFGR |= 0b10U;
    while ((*REG_RCC_CFGR & 0xCU) != 0x8U);

    // (8) Set APB frequency to 24 MHz
    *REG_RCC_CFGR |= 0b001U << 8U;
}

void board_gpio_init(){
 // (1) Enable GPIOC clocking:
    REG_SET_1(REG_RCC_AHBENR, ON_CLOCKING);

// (2) Configure PC8 mode:
    MODIFY_REG(GPIOC_MODER, SET_BLUE_LED, (2 * BIT_8));

// (3) Configure PC8 type:
    MODIFY_REG(GPIOC_TYPER,  SET_OUTPUT,  BIT_8);

// (4) Configure PC9 mode:
    MODIFY_REG(GPIOC_MODER, SET_GREEN_LED, (2 * BIT_9));

// (5) Configure PC9 type:
    MODIFY_REG(GPIOC_TYPER, SET_OUTPUT, BIT_9);
}

void time_1000ms(){
    for (uint32_t i = 0; i < 1000U * ONE_MILLISECOND; ++i){
        // Insert NOP for power consumption:
        __asm__ volatile("nop");
    }
}

int main(){
#ifndef INSIDE_QEMU
    board_clocking_init();
#endif
    board_gpio_init();

    while(1){
        REG_SET_1(GPIOC_ODR, BLUE_LED);
        time_1000ms();
        REG_SET_0(GPIOC_ODR, BLUE_LED);
        time_1000ms();
 
        REG_SET_1(GPIOC_ODR, GREEN_LED);
        time_1000ms();
        REG_SET_0(GPIOC_ODR, GREEN_LED);
        time_1000ms();
    }
}