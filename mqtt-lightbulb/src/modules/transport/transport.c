/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/smf.h>
#include <net/mqtt_helper.h>

#include "client_id.h"
#if defined(CONFIG_MODEM_KEY_MGMT)
#include "credentials_provision.h"
#endif /* CONFIG_MODEM_KEY_MGMT */
#include "message_channel.h"

/* Register log module */
LOG_MODULE_REGISTER(transport, CONFIG_MQTT_SAMPLE_TRANSPORT_LOG_LEVEL);

/* Register subscriber */
ZBUS_SUBSCRIBER_DEFINE(transport, CONFIG_MQTT_SAMPLE_TRANSPORT_MESSAGE_QUEUE_SIZE);

/* ID for subscribe topic - Used to verify that a subscription succeeded in on_mqtt_suback(). */
#define SUBSCRIBE_TOPIC_ID 2469

/* Forward declarations */
static const struct smf_state state[];
static void connect_work_fn(struct k_work *work);

/* Define connection work - Used to handle reconnection attempts to the MQTT broker */
static K_WORK_DELAYABLE_DEFINE(connect_work, connect_work_fn);

/* Define stack_area of application workqueue */
K_THREAD_STACK_DEFINE(stack_area, 4096);

/* Declare application workqueue. This workqueue is used to call mqtt_helper_connect(), and
 * schedule reconnectionn attempts upon network loss or disconnection from MQTT.
 */
struct k_work_q queue;

/* Internal states */
enum module_state { MQTT_CONNECTED, MQTT_DISCONNECTED };

/* MQTT client ID buffer */
static char client_id[50];

static char ledctrl_topic[100];
static char default_sub_topic[100];
static char default_pub_topic[100];

/* User defined state object.
 * Used to transfer data between state changes.
 */
static struct s_object {
	/* This must be first */
	struct smf_ctx ctx;

	/* Last channel type that a message was received on */
	const struct zbus_channel *chan;

	/* Network status */
	enum network_status status;

	/* Payload */
	struct payload payload;
} s_obj;

/* Callback handlers from MQTT helper library.
 * The functions are called whenever specific MQTT packets are received from the broker, or
 * some library state has changed.
 */
static void on_mqtt_connack(enum mqtt_conn_return_code return_code)
{
	ARG_UNUSED(return_code);

	smf_set_state(SMF_CTX(&s_obj), &state[MQTT_CONNECTED]);
}

static void on_mqtt_disconnect(int result)
{
	ARG_UNUSED(result);

	smf_set_state(SMF_CTX(&s_obj), &state[MQTT_DISCONNECTED]);
}

static void on_mqtt_publish(struct mqtt_helper_buf topic, struct mqtt_helper_buf payload)
{
	LOG_INF("Received payload: %.*s on topic: %.*s", payload.size,
							 payload.ptr,
							 topic.size,
							 topic.ptr);

	if (strncmp(
	    topic.ptr, ledctrl_topic, topic.size) == 0) {
		struct payload command = {0};
		memcpy(command.string, payload.ptr,
		       MIN(ARRAY_SIZE(command.string)-1, payload.size));
		__ASSERT_NO_MSG(zbus_chan_pub(&LEDCTRL_CHAN, &command, K_SECONDS(1)) == 0);
	}
}

static void on_mqtt_suback(uint16_t message_id, int result)
{
	if ((message_id == SUBSCRIBE_TOPIC_ID) && (result == 0)) {
		LOG_INF("Subscribed to topic %s", default_sub_topic);
		LOG_INF("Subscribed to topic %s", ledctrl_topic);
	}
}

/* Local convenience functions */
static void publish(struct payload *payload)
{
	int err;
	struct mqtt_publish_param param = {
		.message.payload.data = payload->string,
		.message.payload.len = strlen(payload->string),
		.message.topic.qos = MQTT_QOS_1_AT_LEAST_ONCE,
		.message_id = k_uptime_get_32(),
		.message.topic.topic.utf8 = default_pub_topic,
		.message.topic.topic.size = strlen(default_pub_topic),
	};

	err = mqtt_helper_publish(&param);
	if (err) {
		LOG_ERR("Failed to send payload, err: %d", err);
		return;
	}

	LOG_INF("Publishing message: \"%.*s\" on topic: \"%.*s\"", param.message.payload.len,
								   param.message.payload.data,
								   param.message.topic.topic.size,
								   param.message.topic.topic.utf8);
}

