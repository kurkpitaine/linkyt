
#include "sdk_common.h"
#include "ble.h"
#include "ble_linkyt.h"
#include "ble_srv_common.h"

#include <inttypes.h>

#define NRF_LOG_MODULE_NAME ble_linkyt
#if BLE_LINKYT_CONFIG_LOG_ENABLED
// #define NRF_LOG_LEVEL BLE_LINKYT_CONFIG_LOG_LEVEL
// #define NRF_LOG_INFO_COLOR BLE_LINKYT_CONFIG_INFO_COLOR
// #define NRF_LOG_DEBUG_COLOR BLE_LINKYT_CONFIG_DEBUG_COLOR
#else // BLE_LINKYT_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL 0
#endif // BLE_LINKYT_CONFIG_LOG_ENABLED
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define BLE_UUID_LINKYT_BASE_CHARACTERISTIC 0x0002  /**< The UUID of the BASE Characteristic. */
#define BLE_UUID_LINKYT_HCHC_CHARACTERISTIC 0x0003  /**< The UUID of the HCHC Characteristic. */
#define BLE_UUID_LINKYT_HCHP_CHARACTERISTIC 0x0004  /**< The UUID of the HCHP Characteristic. */
#define BLE_UUID_LINKYT_IINST_CHARACTERISTIC 0x0005 /**< The UUID of the IINST Characteristic. */
#define BLE_UUID_LINKYT_PAPP_CHARACTERISTIC 0x0006  /**< The UUID of the PAPP Characteristic. */

#define BLE_LINKYT_MAX_RX_CHAR_LEN BLE_LINKYT_MAX_DATA_LEN /**< Maximum length of the RX Characteristic (in bytes). */
#define BLE_LINKYT_MAX_TX_CHAR_LEN BLE_LINKYT_MAX_DATA_LEN /**< Maximum length of the TX Characteristic (in bytes). */

#define LINKYT_BASE_UUID                                                                                \
   {                                                                                                    \
      {                                                                                                 \
         0x66, 0x65, 0x72, 0x72, 0x6f, 0x78, 0x79, 0x64, 0x65, 0x20, 0x6c, 0x69, 0x6e, 0x6b, 0x79, 0x74 \
      }                                                                                                 \
   } /**< Used vendor specific UUID. This is hex for "ferroxyde linkyt" */

