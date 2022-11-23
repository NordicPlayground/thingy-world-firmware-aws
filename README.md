# ThingyWorld Firmware

![Build and Release](https://github.com/NordicPlayground/thingy-world-firmware-aws/workflows/Build%20and%20Release/badge.svg)
[![Commitizen friendly](https://img.shields.io/badge/commitizen-friendly-brightgreen.svg)](http://commitizen.github.io/cz-cli/)
[![Nordic ClangFormat](https://img.shields.io/static/v1?label=Nordic&message=ClangFormat&labelColor=00A9CE&color=337ab7)](https://github.com/nrfconnect/sdk-nrf/blob/main/.clang-format)
[![Zephyr compliance](https://img.shields.io/static/v1?label=Zephyr&message=compliance&labelColor=4e109e&color=337ab7)](https://docs.zephyrproject.org/latest/contribute/index.html#coding-style)

Firmware for devices connecting to the [thingy.world cloud backend](https://github.com/NordicPlayground/thingy-rocks-cloud-aws-js).

1. By default, the Thingy91 will start in single-cell mode and will blink BLUE.
1. Press the button 1x to enable multi-cell mode. The Thingy91 will blink GREEN.
1. Press the button 2x to enable A-GPS mode. The Thingy91 will blink PURPLE.
1. Press and hold the button for 5 seconds and the Thingy91 will return to single-cell mode.
1. Thingy91 will send temperature, humidity and location messages when shaken or every hour.
1. Press the button 3x to enable "all mode". The Thingy91 will blink GREEN/PURPLE. A-GPS, single-cell,and multi-cell data will be sent every 30 seconds.
