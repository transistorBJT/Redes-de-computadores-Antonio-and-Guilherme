#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "app.h"
#include "link.h"


/////////
/// 1 ///
/////////

// Slides 33, 34, 35
// helper functions with packet implementation

#define PACKETCSTART    2
#define PACKETCEND      3
#define PACKETCDATA     1


int createStartPacket(unsigned char* packet, unsigned char* fileName, int fileNameLength)
{
    packet[0] = PACKETCSTART;
    packet[1] = (unsigned char) fileNameLength;

    for(int i=0; i< fileNameLength; i++)
    {
        packet[2 + i] = fileName[i];
    }

    return 2 + fileNameLength; 
}

int createDataPacket(unsigned char* output, unsigned char* data, int dataSize)
{
    output[0] = PACKETCDATA;

    output[1] = (dataSize >> 8) & 0XFF;
    output[2] = dataSize & 0XFF;

    for(int i=0; i< dataSize; i++)
    {
        output[3 + i] = data[i];
    }

    return 3 + dataSize;
}

int createEndPacket(unsigned char* packet, int fileSize)
{
    packet[0] = PACKETCEND;
    packet[1] = 4;

    packet[2] = (fileSize >> 24) & 0XFF;
    packet[3] = (fileSize >> 16) & 0XFF;
    packet[4] = (fileSize >> 8) & 0XFF;
    packet[5] = fileSize & 0XFF;

    return 6;
}


int processStartPacket(unsigned char* output, unsigned char* packet, int packetLength)
{
    if(packet[0] != PACKETCSTART)
    {
        return 0;
    }

    int fileNameLength = (int)packet[1];

    if(fileNameLength <= 0)
    {
        return 0;
    }

    for(int i =0; i< fileNameLength; i++)
    {
        output[i] = packet[2 + i];
    }

    return fileNameLength;
}

int processDataPacket(unsigned char* output, unsigned char* packet)
{
    if(packet[0] != PACKETCDATA)
    {
        return 0;
    }

    int dataSize = packet[1] <<  8 | packet[2];
    
    for(int i=0; i< dataSize; i++)
    {
        output[i] = packet[3 + i];
    }

   return dataSize;
}

int processEndPacket(unsigned char* packet, int packetLength)
{
    if(packet[0] != PACKETCEND)
    {
        return 0;
    }

    if(packet[1] != 4)
    {
        return 0;
    }

    int fileSize = (packet[2] << 24) | (packet[3] << 16) | (packet[4] << 8) | packet[5];

    return fileSize;
}

void processData(unsigned char* data, int dataLength, unsigned char* startPacket, int* startPacketLength, unsigned char* dataPacket, int* dataPacketLength, unsigned char* endPacket, int* endPacketLength)
{   
    // start
    startPacket[0] = data[0];
    startPacket[1] = data[1];

    for(int i=0; i< startPacket[1]; i++)
    {
        startPacket[2 + i] = data[2 + i];
    }

    *startPacketLength = 2 + startPacket[1];


    // data
    int dataPacketStart = *startPacketLength;

    dataPacket[0] = data[dataPacketStart];
    dataPacket[1] = data[dataPacketStart + 1];
    dataPacket[2] = data[dataPacketStart + 2];

    int dataPacketBytes = dataPacket[1] << 8 | dataPacket[2];

    for(int i =0; i < dataPacketBytes; i++)
    {
        dataPacket[3 + i] = data[dataPacketStart + 3 + i];
    }

    *dataPacketLength = 3 + dataPacketBytes;


    //end
    int endPacketStart = (*startPacketLength) + (*dataPacketLength);
    
    for(int i=0; i<6; i++)
    {
        endPacket[i] = data[endPacketStart + i];
    }
    *endPacketLength = 6;

}

/////////
/// 2 ///
/////////

// Slide 55 - open/close port configuration

struct termios oldtio, newtio;

int openPort(char* portName)
{
    int portDescriptor = open(portName, O_RDWR | O_NOCTTY);
    if (portDescriptor < 0)
    {
        perror(portName);
        exit(-1);
    }

    tcgetattr(portDescriptor, &oldtio);
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag      = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag      = IGNPAR;
    newtio.c_oflag      = 0;
    newtio.c_lflag      = 0;

    // timer between charaters
    newtio.c_cc[VTIME]  = 0;

    // block until 0 characters are read ( non-blockling )
    newtio.c_cc[VMIN]   = 0;

    tcflush  (portDescriptor, TCIOFLUSH)        ;
    tcsetattr(portDescriptor, TCSANOW, &newtio) ;

    return portDescriptor;
}

void closePort(int portDescriptor)
{
    tcsetattr(portDescriptor, TCSANOW, &oldtio);
    close(portDescriptor);
}


