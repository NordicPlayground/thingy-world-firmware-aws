/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "events/led_state_event.h"

/* This configuration file is included only once from led_state module and holds
 * information about LED effects associated with Asset Tracker v2 states.
 */

/* This structure enforces the header file to be included only once in the build.
 * Violating this requirement triggers a multiple definition error at link time.
 */
const struct {} led_state_def_include_once;

enum led_id {
	LED_ID_1,

	LED_ID_COUNT,

	LED_ID_CONNECTING = LED_ID_1,
	LED_ID_CLOUD_CONNECTING = LED_ID_1,
	LED_ID_SEARCHING = LED_ID_1,
	LED_ID_PUBLISHING = LED_ID_1,
	LED_ID_ASSOCIATING = LED_ID_1,
	LED_ID_ASSOCIATED = LED_ID_1,
	LED_ID_FOTA_1 = LED_ID_1,
	LED_ID_FOTA_2 = LED_ID_1,
	LED_ID_PASSIVE_MODE_1 = LED_ID_1,
	LED_ID_PASSIVE_MODE_2 = LED_ID_1,
	LED_ID_ACTIVE_MODE = LED_ID_1,

	LED_ID_LOC_MODE = LED_ID_1,
};

#define LED_PERIOD_NORMAL	350
#define LED_PERIOD_RAPID	100
#define LED_TICKS_DOUBLE	2
#define LED_TICKS_TRIPLE	3

static const struct led_effect asset_tracker_led_effect[] = {
	[LED_STATE_GNSS_SEARCHING]	= LED_EFFECT_LED_BREATH(LED_PERIOD_NORMAL,
								LED_COLOR(255, 0, 255)),
	[LED_STATE_LOC_MODE_SCELL]	 LED_EFFECT_LED_BREATH(LED_PERIOD_NORMAL,
								LED_COLOR(0, 0, 255)),
	[LED_STATE_LOC_MODE_MCELL]	 LED_EFFECT_LED_BREATH(LED_PERIOD_NORMAL,
								LED_COLOR(0, 255, 0)),
	[LED_STATE_LOC_MODE_ALL]	 LED_EFFECT_LED_BLINK_2(LED_PERIOD_NORMAL,
								LED_COLOR(255, 0, 255),
								LED_COLOR(0, 255, 0 )),
	[LED_STATE_TURN_OFF]		= LED_EFFECT_LED_OFF(),
};
