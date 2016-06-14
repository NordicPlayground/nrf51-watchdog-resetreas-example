/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/* This example shows functionality of the Watchdog. The watchdog will reset the IC every 3 seconds if
 * it is not reloaded by pressing Button 1. LED 2 will blink during startup, which indicates that the 
 * watchdog has restarted the IC.
 */

#include <stdbool.h>
#include "nrf.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "boards.h"
#include "nrf_delay.h"

#define DELAY 100000															
#define NR_OF_STARTUP_BLINKS 10	

#define PIN_IN BSP_BUTTON_0
#define PIN_OUT BSP_LED_0
#define PIN_WDT BSP_LED_1
#define PIN_WDT_IRQ_HANDLER BSP_LED_2


void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    nrf_drv_gpiote_out_toggle(PIN_OUT);
		NRF_WDT->RR[0] = WDT_RR_RR_Reload;  //Reload watchdog register 0
}

void WDT_IRQHandler(void)
{
	nrf_drv_gpiote_out_toggle(PIN_WDT_IRQ_HANDLER);     //The WDT interrupt handler will have around 2 32kHz clock cycles to execute before reset, so you can not actully see this LED blink with your eye.
}

/**
 * @brief Function for configuring: PIN_IN pin for input, PIN_OUT and PIN_WDT pin for output, 
 * and configures GPIOTE to give an interrupt on pin change.
 */
static void gpio_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
    
    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);

    err_code = nrf_drv_gpiote_out_init(PIN_OUT, &out_config);
    APP_ERROR_CHECK(err_code);
	
	  err_code = nrf_drv_gpiote_out_init(PIN_WDT, &out_config);
    APP_ERROR_CHECK(err_code);
	
		err_code = nrf_drv_gpiote_out_init(PIN_WDT_IRQ_HANDLER, &out_config);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(PIN_IN, &in_config, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(PIN_IN, true);
}


void wdt_init(void)
{
	NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);   //Configure Watchdog. a) Pause watchdog while the CPU is halted by the debugger.  b) Keep the watchdog running while the CPU is sleeping.
	NRF_WDT->CRV = 3*32768;             //ca 3 sek. timout
	NRF_WDT->RREN |= WDT_RREN_RR0_Msk;  //Enable reload register 0
	
	// Enable WDT interrupt:
  NVIC_EnableIRQ(WDT_IRQn);
  NRF_WDT->INTENSET = WDT_INTENSET_TIMEOUT_Msk;	
	
	NRF_WDT->TASKS_START = 1;           //Start the Watchdog timer
}


/**
 * @brief Function for application main entry.
 */
int main(void)
{
	gpio_init();

	//BLINK LED 2 during startup					
	for(int i=0;i<NR_OF_STARTUP_BLINKS;i++)					
	{
		nrf_drv_gpiote_out_toggle(PIN_WDT);
		nrf_delay_us(DELAY);
	}

	wdt_init();     //Initialize watchdog

    while (true)
    {
			__WFE();
			__SEV();
			__WFE();
    }
}


/** @} */
