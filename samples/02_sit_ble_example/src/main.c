/*
 * Copyright (c) 2023
 *
 * Simple BLE Example for DWM3001 Board
 */
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/printk.h>

#include <sit_led/sit_led.h>

#define APP_NAME "SIMPLE BLE EXAMPLE\n"

// Custom service UUID - randomly generated
#define DWM_SERVICE_UUID BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0))

// Custom characteristic UUID - randomly generated
#define DWM_CHAR_UUID    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x87654321, 0x4321, 0x8765, 0x4321, 0x56789abcdef0))

static struct bt_conn *current_conn;

// Characteristic value
static uint8_t char_value = 0;

// Callback when device is connected
static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        printk("Connection failed (err 0x%02x)\n", err);
        return;
    }

    current_conn = bt_conn_ref(conn);
    printk("Connected\n");
    sit_set_led(1, 1);  // Turn on LED1 when connected
}

// Callback when device is disconnected
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    printk("Disconnected (reason 0x%02x)\n", reason);

    if (current_conn) {
        bt_conn_unref(current_conn);
        current_conn = NULL;
    }

    sit_set_led(1, 0);  // Turn off LED1 when disconnected
}

// Connection callbacks structure
BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};

// Read callback for our characteristic
static ssize_t read_char(struct bt_conn *conn, const struct bt_gatt_attr *attr,
            void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &char_value,
                sizeof(char_value));
}

// Write callback for our characteristic
static ssize_t write_char(struct bt_conn *conn, const struct bt_gatt_attr *attr,
             const void *buf, uint16_t len, uint16_t offset,
             uint8_t flags)
{
    if (offset + len > sizeof(char_value)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memcpy(&char_value + offset, buf, len);
    
    // Toggle LED2 when value is written
    sit_toggle_led(2);
    
    return len;
}

// Define GATT service
BT_GATT_SERVICE_DEFINE(dwm_svc,
    BT_GATT_PRIMARY_SERVICE(DWM_SERVICE_UUID),
    BT_GATT_CHARACTERISTIC(DWM_CHAR_UUID,
                BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
                BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                read_char, write_char, NULL),
);

// Advertising data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, 
        0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
        0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12),
};

int main(void)
{
    int err;

    printk(APP_NAME);
    printk("==================\n");

    // Initialize LED subsystem
    sit_led_init();
    
    // Initialize BLE stack
    err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return 0;
    }

    printk("Bluetooth initialized\n");

    // Start advertising
    err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
        return 0;
    }

    printk("Advertising successfully started\n");

    while (1) {
        k_sleep(K_MSEC(1000));
        if (!current_conn) {
            sit_toggle_led(0);  // Blink LED0 when not connected
        }
    }

    return 0;
}