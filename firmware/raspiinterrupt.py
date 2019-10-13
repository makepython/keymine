import time
import RPi.GPIO as GPIO
import smbus


GPIO.setmode(GPIO.BCM)
GPIO.setup(4, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

bus = smbus.SMBus(1)
address = 0x42

def callback(channel):
    data = bus.read_i2c_block_data(address, 0x00, 32)
    print(data)


GPIO.add_event_detect(4, GPIO.RISING, callback=callback)

while True:
    time.sleep(1)



GPIO.cleanup()

