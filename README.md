# serial2osc

serial2osc is a simple cli utility written in C that read a stream of integers sent by an Arduino over a serial connection and redirect it to an Open Sound Control (OSC) client.


Tested on Mac OS X, but should work on any POSIX system.

To compile the code simply use : gcc main.c -o serial2osc -llo

liblo <http://liblo.sourceforge.net> must be installed on your system.

By default, serial2osc will send osc messages on localhost:0777, use a baud rate of 9600 and ask you to select the serial port from a list of available serial ports on your system.

You can modify the default options with the followings command-line arguments :

-s serial port path, -b serial port baud, -p osc port, -t osc target ip, -v verbose mode

eg ./serial2osc -s /dev/tty.usbmodem1411 -b 57600 -p 6666 -t 192.168.0.66 -v

Will open tty.usbmodem1411 at 57600 bauds and send everything on the osc address 192.168.0.66:6666


