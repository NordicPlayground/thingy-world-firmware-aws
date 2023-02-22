/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef _MESSAGE_CHANNEL_H_
#define _MESSAGE_CHANNEL_H_

#include <zephyr/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

struct payload {
	char string[100];
};

enum network_status {
	NETWORK_DISCONNECTED,
	NETWORK_CONNECTED,
};

ZBUS_CHAN_DECLARE(TRIGGER_CHAN, PAYLOAD_CHAN, NETWORK_CHAN, LEDCTRL_CHAN);

#endif /* _MESSAGE_CHANNEL_H_ */
