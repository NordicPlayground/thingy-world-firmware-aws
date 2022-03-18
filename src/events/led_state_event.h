/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef _LED_STATE_EVENT_H_
#define _LED_STATE_EVENT_H_

/**
 * @brief Led State Event
 * @defgroup asset Tracker Led State Event
 * @{
 */
#include <caf/led_effect.h>
#include <event_manager.h>
#include <event_manager_profiler_tracer.h>

/** Create multi-color LED blinking effect initializer.
 *
 * LED color is periodically changed between the selected colors.
 *
 * @param _period	Period of time between LED color switches.
 * @param _color	Selected LED color 1.
 * @param _color	Selected LED color 2.
 */
#define LED_EFFECT_LED_BLINK_2(_period, _color1, _color2)			\
	{									\
		.steps = ((const struct led_effect_step[]) {			\
			{							\
				.color = _color1,				\
				.substep_count = 1,				\
				.substep_time = (_period),			\
			},							\
			{							\
				.color = _color2,				\
				.substep_count = 1,				\
				.substep_time = (_period),			\
			},							\
		}),								\
		.step_count = 2,						\
		.loop_forever = true,						\
	}

/** @brief Asset Tracker led states in the application. */
enum led_state {

	LED_STATE_GNSS_SEARCHING,
	LED_STATE_LOC_MODE_SCELL,
	LED_STATE_LOC_MODE_MCELL,
	LED_STATE_LOC_MODE_ALL,

	LED_STATE_TURN_OFF,

	LED_STATE_COUNT,

	LED_STATE_LTE_CONNECTING,
	LED_STATE_CLOUD_PUBLISHING,
	LED_STATE_CLOUD_CONNECTING,
	LED_STATE_CLOUD_ASSOCIATING,
	LED_STATE_CLOUD_ASSOCIATED,
	LED_STATE_ACTIVE_MODE,
	LED_STATE_PASSIVE_MODE,
	LED_STATE_ERROR_SYSTEM_FAULT,
	LED_STATE_FOTA_UPDATING,
	LED_STATE_FOTA_UPDATE_REBOOT,
};

/** @brief Led state event. */
struct led_state_event {
	struct event_header header; /**< Event header. */

	enum led_state state;
};

EVENT_TYPE_DECLARE(led_state_event);

/** @} */

#endif /* _LED_STATE_EVENT_H_ */
