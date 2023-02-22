/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_event.h>

#include "message_channel.h"

/* Register log module */
LOG_MODULE_REGISTER(network, CONFIG_MQTT_SAMPLE_NETWORK_LOG_LEVEL);

/* This module does not subscribe to any channels */

BUILD_ASSERT((sizeof(CONFIG_MQTT_SAMPLE_NETWORK_WIFI_SSID) > 1), "SSID must be set");
BUILD_ASSERT((sizeof(CONFIG_MQTT_SAMPLE_NETWORK_WIFI_PSK) > 1), "PSK must be set");

#define MGMT_EVENTS (NET_EVENT_WIFI_SCAN_RESULT		| \
		     NET_EVENT_WIFI_SCAN_DONE		| \
		     NET_EVENT_WIFI_CONNECT_RESULT	| \
		     NET_EVENT_WIFI_DISCONNECT_RESULT)

static struct net_mgmt_event_callback net_mgmt_callback;
extern char *net_sprint_ll_addr_buf(const uint8_t *ll, uint8_t ll_len, char *buf, int buflen);

static void scan(void)
{
	struct net_if *iface = net_if_get_default();

	__ASSERT(iface, "Returned network interface is NULL");

	LOG_INF("Scanning for Wi-Fi networks...");

	if (net_mgmt(NET_REQUEST_WIFI_SCAN, iface, NULL, 0)) {
		LOG_ERR("Wi-Fi scan failed");
		return;
	}
}

static void connect(void)
{
	struct net_if *iface = net_if_get_default();

	__ASSERT(iface, "Returned network interface is NULL");

	LOG_INF("Connecting to SSID: %s", CONFIG_MQTT_SAMPLE_NETWORK_WIFI_SSID);

	static struct wifi_connect_req_params cnx_params = {
		.ssid = CONFIG_MQTT_SAMPLE_NETWORK_WIFI_SSID,
		.ssid_length = sizeof(CONFIG_MQTT_SAMPLE_NETWORK_WIFI_SSID) - 1,
		.psk = CONFIG_MQTT_SAMPLE_NETWORK_WIFI_PSK,
		.psk_length = sizeof(CONFIG_MQTT_SAMPLE_NETWORK_WIFI_PSK) - 1,
		.channel = WIFI_CHANNEL_ANY,
		.security = WIFI_SECURITY_TYPE_PSK,
		.timeout = CONFIG_MQTT_SAMPLE_NETWORK_WIFI_CONNECTION_REQUEST_TIMEOUT_SECONDS,
	};

	if (net_mgmt(NET_REQUEST_WIFI_CONNECT, iface,
		     &cnx_params, sizeof(struct wifi_connect_req_params))) {
		LOG_ERR("Connecting to Wi-Fi failed");
	}
}

static void scan_result_handle(struct net_mgmt_event_callback *cb, uint32_t result_index)
{
	const struct wifi_scan_result *entry = (const struct wifi_scan_result *)cb->info;
	uint8_t mac_buf[sizeof("xx:xx:xx:xx:xx:xx")];
	const char *mac_ptr = (entry->mac_length) ? net_sprint_ll_addr_buf(entry->mac,
									   WIFI_MAC_ADDR_LEN,
									   mac_buf,
									   sizeof(mac_buf)) : "";

	if (result_index == 1U) {
		LOG_INF("%-4s | %-32s %-5s | %-13s | %-4s | %-15s | %s",
			"Num", "SSID", "(len)", "Chan (Band)", "RSSI", "Security", "BSSID");
	}

	LOG_INF("%-4d | %-32s %-5u | %-4u (%-6s) | %-4d | %-15s | %s",
		result_index, entry->ssid, entry->ssid_length, entry->channel,
		wifi_band_txt(entry->band),
		entry->rssi,
		wifi_security_txt(entry->security),
		mac_ptr);
}

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
				    uint32_t mgmt_event, struct net_if *iface)
{
	enum network_status status;
	const struct wifi_status *wifi_status = (const struct wifi_status *)cb->info;
	/* Variable incremented for each new scan result. */
	static uint32_t scan_result_index;

	switch (mgmt_event) {
	case NET_EVENT_WIFI_SCAN_RESULT:
		scan_result_index++;
		scan_result_handle(cb, scan_result_index);
		return;
	case NET_EVENT_WIFI_SCAN_DONE:
		if (wifi_status->status) {
			LOG_INF("Scan failed, status code: %d", wifi_status->status);
			return;
		}

		LOG_INF("Scan finished");

		scan_result_index = 0U;

		connect();
		return;
	case NET_EVENT_WIFI_CONNECT_RESULT:
		if (wifi_status->status) {
			LOG_INF("Connection attempt failed, status code: %d", wifi_status->status);
			return;
		}

		LOG_INF("Wi-Fi Connected");

		status = NETWORK_CONNECTED;
		break;
	case NET_EVENT_WIFI_DISCONNECT_RESULT:
		LOG_INF("Disconnected");

		status = NETWORK_DISCONNECTED;
		break;
	default:
		__ASSERT(false, "Unknown event: %d", mgmt_event);
		return;
	}

	__ASSERT_NO_MSG(zbus_chan_pub(&NETWORK_CHAN, &status, K_SECONDS(1)) == 0);
}

static void network_task(void)
{

	net_mgmt_init_event_callback(&net_mgmt_callback, wifi_mgmt_event_handler, MGMT_EVENTS);
	net_mgmt_add_event_callback(&net_mgmt_callback);

	scan();
}

K_THREAD_DEFINE(network_task_id,
		CONFIG_MQTT_SAMPLE_NETWORK_THREAD_STACK_SIZE,
		network_task, NULL, NULL, NULL, 3, 0, 0);
