#define NRF_LOG_MODULE_NAME TELEINFO

#include "teleinfo.h"
#include "tags.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

NRF_LOG_MODULE_REGISTER();

#define TELEINFO_TAG_STR_BUF_LEN (TELEINFO_TAG_MAX_STR_LEN + 1)
#define TELEINFO_VALUE_STR_BUF_LEN (TELEINFO_VALUE_MAX_STR_LEN + 1)

int32_t teleinfo_parse_frame(const uint8_t *buf, const uint16_t buf_len, teleinfo_data_t *data)
{
   int32_t rc = TELEINFO_ERROR;
   uint16_t i = 0;

   uint16_t group_start = 0;
   uint16_t group_end = 0;

   while (i < buf_len)
   {
      /* Ignore STX and ETX chars */
      if (buf[i] == (uint8_t)0x02 || buf[i] == (uint8_t)0x03)
      {
         ++i;
         continue;
      }

      /* Start of group = LF char */
      if (buf[i] == (uint8_t)0x0a)
      {
         group_start = i;
      }

      /* End of group = CR char. Allow it only if we already got the start delimiter */
      if (buf[i] == (uint8_t)0x0d && group_start != (uint16_t)0)
      {
         group_end = i;

         /* Parse group */
         int32_t res = teleinfo_parse_group(buf, group_start, group_end, data);

         if (res == NRF_SUCCESS)
         {
            rc = NRF_SUCCESS;
         }
         else
         {
            NRF_LOG_ERROR("Cannot parse group in range buf[%" PRIu16 "..%" PRIu16 "], res = %" PRIi32, group_start, group_end, res);
            NRF_LOG_HEXDUMP_ERROR(&buf[group_start], group_end - group_start);
         }

         /* Reset vars */
         group_start = 0;
         group_end = 0;
      }

      ++i;
   }

   return rc;
}

/**
 * @brief Handle of a data numeric value.
 */
#define TELEINFO_HANDLE_NUM_TAG_VALUE(VALUE_LEN, VALUE_STORE)              \
   /* Validate data value length */                                        \
   if (data_len != VALUE_LEN)                                              \
   {                                                                       \
      return TELEINFO_ERROR_VALUE_TOO_SHORT;                               \
   }                                                                       \
                                                                           \
   /* Validate data content */                                             \
   int32_t rc = teleinfo_check_value_is_numeric(buf, val_start, data_len); \
   if (rc != TELEINFO_SUCCESS)                                             \
   {                                                                       \
      return rc;                                                           \
   }                                                                       \
                                                                           \
   char *_ununsed;                                                         \
   VALUE_STORE = strtoul(val_buf, &_ununsed, 10);

int32_t teleinfo_parse_group(const uint8_t *buf, const uint16_t start, const uint16_t end, teleinfo_data_t *data)
{
   /* Group length is too short */
   if (end - start < (uint16_t)11)
   {
      return TELEINFO_ERROR_TOO_SHORT;
   }

   /* Compute checksum. Checksum = (S1 & 0x3F) + 0x20. */
   uint8_t checksum = 0;
   uint8_t recv_checksum = buf[end - 1];

   /* S1 part of the checksum. */
   for (uint16_t i = (start + 1); i < (end - 2); ++i)
   {
      checksum += buf[i];
   }

   /* Mask and add. */
   checksum = (checksum & 0x3f) + 0x20;

   /* Compare it to what we received. */
   if (checksum != recv_checksum)
   {
      NRF_LOG_ERROR("Bad checksum. calc=%" PRIu8 " recv=%" PRIu8);
      return TELEINFO_ERROR_BAD_CHECKSUM;
   }

   /* Get data tag */
   uint8_t tag_start = start + 1; // Start index.
   uint8_t tag_end = tag_start;   // Past end index.

   /* Find tag delimiter: SP char. */
   for (uint16_t i = tag_start; i < (end - 1); ++i)
   {
      if (buf[i] == (uint8_t)0x20)
      {
         tag_end = i;
         break;
      }
   }

   /* Check if we found the tag delimiter */
   if (tag_start == tag_end)
   {
      return TELEINFO_ERROR_GROUP_MALFORMED;
   }

   /* Take care of overflows */
   if ((tag_end - tag_start) >= TELEINFO_TAG_STR_BUF_LEN)
   {
      return TELEINFO_ERROR_NOMEM;
   }

   /* Retrieve tag string */
   char tag_buf[TELEINFO_TAG_STR_BUF_LEN] = {'\0'};
   memcpy(&tag_buf[0], &buf[tag_start], tag_end - tag_start);

   /* Get data value */
   uint16_t val_start = tag_end + 1; // Start index.
   uint16_t val_end = end - 2;       // Past end index.

   /* Check remaining data in buf does not under/overflow + data value delimiter char */
   if (val_start >= val_end || buf[val_end] != (uint8_t)0x20)
   {
      return TELEINFO_ERROR_GROUP_MALFORMED;
   }

   /* Get data length in frame */
   uint16_t data_len = val_end - val_start;

   /* Retrieve value string */
   char val_buf[TELEINFO_VALUE_STR_BUF_LEN] = {'\0'};
   memcpy(&val_buf[0], &buf[val_start], val_end - val_start);

   /* Tag specific stuff */
   if (strcmp(TELEINFO_TAG_BASE, tag_buf) == 0)
   {
      TELEINFO_HANDLE_NUM_TAG_VALUE(TELEINFO_TAG_BASE_VALUE_LEN, data->base);
      TELEINFO_DATA_SET_PRESENCE_BIT(data, TELEINFO_DATA_BASE_PRESENCE_MASK);
   }
   else if (strcmp(TELEINFO_TAG_HCHC, tag_buf) == 0)
   {
      TELEINFO_HANDLE_NUM_TAG_VALUE(TELEINFO_TAG_HCHC_VALUE_LEN, data->hchc);
      TELEINFO_DATA_SET_PRESENCE_BIT(data, TELEINFO_DATA_HCHC_PRESENCE_MASK);
   }
   else if (strcmp(TELEINFO_TAG_HCHP, tag_buf) == 0)
   {
      TELEINFO_HANDLE_NUM_TAG_VALUE(TELEINFO_TAG_HCHP_VALUE_LEN, data->hchp);
      TELEINFO_DATA_SET_PRESENCE_BIT(data, TELEINFO_DATA_HCHP_PRESENCE_MASK);
   }
   else if (strcmp(TELEINFO_TAG_IINST, tag_buf) == 0)
   {
      TELEINFO_HANDLE_NUM_TAG_VALUE(TELEINFO_TAG_IINST_VALUE_LEN, data->iinst);
      TELEINFO_DATA_SET_PRESENCE_BIT(data, TELEINFO_DATA_IINST_PRESENCE_MASK);
   }
   else if (strcmp(TELEINFO_TAG_PAPP, tag_buf) == 0)
   {
      TELEINFO_HANDLE_NUM_TAG_VALUE(TELEINFO_TAG_PAPP_VALUE_LEN, data->papp);
      TELEINFO_DATA_SET_PRESENCE_BIT(data, TELEINFO_DATA_PAPP_PRESENCE_MASK);
   }

   return TELEINFO_SUCCESS;
}

int32_t teleinfo_check_value_is_numeric(const uint8_t *buf, const uint16_t val_start, const uint16_t data_len)
{
   for (uint16_t i = val_start; i < data_len; ++i)
   {
      if (!isdigit(buf[i]))
      {
         return TELEINFO_ERROR_BAD_VALUE;
      }
   }

   return TELEINFO_SUCCESS;
}
