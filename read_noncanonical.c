#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define BUF_SIZE 256

// SET buffer values
#define FLAG        0x7E
#define A           0x03
#define C           0x03
#define C_UA        0x07
#define BCC         (A^C)

#define START       0
#define FLAG_RCV    1
#define A_RCV       2
#define C_RCV       3
#define BCC_OK      4
#define S_STOP      5

volatile int STOP = FALSE;

int main(int argc, char *argv[])
{
    // Program usage: Uses either COM1 or COM2
    const char *serialPortName = argv[1];

    if (argc < 2)
    {
        printf("Incorrect program usage\n"
               "Usage: %s <SerialPort>\n"
               "Example: %s /dev/ttyS1\n",
               argv[0],
               argv[0]);
        exit(1);
    }

    // Open serial port device for reading and writing and not as controlling tty
    // because we don't want to get killed if linenoise sends CTRL-C.
    int fd = open(serialPortName, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(serialPortName);
        exit(-1);
    }

    struct termios oldtio;
    struct termios newtio;

    // Save current port settings
    if (tcgetattr(fd, &oldtio) == -1)
    {
        perror("tcgetattr");
        exit(-1);
    }

    // Clear struct for new port settings
    memset(&newtio, 0, sizeof(newtio));

    newtio.c_cflag      = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag      = IGNPAR;
    newtio.c_oflag      = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag      =   0;
    newtio.c_cc[VTIME]  =   0; // Inter-character timer unused
    newtio.c_cc[VMIN]   =   0;  // Blocking read until 5 chars received

    // VTIME e VMIN should be changed in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_selector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }
    //printf(fd);
    //printf("\n");

    printf("New termios structure set\n");

    // Loop for input
    //unsigned char byte[5] = {0};
    unsigned char byte = 0;
    
    int state = START;

    int bytes = 0;
    
    while (STOP == FALSE)
    {

        bytes = read(fd, &byte, 1);
        
       if(bytes >0){    
        printf("%x\n", byte);
        switch(state)
            {
                case START:
                    printf("started\n");
                    //printf("Start1");
                    if(byte == FLAG){
                        printf("FlagRCV\n");
                        state = FLAG_RCV;
               
                    }
                    break;
                case FLAG_RCV:
                    if (byte == A){
                        state = A_RCV;
                        printf("a_RCV\n");
                    }
                    else if(byte != FLAG)           
                        state = START;
                    break;
                case A_RCV:
                    if (byte == C){
                        printf("C_RCV\n");
                        state = C_RCV;
                    }
                    else if (byte == FLAG)
                        state = FLAG_RCV;
                    else
                        state = START;
                    break;
                case C_RCV:
                    if (byte == BCC){
                        state = BCC_OK;
                        printf("BCC\n");
                    }
                    else if (byte == FLAG)
                        state = FLAG_RCV;
                    else
                        state = START;
                    break;
                case BCC_OK:
                    if (byte == FLAG){
                        STOP = TRUE;
                        printf("STOP\n");                    
                    }
                    else
                        state = START;
                    break;
            }
        }
    }
    sleep(6);
    
    // Returns after 5 chars have been input
    unsigned char buf[5];
    buf[0] = FLAG;
    buf[1] = A;
    buf[2] = C_UA;
    buf[3] = BCC;
    buf[4] = FLAG;
    bytes = write(fd, buf, sizeof(buf));
    printf("%d bytes written\n", bytes);
    

    // The while() cycle should be changed in order to respect the specifications
    // of the protocol indicated in the Lab guide

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 0;
}
