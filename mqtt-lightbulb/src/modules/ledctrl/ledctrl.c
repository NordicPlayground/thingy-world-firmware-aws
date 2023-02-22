/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/drivers/pwm.h>


#include "stdio.h"

#include "message_channel.h"


/* Register log module */
LOG_MODULE_REGISTER(ledctrl, CONFIG_MQTT_SAMPLE_TRIGGER_LOG_LEVEL);

static const struct pwm_dt_spec led_red = PWM_DT_SPEC_GET(DT_ALIAS(led_red));
static const struct pwm_dt_spec led_green = PWM_DT_SPEC_GET(DT_ALIAS(led_green));
static const struct pwm_dt_spec led_blue = PWM_DT_SPEC_GET(DT_ALIAS(led_blue));

const char *on_commands[] = {
	"on",
	"ON",
	"1"
};

const char *off_commands[] = {
	"off",
	"OFF",
	"0"
};

void ledctrl_callback(const struct zbus_channel *chan)
{
	const struct payload *payload;
	uint8_t r = 0;
	uint8_t b = 0;
	uint8_t g = 0;
	int ret;
	bool parse_successful = false;

	if (&LEDCTRL_CHAN == chan) {
		/* Get command string from channel */
		payload = zbus_chan_const_msg(chan);

		/* command strings must be NULL-terminated */
		__ASSERT_NO_MSG(payload->string[ARRAY_SIZE(payload->string)-1] == 0);

		/* Parse on command */
		for (size_t i = 0; i < ARRAY_SIZE(on_commands); ++i) {
			if (strncmp(
			    on_commands[i], payload->string, ARRAY_SIZE(payload->string)) == 0) {
				r = b = g = 255;
				parse_successful = true;
			}
		}

		/* Parse off command */
		for (size_t i = 0; i < ARRAY_SIZE(off_commands); ++i) {
			if (strncmp(
			    off_commands[i], payload->string, ARRAY_SIZE(payload->string)) == 0) {
				r = b = g = 0;
				parse_successful = true;
			}
		}

		/* Parse r,g,b command */
		if (sscanf(payload->string, "%hhd, %hhd, %hhd", &r, &g, &b) == 3) {
			parse_successful = true;
		}

		/* Apply Brightness settings */
		ret = pwm_set_pulse_dt(&led_red, led_red.period * r/255);
		if (ret) {
			LOG_ERR("Error %d: failed to set pulse width", ret);
			return;
		}
		ret = pwm_set_pulse_dt(&led_green, led_green.period * b/255);
		if (ret) {
			LOG_ERR("Error %d: failed to set pulse width", ret);
			return;
		}
		ret = pwm_set_pulse_dt(&led_blue, led_blue.period * g/255);
		if (ret) {
			LOG_ERR("Error %d: failed to set pulse width", ret);
			return;
		}
	}
}

ZBUS_LISTENER_DEFINE(ledctrl, ledctrl_callback);

static void ledctrl_task(void)
{
	__ASSERT_NO_MSG(device_is_ready(led_red.dev));
	__ASSERT_NO_MSG(device_is_ready(led_green.dev));
	__ASSERT_NO_MSG(device_is_ready(led_blue.dev));
}
K_THREAD_DEFINE(ledctrl_task_id,
		CONFIG_MQTT_SAMPLE_LEDCTRL_THREAD_STACK_SIZE,
		ledctrl_task, NULL, NULL, NULL, 3, 0, 0);
