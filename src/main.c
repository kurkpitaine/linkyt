#include <zephyr/types.h>
#include <zephyr.h>
#include <drivers/uart.h>
#include <drivers/gpio.h>

#include <device.h>
#include <devicetree.h>
#include <soc.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>

#include <bluetooth/services/nus.h>

#include <settings/settings.h>

#include <stdio.h>

#include <logging/log.h>

#define LOG_MODULE_NAME peripheral_uart
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define STACKSIZE CONFIG_BT_NUS_THREAD_STACK_SIZE
#define PRIORITY 7

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

/* Leds config */
/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)

#define LED0 DT_GPIO_LABEL(LED0_NODE, gpios)
#define LED1 DT_GPIO_LABEL(LED1_NODE, gpios)

#define PIN_LED0 DT_GPIO_PIN(LED0_NODE, gpios)
#define PIN_LED1 DT_GPIO_PIN(LED1_NODE, gpios)

#define FLAGS_LED0 DT_GPIO_FLAGS(LED0_NODE, gpios)
#define FLAGS_LED1 DT_GPIO_FLAGS(LED1_NODE, gpios)

#define RUN_LED_BLINK_INTERVAL 1000

/* #define KEY_PASSKEY_ACCEPT DK_BTN1_MSK
#define KEY_PASSKEY_REJECT DK_BTN2_MSK */

#define UART_BUF_SIZE CONFIG_BT_NUS_UART_BUFFER_SIZE
#define UART_WAIT_FOR_BUF_DELAY K_MSEC(50)
#define UART_WAIT_FOR_RX CONFIG_BT_NUS_UART_RX_WAIT_TIME

#define NUS_BUF_SIZE UART_BUF_SIZE

static K_SEM_DEFINE(ble_init_ok, 0, 1);

static struct bt_conn *current_conn;
static struct bt_conn *auth_conn;

static const struct device *uart;
static const struct device *led0;
static const struct device *led1;
static struct k_delayed_work uart_work;

struct uart_data_t
{
   void *fifo_reserved;
   uint8_t data[UART_BUF_SIZE];
   uint16_t len;
};

static K_FIFO_DEFINE(fifo_uart_tx_data);
static K_FIFO_DEFINE(fifo_uart_rx_data);

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

/*
 *Called when data has been received from UART and allocated in memory
*/
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
   ARG_UNUSED(dev);

   static size_t aborted_len;
   struct uart_data_t *buf;
   static uint8_t *aborted_buf;
   static int uart_cnt = 0;

   switch (evt->type)
   {
   case UART_RX_RDY:
      //LOG_WRN("UART_RX_RDY");

      buf = CONTAINER_OF(evt->data.rx.buf, struct uart_data_t, data);
      buf->len += evt->data.rx.len;

      /* Store into FIFO if buffer is full */
      if (buf->len == UART_BUF_SIZE)
      {
         /* Remove parity bit from received octets */
         for (size_t i = 0; i < buf->len; ++i)
         {
            evt->data.rx.buf[i] &= 0x7f;
         }
         /* Copy and put it inside FIFO
         struct uart_data_t *buf_cpy = k_malloc(sizeof(*buf));
         memcpy(buf_cpy, buf, sizeof(*buf));
         k_fifo_put(&fifo_uart_rx_data, buf_cpy);*/
         k_fifo_put(&fifo_uart_rx_data, buf);
         uart_cnt++;
         LOG_INF("uart_cnt: %d", uart_cnt);
      }

      break;

   case UART_RX_DISABLED:
      //LOG_WRN("UART_RX_DISABLED");
      buf = k_malloc(sizeof(*buf));
      if (buf)
      {
         buf->len = 0;
      }
      else
      {
         LOG_WRN("Not able to allocate UART receive buffer 1");
         k_delayed_work_submit(&uart_work,
                               UART_WAIT_FOR_BUF_DELAY);
         return;
      }

      uart_rx_enable(uart, buf->data, sizeof(buf->data),
                     UART_WAIT_FOR_RX);

      break;

   case UART_RX_BUF_REQUEST:
      /*
      * Allocates memory to store data received from UART
      */
      //LOG_WRN("UART_RX_BUF_REQUEST");
      buf = k_malloc(sizeof(*buf));
      if (buf)
      {
         buf->len = 0;
         uart_rx_buf_rsp(uart, buf->data, sizeof(buf->data));
      }
      else
      {
         LOG_WRN("Not able to allocate UART receive buffer 2");
      }

      break;

   case UART_RX_BUF_RELEASED:
      //LOG_WRN("UART_RX_BUF_RELEASED");
      buf = CONTAINER_OF(evt->data.rx_buf.buf, struct uart_data_t,
                         data);

      //k_free(buf);

      break;

   case UART_RX_STOPPED:
      //LOG_WRN("UART_RX_STOPPED");
      break;

   case UART_TX_ABORTED:
      if (!aborted_buf)
      {
         aborted_buf = (uint8_t *)evt->data.tx.buf;
      }

      aborted_len += evt->data.tx.len;
      buf = CONTAINER_OF(aborted_buf, struct uart_data_t,
                         data);

      uart_tx(uart, &buf->data[aborted_len],
              buf->len - aborted_len, SYS_FOREVER_MS);

      break;

   case UART_TX_DONE:
      if ((evt->data.tx.len == 0) ||
          (!evt->data.tx.buf))
      {
         return;
      }

      if (aborted_buf)
      {
         buf = CONTAINER_OF(aborted_buf, struct uart_data_t,
                            data);
         aborted_buf = NULL;
         aborted_len = 0;
      }
      else
      {
         buf = CONTAINER_OF(evt->data.tx.buf, struct uart_data_t,
                            data);
      }

      k_free(buf);

      buf = k_fifo_get(&fifo_uart_tx_data, K_NO_WAIT);
      if (!buf)
      {
         return;
      }

      if (uart_tx(uart, buf->data, buf->len, SYS_FOREVER_MS))
      {
         LOG_WRN("Failed to send data over UART");
      }

      break;

   default:
      break;
   }
}

