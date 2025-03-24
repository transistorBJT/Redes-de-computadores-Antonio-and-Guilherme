#define FLAG            0X7E

#define TXSEND          0X03
#define RXSEND          0X01

#define CSETUP          0X03
#define UACK            0X07
#define RXREADY0        0X05
#define RXREADY1        0X85
#define RXREJ0          0X01
#define RXREJ1          0X81
#define CONECTIONFINISH 0X0B
#define CONTROL0        0X00
#define CONTROL1        0X40

#define ESC1            0X7D
#define ESC2            0X5E
#define ESC3            0x5D

#define MAXBUFSIZE      2048

#define TXTYPE          1337
#define RXTYPE          1048

#define BAUDRATE B38400


int openPort(char* portName);
void closePort(int portDescriptor);

int byteStuffing(unsigned char* input, unsigned char* output, int inputLength);
int byteDestuffing(unsigned char* input, unsigned char* output, int inputLength);

int createSetFrame(unsigned char* output, int type);
int createUaFrame(unsigned char* output, int type);
int createRR0Frame(unsigned char* output, int type);
int createRR1Frame(unsigned char* output, int type);
int createREJ0Frame(unsigned char* output, int type);
int createREJ1Frame(unsigned char* output, int type);
int createDiscFrame(unsigned char* output, int type);
int createInformationFrame(unsigned char* output, int type, int frameCounter, unsigned char* data, int dataLength);

int validateSetFrame(unsigned char* frame, int type);
int validateUaFrame(unsigned char* frame, int type);
int validateRR0Frame(unsigned char* frame, int type);
int validateRR1Frame(unsigned char* frame, int type);
int validateREJ0Frame(unsigned char* frame, int type);
int validateREJ1Frame(unsigned char* frame, int type);
int validateDiscFrame(unsigned char* frame, int type);
int validateInformationFrame(unsigned char* frame, int frameLength, int type, int frameCounter);


void writeFrame(int portDescriptor, unsigned char* input, int frameLength);
int readFrame(int portDescriptor, unsigned char* output);