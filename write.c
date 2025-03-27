#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app.h"

#define TXTYPE 1337


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


    //////////////////
    /// Setup Data ///
    //////////////////

    // 1 - filename
    unsigned char fileName[MAXFILENAMELENGTH] = "penguin.gif";
    int fileNameLength = strlen((char*)fileName);
    
    // 2 - fileContents
    unsigned char fileLocation[MAXFILENAMELENGTH] = "writeFiles/";
    strcat((char*)fileLocation, (char*)fileName);
    

    FILE *fileToSend;
    fileToSend = fopen((char*) fileLocation, "r");
    if(fileToSend == NULL)
    {
    
        exit(1);
    }
   
    unsigned char fileContents[MAXFILESIZE] = "";

    int currentChar;

    int fileSize = 0;
    while(1)
    {
        currentChar = getc(fileToSend);
        if(currentChar == EOF) 
        {
            break;
        }

        fileContents[fileSize] = (unsigned char) currentChar;

        fileSize++;
    }
    fclose(fileToSend);

    // 3 - create packets
    unsigned char startPacket[2*MAXFILENAMELENGTH];
    int startPacketLength = createStartPacket(startPacket, fileName, fileNameLength);

    unsigned char dataPacket[MAXFILESIZE];
    int dataPacketLength = createDataPacket(dataPacket, fileContents, fileSize);

    unsigned char endPacket[6];
    int endPacketLength = createEndPacket(endPacket, fileSize);

    unsigned char allPackets[MAXFILESIZE + 1000];
    int allPacketsLength = 0;
    for(int i = 0; i < startPacketLength; i++)
    {
        allPackets[allPacketsLength] = startPacket[i];
        allPacketsLength++;
    }
    for(int i = 0; i < dataPacketLength; i++)
    {
        allPackets[allPacketsLength] = dataPacket[i];
        allPacketsLength++;
    }
    for(int i = 0; i < endPacketLength; i++)
    {
        allPackets[allPacketsLength] = endPacket[i];
        allPacketsLength++;
    }

    /////////////////
    /// Send Data ///
    /////////////////

    // start connection
    int returnValue = llOpen(portDescriptor, TXTYPE);
    if(returnValue < 0)
    {
        printf("Could not start connection.\n Exiting.");
        return -1;
    }

    // send packets
    returnValue = llWrite(portDescriptor, allPackets, allPacketsLength, TXTYPE);
    if(returnValue < 0) 
    {
        printf("Could not send packets. Exiting.\n");
        return -1;
    }

    // finish connection
    returnValue = llClose(portDescriptor, TXTYPE);
    if(returnValue < 0)
    {
        printf("Could not start connection.\n Exiting");
        return -1;
    }


    /////////////////////
    /// Close program ///
    /////////////////////
    closePort(portDescriptor);
}