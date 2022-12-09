#ifndef LINKYT_TELEINFO_H
#define LINKYT_TELEINFO_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Teleinfo frame parser result.
 */
typedef enum
{
   TELEINFO_SUCCESS = 0,               /** Parsing success */
   TELEINFO_ERROR = 1,                 /** Parsing error */
   TELEINFO_ERROR_TOO_SHORT = 2,       /** Frame/group too short */
   TELEINFO_ERROR_BAD_CHECKSUM = 3,    /** Bad group checksum */
   TELEINFO_ERROR_GROUP_MALFORMED = 4, /** Malformed group, delimiter(s) not found */
   TELEINFO_ERROR_TAG_UNKOWN = 5,      /** Tag name is unknown */
   TELEINFO_ERROR_BAD_VALUE = 6,       /** Unexpected value data */
   TELEINFO_ERROR_VALUE_TOO_SHORT = 7, /** Value string length is too short */
   TELEINFO_ERROR_NOMEM = 8            /** Not enough memory */
} teleinfo_parse_result_t;

/**
 * @brief Contains parsed Teleinfo data.
 */
typedef struct
{
   uint32_t base;     /** Base option index in Wh unit */
   uint32_t hchc;     /** Heures creuses in Wh unit */
   uint32_t hchp;     /** Heures pleines in Wh unit */
   uint16_t iinst;    /** Instant current in A unit */
   uint32_t papp;     /** Power in VA unit */
   uint32_t presence; /** Value presence bitset. Use masks to read/set the bits */
} teleinfo_data_t;

/* teleinfo_data_t data fields size. */
#define TELEINFO_DATA_BASE_SIZE sizeof(uint32_t)  /** Size of teleinfo_data_t base field*/
#define TELEINFO_DATA_HCHC_SIZE sizeof(uint32_t)  /** Size of teleinfo_data_t hchc field*/
#define TELEINFO_DATA_HCHP_SIZE sizeof(uint32_t)  /** Size of teleinfo_data_t hchp field*/
#define TELEINFO_DATA_IINST_SIZE sizeof(uint16_t) /** Size of teleinfo_data_t iinst field*/
#define TELEINFO_DATA_PAPP_SIZE sizeof(uint32_t)  /** Size of teleinfo_data_t papp field*/

/* teleinfo_data_t presence bitmasks. */
#define TELEINFO_DATA_BASE_PRESENCE_POS 0                                       /** Position of base presence bit */
#define TELEINFO_DATA_BASE_PRESENCE_MASK (1 << TELEINFO_DATA_BASE_PRESENCE_POS) /** Bitmask of base presence bit */

#define TELEINFO_DATA_HCHC_PRESENCE_POS 1                                       /** Position of hchc presence bit */
#define TELEINFO_DATA_HCHC_PRESENCE_MASK (1 << TELEINFO_DATA_HCHC_PRESENCE_POS) /** Bitmask of hchc presence bit */

#define TELEINFO_DATA_HCHP_PRESENCE_POS 2                                       /** Position of hchp presence bit */
#define TELEINFO_DATA_HCHP_PRESENCE_MASK (1 << TELEINFO_DATA_HCHP_PRESENCE_POS) /** Bitmask of hchp presence bit */

#define TELEINFO_DATA_IINST_PRESENCE_POS 3                                        /** Position of iinst presence bit */
#define TELEINFO_DATA_IINST_PRESENCE_MASK (1 << TELEINFO_DATA_IINST_PRESENCE_POS) /** Bitmask of iinst presence bit */

#define TELEINFO_DATA_PAPP_PRESENCE_POS 4                                       /** Position of papp presence bit */
#define TELEINFO_DATA_PAPP_PRESENCE_MASK (1 << TELEINFO_DATA_PAPP_PRESENCE_POS) /** Bitmask of papp presence bit */

/**
 * @brief Set the presence bit in teleinfo_data_t data for field FIELD_MASK.
 */
#define TELEINFO_DATA_SET_PRESENCE_BIT(DATA, FIELD_MASK) \
   DATA->presence |= FIELD_MASK;

/**
 * @brief Get the presence bit in teleinfo_data_t data for field FIELD_MASK.
 */
#define TELEINFO_DATA_GET_PRESENCE_BIT(DATA, FIELD_MASK) \
   DATA->presence &FIELD_MASK != 0;

/**
 * @brief Parse a Teleinfo frame.
 * @warning This function will return TELEINFO_SUCCESS if at least one sub-group has been successfully parsed.
 *
 * @param buf pointer to the start of the buffer.
 * @param buf_len buffer length.
 * @param data pointer to a teleinfo_data_t to fill.
 *
 * @return TELEINFO_SUCCESS on parsing success.
 * @return TELEINFO_ERROR on parsing error. No valid sub-group found.
 */
int32_t teleinfo_parse_frame(const uint8_t *buf, const uint16_t buf_len, teleinfo_data_t *data);

/**
 * @brief Parse a Teleinfo group contained in a frame. Value is set in data only if return code is TELEINFO_SUCCESS.
 *
 * @param buf pointer to the buffer.
 * @param start index of the group start delimiter in the buffer.
 * @param end index of the group end delimiter in the buffer.
 * @param data pointer to the struct where to store the extracted data value.
 *
 * @return TELEINFO_SUCCESS on parsing success.
 * @return TELEINFO_ERROR_TOO_SHORT if group length is too short.
 * @return TELEINFO_ERROR_BAD_CHECKSUM if checksum verification does not pass.
 * @return TELEINFO_ERROR_GROUP_MALFORMED if group structure is malformed.
 * @return TELEINFO_ERROR_NOMEM if group size is too big to fit into memory.
 * @return TELEINFO_ERROR_TAG_UNKOWN if group contains an unknown/unmanaged tag.
 * @return TELEINFO_ERROR_VALUE_TOO_SHORT if group data value for a tag does not match specification length.
 */
int32_t teleinfo_parse_group(const uint8_t *buf, const uint16_t start, const uint16_t end, teleinfo_data_t *data);

/**
 * @brief Checks the content of the buffer, at val_start for length data_len contains numerical characters.
 *
 * @param buf pointer to the buffer.
 * @param val_start index inside the buffer where to start the check.
 * @param data_len length of data in the buffer to check.
 *
 * @return TELEINFO_SUCCESS if value is numeric, TELEINFO_ERROR_BAD_VALUE if not.
 */
int32_t teleinfo_check_value_is_numeric(const uint8_t *buf, const uint16_t val_start, const uint16_t data_len);

#endif /* LINKYT_TELEINFO_H */
