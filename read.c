#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "link.h"


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <SerialPort>\nExample: %s /dev/ttyS1\n", argv[0], argv[0]);
        exit(1);
    }
    char *serialPortName = argv[1];

    unsigned char txBuffer[MAXBUFSIZE] = "";
    int txFrameLength = 0;
    unsigned char rxBuffer[MAXBUFSIZE] = "";
    int rxFrameLength = 0;
    unsigned char destuffingBuffer[MAXBUFSIZE] = "";
    int destuffingBufferFrameLength = 0;

    // open port
    int portDescriptor = openPort(serialPortName);

    // establish connection
    rxFrameLength = readFrame(portDescriptor, rxBuffer);
    if(validateSetFrame(rxBuffer, RXTYPE) != 1)
    {
        printf("Could not establish connection! Please try again");
        return 0;
    }

    txFrameLength = createUaFrame(txBuffer, RXTYPE);
    writeFrame(portDescriptor, txBuffer, txFrameLength);

    printf("Connection Established \n");
    
    // close connection
    memset(rxBuffer, 0, MAXBUFSIZE);
    rxFrameLength = readFrame(portDescriptor, rxBuffer);
    if(validateDiscFrame(rxBuffer, RXTYPE) != 1)
    {
        printf("Could not close connection");
        return 0;
    }

    memset(txBuffer, 0, MAXBUFSIZE);
    txFrameLength = createDiscFrame(txBuffer, RXTYPE);
    writeFrame(portDescriptor, txBuffer, txFrameLength);

    memset(rxBuffer, 0, MAXBUFSIZE);
    rxFrameLength = readFrame(portDescriptor, rxBuffer);
    if(validateUaFrame(rxBuffer, RXTYPE) != 1)
    {
        printf("Could not confirm the close connection from tx side");
        return 0;
    }
    
    printf("Conection close \n");
    // close port
    closePort(portDescriptor);

    return 0;
}