/////////
/// 3 ///
/////////

#define FAIL -1
#define SUCCESS 1

extern int state;

unsigned char txBuffer[MAXBUFSIZE] = "";
int txBufferLength = 0;
unsigned char rxBuffer[MAXBUFSIZE] = "";
int rxBufferLength = 0;

int llOpen(int portDescriptor, int type)
{
    // transmitter mode ( TX )
    if(type == TXTYPE) 
    {
        // setup alarm
        (void)signal(SIGALRM, timeoutHandler);

        state = transmitting;

        while(1)
        {  
            if(state == transmitting)
            {   
                // send SET frame
                memset(txBuffer, 0, MAXBUFSIZE);
                txBufferLength = createSetFrame(txBuffer, TXTYPE);
                writeFrame(portDescriptor, txBuffer, txBufferLength);
                
                // activate alarm
                state = receiving;
                alarm(alarmDuration);
            }
        
            if(state == receiving)
            {   
                // wait for UA answer.
                // if alarm goes off, the function changes the "state" variable
                // to "transmitting" so the retransmission is done.
                memset(rxBuffer, 0, MAXBUFSIZE);
                rxBufferLength = readFrame(portDescriptor, rxBuffer);
                if(rxBufferLength == 0)
                {
                    continue;
                }
                if(validateUaFrame(rxBuffer, TXTYPE) == 1)
                {   
                    alarm(alarmCancel);
                    return SUCCESS;
                } 
            }     
        }
    }

    // receiver program ( RX )
    if(type == RXTYPE)
    {
        // wait for SET frame
        memset(rxBuffer, 0, MAXBUFSIZE);
        rxBufferLength = readFrame(portDescriptor, rxBuffer);

        // return failure if the frame is not of type SET
        if(validateSetFrame(rxBuffer, RXTYPE) != 1)
        {
            return FAIL;
        }
    
        // send UA
        memset(txBuffer, 0, MAXBUFSIZE);
        txBufferLength = createUaFrame(txBuffer, RXTYPE);
        writeFrame(portDescriptor, txBuffer, txBufferLength);
        
        return SUCCESS;
    }

    return FAIL;
}

int llClose(int portDescriptor, int type)
{
    if(type == TXTYPE) 
    {
        state = transmitting;

        while (1)
        {   
            if(state == transmitting)
            {
                // send DISC frame
                memset(txBuffer, 0, MAXBUFSIZE); 
                txBufferLength = createDiscFrame(txBuffer, TXTYPE);
                writeFrame(portDescriptor, txBuffer, txBufferLength);

                // activate alarm
                state = receiving;
                alarm(alarmDuration);
                
            }

            if(state == receiving)
            {
                // wait for DISC answer.
                // if alarm goes off, the function changes the "state" variable
                // to "transmitting" so the retransmission is done.
                memset(rxBuffer, 0, MAXBUFSIZE);
                rxBufferLength = readFrame(portDescriptor, rxBuffer);
                if(validateDiscFrame(rxBuffer, TXTYPE) == 1)
                {
                    alarm(alarmCancel);
                    break;
                }
            }
        }
        
        // send UA frame and we're done!
        memset(txBuffer, 0, MAXBUFSIZE);
        txBufferLength = createUaFrame(txBuffer, TXTYPE);
        writeFrame(portDescriptor, txBuffer, txBufferLength);

        return SUCCESS;
    }
    
    if(type == RXTYPE)
    {
        // close connection
        // already received disconnect frame on the transmission loop, so just need to validate
        if(validateDiscFrame(rxBuffer, RXTYPE) != 1)
        {
            return FAIL;
        }

        // send DISC frame
        memset(txBuffer, 0, MAXBUFSIZE);
        txBufferLength = createDiscFrame(txBuffer, RXTYPE);
        writeFrame(portDescriptor, txBuffer, txBufferLength);

        // receive UA frame 
        memset(rxBuffer, 0, MAXBUFSIZE);
        rxBufferLength = readFrame(portDescriptor, rxBuffer);
        if(validateUaFrame(rxBuffer, RXTYPE) != 1)
        {
            return FAIL;
        }

        return SUCCESS;
    }

    return FAIL;
}

