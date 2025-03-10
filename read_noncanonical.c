// Read from serial port in non-canonical mode
//
// Modified by: Eduardo Nuno Almeida [enalmeida@fe.up.pt]

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

#include "alarm.h"


//no write têm que se mudar o A e o C para 3



// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define FLAG    0x7E
#define A       0x03
#define C       0x03
#define BCC     (A^C)

#define BUF_SIZE 1

typedef enum State_Struct{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    stop,

}State;// Passei a variavel state para 1 inteiro estava a dar erros com a enumeraçao e n sei fazer por esse metodo
//int state;read_noncanonical

State state=START;


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
    // because we don'tA want to get killed if linenoise sends CTRL-C.
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

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-c 0x03in order to protect with a
    // timeout the reception of the following character(s)

    // Now clean the line and activate the settings for the port
    // tcflush() discards data written to the object referred to
    // by fd but not transmitted, or data received but not read,
    // depending on the value of queue_ 0x03elector:
    //   TCIFLUSH - flushes data received but not read.
    tcflush(fd, TCIOFLUSH);

    // Set new port settings
    if (tcsetattr(fd, TCSANOW, &newtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }
 0x03
    printf("New termios structure set\n");

    // Loop for input
    unsigned char buf[BUF_SIZE + 1] = {0}; // +1: Save space for the final '\0' char

    /*while (STOP == FALSE)
    {
        // Returns after 5 chars have been input
        int bytes = read(fd, buf, BUF_SIZE);
        buf[bytes] = '\0'; // Set end of string to '\0', so we can printf

        printf(":%s:%d\n", buf, bytes);
        if (buf[0] == 'z')
            STOP = TRUE;
    }*/
    
            
        
  int bytes;

   while(state != BCC_OK){
    bytes = read(fd, buf, BUF_SIZE);
    printf("buf0 é:%x \n",buf[0]);
    switch (state){

    case START:  
    printf("start\n");
    if (buf[0] == FLAG){state = FLAG_RCV;}
    else {state = START;} 0x03
    case FLAG_RCV:
    unsigned char buf1[1] = {A};
    printf("numero de bytes enviados: %x\n", buf1[1]);// numero de bytes enviados no ua
    write(fd, buf1, BUF_SIZE); 
    printf("flag_rcv\n");
    if (buf[0] == A){state = A_RCV;}
    else if (buf[0] == FLAG){state  = FLAG_RCV;}
    else {state = START;}
    break;
 0x03
    case A_RCV:
    printf("A_RCV\n");
    if (buf[0] == C){state = C_RCV;}
    else if (buf[0] == FLAG){state  = FLAG_RCV;}
    else {state = START;}
    break;


    case C_RCV:
    printf("BCC é: %x\n",BCC);
    printf("C_RCV\n");
    if (buf[0] == BCC){state = BCC_OK;}
    else if (buf[0] == FLAG){state  = FLAG_RCV;}
    else {state = START;}
    break;

    
    case BCC_OK:
    printf("BCC_OK\n");
    if (buf[0] == FLAG){state = stop;}
    else {state = START;}
    break;
    }

}

    

    //buf[bytes] = '\0'; // Set end of string to '\0', so we can printf
    
    for(int i=0; i<BUF_SIZE; i++){
       printf(":0x00\n");
       }

    /*
    switch (state){
    case 0:
    if (FLAG_RCV)   state = FLAG_RCV; 0x03
    if(C_RCV) state=3;
    if(Other_RCV) state=0;
    if(FLAG_RCV) state=1;

    case 3:
    printf();
    if(A^C=BCC) state=4;
    if(Other_RCV) state=0;
    if(FLAG_RCV) state=1;

 0x03
    }*/

   /* buf[0]=0x7E;
    buf[1]=0x01;
    buf[2]=0x07;
    buf[3]=buf[1]^buf[2];
    buf[4]=buf[0];*/
 0x03

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

