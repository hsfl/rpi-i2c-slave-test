// This code is based on the example from this StackExchange answer:
// https://raspberrypi.stackexchange.com/questions/76109/raspberry-as-an-i2c-slave/91438#91438
// Usage:
// sudo ./i2c-slave-test

#include <cctype>   // For toupper
#include <csignal>  // For signal handling
#include <pigpio.h> // For GPIO
#include <iostream>

using namespace std;

void runSlave();
void closeSlave();
int getControlBits(int, bool);

const int slaveAddress = 0x0f; // <-- Your address of choice
bsc_xfer_t xfer; // Struct to control data flow

void signalHandler(int signum)
{
    std::cout << "Interrupt signal (" << signum << ") received." << std::endl;
    closeSlave();
    exit(signum);
}

int main(){
    gpioInitialise();
    cout << "Initialized GPIOs\n";

    // Override gpio's default signal handling
    struct sigaction sigIntHandler;
    // Set the signal handler function
    sigIntHandler.sa_handler = signalHandler;
    // Clear the signal mask to ensure no signals are blocked
    sigemptyset(&sigIntHandler.sa_mask);
    // Set flags to 0 for default behavior
    sigIntHandler.sa_flags = 0;
    // Register the signal handler for SIGINT
    sigaction(SIGINT, &sigIntHandler, NULL);

    runSlave();

    return 0;
}

void flush_tx_buffer(bsc_xfer_t &xf)
{
    xf.txCnt = 0;
    for (int i = 0; i < sizeof(xf.txBuf); ++i) {
        xf.txBuf[i] = 0;
    }
}

void runSlave() {
    // Close old device (if any)
    xfer.control = getControlBits(slaveAddress, false); // To avoid conflicts when restarting
    flush_tx_buffer(xfer);
    bscXfer(&xfer);
    // Set I2C slave Address
    xfer.control = getControlBits(slaveAddress, true);
    flush_tx_buffer(xfer);
    int status = bscXfer(&xfer); // Should now be visible in I2C-Scanners
    
    if (status >= 0)
    {
        cout << "Opened slave\n";
        xfer.rxCnt = 0;
        flush_tx_buffer(xfer);
        while(1) {
            bscXfer(&xfer);
            flush_tx_buffer(xfer);
            if(xfer.rxCnt > 0) {
                // If the entire message are control bytes 0x00, then skip
                bool allZero = true;
                for (size_t i=0; i < xfer.rxCnt; ++i) {
                    if (xfer.rxBuf[i] != 0x00) {
                        allZero = false;
                        break;
                    }
                }
                if (allZero) {
                    continue;
                }
                cout << "Received " << xfer.rxCnt << " bytes: ";
                for(int i = 0; i < xfer.rxCnt; i++) {
                    cout << xfer.rxBuf[i];
                }
                cout << "\n";

                // Check if the received message exceeds the 16-byte TX FIFO size.
                // The +1 is to account for the terminator byte.
                if (xfer.rxCnt > 16+1) {
                    cout << "Warning: Response message will have bad characters at the end because the message exceeds tx FIFO size!\n";
                }

                // Capitalize and echo back the received data
                xfer.txCnt = xfer.rxCnt;
                cout << "Writing " << xfer.txCnt << " bytes: ";
                for(int i = 0; i < xfer.txCnt; i++) {
                    xfer.txBuf[i] = toupper(xfer.rxBuf[i]);
                    cout << xfer.txBuf[i];
                }
                cout << "\n";
            }
            gpioDelay(1000);
        }
    } else {
        cout << "Failed to open slave!!!\n";
    }
}

void closeSlave() {
    xfer.control = getControlBits(slaveAddress, false);
    flush_tx_buffer(xfer);
    bscXfer(&xfer);
    cout << "Closed slave.\n";

    gpioTerminate();
    cout << "Terminated GPIOs.\n";
}


int getControlBits(int address /* max 127 */, bool open) {
    /*
    Excerpt from http://abyz.me.uk/rpi/pigpio/cif.html#bscXfer regarding the control bits:

    22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    a  a  a  a  a  a  a  -  -  IT HC TF IR RE TE BK EC ES PL PH I2 SP EN

    Bits 0-13 are copied unchanged to the BSC CR register. See pages 163-165 of the Broadcom 
    peripherals document for full details. 

    aaaaaaa defines the I2C slave address (only relevant in I2C mode)
    IT  invert transmit status flags
    HC  enable host control
    TF  enable test FIFO
    IR  invert receive status flags
    RE  enable receive
    TE  enable transmit
    BK  abort operation and clear FIFOs
    EC  send control register as first I2C byte
    ES  send status register as first I2C byte
    PL  set SPI polarity high
    PH  set SPI phase high
    I2  enable I2C mode
    SP  enable SPI mode
    EN  enable BSC peripheral
    */

    // Flags like this: 0b/*IT:*/0/*HC:*/0/*TF:*/0/*IR:*/0/*RE:*/0/*TE:*/0/*BK:*/0/*EC:*/0/*ES:*/0/*PL:*/0/*PH:*/0/*I2:*/0/*SP:*/0/*EN:*/0;

    int flags;
    if(open)
        flags = /*RE:*/ (1 << 9) | /*TE:*/ (1 << 8) | /*I2:*/ (1 << 2) | /*EN:*/ (1 << 0);
    else // Close/Abort
        flags = /*BK:*/ (1 << 7) | /*I2:*/ (0 << 2) | /*EN:*/ (0 << 0);

    return (address << 16 /*= to the start of significant bits*/) | flags;
}
