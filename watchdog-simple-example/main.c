/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the license.txt file.
 */

/** @file
 * @brief Watchdog example
 *
 * This example demonstrates the use of the watchdog. LED_1 blinks slowly at startup.
 * Watchdog timer is then started and LED_1 starts to blink faster. Watchdog is set 
 * to 3 seconds and will cause a reset after that time. The watchdog can be reloaded, 
 * thereby cancelling the reset, by pressing BUTTON_0. LED_0 will blink to indicate 
 * the button press.
 */

#include <stdbool.h>
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"

#define DELAY 200000															
#define NR_OF_STARTUP_BLINKS 8	

#define BTN_PRESSED 0
#define BTN_RELEASED 1

/**
 * Configures pin P0.00 for input and pin P0.08, P0.09 for output (pins P0.0 and P1.0, P1.1 respectively on the nRFgo motherboard).
 * Configures GPIOTE to give interrupt on P0.00 pin change.
 */
static void gpio_init(void)
{
	//Configure LEDs and button
  nrf_gpio_cfg_input(BUTTON_0, NRF_GPIO_PIN_NOPULL);
	nrf_gpio_cfg_output(LED_0);
	nrf_gpio_cfg_output(LED_1);

	//Configure GPIOTE channel to toggle pin
  NRF_GPIOTE->CONFIG[0] =  (GPIOTE_CONFIG_POLARITY_Toggle << GPIOTE_CONFIG_POLARITY_Pos)
              | (0 << GPIOTE_CONFIG_PSEL_Pos)
              | (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos);	
	
  // Enable GPIOTE interrupt:
  NVIC_EnableIRQ(GPIOTE_IRQn);
  NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Set << GPIOTE_INTENSET_IN0_Pos;	
}

/** GPIOTE interrupt handler.
 * Triggered on pin P0.00 (BUTTON_0) change, i.e. it is triggered when BUTTON_0 is pushed and also when BUTTON_0 is released.
 */
void GPIOTE_IRQHandler(void)
{
  // Event causing the interrupt must be cleared
  if ((NRF_GPIOTE->EVENTS_IN[0] == 1) && (NRF_GPIOTE->INTENSET & GPIOTE_INTENSET_IN0_Msk))
  {
    NRF_GPIOTE->EVENTS_IN[0] = 0;
  }
	
	//Toogle LED_0 to indicate a button press and watchdog reload. 
	if(nrf_gpio_pin_read(BUTTON_0) == BTN_PRESSED)
	{
		nrf_gpio_pin_set(LED_0);
	}
	else
	{
		nrf_gpio_pin_clear(LED_0);
	}
           
	NRF_WDT->RR[0] = WDT_RR_RR_Reload;  //Reload watchdog register 0
}

/** Watchdog initialization.
 * Triggered on pin P0.00 change
 */
void wdt_init(void)
{
	NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);   //Configure Watchdog. a) Pause watchdog while the CPU is halted by the debugger.  b) Keep the watchdog running while the CPU is sleeping.
	NRF_WDT->CRV = 3*32768;             //ca 3 sek. timout
	NRF_WDT->RREN |= WDT_RREN_RR0_Msk;  //Enable reload register 0
	NRF_WDT->TASKS_START = 1;           //Start the Watchdog timer
}

/**
 * main function
 */
int main(void)
{
  gpio_init();    //Initialize buttons and LEDs
	
	//BLINK LED 1 slowly during startup					
	for(int i=0;i<NR_OF_STARTUP_BLINKS;i++)					
	{
		nrf_gpio_pin_toggle(LED_1);
		nrf_delay_us(DELAY);
	}

	wdt_init();     //Initialize watchdog
	
  while (true)
  {
		//Blink LED 1 fast until watchdog triggers reset
		nrf_gpio_pin_toggle(LED_1);
		nrf_delay_us(DELAY/3);
  }
}

/**
 *@}
 **/
