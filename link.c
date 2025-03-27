#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "link.h"


/////////
/// 1 ///
/////////

// Slide 30 - Stuffing/Destuffing

int byteStuffing(unsigned char* input, unsigned char* output, int inputLength)
{
    int outputCounter = 0;

    for(int inputCounter = 0; inputCounter < inputLength; inputCounter++)
    {
        if(input[inputCounter] == FLAG)
        {
            output[outputCounter] = ESC1;
            output[outputCounter + 1] = ESC2;

            outputCounter = outputCounter + 2 ;

            continue;
        }

        if (input[inputCounter] == ESC1)
        {
            output[outputCounter] = ESC1;
            output[outputCounter + 1] = ESC3;

            outputCounter = outputCounter + 2;

            continue;
        }

        output[outputCounter] = input[inputCounter];
        outputCounter++;
    }

    return outputCounter;
}

int byteDestuffing(unsigned char* input, unsigned char* output, int inputLength)
{
    int outputCounter = 0;
    
    for(int inputCounter = 0; inputCounter < inputLength; inputCounter++)
    {
        if(input[inputCounter] == ESC1 && input[inputCounter + 1] == ESC3)
        {
            output[outputCounter] = ESC1;

            outputCounter++;
            inputCounter++;

            continue;
        }

        if(input[inputCounter] == ESC1 && input[inputCounter + 1] == ESC2)
        {
            output[outputCounter] = FLAG;

            outputCounter++;
            inputCounter++;

            continue;
        }

        output[outputCounter] = input[inputCounter];
        outputCounter++;
    }

    return outputCounter;
}

/////////
/// 2 ///
/////////

// Slide 16 - Create Supervision frames
int createSetFrame(unsigned char* output, int type)
{
    // flag
    output[0] = FLAG;

    // address
    if (type == TXTYPE)
    {
        output[1] = TXSEND;
    }

    if (type == RXTYPE)
    {
        output[1] = RXSEND;
    }

    // control
    output[2] = CSETUP;

    // bcc
    output[3] = output[1] ^ output[2];

    // flag
    output[4] = FLAG;

    return 5;
}

int createUaFrame(unsigned char* output, int type)
{
    //flag
    output[0] = FLAG;

    //adress
    if (type == TXTYPE)
    {
        output[1] = TXSEND;
    }

    if (type == RXTYPE)
    {
        output[1] = RXSEND;
    }

    //control
    output[2] = UACK;

    //bcc
    output[3] = output[1] ^ output[2];

    //flags
    output[4] = FLAG;

    return 5;
}

int createRR0Frame(unsigned char* output, int type)
{
    //flag
    output[0] = FLAG;

    //adress
    if (type == TXTYPE)
    {
        output[1] = TXSEND;
    }

    if (type == RXTYPE)
    {
        output[1] = RXSEND;
    }

    //control
    output[2] = RXREADY0;

    //bcc
    output[3] = output[1] ^ output[2];

    //flags
    output[4] = FLAG;

    return 5;
}

int createRR1Frame(unsigned char* output, int type)
{
    //flag
    output[0] = FLAG;

    //adress
    if (type == TXTYPE)
    {
        output[1] = TXSEND;
    }

    if (type == RXTYPE)
    {
        output[1] = RXSEND;
    }

    //control
    output[2] = RXREADY1;

    //bcc
    output[3] = output[1] ^ output[2];

    //flags
    output[4] = FLAG;

    return 5;
}

int createREJ0Frame(unsigned char* output, int type)
{
    //flag
    output[0] = FLAG;

    //adress
    if (type == TXTYPE)
    {
        output[1] = TXSEND;
    }

    if (type == RXTYPE)
    {
        output[1] = RXSEND;
    }

    //control
    output[2] = RXREJ0;

    //bcc
    output[3] = output[1] ^ output[2];

    //flags
    output[4] = FLAG;

    return 5;
}

int createREJ1Frame(unsigned char* output, int type)
{
    //flag
    output[0] = FLAG;

    //adress
    if (type == TXTYPE)
    {
        output[1] = TXSEND;
    }

    if (type == RXTYPE)
    {
        output[1] = RXSEND;
    }

    //control
    output[2] = RXREJ1;

    //bcc
    output[3] = output[1] ^ output[2];

    //flags
    output[4] = FLAG;

    return 5;
}

int createDiscFrame(unsigned char* output, int type)
{
    //flag
    output[0] = FLAG;

    //adress
    if (type == TXTYPE)
    {
        output[1] = TXSEND;
    }

    if (type == RXTYPE)
    {
        output[1] = RXSEND;
    }

    //control
    output[2] = CONECTIONFINISH;

    //bcc
    output[3] = output[1] ^ output[2];

    //flags
    output[4] = FLAG;

    return 5;
}

