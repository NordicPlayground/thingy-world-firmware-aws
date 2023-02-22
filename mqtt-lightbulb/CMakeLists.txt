#
# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mqtt)


# Include files that are common for all modules
add_subdirectory(src/common)

# Include enabled module source folders
add_subdirectory_ifdef(CONFIG_MQTT_SAMPLE_TRIGGER       src/modules/trigger)
add_subdirectory_ifdef(CONFIG_MQTT_SAMPLE_SAMPLER       src/modules/sampler)
add_subdirectory_ifdef(CONFIG_MQTT_SAMPLE_NETWORK       src/modules/network)
add_subdirectory_ifdef(CONFIG_MQTT_SAMPLE_TRANSPORT     src/modules/transport)
add_subdirectory_ifdef(CONFIG_MQTT_SAMPLE_LED           src/modules/led)
add_subdirectory(src/modules/ledctrl)