static void on_write(ble_linkyt_t *p_linkyt, ble_evt_t const *p_ble_evt)
{
   ret_code_t err_code;
   ble_linkyt_client_context_t *p_client = NULL;

   ble_gatts_evt_write_t const *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
   err_code = blcm_link_ctx_get(p_linkyt->p_link_ctx_storage,
                                p_ble_evt->evt.gatts_evt.conn_handle,
                                (void *)&p_client);
   if (err_code != NRF_SUCCESS)
   {
      NRF_LOG_ERROR("Link context for 0x%02X connection handle could not be fetched. err_code=%" PRIu32,
                    p_evt_write->handle, err_code);
      return;
   }

   if (p_client == NULL)
   {
      NRF_LOG_ERROR("p_client is NULL.");
      return;
   }

   NRF_LOG_INFO("Write handle: %" PRIx16, p_evt_write->handle);

   if (p_evt_write->handle == p_linkyt->base_handles.cccd_handle)
   {
      NRF_LOG_INFO("p_evt_write->handle == p_linkyt->base_handles.cccd_handle");
      if (ble_srv_is_notification_enabled(p_evt_write->data))
      {
         NRF_LOG_INFO("Notification is enabled");
         p_client->is_charac_notif_enabled |= BLE_LINKY_CTX_BASE_NOTIF_EN_MASK;
      }
      else
      {
         NRF_LOG_INFO("Notification is disabled");
         p_client->is_charac_notif_enabled &= ~BLE_LINKY_CTX_BASE_NOTIF_EN_MASK;
      }
   }
   else if (p_evt_write->handle == p_linkyt->hchc_handles.cccd_handle)
   {
      NRF_LOG_INFO("p_evt_write->handle == p_linkyt->hchc_handles.cccd_handle");
      if (ble_srv_is_notification_enabled(p_evt_write->data))
      {
         NRF_LOG_INFO("Notification is enabled");
         p_client->is_charac_notif_enabled |= BLE_LINKY_CTX_HCHC_NOTIF_EN_MASK;
      }
      else
      {
         NRF_LOG_INFO("Notification is disabled");
         p_client->is_charac_notif_enabled &= ~BLE_LINKY_CTX_HCHC_NOTIF_EN_MASK;
      }
   }
   else if (p_evt_write->handle == p_linkyt->hchp_handles.cccd_handle)
   {
      NRF_LOG_INFO("p_evt_write->handle == p_linkyt->hchp_handles.cccd_handle");
      if (ble_srv_is_notification_enabled(p_evt_write->data))
      {
         NRF_LOG_INFO("Notification is enabled");
         p_client->is_charac_notif_enabled |= BLE_LINKY_CTX_HCHP_NOTIF_EN_MASK;
      }
      else
      {
         NRF_LOG_INFO("Notification is disabled");
         p_client->is_charac_notif_enabled &= ~BLE_LINKY_CTX_HCHP_NOTIF_EN_MASK;
      }
   }
   else if (p_evt_write->handle == p_linkyt->iinst_handles.cccd_handle)
   {
      NRF_LOG_INFO("p_evt_write->handle == p_linkyt->iinst_handles.cccd_handle");
      if (ble_srv_is_notification_enabled(p_evt_write->data))
      {
         NRF_LOG_INFO("Notification is enabled");
         p_client->is_charac_notif_enabled |= BLE_LINKY_CTX_IINST_NOTIF_EN_MASK;
      }
      else
      {
         NRF_LOG_INFO("Notification is disabled");
         p_client->is_charac_notif_enabled &= ~BLE_LINKY_CTX_IINST_NOTIF_EN_MASK;
      }
   }
   else if (p_evt_write->handle == p_linkyt->papp_handles.cccd_handle)
   {
      NRF_LOG_INFO("p_evt_write->handle == p_linkyt->papp_handles.cccd_handle");
      if (ble_srv_is_notification_enabled(p_evt_write->data))
      {
         NRF_LOG_INFO("Notification is enabled");
         p_client->is_charac_notif_enabled |= BLE_LINKY_CTX_PAPP_NOTIF_EN_MASK;
      }
      else
      {
         NRF_LOG_INFO("Notification is disabled");
         p_client->is_charac_notif_enabled &= ~BLE_LINKY_CTX_PAPP_NOTIF_EN_MASK;
      }
   }
}

void ble_linkyt_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context)
{
   if ((p_context == NULL) || (p_ble_evt == NULL))
   {
      return;
   }

   ble_linkyt_t *p_linkyt = (ble_linkyt_t *)p_context;

   switch (p_ble_evt->header.evt_id)
   {
   case BLE_GATTS_EVT_WRITE:
      on_write(p_linkyt, p_ble_evt);
      break;

   default:
      // No implementation needed.
      break;
   }
}

