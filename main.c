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
 * and will cause a reset after that time. The watchdog can be reloaded by pressing WATCHDOG_RELOAD_BUTTON.
 * 
 * Software reset is performed with pressing SOFTWARE_RESET_BUTTON (pin 1).
 *
 * The device is put into System Off low power mode when SYSTEM_OFF_BUTTON (pin 2) is pressed 
 * and wakes up from System off when WAKEUP_BUTTON (pin 3) is pressed. Led 0 (pin 8) is blinking 
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

#include <stdbool.h>
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"

#define DELAY          200000
#define BTN_PRESSED    0
#define BTN_RELEASED   1

#define WATCHDOG_RELOAD_CONSTANT 0x6E524635

#define WATCHDOG_RELOAD_BUTTON BUTTON_0
#define SOFTWARE_RESET_BUTTON  BUTTON_1
#define SYSTEM_OFF_BUTTON      BUTTON_2
#define WAKEUP_BUTTON          BUTTON_3

#define STARTUP_TOGGLE_ITERATIONS       7


/**
 * Configures LEDs and buttons from boards.h as outputs and inputs. 
 */
static void gpio_init(void)
{
    nrf_gpio_cfg_input(WATCHDOG_RELOAD_BUTTON, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(SOFTWARE_RESET_BUTTON, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(SYSTEM_OFF_BUTTON, NRF_GPIO_PIN_NOPULL);	
    nrf_gpio_cfg_sense_input(WAKEUP_BUTTON, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);
    
    nrf_gpio_range_cfg_output(LED_0, LED_7);
}


static void gpiote_init(void)
{
    //Configure GPIOTE channel to trigger on WATCHDOG_RELOAD_BUTTON toggle.
    NRF_GPIOTE->CONFIG[0] = (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos) |
                            (WATCHDOG_RELOAD_BUTTON << GPIOTE_CONFIG_PSEL_Pos) |
                            (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);    

    NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos;
    NVIC_EnableIRQ(GPIOTE_IRQn);
}



void wdt_init(void)
{
    NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | 
                      (WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);
                      
    NRF_WDT->CRV = 3*32768;   //ca 3 sec. timout
    NRF_WDT->RREN |= WDT_RREN_RR0_Msk;  //Enable reload register 0
    NRF_WDT->TASKS_START = 1;
}


/** GPIOTE interrupt handler.
 * Triggered on WATCHDOG_RELOAD_BUTTON change
 */
void GPIOTE_IRQHandler(void)
{
    // Event causing the interrupt must be cleared
    if ((NRF_GPIOTE->EVENTS_IN[0] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN0_Msk))
    {
        NRF_GPIOTE->EVENTS_IN[0] = 0;
        
        NRF_WDT->RR[0] = WATCHDOG_RELOAD_CONSTANT; 
        
        nrf_gpio_pin_toggle(LED_0);
    }
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
    nrf_gpio_pin_write(LED_1, NRF_POWER->RESETREAS & POWER_RESETREAS_RESETPIN_Msk); //Bit A in RESETREAS
    nrf_gpio_pin_write(LED_2, NRF_POWER->RESETREAS & POWER_RESETREAS_DOG_Msk);      //Bit B in RESETREAS
    nrf_gpio_pin_write(LED_3, NRF_POWER->RESETREAS & POWER_RESETREAS_SREQ_Msk);     //Bit C in RESETREAS
    nrf_gpio_pin_write(LED_4, NRF_POWER->RESETREAS & POWER_RESETREAS_LOCKUP_Msk);   //Bit D in RESETREAS
    nrf_gpio_pin_write(LED_5, NRF_POWER->RESETREAS & POWER_RESETREAS_OFF_Msk);      //Bit E in RESETREAS
    nrf_gpio_pin_write(LED_6, NRF_POWER->RESETREAS & POWER_RESETREAS_LPCOMP_Msk);   //Bit F in RESETREAS
    nrf_gpio_pin_write(LED_7, NRF_POWER->RESETREAS & POWER_RESETREAS_DIF_Msk);      //Bit G in RESETREAS    

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

        // If SYSTEM_OFF_BUTTON is pressed.. enter System Off mode
        if(nrf_gpio_pin_read(SYSTEM_OFF_BUTTON) == BTN_PRESSED)
        {
            // Clear PORT 1 (pins 8-15)
            nrf_gpio_port_clear(NRF_GPIO_PORT_SELECT_PORT1, 0xFF);

            // Enter system OFF. After wakeup the chip will be reset, and the program will run from the top 
            NRF_POWER->SYSTEMOFF = 1;
        }

        // If SOFTWARE_RESET_BUTTON is pressed.. soft-reset
        if(nrf_gpio_pin_read(SOFTWARE_RESET_BUTTON) == BTN_PRESSED)
        {
            NVIC_SystemReset();
        }
    }
}

/**
 *@}
 **/
