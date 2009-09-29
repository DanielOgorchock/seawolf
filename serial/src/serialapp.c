
#include "seawolf.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stropts.h>
#include <unistd.h>
#include <termios.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>

typedef enum{PT_UNMANAGED,
             PT_IMU,
             PT_ALTIMETER,
             PT_DEPTH,
             PT_THRUSTER12,
             PT_THRUSTER3,
             PT_THRUSTER45,
             PT_MISSIONSTATUS} PeripheralType;

struct comm_device {
    const char* device;
    SerialPort sp;
    PeripheralType peripheral_type;
};

struct comm_assignment {
    const char* bin;
    bool started;
};

void cycleDTR(SerialPort sp) {
    /* Do NOT change this. It is *critical* and I don't know how the fuck it works. */
    unsigned int base, copy;

    Logging_log(DEBUG, "Cycling DTR");
    
    /* Copy out ioctl settings */
    ioctl(sp, TIOCMGET, &base);

    /* Assert DTR */
    copy = base;
    copy |= TIOCM_DTR;
    ioctl(sp, TIOCMSET, &copy);

    /* Sleep between */
    Util_usleep(0.25);

    /* Unassert DTR */
    copy = base;
    copy &= ~TIOCM_DTR;
    ioctl(sp, TIOCMSET, &copy);
    
    /* Sleep after */
    Util_usleep(0.25);
}

int getPeripheralType(SerialPort sp) {
    char id[32];
    const unsigned char command = 0xF0;
    char* c;
    int tmp;
    char buffer[64];

    /* Set to IMU's baud rate */
    Serial_setBaud(sp, 38400);

    /* Reset IMU to polling mode */
    Serial_sendByte(sp, 0x10);
    Serial_sendByte(sp, 0x00);
    Serial_sendByte(sp, 0x00);
    Util_usleep(0.25);

    Serial_flush(sp);
    Util_usleep(0.25);

    /* Probe IMU by sending version number command */
    tmp = write(sp, &command, 1);
    Util_usleep(0.25);

    if(tmp == -1) {
        /* Writing failed so device is definitely not ready */
        Logging_log(ERROR, "Attempted to fingerprint unready device");
        return -1;
    }

    if(read(sp, &tmp, 1) == -1) {
        /* Read error, skip */
    } else {
        if(tmp == 0xF0) {
            /* Received IMU response */
            Logging_log(DEBUG, "IMU Found");
            return PT_IMU;
        }
    }

    /* Attempt altimeter */
    Serial_setBaud(sp, 4800);
    tmp = read(sp, buffer, 63);
    buffer[tmp] = 0;

    if(tmp > 0 && ((c = strchr(buffer, '$')) != NULL) && (c[1] == 'S' || c[1] == 'Y')) {
        return PT_ALTIMETER;
    }

    /* Fallback to Arduino, get ID */
    Serial_setBaud(sp, 9600);
    if(ArdComm_getId(sp, id) != -1) {
        if(strcmp(id, "PressureSensor") == 0) {
            return PT_DEPTH;
        } else if(strcmp(id, "Thruster12") == 0) {
            return PT_THRUSTER12;
        } else if(strcmp(id, "Thruster3") == 0) {
            return PT_THRUSTER3;
        } else if(strcmp(id, "Thruster45") == 0) {
            return PT_THRUSTER45;
        } else if(strcmp(id, "MissionStatus") == 0) {
            return PT_MISSIONSTATUS;
        } else {
            Logging_log(ERROR, Util_format("Received unknown ID '%s'", id));
        }
    }

    /* Fingerprinting failed - cycle DTR and return error code */
    cycleDTR(sp);
    return -1;
}

