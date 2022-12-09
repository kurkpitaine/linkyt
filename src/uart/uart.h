#ifndef LINKYT_UART_H
#define LINKYT_UART_H

#include "app_uart.h"

#define UART_TX_BUF_SIZE 8    /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 2048 /**< UART RX buffer size. */

/**
 * @brief UART handler type.
 */
typedef void (*linkyt_uart_handler_t)(const uint8_t *frame, const uint16_t frame_len);

/**
 * @brief UART config structure.
 */
typedef struct linkyt_uart_config_s
{
   linkyt_uart_handler_t evt_handler; /**< Received frame handler. */
} linkyt_uart_config_t;

/**
 * @brief Initialize UART peripheral with the default Teleinfo parameters.
 *
 * @param p_config Pointer to the configuration data.
 * @returns NRF_SUCCESS on successful init
 * @returns NRF_ERROR_INVALID_PARAM when evt_handler is NULL in p_config.
 */
uint32_t uart_init(linkyt_uart_config_t *p_config);

/**
 * @brief Handles UART events from the UART module.
 * Build a Teleinfo frame from the stream of received bytes.
 * Teleinfo frames starts with the ETX char (0x02) and ends with the STX char (0x03).
 *
 * @param p_event pointer to the UART event to handle.
 */
void uart_event_handle(app_uart_evt_t *p_event);

#endif /* LINKYT_UART_H */
