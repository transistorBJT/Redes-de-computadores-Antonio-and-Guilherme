#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "link.h"

#define packetSize 3

#define transmitting    7
#define receiving       30

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

    // establish connection (slide 31)
    txFrameLength = createSetFrame(txBuffer, TXTYPE);
    writeFrame(portDescriptor, txBuffer, txFrameLength);

    rxFrameLength = readFrame(portDescriptor, rxBuffer);
    if(validateUaFrame(rxBuffer, TXTYPE) != 1)
    {
        printf("Could not establish connection!");
        return 0;   
    }

    printf("Connection Established \n");

    // send information frames
    unsigned char data[20] = "COMPEN\x7ESAS0";
    int totalBytesToSend = 11;

    int dataBytesToSend = 0;
    int totalBytesSent = 0;

    int state = transmitting;

    int frameCounter = 0;
    
    unsigned char dataBuffer[packetSize] = "";


/*  while(1)
    {
        if(state == transmitting)
        {   
            dataBytesToSend = totalBytesToSend;
            if(totalBytesToSend >= packetSize)
            {   
                dataBytesToSend = packetSize;
                totalBytesToSend = totalBytesToSend - 3;
            } 

            memset(dataBuffer, 0, packetSize);
            totalBytesSent = frameCounter * packetSize;
            for(int i=0; i< packetSize; i++)
            {
                dataBuffer[i] = data[totalBytesSent + i];
            }

            memset(txBuffer, 0, MAXBUFSIZE);
            txFrameLength = createInformationFrame(txBuffer, TXTYPE, frameCounter, data, dataBytesToSend);
            writeFrame(portDescriptor, txBuffer, txFrameLength);

            state = receiving;
        }

        if(state == receiving)
        {
            memset(rxBuffer, 0, MAXBUFSIZE);
            rxFrameLength = readFrame(portDescriptor, rxBuffer);

            if(frameCounter%2 == 0)
            {
                if(validateRR1Frame(rxBuffer, TXTYPE) != 1)
                {
                    printf("Re")
                }
            }
        }
    }
    */













    // close connection 
    memset(txBuffer, 0, MAXBUFSIZE); 
    txFrameLength = createDiscFrame(txBuffer, TXTYPE);
    writeFrame(portDescriptor, txBuffer, txFrameLength);

    memset(rxBuffer, 0, MAXBUFSIZE);
    rxFrameLength = readFrame(portDescriptor, rxBuffer);
    if(validateDiscFrame(rxBuffer, TXTYPE) != 1)
    {
        printf("Could not close connection!");
        return 0;
    }

    memset(txBuffer, 0, MAXBUFSIZE);
    txFrameLength = createUaFrame(txBuffer, TXTYPE);
    writeFrame(portDescriptor, txBuffer, txFrameLength);


    // close port
    closePort(portDescriptor);


    return 0;
}

