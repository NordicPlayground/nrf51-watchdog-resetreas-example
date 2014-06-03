ble_app_template_with_Watchdog_timer
==================

 * This file shows watchdog functionality with the softdevice enabled.
 * It is an add on to the ble_app_template example in the nRF51 SDK. If you want 
 * to see what has been added to the original ble_app_template to obtain watchdog
 * functionality, you can compare the main.c file with the original file in the 
 * nRF51 SDK, with e.g. WinDiff.
 *
 * LED4 is toggled at startup. Watchdog is set to 3 seconds and will cause
 * a reset after that time. The watchdog can be reloaded by pressing BUTTON3,
 * to prevent reset. LED3 will blink to indicate the button press.
 * Note that the watchdog does not work in debug mode. Since the chip will start up in
 * debug mode after flashing, the watchdog will only work after a powercycle.
 *
 * Note: To demonstrate watchdog functionality, the powermanage() function in the main
 * loop has been commented out in this example which will cause higher current
 * consumption.
 
Requirements
------------
- nRF51 SDK version 5.2.0
- S110 SoftDevice version 6.0.0
- nRF51822 Development Kit version 2.1.0 or later

The project may need modifications to work with other versions or other boards. 

To compile it, clone the repository in the nrf51822/Board/pca10001/s110/ folder.

About this project
------------------
This application is one of several applications that has been built by the support team at Nordic Semiconductor, as a demo of some particular feature or use case. It has not necessarily been thoroughly tested, so there might be unknown issues. It is hence provided as-is, without any warranty. 

However, in the hope that it still may be useful also for others than the ones we initially wrote it for, we've chosen to distribute it here on GitHub. 

The application is built to be used with the official nRF51 SDK, that can be downloaded from https://www.nordicsemi.no, provided you have a product key for one of our kits.

Please post any questions about this project on https://devzone.nordicsemi.com.
