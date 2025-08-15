# Instructions
Build and run this program on a Raspberry PI to use it as an I2C slave.

# Setup
## Slave RPI
On the slave RPI, connect the I2C wires to:

GPIO used for models other than those based on the BCM2711 (e.g., the PiZero):
|   |SDA|SCL|
|---|---|---|
I2C	| 18| 19|

GPIO used for models based on the BCM2711 (e.g. the Pi4B):
|   |SDA|SCL|
|---|---|---|
I2C	| 10| 11|

References:
- https://www.raspberrypi.com/documentation/computers/processors.html
- https://abyz.me.uk/rpi/pigpio/pdif2.html#bsc_xfer

And make sure that this line is commented out in `/boot/firmware/config.txt`
```
# dtparam=i2c_arm=on
```


## Master RPI
For using an RPI as a master I2C device, uncomment the `dtparam` line mentioned above in the master's `/boot/firmware/config.txt` and connect the wires to the GPIO pins labelled SDA and SCL on the pinout (e.g., GPIO 2 and 3 on the Raspberry PI Zero), as these pins are intended to be used as the I2C pins in master-mode.

# Software
## Slave RPI
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

## Master RPI
### Requirements
```
sudo apt install python3 python3-smbus
```

### Test
Check that the I2C slave RPI is recognized by the master RPI:

If needed, change the I2C port number from 1 to what shows up in `/dev/i2c-X`
```
i2cdetect -y 1
```

Example usage for the python script:
```
python3 i2c-master-test.py -m "Capitalize me!"
```

