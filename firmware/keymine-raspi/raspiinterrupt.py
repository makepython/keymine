import time
import RPi.GPIO as GPIO
import smbus


GPIO.setmode(GPIO.BCM)
GPIO.setup(4, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

bus = smbus.SMBus(1)
address = 0x42

scans = 0
fails = 0

def callback(channel):
    global scans
    global fails
    try:
        data = bus.read_i2c_block_data(address, 0x00, 32)
    except Exception as e:
        print(e)
        fails += 1
    scans += 1
    if (scans % 100) == 0:
        print(scans, fails)


GPIO.add_event_detect(4, GPIO.RISING, callback=callback)

while True:
    time.sleep(1)



GPIO.cleanup()

