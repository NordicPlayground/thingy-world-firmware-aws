/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/random/rand32.h>
#include <hw_id.h>
#include <stdio.h>
#include <string.h>

int client_id_get(char *buffer, size_t buffer_size)
{
	int err;

	if (strlen(CONFIG_MQTT_SAMPLE_TRANSPORT_CLIENT_ID) > 0) {
		snprintk(buffer, buffer_size, "%s", CONFIG_MQTT_SAMPLE_TRANSPORT_CLIENT_ID);
		buffer[buffer_size - 1] = '0';
		return 0;
	}

	if (IS_ENABLED(CONFIG_BOARD_NATIVE_POSIX)) {
		snprintk(buffer, buffer_size, "%d", sys_rand32_get());
		buffer[buffer_size - 1] = '0';
		return 0;
	}

	err = hw_id_get(buffer, buffer_size);
	if (err) {
		return err;
	}

	return 0;
}