static void uart_work_handler(struct k_work *item)
{
   struct uart_data_t *buf;
   LOG_WRN("uart_work_handler");

   buf = k_malloc(sizeof(*buf));
   if (buf)
   {
      buf->len = 0;
   }
   else
   {
      LOG_WRN("Not able to allocate UART receive buffer 3");
      k_delayed_work_submit(&uart_work, UART_WAIT_FOR_BUF_DELAY);
      return;
   }

   uart_rx_enable(uart, buf->data, sizeof(buf->data), UART_WAIT_FOR_RX);
}

static int uart_init(void)
{
   int err;
   struct uart_data_t *rx;

   uart = device_get_binding(DT_LABEL(DT_NODELABEL(uart0)));
   if (!uart)
   {
      return -ENXIO;
   }

   struct uart_config uart_cfg = {
       .baudrate = 1200,
       .data_bits = UART_CFG_DATA_BITS_8,
       .flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
       .parity = UART_CFG_PARITY_NONE,
       .stop_bits = UART_CFG_STOP_BITS_1};

   /*    struct uart_config uart_cfg_init;
   uart_config_get(uart, &uart_cfg_init);
   LOG_INF("baudrate: %u", uart_cfg_init.baudrate);
   LOG_INF("parity: %u", uart_cfg_init.parity);
   LOG_INF("stop_bits: %u", uart_cfg_init.stop_bits);
   LOG_INF("data_bits: %u", uart_cfg_init.data_bits);
   LOG_INF("flow_ctrl: %u", uart_cfg_init.flow_ctrl); */

   /* Configure speed */
   err = uart_configure(uart, &uart_cfg);
   if (err)
   {
      LOG_ERR("Failed to configure UART (err %d)", err);
      return err;
   }

   rx = k_malloc(sizeof(*rx));
   if (rx)
   {
      rx->len = 0;
   }
   else
   {
      return -ENOMEM;
   }

   k_delayed_work_init(&uart_work, uart_work_handler);

   err = uart_callback_set(uart, uart_cb, NULL);
   if (err)
   {
      LOG_ERR("Failed to configure UART callback (err %d)", err);
      return err;
   }

   return uart_rx_enable(uart, rx->data, sizeof(rx->data), UART_WAIT_FOR_RX);
}

static void connected(struct bt_conn *conn, uint8_t err)
{
   char addr[BT_ADDR_LE_STR_LEN];

   if (err)
   {
      LOG_ERR("Connection failed (err %u)", err);
      return;
   }

   bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
   LOG_INF("Connected %s", log_strdup(addr));

   current_conn = bt_conn_ref(conn);

   gpio_pin_set(led1, PIN_LED1, 1);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
   char addr[BT_ADDR_LE_STR_LEN];

   bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

   LOG_INF("Disconnected: %s (reason %u)", log_strdup(addr), reason);

   if (auth_conn)
   {
      bt_conn_unref(auth_conn);
      auth_conn = NULL;
   }

   if (current_conn)
   {
      bt_conn_unref(current_conn);
      current_conn = NULL;
      gpio_pin_set(led1, PIN_LED1, 0);
   }
}

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void security_changed(struct bt_conn *conn, bt_security_t level,
                             enum bt_security_err err)
{
   char addr[BT_ADDR_LE_STR_LEN];

   bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

   if (!err)
   {
      LOG_INF("Security changed: %s level %u", log_strdup(addr),
              level);
   }
   else
   {
      LOG_WRN("Security failed: %s level %u err %d", log_strdup(addr),
              level, err);
   }
}
#endif

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
    .security_changed = security_changed,
#endif
};

#if defined(CONFIG_BT_NUS_SECURITY_ENABLED)
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
   char addr[BT_ADDR_LE_STR_LEN];

   bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

   LOG_INF("Passkey for %s: %06u", log_strdup(addr), passkey);
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
   char addr[BT_ADDR_LE_STR_LEN];

   auth_conn = bt_conn_ref(conn);

   bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

   LOG_INF("Passkey for %s: %06u", log_strdup(addr), passkey);
   LOG_INF("Press Button 1 to confirm, Button 2 to reject.");
}

static void auth_cancel(struct bt_conn *conn)
{
   char addr[BT_ADDR_LE_STR_LEN];

   bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

   LOG_INF("Pairing cancelled: %s", log_strdup(addr));
}

static void pairing_confirm(struct bt_conn *conn)
{
   char addr[BT_ADDR_LE_STR_LEN];

   bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

   bt_conn_auth_pairing_confirm(conn);

   LOG_INF("Pairing confirmed: %s", log_strdup(addr));
}

static void pairing_complete(struct bt_conn *conn, bool bonded)
{
   char addr[BT_ADDR_LE_STR_LEN];

   bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

   LOG_INF("Pairing completed: %s, bonded: %d", log_strdup(addr),
           bonded);
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
   char addr[BT_ADDR_LE_STR_LEN];

   bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

   LOG_INF("Pairing failed conn: %s, reason %d", log_strdup(addr),
           reason);
}

static struct bt_conn_auth_cb conn_auth_callbacks = {
    .passkey_display = auth_passkey_display,
    .passkey_confirm = auth_passkey_confirm,
    .cancel = auth_cancel,
    .pairing_confirm = pairing_confirm,
    .pairing_complete = pairing_complete,
    .pairing_failed = pairing_failed};
#else
static struct bt_conn_auth_cb conn_auth_callbacks;
#endif

static void bt_receive_cb(struct bt_conn *conn, const uint8_t *const data,
                          uint16_t len)
{
   int err;
   char addr[BT_ADDR_LE_STR_LEN] = {0};

   bt_addr_le_to_str(bt_conn_get_dst(conn), addr, ARRAY_SIZE(addr));

   LOG_INF("Received data from: %s", log_strdup(addr));

   for (uint16_t pos = 0; pos != len;)
   {
      struct uart_data_t *tx = k_malloc(sizeof(*tx));

      if (!tx)
      {
         LOG_WRN("Not able to allocate UART send data buffer");
         return;
      }

      /* Keep the last byte of TX buffer for potential LF char. */
      size_t tx_data_size = sizeof(tx->data) - 1;

      if ((len - pos) > tx_data_size)
      {
         tx->len = tx_data_size;
      }
      else
      {
         tx->len = (len - pos);
      }

      memcpy(tx->data, &data[pos], tx->len);

      pos += tx->len;

      /* Append the LF character when the CR character triggered
       * transmission from the peer.
       */
      if ((pos == len) && (data[len - 1] == '\r'))
      {
         tx->data[tx->len] = '\n';
         tx->len++;
      }

      err = uart_tx(uart, tx->data, tx->len, SYS_FOREVER_MS);
      if (err)
      {
         k_fifo_put(&fifo_uart_tx_data, tx);
      }
   }
}

static struct bt_nus_cb nus_cb = {
    .received = bt_receive_cb,
};

void error(void)
{
   gpio_pin_set(led0, PIN_LED0, 0);
   gpio_pin_set(led1, PIN_LED1, 0);

   while (true)
   {
      /* Spin for ever */
      k_sleep(K_MSEC(1000));
   }
}

/* static void num_comp_reply(bool accept)
{
   if (accept) {
      bt_conn_auth_passkey_confirm(auth_conn);
      LOG_INF("Numeric Match, conn %p", auth_conn);
   } else {
      bt_conn_auth_cancel(auth_conn);
      LOG_INF("Numeric Reject, conn %p", auth_conn);
   }

   bt_conn_unref(auth_conn);
   auth_conn = NULL;
}

void button_changed(uint32_t button_state, uint32_t has_changed)
{
   uint32_t buttons = button_state & has_changed;

   if (auth_conn) {
      if (buttons & KEY_PASSKEY_ACCEPT) {
         num_comp_reply(true);
      }

      if (buttons & KEY_PASSKEY_REJECT) {
         num_comp_reply(false);
      }
   }
} */