uint32_t ble_linkyt_init(ble_linkyt_t *p_linkyt)
{
   ret_code_t err_code;
   ble_uuid_t ble_uuid;
   ble_uuid128_t nus_base_uuid = LINKYT_BASE_UUID;
   ble_add_char_params_t add_char_params;

   VERIFY_PARAM_NOT_NULL(p_linkyt);

   // Add a custom base UUID.
   err_code = sd_ble_uuid_vs_add(&nus_base_uuid, &p_linkyt->uuid_type);
   NRF_LOG_INFO("sd_ble_uuid_vs_add: %lu", err_code);
   VERIFY_SUCCESS(err_code);

   ble_uuid.type = p_linkyt->uuid_type;
   ble_uuid.uuid = BLE_UUID_LINKYT_SERVICE;

   // Add the service.
   err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                       &ble_uuid,
                                       &p_linkyt->service_handle);
   NRF_LOG_INFO("sd_ble_gatts_service_add: %lu", err_code);
   VERIFY_SUCCESS(err_code);

   // Add the BASE Characteristic.
   memset(&add_char_params, 0, sizeof(add_char_params));
   add_char_params.uuid = BLE_UUID_LINKYT_BASE_CHARACTERISTIC;
   add_char_params.uuid_type = p_linkyt->uuid_type;
   add_char_params.max_len = BLE_LINKYT_MAX_TX_CHAR_LEN;
   add_char_params.init_len = TELEINFO_DATA_BASE_SIZE;
   add_char_params.is_var_len = true;
   add_char_params.char_props.notify = 1;

   add_char_params.read_access = SEC_OPEN;
   add_char_params.write_access = SEC_OPEN;
   add_char_params.cccd_write_access = SEC_OPEN;

   err_code = characteristic_add(p_linkyt->service_handle, &add_char_params, &p_linkyt->base_handles);

   if (err_code != NRF_SUCCESS)
   {
      NRF_LOG_ERROR("characteristic_add 1: %lu", err_code);
      return err_code;
   }

   // Add the HCHC Characteristic.
   memset(&add_char_params, 0, sizeof(add_char_params));
   add_char_params.uuid = BLE_UUID_LINKYT_HCHC_CHARACTERISTIC;
   add_char_params.uuid_type = p_linkyt->uuid_type;
   add_char_params.max_len = BLE_LINKYT_MAX_TX_CHAR_LEN;
   add_char_params.init_len = TELEINFO_DATA_HCHC_SIZE;
   add_char_params.is_var_len = true;
   add_char_params.char_props.notify = 1;

   add_char_params.read_access = SEC_OPEN;
   add_char_params.write_access = SEC_OPEN;
   add_char_params.cccd_write_access = SEC_OPEN;

   err_code = characteristic_add(p_linkyt->service_handle, &add_char_params, &p_linkyt->hchc_handles);

   if (err_code != NRF_SUCCESS)
   {
      NRF_LOG_ERROR("characteristic_add 2: %lu", err_code);
      return err_code;
   }

   // Add the HCHP Characteristic.
   memset(&add_char_params, 0, sizeof(add_char_params));
   add_char_params.uuid = BLE_UUID_LINKYT_HCHP_CHARACTERISTIC;
   add_char_params.uuid_type = p_linkyt->uuid_type;
   add_char_params.max_len = BLE_LINKYT_MAX_TX_CHAR_LEN;
   add_char_params.init_len = TELEINFO_DATA_HCHP_SIZE;
   add_char_params.is_var_len = true;
   add_char_params.char_props.notify = 1;

   add_char_params.read_access = SEC_OPEN;
   add_char_params.write_access = SEC_OPEN;
   add_char_params.cccd_write_access = SEC_OPEN;

   err_code = characteristic_add(p_linkyt->service_handle, &add_char_params, &p_linkyt->hchp_handles);

   if (err_code != NRF_SUCCESS)
   {
      NRF_LOG_ERROR("characteristic_add 3: %lu", err_code);
      return err_code;
   }

   // Add the IINST Characteristic.
   memset(&add_char_params, 0, sizeof(add_char_params));
   add_char_params.uuid = BLE_UUID_LINKYT_IINST_CHARACTERISTIC;
   add_char_params.uuid_type = p_linkyt->uuid_type;
   add_char_params.max_len = BLE_LINKYT_MAX_TX_CHAR_LEN;
   add_char_params.init_len = TELEINFO_DATA_IINST_SIZE;
   add_char_params.is_var_len = true;
   add_char_params.char_props.notify = 1;

   add_char_params.read_access = SEC_OPEN;
   add_char_params.write_access = SEC_OPEN;
   add_char_params.cccd_write_access = SEC_OPEN;

   err_code = characteristic_add(p_linkyt->service_handle, &add_char_params, &p_linkyt->iinst_handles);

   if (err_code != NRF_SUCCESS)
   {
      NRF_LOG_ERROR("characteristic_add 4: %lu", err_code);
      return err_code;
   }

   // Add the PAPP Characteristic.
   memset(&add_char_params, 0, sizeof(add_char_params));
   add_char_params.uuid = BLE_UUID_LINKYT_PAPP_CHARACTERISTIC;
   add_char_params.uuid_type = p_linkyt->uuid_type;
   add_char_params.max_len = BLE_LINKYT_MAX_TX_CHAR_LEN;
   add_char_params.init_len = TELEINFO_DATA_PAPP_SIZE;
   add_char_params.is_var_len = true;
   add_char_params.char_props.notify = 1;

   add_char_params.read_access = SEC_OPEN;
   add_char_params.write_access = SEC_OPEN;
   add_char_params.cccd_write_access = SEC_OPEN;

   return characteristic_add(p_linkyt->service_handle, &add_char_params, &p_linkyt->papp_handles);
}

