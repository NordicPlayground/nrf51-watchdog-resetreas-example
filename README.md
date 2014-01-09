Watchdog, System Off and RESETREAS example

 * This example demonstrates the use of the watchdog, System Off and displays RESETREAS. 
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
 * This example is tested with nRF51822 QFAA G0 and nRF51 SDK v5.1.0