/////////
/// 2 ///
/////////
// Slide 17 - Create Information frames
int createInformationFrame(unsigned char* output, int type, int frameCounter, unsigned char* data, int dataLength)
{
    // 1 - flag
    output[0] = FLAG;
    
    // 2 - address
    if(type == TXTYPE){
        output[1] = TXSEND;
    }

    if(type == RXTYPE)
    {
        output[1] = RXSEND;
    }

    // 3 - control
    if(frameCounter%2 == 0)
    {
        output[2] = CONTROL0;
    }

    if(frameCounter%2 == 1)
    {
        output[2] = CONTROL1;
    }

    // 4 - bcc1 
    output[3] = output[1] ^ output[2];

    // 5 - data + bcc2

    // 5.1 - calculate bcc2
    unsigned char bcc2 = data[0];
    for(int i=1; i < dataLength; i++)
    {
        bcc2 = bcc2 ^ data[i];
    }

    // 5.2 - add bcc2 to data
    unsigned char dataWithBcc2[MAXBUFSIZE] = "";
    for(int i=0; i < dataLength; i++)
    {
        dataWithBcc2[i] = data[i];
    }

    dataWithBcc2[dataLength] = bcc2;

    // 5.3 - byte stuffing
    unsigned char stuffedDataWithBcc2[MAXBUFSIZE] = "";
    int stuffedDataWithBcc2Length = byteStuffing(dataWithBcc2, stuffedDataWithBcc2, dataLength + 1);


    // 5.4 - insert data on frame
    for(int i=0; i < stuffedDataWithBcc2Length; i++)
    {
        output[4 + i] = stuffedDataWithBcc2[i];
    }

    // 6 - flag
    output[4 + stuffedDataWithBcc2Length] = FLAG;

    // frame size
    return 5 + stuffedDataWithBcc2Length;
}

/////////
/// 3 ///
/////////

// Slide 24 - Validate Supervision frames

int validateSetFrame(unsigned char* frame, int type)
{   
    //validate address
    if(type == RXTYPE)
    {
        if(frame[1] != TXSEND)
        {   
            return 0;
        }
    }

    if(type == TXTYPE)
    {
        if(frame[1] != RXSEND)
        {   
            return 0;
        }
    }

    //validate control
    if(frame[2] != CSETUP)
    {   
        return 0;
    }

    //validate BCC
    unsigned char validateBcc1 = frame[1] ^ frame[2];

    if(frame[3] != validateBcc1)
    {   
        return 0;
    }

    return 1;
}

int validateUaFrame(unsigned char* frame, int type)
{
    //validate addresss
    if(type == RXTYPE)
    {
        if(frame[1] != TXSEND)
        {
            return 0;
        }
    }

    if(type == TXTYPE)
    {
        if(frame[1] != RXSEND)
        {
            return 0;
        }
    }

    //validate control
    if(frame[2] != UACK)
    {
        return 0;
    }

    //validate BCC
    unsigned char validateBcc1 = frame[1] ^ frame[2];

 
    if(frame[3] != validateBcc1)
    {
        return 0;
    }

    return 1;
}

int validateRR0Frame(unsigned char* frame, int type)
{   
    //validate address
    if(type == RXTYPE)
    {
        if(frame[1] != TXSEND)
        {
            return 0;
        }
    }

    if(type == TXTYPE)
    {
        if(frame[1] != RXSEND)
        {
            return 0;
        }
    }

    //validate control
    if(frame[2] != RXREADY0)
    {
        return 0;
    }

    //validate BCC
    unsigned char validateBcc1 = frame[1] ^ frame[2];

    if(frame[3] != validateBcc1)
    {
        return 0;
    }

    return 1;
}

int validateRR1Frame(unsigned char* frame, int type)
{   
    //validate address
    if(type == RXTYPE)
    {
        if(frame[1] != TXSEND)
        {
            return 0;
        }
    }

    if(type == TXTYPE)
    {
        if(frame[1] != RXSEND)
        {
            return 0;
        }
    }

    //validate control
    if(frame[2] != RXREADY1)
    {
        return 0;
    }

    //validate BCC
    unsigned char validateBcc1 = frame[1] ^ frame[2];

    if(frame[3] != validateBcc1)
    {
        return 0;
    }

    return 1;
}

