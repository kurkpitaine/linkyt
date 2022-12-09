#define NRF_LOG_MODULE_NAME UART

#include "uart.h"
#include "error_handlers.h"
#include "boards.h"
#include "nrf.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#if defined(UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined(UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

static linkyt_uart_config_t __linkyt_uart_config;

uint32_t uart_init(linkyt_uart_config_t *p_config)
{
   uint32_t err_code;
   if (p_config->evt_handler == NULL)
   {
      return NRF_ERROR_INVALID_PARAM;
   }

   __linkyt_uart_config.evt_handler = p_config->evt_handler;

   app_uart_comm_params_t const comm_params =
   {
      .rx_pin_no = RX_PIN_NUMBER,
      .tx_pin_no = TX_PIN_NUMBER,
      .rts_pin_no = RTS_PIN_NUMBER,
      .cts_pin_no = CTS_PIN_NUMBER,
      .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
      .use_parity = false,
#if defined(UART_PRESENT)
      .baud_rate = NRF_UART_BAUDRATE_1200
#else
      .baud_rate = NRF_UARTE_BAUDRATE_1200
#endif
   };

   APP_UART_FIFO_INIT(&comm_params,
                      UART_RX_BUF_SIZE,
                      UART_TX_BUF_SIZE,
                      uart_event_handle,
                      APP_IRQ_PRIORITY_LOWEST,
                      err_code);
   return err_code;
}

void uart_event_handle(app_uart_evt_t *p_event)
{
   static uint8_t data_array[UART_RX_BUF_SIZE];
   static uint16_t index = 0;
   static bool stx_found = false; /** Frame start delimiter found flag */
   static bool etx_found = false; /** Frame end delimiter found flag */

   switch (p_event->evt_type)
   {
   case APP_UART_DATA_READY:
   {
      /* An event indicating that UART data has been received. The data is available in the FIFO. */
      uint32_t rc = NRF_SUCCESS;

      if (index >= (uint16_t)UART_RX_BUF_SIZE)
      {
         /* Rx buffer is full. If we fall here, we are fed with incorrect frames. */
         index = 0;
         stx_found = false;
         etx_found = false;
      }

      /* Teleinfo framing */
      do
      {
         rc = app_uart_get(&data_array[index]);

         if (rc == (uint32_t)NRF_SUCCESS)
         {
            /* Mask parity bit */
            data_array[index] &= 0x7f;

            /* The STX char */
            if (data_array[index] == 0x02)
            {
               stx_found = true;
            }

            /* The ETX char */
            if (data_array[index] == 0x03)
            {
               etx_found = true;
            }

            if (stx_found && etx_found)
            {
               /* We have a complete frame, call the external handler */
               __linkyt_uart_config.evt_handler(data_array, index + 1);

               /* Reset flags */
               stx_found = false;
               etx_found = false;
               index = 0;
            }

            if (stx_found)
            {
               index++;
            }
         }
      } while (rc == (uint32_t)NRF_SUCCESS);

      break;
   }
   case APP_UART_FIFO_ERROR:
   {
      if (p_event->data.error_code == NRF_ERROR_NO_MEM)
      {
         NRF_LOG_WARNING("UART Rx FIFO full");
         /* If we fall here, we are not fast enough to compute the data.
            Flush the FIFO and wait for the next complete frame.
         */
         app_uart_flush();
         stx_found = false;
         etx_found = false;
         index = 0;
      }
      else
      {
         /* An error in the FIFO module used by the app_uart module has occurred. */
         NRF_LOG_ERROR("APP_UART_FIFO_ERROR: %s\n", nrf_error_string(p_event->data.error_code));
         APP_ERROR_HANDLER(p_event->data.error_code);
      }
      break;
   }
   case APP_UART_COMMUNICATION_ERROR:
   {
      /* An communication error has occured during reception */
      NRF_LOG_ERROR("APP_UART_COMMUNICATION_ERROR: %s\n", uart_error_string(p_event->data.error_communication));
      break;
   }
   case APP_UART_TX_EMPTY:
   case APP_UART_DATA:
   default:
   {
      /* Cannot fall here */
      break;
   }
   }
}
