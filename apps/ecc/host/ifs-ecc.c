/*
 * Copyright (c) 2016, Linaro Limited
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
/* set serial port variable */
#include <unistd.h>    /* unix func def */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      /* file control def */
#include <termios.h>    /* PPSIX terminal ctrl def */
#include <errno.h>
#include <stdlib.h>

#define TA_CRYPT_DEMO_ECC_UUID {0xde8c5e8f, 0x2e10, 0x41ea, \
    { 0xa8, 0x59, 0xca, 0x8f, 0xed, 0xdb, 0x88, 0x88 } }

/* The Trusted Application Function ID(s) implemented in this TA */
#define TA_ENCRYPT_DEMO_CMD_AES 0
#define TA_ENCRYPT_DEMO_CMD_RSA 1
#define TA_ENCRYPT_DEMO_CMD_DIGEST 2
#define TA_SIGN_DEMO_CMD_ECC 3

static TEEC_Context ctx;
static TEEC_Session sess;

/* set option of serial port */
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio,oldtio;
    //#if 0
    if  ( tcgetattr( fd,&oldtio)  !=  0)
    {
        perror("SetupSerial 1");
        return -1;
    }
    //#endif
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
    newtio.c_cc[VMIN] = 64;
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
    TEEC_UUID uuid = TA_CRYPT_DEMO_ECC_UUID;
    uint32_t err_origin;
    uint8_t i = 0;

    /* ciphertext variable */
//    uint8_t outData[64]="0";
    uint8_t outData[64]={0};

    /* serial variable */
    int nread1,nread2,temp;
//    char buff[32] = "";
    char buff[64] = {0};
    int fd;

    /* Initialize a context connecting us to the TEE */
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

    res = TEEC_OpenSession(&ctx, &sess, &uuid,
            TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
                res, err_origin);



    if((fd=open_port(fd,3))<0)
    {
        perror("open_port error");
        return 0;
    }
    if((i=set_opt(fd,115200,8,'N',1))<0)
    {
        perror("set_opt error");
        return 0;
    }
    printf("fd=%d\n",fd);

    do {
        /* read from serial */
        memset(buff,0,sizeof(buff));
        nread1 = read(fd, buff, 64);//64 byte input
#if 0
        if(nread1 != -1)
            printf("read1 complete \n");
        if(nread2 != -1)
            printf("read2 complete \n");
        for (i = 0 ; i<64; i ++)
            printf("%02x ", buff[i]);
        printf("\n");
        printf("the size is %d \n",(int )sizeof(buff));
#endif
        memset(&op, 0, sizeof(op));
        op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,
                TEEC_MEMREF_TEMP_INOUT,
                TEEC_VALUE_INPUT,
				TEEC_NONE);

        /* normal route */
        op.params[0].tmpref.buffer = buff;
        op.params[0].tmpref.size = sizeof (buff);
        op.params[1].tmpref.buffer = outData;
        op.params[1].tmpref.size = sizeof (outData); //outData = 64 bytes

	//printf("I'm starting to invoke command\n");
        res = TEEC_InvokeCommand(&sess, TA_SIGN_DEMO_CMD_ECC, &op,
                &err_origin);
	//printf("Hello, invoking command finished\n");
        if (res != TEEC_SUCCESS)
            errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
                    res, err_origin);

        /* write to serial */
        temp = write(fd, outData, op.params[1].tmpref.size);
#if 0
        if (temp != -1){
            printf("write complete\n");
        }


        /* test purpose only */
	for(int j=0; j<64; j++) {
	   printf("%02x ", outData[j]);
	}
	printf("\n");
        printf("ecc CA execution done!\n");
#endif
    } while(1);
    close(fd);

    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);

    return 0;
}