int validateREJ0Frame(unsigned char* frame, int type)
{   

    //validate address
    if(type == RXTYPE)
    {
        if(frame[1] != TXSEND)
        {
            return 0;
        }
    }

    if(type == TXTYPE)
    {
        if(frame[1] != RXSEND)
        {
            return 0;
        }
    }

    //validate control
    if(frame[2] != RXREJ0)
    {
        return 0;
    }

    //validate BCC
    unsigned char validateBcc1 = frame[1] ^ frame[2];

    if(frame[3] != validateBcc1)
    {
        return 0;
    }

    return 1;
    
}

int validateREJ1Frame(unsigned char* frame, int type)
{   
    //validate address
    if(type == RXTYPE)
    {
        if(frame[1] != TXSEND)
        {
            return 0;
        }
    }

    if(type == TXTYPE)
    {
        if(frame[1] != RXSEND)
        {
            return 0;
        }
    }

    //validate control
    if(frame[2] != RXREJ1)
    {
        return 0;
    }

    //validate BCC
    unsigned char validateBcc1 = frame[1] ^ frame[2];

    if(frame[3] != validateBcc1)
    {
        return 0;
    }
    
    return 1;
}

int validateDiscFrame(unsigned char* frame, int type)
{   
    //validate address
    if(type == RXTYPE)
    {
        if(frame[1] != TXSEND)
        {
            return 0;
        }
    }

    if(type == TXTYPE)
    {
        if(frame[1] != RXSEND)
        {
            return 0;
        }
    } 

    //validate control
    if(frame[2] != CONECTIONFINISH)
    {
        return 0;
    }

    //validate BCC
    unsigned char validateBcc1 = frame[1] ^ frame[2];

    if(frame[3] != validateBcc1)
    {
        return 0;
    }

    return 1;
}

/////////
/// 3 ///
/////////

// Slide 25 - Validate information frames
int validateInformationFrame(unsigned char* frame, int frameLength, int type, int frameCounter)
{   
   
    //validate address
    if(type == RXTYPE)
    {
        if(frame[1] != TXSEND)
        {
            return 0;
        }
    }

    if(type == TXTYPE)
    {
        if(frame[1] != RXSEND)
        {
            return 0;
        }
    }
   
    //validate Control
    if(frameCounter%2 == 0 )
    {
        if(frame[2] != CONTROL0)
        {
            return 0;
        }
    }

    if(frameCounter%2 == 1)
    {
        if(frame[2] != CONTROL1)
        {
            return 0;
        }
    }

    //validate bcc1
    unsigned char validateBcc1 = frame[1] ^ frame [2];

    if(frame[3] != validateBcc1 )
    {
        return 0;
    }

    //validate bcc2
    unsigned char validateBcc2 = frame[4];

    for(int i=5; i< frameLength - 2; i++)
    {
        validateBcc2 = validateBcc2 ^ frame[i];
    }

    if(validateBcc2 != frame[frameLength -2])
    {
        return 0;
    }

    //frame OK
    return 1;    
}

/////////
/// 4 ///
/////////

// Slides 21, 22, 23.
//
// The state machine logic was not implemented exactly as the slides.
// It is similar, but this way was more "intuitive" for the group and worked.

int state = 0;

void writeFrame(int portDescriptor, unsigned char* input, int frameLength)
{

    for(int i=0; i < frameLength; i++)
    {
        write(portDescriptor, &input[i], 1);
    }

}

int readFrame(int portDescriptor, unsigned char* output)
{
    
    int flagCounter = 0;
    int bytesRead = 0;
    unsigned char buffer;
    int outputCounter = 0;

    while(1)
    {   
        usleep(1000);

        if(state == transmitting)
        {
            return 0;
        }

        if(flagCounter == 2)
        {
            return outputCounter;
        }

        bytesRead = read(portDescriptor, &buffer, 1);
        
        if(bytesRead == 0)
        {
            continue;
        }

        if(flagCounter == 0)
        {
            if(buffer != FLAG)
            {
                continue;
            }

            flagCounter++;

            output[outputCounter] = FLAG;
            outputCounter++;

            continue;
        }

        if(flagCounter == 1)
        {
            output[outputCounter] = buffer;
            outputCounter ++;

            if(buffer == FLAG)
            {
                flagCounter++;
                continue;
            }
        }
    }

}

/////////
/// 5 ///
/////////

// Slides 26, 27, 28, 29
// 
// Alarm implementation according to the slides.
//

int timeoutCounter = 0;
void timeoutHandler(int signal)
{
    timeoutCounter++;
    if(timeoutCounter ==  maxTimeouts)
    {
        printf("Reached maximum amount of timeouts! Transmisson stopping!\n");
        exit(1);    
    }

    state = transmitting;
}
