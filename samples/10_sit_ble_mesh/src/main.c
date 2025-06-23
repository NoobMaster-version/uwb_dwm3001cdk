/**
 * Copyright (c) 2015 - Decawave Ltd, Dublin, Ireland.
 * Copyright (c) 2022 - Sven Hoyer
 * 
 * This is an Simple Bluetooth Low Energy Example for the 
 * DWM1001-DEV Board Zephyr
 */

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/bluetooth/bluetooth.h>

#include <sit_mesh/sit_mesh.h>
#include <sit_led/sit_led.h>

#define APP_NAME "SIMPLE MESH EXAMPLE\n"

/* size of stack area used by each thread */
#define STACKSIZE 2048

int main(void) {
	printk(APP_NAME);
	printk("==================\n");
	k_sleep(K_SECONDS(2));
	sit_mesh_init();
	sit_led_init();

	while(1) {
		if (sit_mesh_is_provisioned()) {
			sit_toggle_led(3);
		} else {
			sit_toggle_led(0);
		}
		k_sleep(K_SECONDS(1));
	}
	return 0;
}
