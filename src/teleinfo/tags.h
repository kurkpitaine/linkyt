#ifndef LINKYT_TAGS_H
#define LINKYT_TAGS_H

/**
 * This file contains the necessary tags and expected data length.
 */

#define TELEINFO_TAG_MAX_STR_LEN 8    /** Max tag length, excluding null termination */
#define TELEINFO_VALUE_MAX_STR_LEN 12 /** Max value length, excluding null termination */

#define TELEINFO_TAG_BASE "BASE"
#define TELEINFO_TAG_BASE_VALUE_LEN 9
#define TELEINFO_TAG_HCHC "HCHC"
#define TELEINFO_TAG_HCHC_VALUE_LEN 9
#define TELEINFO_TAG_HCHP "HCHP"
#define TELEINFO_TAG_HCHP_VALUE_LEN 9
#define TELEINFO_TAG_IINST "IINST"
#define TELEINFO_TAG_IINST_VALUE_LEN 3
#define TELEINFO_TAG_PAPP "PAPP"
#define TELEINFO_TAG_PAPP_VALUE_LEN 5

#endif /* LINKYT_TAGS_H */
