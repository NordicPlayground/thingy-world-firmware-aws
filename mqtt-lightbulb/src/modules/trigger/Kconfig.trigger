#
# Copyright (c) 2023 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

menuconfig MQTT_SAMPLE_TRIGGER
	bool "Trigger"
	select DK_LIBRARY if !BOARD_NATIVE_POSIX
	default y

if MQTT_SAMPLE_TRIGGER

config MQTT_SAMPLE_TRIGGER_THREAD_STACK_SIZE
	int "Thread stack size"
	default 1024

config MQTT_SAMPLE_TRIGGER_TIMEOUT_SECONDS
	int "Trigger timer timeout"
	default 60

endif # MQTT_SAMPLE_TRIGGER

module = MQTT_SAMPLE_TRIGGER
module-str = Trigger
source "subsys/logging/Kconfig.template.log_config"