int llWrite(int portDescriptor, unsigned char* data, int dataLength, int type)
{
    int remainingDataBytes = dataLength;

    int dataBytesToSend = 0;
    int totalBytesSent = 0;

    int frameCounter = 0;
    
    unsigned char dataBuffer[iFrameMaxDataSize] = "";

    state = transmitting;

    while(1)
    {   
        if(totalBytesSent == dataLength)
        {
            return totalBytesSent;
        }

        if(state == transmitting)
        {   
            // calculate data bytes to send on this frame
            remainingDataBytes = dataLength - totalBytesSent;
            dataBytesToSend = remainingDataBytes;
            if(remainingDataBytes >= iFrameMaxDataSize)
            {   
                dataBytesToSend = iFrameMaxDataSize;
            } 

            // put data to send on buffer
            memset(dataBuffer, 0, iFrameMaxDataSize);
            for(int i=0; i< dataBytesToSend; i++)
            {
                dataBuffer[i] = data[totalBytesSent + i];
            }

            // create and send information frame
            memset(txBuffer, 0, MAXBUFSIZE);
            txBufferLength = createInformationFrame(txBuffer, TXTYPE, frameCounter, dataBuffer, dataBytesToSend);

            writeFrame(portDescriptor, txBuffer, txBufferLength);

            state = receiving;
            alarm(alarmDuration);
        }

        if(state == receiving)
        {
            memset(rxBuffer, 0, MAXBUFSIZE);
            rxBufferLength = readFrame(portDescriptor, rxBuffer);

            if(frameCounter%2 == 0)
            {
                if(validateRR1Frame(rxBuffer, TXTYPE) == 1)
                {
                    frameCounter++;
                    totalBytesSent = totalBytesSent + dataBytesToSend;
                    
                    state = transmitting;
                    alarm(alarmCancel);

                    continue;
                }

                if(validateREJ0Frame(rxBuffer, TXTYPE) == 1)
                {   
                    alarm(alarmCancel);
                    writeFrame(portDescriptor, txBuffer, txBufferLength);
                    alarm(alarmDuration);

                    continue;
                }
            }

            if(frameCounter%2 == 1)
            {
                if(validateRR0Frame(rxBuffer, TXTYPE) == 1)
                {
                    frameCounter++;
                    totalBytesSent = totalBytesSent + dataBytesToSend;

                    state = transmitting;
                    alarm(alarmCancel);

                    continue;
                }

                if(validateREJ1Frame(rxBuffer, TXTYPE == 1))
                {   
                    alarm(alarmCancel);
                    writeFrame(portDescriptor, txBuffer, txBufferLength);
                    alarm(alarmDuration);

                    continue;
                }                
            }           
        }
    }
    
    return FAIL;
}

int llRead(int portDescriptor, unsigned char* data, int type)
{

    unsigned char destuffingBuffer[MAXBUFSIZE] = "";
    int destuffingBufferLength = 0;

    int frameCounter = 0;

    int totalBytesReceived = 0;
    
    int state = receiving;
    while(1)
    {

        if(state == receiving)
        {   
            memset(rxBuffer, 0, MAXBUFSIZE);
            rxBufferLength = readFrame(portDescriptor, rxBuffer);

            // If we received the DISC frame, we're done!
            if(rxBufferLength == 5) 
            {
                return totalBytesReceived;
            }

            // destuff the frame we received and proceed to validation
            memset(destuffingBuffer, 0, MAXBUFSIZE);
            destuffingBufferLength = byteDestuffing(rxBuffer, destuffingBuffer, rxBufferLength);
            
            state = transmitting;
        }

        if(state == transmitting)
        {
            // frame is OK!
            if(validateInformationFrame(destuffingBuffer, destuffingBufferLength, RXTYPE, frameCounter) == 1)
            {
                // copy the received bytes to "data"
                for(int i = 4; i < destuffingBufferLength - 2 ; i++)
                {
                    data[totalBytesReceived] = destuffingBuffer[i];
                    totalBytesReceived++;
                }
                
                // send either RR0 or RR1, depending on the next expected frame
                frameCounter++;
                
                if(frameCounter%2 == 0)
                {
                    memset(txBuffer, 0, MAXBUFSIZE);
                    txBufferLength = createRR0Frame(txBuffer, RXTYPE);
                    writeFrame(portDescriptor, txBuffer, txBufferLength);
                }

                if(frameCounter%2 == 1)
                {
                    memset(txBuffer, 0, MAXBUFSIZE);
                    txBufferLength = createRR1Frame(txBuffer, RXTYPE);
                    writeFrame(portDescriptor, txBuffer, txBufferLength);
                }

                state = receiving;
                continue;
            }

            // frame is NOK
            // send either REJ1 or REJ0 - depending on the frameCounter

            if(frameCounter%2 == 0)
            {
                memset(txBuffer, 0, MAXBUFSIZE);
                txBufferLength = createREJ1Frame(txBuffer, RXTYPE);
                writeFrame(portDescriptor, txBuffer, txBufferLength);
            }

            if(frameCounter%2 == 1)
            {
                memset(txBuffer, 0, MAXBUFSIZE);
                txBufferLength = createREJ0Frame(txBuffer, RXTYPE);
                writeFrame(portDescriptor, txBuffer, txBufferLength);
            }

            state = receiving;       
        }            
    }   
}