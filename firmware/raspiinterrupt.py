import time
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
GPIO.setup(4, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

def callback(channel):
    print("interrupt")


GPIO.add_event_detect(4, GPIO.RISING, callback=callback)

while True:
    time.sleep(1)



GPIO.cleanup()

