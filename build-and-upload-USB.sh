#!/bin/bash

pio run --target upload -e usb && \
  pio device monitor -e usb
