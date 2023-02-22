/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <modem/pdn.h>
#include <modem/lte_lc.h>
#include <nrf_modem.h>

#include "message_channel.h"

/* Register log module */
LOG_MODULE_REGISTER(network, CONFIG_MQTT_SAMPLE_NETWORK_LOG_LEVEL);

/* This module does not subscribe to any channels */

/* Handler that is called when the modem hard faults. */
void nrf_modem_fault_handler(struct nrf_modem_fault_info *fault_info)
{
	__ASSERT(false, "Modem error: 0x%x, PC: 0x%x", fault_info->reason,
						       fault_info->program_counter);
}

/* Handler that is used to notify the application about LTE link specific events. */
static void lte_event_handler(const struct lte_lc_evt *const evt)
{
	if ((evt->type == LTE_LC_EVT_MODEM_EVENT) &&
	    (evt->modem_evt == LTE_LC_MODEM_EVT_RESET_LOOP)) {
		LOG_WRN("The modem has detected a reset loop.");
		LOG_WRN("LTE network attach is now "
			"restricted for the next 30 minutes.");
		LOG_WRN("Power-cycle the device to "
			"circumvent this restriction.");
		LOG_WRN("For more information see the nRF91 AT Commands - Command "
			"Reference Guide v2.0 - chpt. 5.36");
	}
}

/* Handler that notifies the application of events related to the default PDN context, CID 0. */
void pdn_event_handler(uint8_t cid, enum pdn_event event, int reason)
{
	ARG_UNUSED(cid);

	enum network_status status;

	switch (event) {
	case PDN_EVENT_CNEC_ESM:
		LOG_DBG("Event: PDP context %d, %s", cid, pdn_esm_strerror(reason));
		return;
	case PDN_EVENT_ACTIVATED:
		LOG_INF("PDN connection activated, IPv4 up");
		status = NETWORK_CONNECTED;
		break;
	case PDN_EVENT_DEACTIVATED:
		LOG_INF("PDN connection deactivated");
		status = NETWORK_DISCONNECTED;
		break;
	case PDN_EVENT_IPV6_UP:
		LOG_DBG("PDN_EVENT_IPV6_UP");
		return;
	case PDN_EVENT_IPV6_DOWN:
		LOG_DBG("PDN_EVENT_IPV6_DOWN");
		return;
	default:
		LOG_ERR("Unexpected PDN event!");
		return;
	}

	__ASSERT_NO_MSG(zbus_chan_pub(&NETWORK_CHAN, &status, K_SECONDS(1)) == 0);
}

static void network_task(void)
{
	int ret;
	/* Setup a callback for the default PDP context. */
	__ASSERT_NO_MSG(pdn_default_ctx_cb_reg(pdn_event_handler) == 0);

	/* Register handler to receive LTE link specific events. */
	lte_lc_register_handler(lte_event_handler);

	/* Disable PSM */
	ret = lte_lc_psm_req(false);
	if (ret) {
		LOG_ERR("lte_lc_psm_req failed: %d", ret);
	}

	/* Disable eDRX */
	ret = lte_lc_edrx_req(false);
	if (ret) {
		LOG_ERR("lte_lc_edrx_req failed: %d", ret);
	}

	/* Subscribe to modem domain events (AT%MDEV).
	 * Modem domain events is received in the lte_event_handler().
	 *
	 * This function fails for modem firmware versions < 1.3.0 due to not being supported.
	 * Therefore we ignore its return value.
	 */
	(void)lte_lc_modem_events_enable();

	LOG_INF("Connecting to LTE...");

	/* Initialize the link controller and connect to LTE network. */
	__ASSERT_NO_MSG(lte_lc_init_and_connect() == 0);
}

K_THREAD_DEFINE(network_task_id,
		CONFIG_MQTT_SAMPLE_NETWORK_THREAD_STACK_SIZE,
		network_task, NULL, NULL, NULL, 3, 0, 0);
