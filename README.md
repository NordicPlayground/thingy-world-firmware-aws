# ThingyWorld Firmware

[![Build and Release](https://github.com/NordicPlayground/thingy-world-firmware-aws/actions/workflows/build-and-release.yaml/badge.svg?branch=saga)](https://github.com/NordicPlayground/thingy-world-firmware-aws/actions/workflows/build-and-release.yaml)
[![@commitlint/config-conventional](https://img.shields.io/badge/%40commitlint-config--conventional-brightgreen)](https://github.com/conventional-changelog/commitlint/tree/master/@commitlint/config-conventional)

Firmware for devices connecting to the
[thingy.world cloud backend](https://github.com/NordicPlayground/thingy-rocks-cloud-aws-js)
based on the `asset_tracker_v2` from the
[nRF Connect SDK](https://github.com/nrfconnect/sdk-nrf).

## Memfault demonstration (`double-tap-to-crash`)

The builds with `double-tap-to-crash` to feature will crash when the button is
double-tapped, meaning pressed in very short succession.

Because there is not crash notification from Memfault, the backend needs to poll
for the crash dump so it can show up quickly on the ThingyWorld dasboard.

For this to happen, press the button **once** before double tapping. These
builds are sending the button ID `42` which is the signal for the backend to
scan for new crash reports for 120 seconds.