static int configure_gpio(void)
{
   int ret = 0;
   led0 = device_get_binding(LED0);
   led1 = device_get_binding(LED1);
   if (!led0 || !led1)
   {
      return -ENXIO;
   }

   ret = gpio_pin_configure(led0, PIN_LED0, GPIO_OUTPUT_ACTIVE | FLAGS_LED0);
   if (ret < 0)
   {
      return ret;
   }

   ret = gpio_pin_configure(led1, PIN_LED1, GPIO_OUTPUT_INACTIVE | FLAGS_LED1);
   if (ret < 0)
   {
      return ret;
   }
   return ret;
}

void main(void)
{
   int blink_status = 0;
   int err = 0;

   err = configure_gpio();
   if (err)
   {
      error();
   }

   err = uart_init();
   if (err)
   {
      error();
   }

   bt_conn_cb_register(&conn_callbacks);

   if (IS_ENABLED(CONFIG_BT_NUS_SECURITY_ENABLED))
   {
      bt_conn_auth_cb_register(&conn_auth_callbacks);
   }

   err = bt_enable(NULL);
   if (err)
   {
      error();
   }

   LOG_INF("Bluetooth initialized");

   k_sem_give(&ble_init_ok);

   if (IS_ENABLED(CONFIG_SETTINGS))
   {
      settings_load();
   }

   err = bt_nus_init(&nus_cb);
   if (err)
   {
      LOG_ERR("Failed to initialize UART service (err: %d)", err);
      return;
   }

   err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd,
                         ARRAY_SIZE(sd));
   if (err)
   {
      LOG_ERR("Advertising failed to start (err %d)", err);
   }

   printk("Starting Linky UART service\n");

   for (;;)
   {
      gpio_pin_set(led0, PIN_LED0, (++blink_status) % 2);
      k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
   }
}

void ble_write_thread(void)
{
   /* Buffer containing one line of data to send over BLE */
   static uint8_t data_line[NUS_BUF_SIZE] = {0};
   static size_t data_line_len = 0;
   static bool should_store = false;
   static int data_cnt = 0;

   /* Don't go any further until BLE is initialized */
   k_sem_take(&ble_init_ok, K_FOREVER);

   for (;;)
   {
      /* Wait indefinitely for data to be sent over bluetooth */
      struct uart_data_t *data_buf = k_fifo_get(&fifo_uart_rx_data,
                                                K_FOREVER);
      data_cnt++;
      LOG_INF("data_cnt: %d", data_cnt);

      LOG_WRN("ble_write_thread; %d", data_buf->len);

      /* Walk char by char */
      size_t i_char = 0;
      for (i_char = 0; i_char < data_buf->len; ++i_char)
      {
         LOG_INF("data_buf->data[%d]= %#02x | %c", i_char, data_buf->data[i_char], data_buf->data[i_char]);
         /* The STX or ETX char */
         if (data_buf->data[i_char] == 0x02 || data_buf->data[i_char] == 0x03)
         {
            int r = bt_nus_send(NULL, &data_buf->data[i_char], 1);
            if (r < 0)
            {
               LOG_WRN("Failed to send STX or ETX over BLE connection, err(%d)", r);
            }
         }
         /* The CR char */
         else if (data_buf->data[i_char] == 0x0d)
         {
            /* This is the end of a sentence, send it! */
            if (should_store && data_line_len > 2)
            {
               int r = bt_nus_send(NULL, data_line, data_line_len - 2);
               if (r < 0)
               {
                  LOG_WRN("Failed to send data over BLE connection, err(%d)", r);
               }
            }
            else
            {
               should_store = false;
               data_line_len = 0;
            }
         }
         /* The LF char */
         else if (data_buf->data[i_char] == 0x0a)
         {
            /* This is the start of a sentence, store next chars */
            data_line_len = 0;
            should_store = true;
         }
         else if (should_store)
         {
            /* Should we store this? */
            if (data_line_len < NUS_BUF_SIZE)
            {
               data_line[data_line_len] = data_buf->data[i_char];
               data_line_len++;
            }
            else
            {
               LOG_WRN("Sending buffer is full! Sentence is too long!");
               should_store = false;
            }
         }
      }

      k_free(data_buf);
   }
}

K_THREAD_DEFINE(ble_write_thread_id, STACKSIZE, ble_write_thread, NULL, NULL,
                NULL, PRIORITY, 0, 0);