uint32_t ble_linkyt_data_send(ble_linkyt_t *p_linkyt,
                              teleinfo_data_t *p_data,
                              uint16_t conn_handle)
{
   ret_code_t err_code = NRF_SUCCESS;
   ble_gatts_hvx_params_t hvx_params;
   ble_linkyt_client_context_t *p_client;

   VERIFY_PARAM_NOT_NULL(p_linkyt);

   err_code = blcm_link_ctx_get(p_linkyt->p_link_ctx_storage, conn_handle, (void *)&p_client);
   NRF_LOG_INFO("blcm_link_ctx_get conn_handle=%" PRIx16 " rc=%" PRIu32, conn_handle, err_code);
   VERIFY_SUCCESS(err_code);

   if ((conn_handle == BLE_CONN_HANDLE_INVALID) || (p_client == NULL))
   {
      NRF_LOG_ERROR("conn_handle: %u, p_client: %p", conn_handle, p_client);
      return NRF_ERROR_NOT_FOUND;
   }

   /* No notifications enabled at all */
   if (!p_client->is_charac_notif_enabled)
   {
      return NRF_ERROR_INVALID_STATE;
   }

   /* Check data is present */
   if (p_data->presence == 0)
   {
      return NRF_ERROR_INVALID_PARAM;
   }

   uint16_t len_val;

   /* Base data */
   if ((p_data->presence & TELEINFO_DATA_BASE_PRESENCE_MASK) != 0)
   {
      NRF_LOG_INFO("BASE data is present.");
      memset(&hvx_params, 0, sizeof(hvx_params));

      len_val = TELEINFO_DATA_BASE_SIZE;
      hvx_params.handle = p_linkyt->base_handles.value_handle;
      hvx_params.p_data = (uint8_t *)&p_data->base;
      hvx_params.p_len = &len_val;
      hvx_params.type = BLE_GATT_HVX_NOTIFICATION;

      err_code = sd_ble_gatts_hvx(conn_handle, &hvx_params);
   }
   if ((p_data->presence & TELEINFO_DATA_HCHC_PRESENCE_MASK) != 0)
   {
      NRF_LOG_INFO("HCHC data is present.");
      memset(&hvx_params, 0, sizeof(hvx_params));

      len_val = TELEINFO_DATA_HCHC_SIZE;
      hvx_params.handle = p_linkyt->hchc_handles.value_handle;
      hvx_params.p_data = (uint8_t *)&p_data->hchc;
      hvx_params.p_len = &len_val;
      hvx_params.type = BLE_GATT_HVX_NOTIFICATION;

      err_code = sd_ble_gatts_hvx(conn_handle, &hvx_params);
   }
   if ((p_data->presence & TELEINFO_DATA_HCHP_PRESENCE_MASK) != 0)
   {
      NRF_LOG_INFO("HCHP data is present.");
      memset(&hvx_params, 0, sizeof(hvx_params));

      len_val = TELEINFO_DATA_HCHP_SIZE;
      hvx_params.handle = p_linkyt->hchp_handles.value_handle;
      hvx_params.p_data = (uint8_t *)&p_data->hchp;
      hvx_params.p_len = &len_val;
      hvx_params.type = BLE_GATT_HVX_NOTIFICATION;

      err_code = sd_ble_gatts_hvx(conn_handle, &hvx_params);
   }
   if ((p_data->presence & TELEINFO_DATA_IINST_PRESENCE_MASK) != 0)
   {
      NRF_LOG_INFO("IINST data is present.");
      memset(&hvx_params, 0, sizeof(hvx_params));

      len_val = TELEINFO_DATA_IINST_SIZE;
      hvx_params.handle = p_linkyt->iinst_handles.value_handle;
      hvx_params.p_data = (uint8_t *)&p_data->iinst;
      hvx_params.p_len = &len_val;
      hvx_params.type = BLE_GATT_HVX_NOTIFICATION;

      err_code = sd_ble_gatts_hvx(conn_handle, &hvx_params);
   }
   if ((p_data->presence & TELEINFO_DATA_PAPP_PRESENCE_MASK) != 0)
   {
      NRF_LOG_INFO("PAPP data is present.");
      memset(&hvx_params, 0, sizeof(hvx_params));

      len_val = TELEINFO_DATA_PAPP_SIZE;
      hvx_params.handle = p_linkyt->papp_handles.value_handle;
      hvx_params.p_data = (uint8_t *)&p_data->papp;
      hvx_params.p_len = &len_val;
      hvx_params.type = BLE_GATT_HVX_NOTIFICATION;

      err_code = sd_ble_gatts_hvx(conn_handle, &hvx_params);
   }

   return err_code;
}
