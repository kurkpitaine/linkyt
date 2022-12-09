/**
 * Adafuit Feather 52840 board definition file.
 *
*/
#ifndef FEATHER_52840_H
#define FEATHER_52840_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// LEDs definitions
#define LEDS_NUMBER    2

#define LED_1          NRF_GPIO_PIN_MAP(1,10)
#define LED_2          NRF_GPIO_PIN_MAP(1,15)

#define LEDS_ACTIVE_STATE 1

#define LEDS_LIST { LED_1, LED_2 }

#define LEDS_INV_MASK  LEDS_MASK

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2

#define BUTTONS_NUMBER 1

#define BUTTON_1       NRF_GPIO_PIN_MAP(1,2)
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1 }

#define BSP_BUTTON_0   BUTTON_1

#define RX_PIN_NUMBER  NRF_GPIO_PIN_MAP(0,24)   // UART RX pin number.
#define TX_PIN_NUMBER  NRF_GPIO_PIN_MAP(0,25)   // UART TX pin number.
#define CTS_PIN_NUMBER 0u                       // UART Clear To Send pin number. Not used if HWFC is set to false.
#define RTS_PIN_NUMBER 0u                       // UART Request To Send pin number. Not used if HWFC is set to false.
#define HWFC           false                    // UART hardware flow control.

#define SPIM0_SCK_PIN   NRF_GPIO_PIN_MAP(0,14)  // SPI clock GPIO pin number.
#define SPIM0_MOSI_PIN  NRF_GPIO_PIN_MAP(0,13)  // SPI Master Out Slave In GPIO pin number.
#define SPIM0_MISO_PIN  NRF_GPIO_PIN_MAP(0,15)  // SPI Master In Slave Out GPIO pin number.
#define SPIM0_SS_PIN    NRF_GPIO_PIN_MAP(1,09)  // SPI Slave Select GPIO pin number.

#define QSPIM_SCK_PIN    NRF_GPIO_PIN_MAP(0,19)  // QSPI clock GPIO pin number.
#define QSPIM_DATA0_PIN  NRF_GPIO_PIN_MAP(0,17)  // QSPI Master DATA 0 GPIO pin number.
#define QSPIM_DATA1_PIN  NRF_GPIO_PIN_MAP(0,22)  // QSPI Master DATA 1 GPIO pin number.
#define QSPIM_DATA2_PIN  NRF_GPIO_PIN_MAP(0,23)  // QSPI Master DATA 2 GPIO pin number.
#define QSPIM_DATA3_PIN  NRF_GPIO_PIN_MAP(0,21)  // QSPI Master DATA 3 GPIO pin number.
#define QSPIM_CS_PIN     NRF_GPIO_PIN_MAP(0,20)  // QSPI Slave Select GPIO pin number.

#ifdef __cplusplus
}
#endif

#endif // FEATHER_52840_H
