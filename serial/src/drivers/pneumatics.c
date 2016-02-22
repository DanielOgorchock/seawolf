#include "seawolf.h"

#include <stdio.h>
#include <unistd.h>

void run(SerialPort);

const char* app_name = "Serial : Pneumatics";

int main(int argc, char *argv[])
{
    /* Configuration */
    Seawolf_loadConfig("../conf/seawolf.conf");

    /* Init libseawolf */
    Seawolf_init(app_name);

    /* Check arguments */
    if(argc != 2) {
        Logging_log(ERROR, Util_format("%s spawned with invalid argument count of %d", argv[0], argc));
        exit(1);
    }

    /* Attempt to open serial device */
    char* device = argv[1];
    SerialPort sp = Serial_open(device);

    /* Error opening device */
    if(sp == 0) {
        Logging_log(ERROR, Util_format("%s could not open device %s", argv[0], device));
        exit(1);
    }

    /* Set baud rate */
    Serial_setBaud(sp, 9600);

    /* Error checking done */
    Logging_log(INFO, Util_format("%s running successfully with device %s", argv[0], device));
   
    /* Complete handshake */
    while(ArdComm_handshake(sp) == -1) {
        Util_usleep(0.5);
    } 

    run(sp);
    
    Serial_closePort(sp);
    Seawolf_close();
    return 0; 
}

void run(SerialPort sp)
{
    Serial_setBlocking(sp);

    while(1)
    {
        //let's just iterate through all of the commands for testing purposes...
        for(int i=1; i<8; ++i)
        {
            sleep(5);
            Serial_sendByte(sp, i);
        }
    }    
}




