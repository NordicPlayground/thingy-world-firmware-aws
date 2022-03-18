ThingyWorld Firmware
#########################

1. By default, the Thingy91 will start in single-cell mode and will blink BLUE.
2. Press the button 1x to enable multi-cell mode. The Thingy91 will blink GREEN.
3. Press the button 2x to enable A-GPS mode. The Thingy91 will blink PURPLE.
4. Press and hold the button for 5 seconds and the Thingy91 will return to single-cell mode.
5. Thingy91 will send temperature, humidity and location messages when shaken or every hour.
6. Press the button 3x to enable "all mode". The Thingy91 will blink GREEN/PURPLE. A-GPS, single-cell, and multi-cell data will be sent every 30 seconds.


Build Instructions:
###################

Initialize the repo:
``west init -m https://github.com/nRFCloud/thingyworld_fw``

Enter the project:
``cd thingyworld_fw/``

Fetch dependencies:
``west update``

Setup environment:
``source ../zephyr/zephyr-env.sh``

Build for the Thingy91:
``west build -b thingy91_nrf9160_ns -d <desired_build_directory> -p always``
