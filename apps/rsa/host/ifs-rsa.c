/* copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <err.h>
#include <stdio.h>
#include <string.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

#include <unistd.h>    /* unix func def */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      /* file control def */
#include <termios.h>    /* PPSIX terminal ctrl def */
#include <errno.h>
#include <stdlib.h>


#define TA_RSA_DEMO_UUID { 0x5b85f6c9, 0xebcf, 0x490d, \
    { 0xbe, 0x73, 0x60, 0xa7, 0xa2, 0x5a, 0x61, 0xdf } }

/* The Trusted Application Function ID(s) implemented in this TA */
#define TA_DECRYPT_DEMO_CMD_RSA 0



static TEEC_Context ctx;
static TEEC_Session sess;

int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio,oldtio;
    if  ( tcgetattr( fd,&oldtio)  !=  0)
    {
        perror("SetupSerial 1");
        return -1;
    }
    bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag  |=  CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;
    newtio.c_iflag &= ~(IXON | IXOFF | IXANY);
    newtio.c_iflag &= ~(INLCR | ICRNL | IGNCR);
    newtio.c_oflag &= ~(ONLCR | OCRNL);

    switch( nBits )
    {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
    }

    switch( nEvent )
    {
        case 'O':
            newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            newtio.c_iflag |= (INPCK | ISTRIP);
            break;
        case 'E':
            newtio.c_iflag |= (INPCK | ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;
        case 'N':
            newtio.c_cflag &= ~PARENB;
            break;
    }

    switch( nSpeed )
    {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        default:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
    }
    if( nStop == 1 )
    {
        newtio.c_cflag &=  ~CSTOPB;
    }
    else if ( nStop == 2 )
    {
        newtio.c_cflag |=  CSTOPB;
    }
    newtio.c_cc[VTIME]  = 0;
    newtio.c_cc[VMIN] = 16;
    tcflush(fd,TCIFLUSH);
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        perror("com set error");
        return -1;
    }
    printf("set done!\n");
    return 0;
}

int open_port(int fd,int comport)
{
    char *dev[]={"/dev/ttyAMA0","/dev/ttyAMA1","/dev/ttyAMA2"};
    long  vdisable;
    if (comport==1)
    {    fd = open( "/dev/ttyAMA0", O_RDWR|O_NOCTTY|O_NDELAY);
        if (-1 == fd)
        {
            perror("Can't Open Serial Port");
            return(-1);
        }
        else
        {
            printf("open ttyAMA0 .....\n");
        }
    }
    else if(comport==2)
    {    fd = open( "/dev/ttyAMA1", O_RDWR|O_NOCTTY|O_NDELAY);
        if (-1 == fd)
        {
            perror("Can't Open Serial Port");
            return(-1);
        }
        else
        {
            printf("open ttyAMA1 .....\n");
        }
    }
    else if (comport==3)
    {
        fd = open( "/dev/ttyAMA2", O_RDWR|O_NOCTTY|O_NDELAY);
        if (-1 == fd)
        {
            perror("Can't Open Serial Port");
            return(-1);
        }
        else
        {
            printf("open ttyAMA2 .....\n");
        }
    }
    else if(comport==4)
    {    fd = open( "/dev/ttyAMA3", O_RDWR|O_NOCTTY|O_NDELAY);
        if (-1 == fd)
        {
            perror("Can't Open Serial Port");
            return(-1);
        }
        else
        {
            printf("open ttyAMA3 .....\n");
        }
    }
    if(fcntl(fd, F_SETFL, 0)<0)
    {
        printf("fcntl failed!\n");
    }
    else
    {
        printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
    }
    if(isatty(STDIN_FILENO)==0)
    {
        printf("standard input is not a terminal device\n");
    }
    else
    {
        printf("isatty success!\n");
    }
    printf("fd-open=%d\n",fd);
    return fd;
}


int main(int argc, char *argv[])
{
    TEEC_Result res;
    TEEC_Operation op;
    TEEC_UUID uuid = TA_RSA_DEMO_UUID;
    uint32_t err_origin;
    int i;
    /* set variable */
    char outData[2048]={"0"};


    int nread,temp;
    char buff[16]="";
    int fd;
    /* Initialize a context connecting us to the TEE */
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

    /* open a session */
    res = TEEC_OpenSession(&ctx, &sess, &uuid,
            TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
                res, err_origin);
    /* -= start RSA algorithm =- */

    if((fd=open_port(fd,3))<0)
    {
        perror("open_port error");
        return 1;
    }
    if((i=set_opt(fd,115200,8,'N',1))<0)
    {
        perror("set_opt error");
        return 1;
    }
    printf("fd=%d\n",fd);
    while(1){
        /* read from serial */
        memset(buff,0,sizeof(buff));
        nread = read(fd, buff,16);//16 byte input
        /*if(nread != -1)
            printf("read complete \n");
        printf("input is :\n");
        for (i = 0 ; i < 16; i ++) {
            printf("%02x ", buff[i]);
        }
        printf("\n");*/

        memset(&op, 0, sizeof(op));
        op.paramTypes = TEEC_PARAM_TYPES (TEEC_MEMREF_TEMP_INOUT,
                TEEC_MEMREF_TEMP_INOUT,
                TEEC_VALUE_INPUT,
                TEEC_NONE);

        op.params[0].tmpref.buffer = buff;
        op.params[0].tmpref.size = sizeof (buff);
        op.params[1].tmpref.buffer = outData;
        op.params[1].tmpref.size = sizeof (outData);
        op.params[2].value.a = 1;

        res = TEEC_InvokeCommand(&sess, TA_DECRYPT_DEMO_CMD_RSA, &op,
                &err_origin);
        if (res != TEEC_SUCCESS)
            errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
                    res, err_origin);
        /*printf("output is :\n");
        for (i = 0 ; i < op.params[1].tmpref.size; i ++) {
            printf("%02x ", outData[i]);
        }
        printf("\n");*/

        /* write to serial */
        temp = write(fd, outData,op.params[1].tmpref.size);
        /*if (temp != -1){
            printf("write complete\n");
        }*/
    }

    close(fd);
    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);
    return 0;
}