static void subscribe(void)
{
	int err;
	
	
	struct mqtt_topic topics[] = {
		{
			.topic.utf8 = default_sub_topic,
			.topic.size = strlen(default_sub_topic),
		},
		{
			.topic.utf8 = ledctrl_topic,
			.topic.size = strlen(ledctrl_topic),
		}
	};
	struct mqtt_subscription_list list = {
		.list = topics,
		.list_count = ARRAY_SIZE(topics),
		.message_id = SUBSCRIBE_TOPIC_ID,
	};

	err = mqtt_helper_subscribe(&list);
	if (err) {
		LOG_ERR("Failed to subscribe to topics, error: %d", err);
		return;
	}
}

/* Connect work - Used to establish a connection to the MQTT broker and schedule reconnection
 * attempts.
 */
static void connect_work_fn(struct k_work *work)
{
	ARG_UNUSED(work);

	__ASSERT(client_id_get(client_id, sizeof(client_id)) == 0, "Failed getting Client ID");

	if (
		snprintf(ledctrl_topic, ARRAY_SIZE(ledctrl_topic),
			 CONFIG_MQTT_SAMPLE_TRANSPORT_LEDCTRL_SUBSCRIBE_TOPIC, client_id)
			 >= ARRAY_SIZE(ledctrl_topic)) {
		LOG_ERR("ledctrl_topic is overflowed!");
		return;
	}

	if (
		snprintf(default_pub_topic, ARRAY_SIZE(default_pub_topic),
			 CONFIG_MQTT_SAMPLE_TRANSPORT_PUBLISH_TOPIC, client_id)
			 >= ARRAY_SIZE(default_pub_topic)) {
		LOG_ERR("default_pub_topic is overflowed!");
		return;
	}

	if (
		snprintf(default_sub_topic, ARRAY_SIZE(default_sub_topic),
			 CONFIG_MQTT_SAMPLE_TRANSPORT_SUBSCRIBE_TOPIC, client_id)
			 >= ARRAY_SIZE(default_sub_topic)) {
		LOG_ERR("default_sub_topic is overflowed!");
		return;
	}

	struct mqtt_helper_conn_params conn_params = {
		.hostname.ptr = CONFIG_MQTT_SAMPLE_TRANSPORT_BROKER_HOSTNAME,
		.hostname.size = strlen(CONFIG_MQTT_SAMPLE_TRANSPORT_BROKER_HOSTNAME),
		.device_id.ptr = client_id,
		.device_id.size = strlen(client_id),
	};

	int err = mqtt_helper_connect(&conn_params);

	if (err) {
		LOG_ERR("Failed connecting to MQTT, error code: %d", err);
	}

	k_work_reschedule(&connect_work,
			  K_SECONDS(CONFIG_MQTT_SAMPLE_TRANSPORT_RECONNECTION_TIMEOUT_SECONDS));
}

/* Zephyr State Machine framework handlers */
static void disconnected_entry(void *o)
{
	struct s_object *od = o;

	/* Reschedule a connection attempt if we are connected to network and we enter the
	 * disconnected state.
	 */
	if (od->status == NETWORK_CONNECTED) {
		k_work_reschedule_for_queue(&queue, &connect_work, K_NO_WAIT);
	}
}

static void disconnected_run(void *o)
{
	struct s_object *od = o;

	if ((od->status == NETWORK_CONNECTED) && (od->chan == &NETWORK_CHAN)) {

		/* Wait for 5 seconds to ensure that the network stack is ready before
		 * attempting to connect to MQTT. This delay is only needed when building for
		 * Wi-Fi.
		 */
		k_work_reschedule_for_queue(&queue, &connect_work, K_SECONDS(5));
	}
}

