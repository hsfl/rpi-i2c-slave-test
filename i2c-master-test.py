# Run on a Raspberry PI to test I2C communications with the rpi i2c slave device.
# Requirements:
# sudo apt install python3 python3-smbus
# Usage:
# python3 i2c-master-test.py
# python3 i2c-master-test.py -m "Capitalize me!"

import argparse # For terminal arguments
from smbus import SMBus
import time

i2c_slave_address = 0x0f
i2c_port = 1
message = 'Hello World!'

I2C_SLAVE_TX_BUFFER_SIZE = 16

def flush_i2c(bus, slave_address):
    for _ in range(I2C_SLAVE_TX_BUFFER_SIZE):
        # Flush any existing messages
        try:
            bus.read_i2c_block_data(slave_address, 0, 1)
        except Exception as e:
            print(f"Error flushing I2C: {e}")
            break

def write_and_read_data(slave_address, port, data):
    bus = SMBus(port)
    flush_i2c(bus, slave_address)
    ints_of_data = list(map(ord, data))
    # Write the message
    bus.write_i2c_block_data(slave_address, ints_of_data[0], ints_of_data[1:])
    time.sleep(0.2) # Wait for the slave to process the data
    # Read back the modified message
    int_list = bus.read_i2c_block_data(slave_address, 0, len(ints_of_data))
    read_message = ''.join(map(chr, int_list))
    print(f"Read data: {read_message}")
    bus.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-a", "--slave_address", type=int, default=i2c_slave_address, help='I2C slave address (default: 0x0f)')
    parser.add_argument("-p", "--port", type=int, default=i2c_port, help='I2C port number (default: 1)')
    parser.add_argument("-m", "--message", type=str, default='Hello World!', help='Message to send to the slave device (default: "Hello World!")')
    args = parser.parse_args()
    i2c_slave_address = args.slave_address
    i2c_port = args.port
    message = args.message

    if (len(message) > I2C_SLAVE_TX_BUFFER_SIZE):
        print(f"Message too long, must be less than {I2C_SLAVE_TX_BUFFER_SIZE} characters.")
        exit(1)

    write_and_read_data(i2c_slave_address, i2c_port, message)
