#ifndef BLE_LINKYT_H
#define BLE_LINKYT_H

#include <stdint.h>
#include <stdbool.h>
#include "sdk_config.h"
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "ble_link_ctx_manager.h"
#include "teleinfo.h"

/**
 * @brief   Macro for defining a ble_linkyt instance.
 *
 * @param     _name            Name of the instance.
 * @param[in] _linkyt_max_clients Maximum number of Linkyt clients connected at a time.
 * @hideinitializer
 */
#define BLE_LINKYT_DEF(_name, _linkyt_max_clients)                      \
    BLE_LINK_CTX_MANAGER_DEF(CONCAT_2(_name, _link_ctx_storage),        \
                             (_linkyt_max_clients),                     \
                             sizeof(ble_linkyt_client_context_t));      \
    static ble_linkyt_t _name =                                         \
        {                                                               \
            .p_link_ctx_storage = &CONCAT_2(_name, _link_ctx_storage)}; \
    NRF_SDH_BLE_OBSERVER(_name##_obs,                                   \
                         BLE_NUS_BLE_OBSERVER_PRIO,                     \
                         ble_linkyt_on_ble_evt,                         \
                         &_name)

#define BLE_UUID_LINKYT_SERVICE 0x0001 /** The UUID of the Linkyt service. */

#define OPCODE_LENGTH 1
#define HANDLE_LENGTH 2

/**
 * @brief   Maximum length of data (in bytes) that can be transmitted to the peer by the Linkyt service module.
 */
#if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 0)
#define BLE_LINKYT_MAX_DATA_LEN (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH)
#else
#define BLE_LINKYT_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH)
#warning NRF_SDH_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif

/**
 * @brief Linkyt service event types.
 */
typedef enum
{
    BLE_LINKYT_EVT_TX_RDY,       /** Service is ready to accept new data to be transmitted. */
    BLE_LINKYT_EVT_COMM_STARTED, /** Notification has been enabled. */
    BLE_LINKYT_EVT_COMM_STOPPED, /** Notification has been disabled. */
} ble_linkyt_evt_type_t;

/* Forward declaration of the ble_linkyt_t type. */
typedef struct ble_linkyt_s ble_linkyt_t;

/**
 * @brief Linkyt service client context structure.
 *
 * @details This structure contains state context related to hosts.
 */
typedef struct
{
    uint32_t is_charac_notif_enabled; /** Bitset variable to indicate if the peer has enabled notification of the characteristic.*/
} ble_linkyt_client_context_t;

/* ble_linkyt_client_context_t notification enabled bitmasks. */
#define BLE_LINKY_CTX_BASE_NOTIF_EN_POS 0                                         /** Position of base notification enable bit */
#define BLE_LINKY_CTX_BASE_NOTIF_EN_MASK (1ul << BLE_LINKY_CTX_BASE_NOTIF_EN_POS) /** Bitmask of base notification enable bit */

#define BLE_LINKY_CTX_HCHC_NOTIF_EN_POS 1                                         /** Position of hchc notification enable bit */
#define BLE_LINKY_CTX_HCHC_NOTIF_EN_MASK (1ul << BLE_LINKY_CTX_HCHC_NOTIF_EN_POS) /** Bitmask of hchc notification enable bit */

#define BLE_LINKY_CTX_HCHP_NOTIF_EN_POS 2                                         /** Position of hchp notification enable bit */
#define BLE_LINKY_CTX_HCHP_NOTIF_EN_MASK (1ul << BLE_LINKY_CTX_HCHP_NOTIF_EN_POS) /** Bitmask of hchp notification enable bit */

#define BLE_LINKY_CTX_IINST_NOTIF_EN_POS 3                                          /** Position of iinst notification enable bit */
#define BLE_LINKY_CTX_IINST_NOTIF_EN_MASK (1ul << BLE_LINKY_CTX_IINST_NOTIF_EN_POS) /** Bitmask of iinst notification enable bit */

#define BLE_LINKY_CTX_PAPP_NOTIF_EN_POS 4                                         /** Position of papp notification enable bit */
#define BLE_LINKY_CTX_PAPP_NOTIF_EN_MASK (1ul << BLE_LINKY_CTX_PAPP_NOTIF_EN_POS) /** Bitmask of papp notification enable bit */

/**
 * @brief Linkyt service event structure.
 *
 * @details This structure is passed to an event coming from service.
 */
typedef struct
{
    ble_linkyt_evt_type_t type;              /** Event type. */
    ble_linkyt_t *p_linkyt;                  /** A pointer to the instance. */
    uint16_t conn_handle;                    /** Connection handle. */
    ble_linkyt_client_context_t *p_link_ctx; /** A pointer to the link context. */
} ble_linkyt_evt_t;

/**
 * @brief Linkyt service structure.
 *
 * @details This structure contains status information related to the service.
 */
struct ble_linkyt_s
{
    uint8_t uuid_type;                                 /** UUID type for Linkyt service Base UUID. */
    uint16_t service_handle;                           /** Handle of Linkyt service (as provided by the SoftDevice). */
    ble_gatts_char_handles_t base_handles;             /** Handles related to the BASE characteristic (as provided by the SoftDevice). */
    ble_gatts_char_handles_t hchc_handles;             /** Handles related to the HCHC characteristic (as provided by the SoftDevice). */
    ble_gatts_char_handles_t hchp_handles;             /** Handles related to the HCHP characteristic (as provided by the SoftDevice). */
    ble_gatts_char_handles_t iinst_handles;            /** Handles related to the IINST characteristic (as provided by the SoftDevice). */
    ble_gatts_char_handles_t papp_handles;             /** Handles related to the PAPP characteristic (as provided by the SoftDevice). */
    blcm_link_ctx_storage_t *const p_link_ctx_storage; /** Pointer to link context storage with handles of all current connections and its context. */
};

/**
 * @brief Function for initializing the Linkyt service.
 *
 * @param[out] p_linkyt   Linkyt service structure. This structure must be supplied
 *                        by the application. It is initialized by this function and will
 *                        later be used to identify this particular service instance.
 *
 * @retval NRF_SUCCESS If the service was successfully initialized. Otherwise, an error code is returned.
 * @retval NRF_ERROR_NULL If either of the pointers p_linkyt or p_linkyt_init is NULL.
 */
uint32_t ble_linkyt_init(ble_linkyt_t *p_linkyt);

/**
 * @brief Function for handling the Linkyt service's BLE events.
 *
 * @details The Linkyt service expects the application to call this function each time an
 * event is received from the SoftDevice. This function processes the event if it
 * is relevant and calls the Linkyt service event handler of the
 * application if necessary.
 *
 * @param[in] p_ble_evt     Event received from the SoftDevice.
 * @param[in] p_context     Linkyt service structure.
 */
void ble_linkyt_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context);

/**
 * @brief Function for sending a data to the peer.
 *
 * @details This function sends the input string as an RX characteristic notification to the
 *          peer.
 *
 * @param[in]     p_linkyt    Pointer to the Linkyt service structure.
 * @param[in]     p_data      String to be sent.
 * @param[in]     conn_handle Connection Handle of the destination client.
 *
 * @retval NRF_SUCCESS If the string was sent successfully. Otherwise, an error code is returned.
 */
uint32_t ble_linkyt_data_send(ble_linkyt_t *p_linkyt,
                              teleinfo_data_t *p_data,
                              uint16_t conn_handle);

#endif /* BLE_LINKYT_H */