int main(void) {
    /* Configuration */
    Seawolf_loadConfig("../conf/seawolf.conf");
    Seawolf_init("Serial");

    /* Peripheral type */
    PeripheralType pt;

    /* Device/app mappings */
    struct comm_device device_pool[] = {{"/dev/ttyUSB0", 0, PT_UNMANAGED},
                                        {"/dev/ttyUSB1", 0, PT_UNMANAGED},
                                        {"/dev/ttyUSB2", 0, PT_UNMANAGED},
                                        {"/dev/ttyUSB3", 0, PT_UNMANAGED},
                                        {"/dev/ttyUSB4", 0, PT_UNMANAGED},
                                        {"/dev/ttyUSB5", 0, PT_UNMANAGED},
                                        {"/dev/ttyUSB6", 0, PT_UNMANAGED},
                                        {"/dev/ttyUSB7", 0, PT_UNMANAGED},
                                        {"/dev/ttyUSB8", 0, PT_UNMANAGED},
                                        {"/dev/ttyUSB9", 0, PT_UNMANAGED}};
    const int device_count = sizeof(device_pool) / sizeof(struct comm_device);

    /* App to executable mappings */
    struct comm_assignment app_pool[] = {[PT_DEPTH]         = {"./apps/depth",          false},
                                         [PT_THRUSTER12]    = {"./apps/thruster12-bin", false},
                                         [PT_THRUSTER3]     = {"./apps/thruster3-bin",  false},
                                         [PT_THRUSTER45]    = {"./apps/thruster45-bin", false},
                                         [PT_ALTIMETER]     = {"./apps/altimeter",      false},
                                         [PT_IMU]           = {"./apps/imu",            false},
                                         [PT_MISSIONSTATUS] = {"./apps/status",         false}};
    const int app_count = sizeof(app_pool) / sizeof(struct comm_assignment);

    int i = 0;
    int apps_waiting = app_count;
    int unassigned_ports = 10;

    /* Open serial ports */
    for(i = 0; i < device_count; i++) {
        /* Open serial device */
        device_pool[i].sp = Serial_open(device_pool[i].device);
        if(device_pool[i].sp) {
            cycleDTR(device_pool[i].sp);
            Logging_log(DEBUG, Util_format("Opening %s", device_pool[i].device));
        } else {
            Logging_log(DEBUG, Util_format("Could not open %s - ignoring", device_pool[i].device));
            unassigned_ports--;
        }
    }

    i = 0;
    while(apps_waiting > 0 && unassigned_ports > 0) {
        if(device_pool[i].peripheral_type == PT_UNMANAGED && device_pool[i].sp != 0) {
            /* Serial port is now open, attempt to fingerprint */
            Logging_log(DEBUG, Util_format("Fingerprinting %s", device_pool[i].device));
            if(device_pool[i].sp && (pt = getPeripheralType(device_pool[i].sp)) != -1) {
                Logging_log(DEBUG, Util_format("Serial port ready %s", device_pool[i].device));
                
                /* Close port and copy device */
                Serial_closePort(device_pool[i].sp);
                unassigned_ports--;

                if(app_pool[pt].started == false) {
                    /* Fork and execute subprocess in child */
                    if(fork() == 0) {
                        Logging_log(INFO, Util_format("Connected to Arduino on %s with identifier %d. Spawning %s", 
                                                      device_pool[i].device, pt, app_pool[pt].bin));
                        execl(app_pool[pt].bin, app_pool[pt].bin, device_pool[i].device, NULL);
                    }

                    /* Decrease number of waiting apps */
                    apps_waiting -= 1;

                    /* Set the device as attached, and the application as running */
                    device_pool[i].peripheral_type = pt;
                    app_pool[pt].started = true;
                } else {
                    Logging_log(ERROR, Util_format("Error, attempt to connect %s twice", device_pool[i].device));
                }
            }
        }

        i = (i + 1) % device_count;
        if(i == 0) {
            /* Sleep 500 milliseconds after exhausting device list */
            Util_usleep(0.5);
        }
    }

    /* Finished identifying all devices */
    if(apps_waiting > 0) {
        Logging_log(WARNING, "All available devices handled but applications remain unhandled");
    } else {
        Logging_log(INFO, "Identified and properly handled all devices");
    }

    /* Send notification of completion */
    Notify_send("COMPLETED", "Serial identification");
    
    while(true) {
        Util_usleep(5.0);
    }

    Seawolf_close();
    return 0;
}
