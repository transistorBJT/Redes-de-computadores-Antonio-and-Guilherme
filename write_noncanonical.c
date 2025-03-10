// Write to serial port in non-canonical mode
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

// Baudrate settings are defined in <asm/termbits.h>, which is
// included by <termios.h>
#define BAUDRATE B38400
#define _POSIX_SOURCE 1 // POSIX compliant source

#define FALSE 0
#define TRUE 1

#define BUF_SIZE 1


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

    // Open serial port device for reading and writing, and not as controlling tty
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

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // Set input mode (non-canonical, no echo,...)
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0; // Inter-character timer unused
    newtio.c_cc[VMIN] = 5;  // Blocking read until 5 chars received

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

    printf("New termios structure set\n");

    // ORIGINAL
    /*
    // Create string to send
    unsigned char buf[BUF_SIZE] = {0};

    for (int i = 0; i < BUF_SIZE; i++)
    {
        buf[i] = 'a' + i % 26;
    }

    // In non-canonical mode, '\n' does not end the writing.
    // Test this condition by placing a '\n' in the middle of the buffer.
    // The whole buffer must be sent even with the '\n'.
    buf[5] = '\n';

    int bytes = write(fd, buf, BUF_SIZE);
    printf("%d bytes written\n", bytes);
    */

    // Create string to send
    unsigned char buf[BUF_SIZE] = {0};
    int bytes;
    
    unsigned char buf2[5] = {0};
    buf2[0]=0x7E;//FLAG
    buf2[1]=0x03;//a
    buf2[2]=0x03;//C
    buf2[3]=buf[1]^buf[2];
    buf2[4]=buf[0];

// envia o set
  /*  for(int i=0;i<5;i++){
    buf[0]=buf2[i];
    bytes=write(fd, buf, BUF_SIZE);
    printf("%x byte enviado\n", buf[0]);
    }*/

//ler ua
unsigned char ua[BUF_SIZE]={0};
bytes=read(fd,ua,BUF_SIZE);
printf("UA=%x\n",ua[0]);


//máquina estados para o alarme
int state=0;
while(state !=2){


switch(state):
case 0:
   for(int i=0;i<5;i++){
    buf[0]=buf2[i];
    bytes=write(fd, buf, BUF_SIZE);
    printf("%x byte enviado\n", buf[0]);
    }
state=1;
break;

case 1:
//espera pelo ua, ou pelo cnt_alarme >3(vai para o 2), ou que passem 3 segundos(vai para o 0)
if((buf[0]==0x03) ||  alarmCount>3  ){state=2;}
else if(alarmEnabled){state=0;}
break;



case 2:
//novo set aqui?
break;
}




  
 /*   bytes = read(fd, buf, BUF_SIZE);
    for (int i = 0; i<BUF_SIZE; i++){
        printf(":0x%x\n", buf[i]);
    }*/


    // Wait until all bytes have been written to the serial port
    sleep(1);

    // Restore the old port settings
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    close(fd);

    return 0;
}

