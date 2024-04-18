# Mise en oeuvre

## Matériel

Le pinout utilisé pour pendant la phase de développement est le suivant:

- USB:
  - VBUS => pin 40
  - GND  => pin 38
  - D+   => pin 1
  - D-   => pin 2

- UART data (uart0):
  - RX => pin 12
  - TX => pin 11

# Installation SW

depuis une ubuntu  LTS:

```
sudo apt install g++
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib srecord
git clone git@github.com:picodevpro/usb_bridge.git
cd usb-bridge
git submodule update --init
cd firmware
mkdir build
cd build
cmake ..
# or, to build for the custom boards:
# PICO_BOARD=remapper cmake ..
make
```

copy remapper.uf2 on RPI