# Arducam External trigger signal generator.
# Copyright (C) 2022, Arducam.

# External module imports
import RPi.GPIO as GPIO
import time

# Pin Definitons:
ledPin = 2 # Broadcom pin 2
# Pin Setup:
GPIO.setmode(GPIO.BCM) # Broadcom pin-numbering scheme
GPIO.setup(ledPin, GPIO.OUT) # LED pin set as output

# Initial state for LEDs:
GPIO.output(ledPin, GPIO.LOW)

print("Start Trigger! Press CTRL+C to exit")
try:
    while 1:
        GPIO.output(ledPin, GPIO.HIGH)
        time.sleep(0.001)
        GPIO.output(ledPin, GPIO.LOW)
        time.sleep(0.032)
except KeyboardInterrupt: # If CTRL+C is pressed, exit cleanly:
    GPIO.cleanup() # cleanup all GPIO
