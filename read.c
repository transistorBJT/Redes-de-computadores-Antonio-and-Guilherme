#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"

#define RXTYPE 1048


int main(int argc, char *argv[])
{  

    /////////////////////
    /// Setup program ///
    /////////////////////
    
    // parse arguments
    if (argc < 2)
    {
        printf("Usage: %s <SerialPort>\nExample: %s /dev/ttyS1\n", argv[0], argv[0]);
        exit(1);
    }
    char *serialPortName = argv[1];

    // open port
    int portDescriptor = openPort(serialPortName);


    ////////////////////
    /// Receive Data ///
    ////////////////////
        
    // setup data to receive
    unsigned char receivedPackets[3*MAXFILESIZE] = "";
    int receivedPacketsLength = 0;


    // start connection
    int returnValue = llOpen(portDescriptor, RXTYPE);
    if(returnValue < 0)
    {
        printf("Could not start connection.\n Exiting.");
        return -1;
    }
    
    // receive data
    receivedPacketsLength = llRead(portDescriptor, receivedPackets, RXTYPE);
    if(receivedPacketsLength < 0) 
    {
        printf("Could not receive message. Exiting.\n");
        return -1;
    }

    // finish connection
    returnValue = llClose(portDescriptor, RXTYPE);
    if(returnValue < 0)
    {
        printf("Could not start connection.\n Exiting");
        return -1;
    }


    ////////////////////
    /// Process Data ///
    ////////////////////

    // split packets
    unsigned char startPacket[2*MAXFILENAMELENGTH] = "";
    int* startPacketLength = malloc(sizeof(int*));

    unsigned char dataPacket[MAXFILESIZE] = "";
    int* dataPacketLength = malloc(sizeof(int*));

    unsigned char endPacket[6] = "";
    int* endPacketLength = malloc(sizeof(int*));

    processData(receivedPackets, receivedPacketsLength, startPacket, startPacketLength, dataPacket, dataPacketLength, endPacket, endPacketLength);


    // get fileName from start packet
    unsigned char fileName[MAXFILENAMELENGTH] = "";
    int fileNameLength = processStartPacket(fileName, startPacket, *startPacketLength);

    // get fileContents from data packet
    unsigned char fileContents[MAXFILESIZE] = "";
    int fileContentsSize = processDataPacket(fileContents, dataPacket);

    
    // create file and print contents
    FILE* fileToCreate;
    unsigned char fileLocation[MAXFILENAMELENGTH] = "readFiles/";
    strcat((char*)fileLocation, (char*)fileName);

    fileToCreate = fopen((char*)fileLocation, "w");

    for(int i=0; i < fileContentsSize; i++)
    {
        fprintf(fileToCreate, "%c", fileContents[i]);
    }

    fclose(fileToCreate);
    


    /////////////////////
    /// Close program ///
    /////////////////////
    closePort(portDescriptor);

}