static void connected_entry(void *o)
{
	LOG_INF("Connected to MQTT broker");
	LOG_INF("Hostname: %s", CONFIG_MQTT_SAMPLE_TRANSPORT_BROKER_HOSTNAME);
	LOG_INF("Client ID: %s", client_id);
	LOG_INF("Port: %d", CONFIG_MQTT_HELPER_PORT);
	LOG_INF("TLS: %s", IS_ENABLED(CONFIG_MQTT_LIB_TLS) ? "Yes" : "No");

	ARG_UNUSED(o);

	/* Cancel any ongoing connect work when we enter connected state */
	k_work_cancel_delayable(&connect_work);

	subscribe();
}

static void connected_run(void *o)
{
	struct s_object *od = o;

	if ((od->status == NETWORK_DISCONNECTED) && (od->chan == &NETWORK_CHAN)) {
		/* Explicitly disconnect the MQTT transport when losing network connectivity.
		 * This is to cleanup any internal library state.
		 * The call to this function will cause on_mqtt_disconnect() to be called.
		 */
		(void)mqtt_helper_disconnect();
		return;
	}

	if (od->chan != &PAYLOAD_CHAN) {
		return;
	}

	publish(&od->payload);
}

static void connected_exit(void *o)
{
	ARG_UNUSED(o);

	LOG_INF("Disconnected from MQTT broker");
}

/* Construct state table */
static const struct smf_state state[] = {
	[MQTT_DISCONNECTED] = SMF_CREATE_STATE(disconnected_entry, disconnected_run, NULL),
	[MQTT_CONNECTED] = SMF_CREATE_STATE(connected_entry, connected_run, connected_exit),
};

static void transport_task(void)
{
	const struct zbus_channel *chan;
	enum network_status status;
	struct payload payload;

#if defined(CONFIG_MODEM_KEY_MGMT)
	__ASSERT(credentials_provision() == 0, "Certificate provisioning failed");
#endif /* CONFIG_MODEM_KEY_MGMT */

	struct mqtt_helper_cfg cfg = {
		.cb = {
			.on_connack = on_mqtt_connack,
			.on_disconnect = on_mqtt_disconnect,
			.on_publish = on_mqtt_publish,
			.on_suback = on_mqtt_suback,
		},
	};

	/* Initialize and start application workqueue.
	 * This workqueue can be used to offload tasks and/or as a timer when wanting to
	 * schedule functionality using the 'k_work' API.
	 */
	k_work_queue_init(&queue);
	k_work_queue_start(&queue, stack_area,
			   K_THREAD_STACK_SIZEOF(stack_area),
			   K_HIGHEST_APPLICATION_THREAD_PRIO,
			   NULL);

	__ASSERT_NO_MSG(mqtt_helper_init(&cfg) == 0);

	/* Set initial state */
	smf_set_initial(SMF_CTX(&s_obj), &state[MQTT_DISCONNECTED]);

	while (!zbus_sub_wait(&transport, &chan, K_FOREVER)) {

		s_obj.chan = chan;

		if (&NETWORK_CHAN == chan) {

			__ASSERT_NO_MSG(zbus_chan_read(&NETWORK_CHAN, &status, K_SECONDS(1)) == 0);

			s_obj.status = status;

			__ASSERT_NO_MSG(smf_run_state(SMF_CTX(&s_obj)) == 0);
		}

		if (&PAYLOAD_CHAN == chan) {

			__ASSERT_NO_MSG(zbus_chan_read(&PAYLOAD_CHAN, &payload, K_SECONDS(1)) == 0);

			s_obj.payload = payload;

			__ASSERT_NO_MSG(smf_run_state(SMF_CTX(&s_obj)) == 0);
		}
	}
}

K_THREAD_DEFINE(transport_task_id,
		CONFIG_MQTT_SAMPLE_TRANSPORT_THREAD_STACK_SIZE,
		transport_task, NULL, NULL, NULL, 3, 0, 0);
