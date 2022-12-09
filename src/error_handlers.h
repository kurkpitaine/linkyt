#ifndef LINKYT_ERROR_HANDLERS_H
#define LINKYT_ERROR_HANDLERS_H

#include <stdint.h>
#include "nrf_error.h"
#include "nrf52840_bitfields.h"

/**
 * @brief Converts an NRF error code to a human readable string.
 * @param error_code NRF error code
 */
const char *nrf_error_string(uint32_t error_code)
{
   switch (error_code)
   {
   case (uint32_t)NRF_SUCCESS:
   {
      return "NRF_SUCCESS";
   }
   case (uint32_t)NRF_ERROR_SVC_HANDLER_MISSING:
   {
      return "NRF_ERROR_SVC_HANDLER_MISSING";
   }
   case (uint32_t)NRF_ERROR_SOFTDEVICE_NOT_ENABLED:
   {
      return "NRF_ERROR_SOFTDEVICE_NOT_ENABLED";
   }
   case (uint32_t)NRF_ERROR_INTERNAL:
   {
      return "NRF_ERROR_INTERNAL";
   }
   case (uint32_t)NRF_ERROR_NO_MEM:
   {
      return "NRF_ERROR_NO_MEM";
   }
   case (uint32_t)NRF_ERROR_NOT_FOUND:
   {
      return "NRF_ERROR_NOT_FOUND";
   }
   case (uint32_t)NRF_ERROR_NOT_SUPPORTED:
   {
      return "NRF_ERROR_NOT_SUPPORTED";
   }
   case (uint32_t)NRF_ERROR_INVALID_PARAM:
   {
      return "NRF_ERROR_INVALID_PARAM";
   }
   case (uint32_t)NRF_ERROR_INVALID_STATE:
   {
      return "NRF_ERROR_INVALID_STATE";
   }
   case (uint32_t)NRF_ERROR_INVALID_LENGTH:
   {
      return "NRF_ERROR_INVALID_LENGTH";
   }
   case (uint32_t)NRF_ERROR_INVALID_FLAGS:
   {
      return "NRF_ERROR_INVALID_FLAGS";
   }
   case (uint32_t)NRF_ERROR_INVALID_DATA:
   {
      return "NRF_ERROR_INVALID_DATA";
   }
   case (uint32_t)NRF_ERROR_DATA_SIZE:
   {
      return "NRF_ERROR_DATA_SIZE";
   }
   case (uint32_t)NRF_ERROR_TIMEOUT:
   {
      return "NRF_ERROR_TIMEOUT";
   }
   case (uint32_t)NRF_ERROR_NULL:
   {
      return "NRF_ERROR_NULL";
   }
   case (uint32_t)NRF_ERROR_FORBIDDEN:
   {
      return "NRF_ERROR_FORBIDDEN";
   }
   case (uint32_t)NRF_ERROR_INVALID_ADDR:
   {
      return "NRF_ERROR_INVALID_ADDR";
   }
   case (uint32_t)NRF_ERROR_BUSY:
   {
      return "NRF_ERROR_BUSY";
   }
   case (uint32_t)NRF_ERROR_CONN_COUNT:
   {
      return "NRF_ERROR_CONN_COUNT";
   }
   case (uint32_t)NRF_ERROR_RESOURCES:
   {
      return "NRF_ERROR_RESOURCES";
   }
   default:
   {
      return "NRF_ERROR_UNKNOWN";
   }
   }
}

/**
 * @brief Converts an NRF UART error code to a human readable string.
 * @param error_code NRF UART error code
 */
const char *uart_error_string(uint32_t error_code)
{
   if ((error_code & UART_ERRORSRC_BREAK_Msk) != 0ul)
   {
      return "UART_ERRORSRC_BREAK";
   }
   else if ((error_code & UART_ERRORSRC_FRAMING_Msk) != 0ul)
   {
      return "UART_ERRORSRC_FRAMING";
   }
   else if ((error_code & UART_ERRORSRC_PARITY_Msk) != 0ul)
   {
      return "UART_ERRORSRC_PARITY";
   }
   else if ((error_code & UART_ERRORSRC_OVERRUN_Msk) != 0ul)
   {
      return "UART_ERRORSRC_OVERRUN";
   }
   else
   {
      return "UART_ERRORSRC_UNKNOWN";
   }
}

#endif /* LINKYT_ERROR_HANDLERS_H */
