/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <caf/events/led_event.h>
#include "events/led_state_event.h"
#include CONFIG_LED_STATE_DEF_PATH

#define MODULE led_state
#include <caf/events/module_state_event.h>
#include <logging/log.h>
LOG_MODULE_REGISTER(led, CONFIG_LED_CONTROL_LOG_LEVEL);


static void send_led_event(size_t led_id, const struct led_effect *led_effect)
{
	__ASSERT_NO_MSG(led_effect);
	__ASSERT_NO_MSG(led_id < LED_ID_COUNT);

	struct led_event *event = new_led_event();

	event->led_id = led_id;
	event->led_effect = led_effect;
	EVENT_SUBMIT(event);
}

static void update_led(enum led_state state)
{
	uint8_t led_bm = 0;

	switch (state)	{
	case LED_STATE_LTE_CONNECTING:
	case LED_STATE_CLOUD_PUBLISHING:
	case LED_STATE_CLOUD_CONNECTING:
	case LED_STATE_CLOUD_ASSOCIATING:
	case LED_STATE_CLOUD_ASSOCIATED:
	case LED_STATE_ACTIVE_MODE:
	case LED_STATE_PASSIVE_MODE:
	case LED_STATE_ERROR_SYSTEM_FAULT:
	case LED_STATE_FOTA_UPDATING:
	case LED_STATE_FOTA_UPDATE_REBOOT:
		/* For Thingyworld, these are don't care LED states */
		return;
		break;
	case LED_STATE_GNSS_SEARCHING:
		send_led_event(LED_ID_LOC_MODE,
				&asset_tracker_led_effect[LED_STATE_GNSS_SEARCHING]);
		led_bm |= BIT(LED_ID_LOC_MODE);
		break;
	case LED_STATE_LOC_MODE_MCELL:
		send_led_event(LED_ID_LOC_MODE,
				&asset_tracker_led_effect[LED_STATE_LOC_MODE_MCELL]);
		led_bm |= BIT(LED_ID_LOC_MODE);
		break;
	case LED_STATE_LOC_MODE_ALL:
		send_led_event(LED_ID_LOC_MODE,
				&asset_tracker_led_effect[LED_STATE_LOC_MODE_ALL]);
		led_bm |= BIT(LED_ID_LOC_MODE);
		break;
	case LED_STATE_LOC_MODE_SCELL:
		send_led_event(LED_ID_LOC_MODE,
				&asset_tracker_led_effect[LED_STATE_LOC_MODE_SCELL]);
		led_bm |= BIT(LED_ID_LOC_MODE);
		break;
	case LED_STATE_TURN_OFF:
		send_led_event(LED_ID_LOC_MODE,
				&asset_tracker_led_effect[LED_STATE_TURN_OFF]);
		break;
	default:
		LOG_WRN("Unrecognized LED state event send");
		break;
	}
}

static bool handle_led_state_event(const struct led_state_event *event)
{
	update_led(event->state);
	return false;
}

static bool event_handler(const struct event_header *eh)
{
	if (is_led_state_event(eh)) {
		return handle_led_state_event(cast_led_state_event(eh));
	}

	/* If event is unhandled, unsubscribe. */
	__ASSERT_NO_MSG(false);

	return false;
}

EVENT_LISTENER(MODULE, event_handler);
EVENT_SUBSCRIBE(MODULE, led_state_event);
