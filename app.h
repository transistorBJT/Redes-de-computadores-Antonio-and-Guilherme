#define MAXFILESIZE 100000
#define MAXFILENAMELENGTH 200

int openPort(char* portName);
void closePort(int portDescriptor);

int createStartPacket(unsigned char* packet, unsigned char* fileName, int fileNameLength);
int createDataPacket(unsigned char* output, unsigned char* data, int dataSize);
int createEndPacket(unsigned char* packet, int fileSize);

void processData(unsigned char* data, int dataLength, unsigned char* startPacket, int* startPacketLength, unsigned char* dataPacket, int* dataPacketLength, unsigned char* endPacket, int* endPacketLength);

int processStartPacket(unsigned char* output, unsigned char* packet, int packetLength);
int processDataPacket(unsigned char* output, unsigned char* packet);

int llOpen(int portDescriptor, int type);
int llClose(int portDescriptor, int type);
int llWrite(int portDescriptor, unsigned char* data, int dataLength, int type);
int llRead(int portDescriptor, unsigned char* data, int type);