# Instructions
Build and run this program on a Raspberry PI to use it as an I2C slave.

# Hardware
### Setup the slave RPI
On the slave RPI, connect the i2c wires to:

GPIO used for models other than those based on the BCM2711 (e.g., the PiZero)
	SDA	SCL
I2C	18	19

GPIO used for models based on the BCM2711 (e.g. the Pi4B).
	SDA	SCL
I2C	10	11

References:
- https://www.raspberrypi.com/documentation/computers/processors.html
- https://abyz.me.uk/rpi/pigpio/pdif2.html#bsc_xfer

And make sure that this line is commented out in `/boot/firmware/config.txt`
```
# dtparam=i2c_arm=on
```

If the master I2C device is another RPI, make sure the above line is uncommented and the wires are connected to the GPIO pins labelled SDA and SCL on the pinout (e.g., GPIO 2 and 3 on the Raspberry PI Zero), as these pins are intended to be used as the I2C pins in master-mode.

# Software
### Requirements
```
sudo apt install g++ pigpio
```

### Build
```
g++ i2c-slave-test.cpp -lpthread -lpigpio -o i2c-slave-test
```

### Run
```
sudo ./i2c-slave-test
```
