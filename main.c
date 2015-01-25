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

//#include <iostream>
#include <fcntl.h>
#include <termios.h>

#include <dirent.h>


#define USB_SERIAL_PORT "/dev/tty.usbmodem1411"

int port_fd;
char portName[__DARWIN_MAXNAMLEN + 1];

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
   
    cfsetispeed(&options, B19200);
    cfsetospeed(&options, B19200);
    
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
    //while (c != ' ')  {
        ascii_int[i++] = c;
        read(fd, &c, 1);
       
    }
     printf("readed %i \n",atoi(ascii_int));
    return atoi(ascii_int);
}

void  INThandler(int sig)
{
    
    close(port_fd);
    exit(0);
    /*
    char  c;
    
    
    signal(sig, SIG_IGN);
    printf("OUCH, did you hit Ctrl-C?\n"
           "Do you really want to quit? [y/n] ");
    c = getchar();
    if (c == 'y' || c == 'Y') {
        close(port_fd);
        exit(0);
    } else
        signal(SIGINT, INThandler);
    getchar(); // Get new line character
     */
}

int getSerialPort() {
    
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
        
        printf("Please select a serial port: ");
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

                        printf("you selected %s\n",portName);
                        
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

int main(int argc, const char * argv[]) {
    // insert code here...
    
    
    signal(SIGINT, INThandler);
    
    printf("Hello, World!\n");
    int portIndex = getSerialPort();
    
    if (portIndex<=-1) {
        printf("Wrong port selection\n");
        exit(0);
    }
    
    lo_address t = lo_address_new("127.0.0.1", "7770");
    
//    if (lo_send(t, "/hello world !", NULL) == -1) {
//        printf("OSC error %d: %s\n", lo_address_errno(t),
//               lo_address_errstr(t));
//    }
    printf("init...\n");

    port_fd = init_serial_input(portName);
    // serial is checked
    if (port_fd == -1)  {
        printf("Couldn't open Serial port\n");
        exit(0);
    }
     printf("Serial port inited\n");
    while(1) {
        if (lo_send(t, "/serial", "i", read_serial_int(port_fd)) == -1) {
            printf("OSC error %d: %s\n", lo_address_errno(t),
                   lo_address_errstr(t));
        }

    }
    
    return 0;
}
