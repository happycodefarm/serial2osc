//
//  main.c
//  serial2osc
//
//  Created by guillaume on 24/01/2015.
//  Copyright (c) 2015 Guillaume Stagnaro. All rights reserved.
//
// gcc main.c -o serial2osc -llo

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <strings.h>
#include <unistd.h>
#include "lo/lo.h"

#include <fcntl.h>
#include <termios.h>

#include <dirent.h>


#define USB_SERIAL_PORT "/dev/tty.usbmodem1411"
#define ARGS    "s:b:p:t:hv"


int port_fd;
char portName[256];
int baudRate = B9600;

char *oscPort = "7770";
char *oscIP = "localhost";

char verbose = 0;
int	c, errflg = 0;

int init_serial_input (char * port) {
    int fd = 0;
    struct termios options;
    
    fd = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1)
        return fd;
    fcntl(fd, F_SETFL, 0);    // clear all flags on descriptor, enable direct I/O
    tcgetattr(fd, &options);   // read serial port options
    // enable receiver, set 8 bit data, ignore control lines
    options.c_cflag |= (CLOCAL | CREAD | CS8);
    // disable parity generation and 2 stop bits
    options.c_cflag &= ~(PARENB | CSTOPB);
   
    cfsetispeed(&options, baudRate);
    cfsetospeed(&options, baudRate);
    
    // set the new port options
    tcsetattr(fd, TCSANOW, &options);
    return fd;
}

int read_serial_int (int fd) {
    char ascii_int[8] = {0};
    char c = -1;
    int i = 0;
    
    read(fd, &c, 1);
    while (c != '\n') {
        ascii_int[i++] = c;
        read(fd, &c, 1);
       
    }
    if (verbose==1) printf("%s\n",ascii_int);
    return atoi(ascii_int);
}

void  INThandler(int sig)
{
    
    close(port_fd);
    exit(0);
 
}

int selectSerialPort() {
    
    DIR           *d;
    struct dirent *dir;
    d = opendir("/dev/");
    
    int choice = -1;
    int serialPortCount = 0;
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            char* pos = strstr(dir->d_name,"tty.");
            if(pos) {
              printf("[%i] %s\n",serialPortCount,  dir->d_name);
                serialPortCount++;
            }
        }
        closedir(d);
        
        printf("\nPlease select a serial port: ");
        char c = getchar();
        
        if (c>='0' || c <= '9') {
            choice  = c-'0';
            serialPortCount = 0;
             d = opendir("/dev/");
            while ((dir = readdir(d)) != NULL)
            {
                char* pos = strstr(dir->d_name,"tty.");
                if(pos) {
                    if (serialPortCount==choice) {
                        sprintf(portName,"/dev/%s", dir->d_name);

                        printf("\nYou selected %s\n",portName);
                        
                        closedir(d);
                        return choice;
                    }
                    serialPortCount++;
                }
            }
        }
        closedir(d);
    }
    
    return -1;
}

int main(int argc, char **argv) {
    char showSerialPortSelection = 1; // show serial port selection by default.
    
    // read arguments
    optarg = NULL;
    while (!errflg && ((c = getopt(argc, argv, ARGS)) != EOF)){
        switch (c) {
            case 's': // serial port path argument
                sprintf(portName,"%s", optarg);
                showSerialPortSelection = 0;
                break;
            case 'b': // baud rate
                baudRate = atoi(optarg);
                break;
            case 'p': // osc udp port
                oscPort=optarg;
                break;
            case 't': // isc ip
                oscIP=optarg;
                break;
            case 'h':
                printf("usage : -s serial port path, -b serial port baud, -p port, -t target ip, -v verbose mode\n");
                return 0;
                break;
            case 'v':
                verbose = 1;
            default :
                errflg++;
        }
    }
    
    signal(SIGINT, INThandler);
    
    printf("\nserial2osc\nwww.happycodefarm.net\n(cc)Atelier Hypermédia - ESAA\n\n");
    
    if (showSerialPortSelection==1) { // show serial port selection
        int portIndex = selectSerialPort();
        
        if (portIndex<=-1) {
            printf("Illegal serial port selection\n");
            exit(1);
        }
    } else if( access( portName, F_OK ) == -1 ) { // check for supplied serialport validity
        printf("Serial port don't exist at %s\n", portName);
        exit(1);
    }
   
    
    lo_address t = lo_address_new(oscIP, oscPort);
    
    port_fd = init_serial_input(portName);

    if (port_fd == -1)  {
        printf("Couldn't open Serial port\n");
        exit(1);
    }
    printf("\nConfiguration\n");
    printf("Port: %s\n", portName);
    printf("Baud rate: %i\n", baudRate);
    printf("OSC ip: %s\n", oscIP);
    printf("OSC port: %s\n\n", oscPort);

    while(1) {
        if (lo_send(t, "/serial", "i", read_serial_int(port_fd)) == -1) {
            printf("OSC error %d: %s\n", lo_address_errno(t),
                   lo_address_errstr(t));
        }
    }
    
    return 0;
}
