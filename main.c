/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the license.txt file.
 */
/*
 * This example demonstrates the use of the watchdog, System Off and RESETREAS. 
 *
 * LED_0 (pin 8) is toggled slowly at startup to show that the device is starting. When 
 * the device is in the main loop LED_0 will flash faster. Watchdog is set to 3 seconds 
 * and will cause a reset after that time. The watchdog can be reloaded by pressing BUTTON_0.
 * 
 * Software reset is performed with pressing BUTTON_1 (pin 1).
 *
 * The device is put into System Off low power mode when button_2 (pin 2) is pressed 
 * and wakes up from System off when button 3 (pin 3) is pressed. Led 0 (pin 8) is blinking 
 * when the device is active but turns off when the device enters System Off mode.
 * After flashing this program, the nRF51 will stay in debug mode. When in debug mode, System
 * Off will be emulated and LED_0 will continue blinking since the main loop will be active.
 * Power reset the nRF51 in order to exit debug mode and enter normal mode.
 *
 * Pins 9-15 show the RESETREAS register value (LEDS 1-7 on the nRF6310 board). 
 * See the nRF51_Reference_Manual v1.1 for RESETREAS documentation.
 * LED_1 is set after pin reset. 
 * LED_2 is set when watchdog triggers the reset. 
 * LED_3 is set when software triggers a reset.
 * LED_4 is set when CPU lock-up triggers a reset.
 * LED_5 is set after waking up from System Off when wakeup source is GPIO pin.
 * LED_6 is set after waking up from System Off when the wakeup source is LPCOMP.
 * LED_7 is set after waking up from System Off wehn the wakeup source is DEBUG mode.
 * To see what peripherals are reset when performing different resets, see section 11.1.6.8
 * in the nRF51 Reference Manual v2.0.
 *
 */

#ifndef BOARD_NRF6310
#define BOARD_NRF6310
#endif

#include <stdbool.h>
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"

#define DELAY          200000
#define BTN_PRESSED    0
#define BTN_RELEASED   1

#define RESET_FROM_PIN                  0x00000001
#define RESET_FROM_WDT                  0x00000002
#define RESET_FROM_SOFTWARE             0x00000004
#define RESET_FROM_LOCKUP               0x00000008
#define RESET_FROM_SYSTEM_OFF_GPIO      0x00010000
#define RESET_FROM_SYSTEM_OFF_LPCOMP    0x00020000
#define RESET_FROM_SYSTEM_OFF_DEBUG     0x00040000

#define STARTUP_TOGGLE_ITERATIONS       7

/**
 * Configures pin 0 for input and pin 8 for output and
 * configures GPIOTE to give interrupt on BUTTON_0 (pin 0) change.
 */
static void gpio_init(void)
{
    //Configure LEDs and button
  nrf_gpio_cfg_input(BUTTON_0, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(BUTTON_1, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(BUTTON_2, NRF_GPIO_PIN_NOPULL);    
    nrf_gpio_port_dir_set(NRF_GPIO_PORT_SELECT_PORT1, NRF_GPIO_PORT_DIR_OUTPUT);

    // Configure BUTTON_3 with SENSE enabled (not possible using nrf_gpio.h)
    NRF_GPIO->PIN_CNF[BUTTON_3] = (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos)
                                                         | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                                                         | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
                                                         | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                                                         | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);    
}

static void gpiote_init(void)
{
    //Configure GPIOTE channel to toggle pin
  NRF_GPIOTE->CONFIG[0] =  (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos)
              | (BUTTON_0 << GPIOTE_CONFIG_PSEL_Pos)
              | (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);    
    
  // Enable GPIOTE interrupt:
  NVIC_EnableIRQ(GPIOTE_IRQn);
  NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos;
}

/** GPIOTE interrupt handler.
 * Triggered on BUTTON_0 (pin 0) change
 */
void GPIOTE_IRQHandler(void)
{
  // Event causing the interrupt must be cleared
  if ((NRF_GPIOTE->EVENTS_IN[0] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN0_Msk))
  {
    NRF_GPIOTE->EVENTS_IN[0] = 0;
  }
  nrf_gpio_pin_toggle(LED_0);
    NRF_WDT->RR[0] = 0x6E524635;  //Reload watchdog register 0
}

void wdt_init(void)
{
    NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);
    NRF_WDT->CRV = 3*32768;   //ca 3 sek. timout
    NRF_WDT->RREN |= WDT_RREN_RR0_Msk;  //Enable reload register 0
    NRF_WDT->TASKS_START = 1;
}

/**
 * main function
 * \return 0. int return type required by ANSI/ISO standard.
 */
int main(void)
{
    int i;
    
  gpio_init();
    gpiote_init();
    wdt_init();
    
    //Write the value of RESETREAS to pins 9-15 (LEDs 1-7)
    nrf_gpio_pin_write(LED_1, NRF_POWER->RESETREAS & RESET_FROM_PIN);                //Bit A in RESETREAS
    nrf_gpio_pin_write(LED_2, NRF_POWER->RESETREAS & RESET_FROM_WDT);                //Bit B in RESETREAS
    nrf_gpio_pin_write(LED_3, NRF_POWER->RESETREAS & RESET_FROM_SOFTWARE);           //Bit C in RESETREAS
    nrf_gpio_pin_write(LED_4, NRF_POWER->RESETREAS & RESET_FROM_LOCKUP);             //Bit D in RESETREAS
    nrf_gpio_pin_write(LED_5, NRF_POWER->RESETREAS & RESET_FROM_SYSTEM_OFF_GPIO);    //Bit E in RESETREAS
    nrf_gpio_pin_write(LED_6, NRF_POWER->RESETREAS & RESET_FROM_SYSTEM_OFF_LPCOMP);  //Bit F in RESETREAS
    nrf_gpio_pin_write(LED_7, NRF_POWER->RESETREAS & RESET_FROM_SYSTEM_OFF_DEBUG);   //Bit G in RESETREAS    
    
    NRF_POWER->RESETREAS = 0xFFFFFFFF;   //Clear the RESETREAS register
    
    for(i=0;i<STARTUP_TOGGLE_ITERATIONS;i++)
    {
        nrf_gpio_pin_toggle(LED_0);
        nrf_delay_us(DELAY);
    }

  while (true)
  {
        //Blink LED 0 fast until watchdog triggers reset
        nrf_gpio_pin_toggle(LED_0);
        nrf_delay_us(DELAY/3);
        
        // If BUTTON_2 is pressed.. enter System Off mode
        if(nrf_gpio_pin_read(BUTTON_2) == BTN_PRESSED)
        {
                // Clear PORT 1 (pins 8-15)
                nrf_gpio_port_clear(NRF_GPIO_PORT_SELECT_PORT1, 0xFF);
            
                // Enter system OFF. After wakeup the chip will be reset, and the program will run from the top 
                NRF_POWER->SYSTEMOFF = 1;
        }
        
        // If BUTTON_1 is pressed.. soft-reset
        if(nrf_gpio_pin_read(BUTTON_1) == BTN_PRESSED)
        {
            NVIC_SystemReset();
        }
  }
}

/**
 *@}
 **/
