
#include "seawolf.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define START_FRAME  '{'
#define TYPE_DIVIDER '|'
#define END_FRAME    '}'

/**
 * Get a raw message from an Arduino
 */
int ArdComm_getMessage(SerialPort sp, char* msgtype, char* buffer) {
    /* Clear lengths */
    int tmp = 0;
    size_t msgtype_s = 0;
    size_t buffer_s = 0;

    /* Find next frame */
    while(tmp != START_FRAME) {
        tmp = Serial_getByte(sp);
        if(tmp == -1) {
            /* Wasn't able to read, return */
            return -1;
        }
    }

    /* Read type */
    while((msgtype[msgtype_s] = Serial_getByte(sp)) != TYPE_DIVIDER) {
        if(msgtype[msgtype_s] == END_FRAME || msgtype[msgtype_s] == START_FRAME) {
            /* Error. Frame ended or new frame started */
            return -1;
        }
        msgtype_s++;
    }
    msgtype[msgtype_s] = '\0';

    /* Read message */
    while((buffer[buffer_s] = Serial_getByte(sp)) != END_FRAME) {
        if(msgtype[buffer_s] == TYPE_DIVIDER || msgtype[buffer_s] == START_FRAME) {
            /* Error. New frame started or invalid type divider */
            return -1;
        }
        buffer_s++;
    }
    buffer[buffer_s] = '\0';
    
    return 1;
}

/**
 * Send a raw message to an Arduino
 */
void ArdComm_sendMessage(SerialPort sp, char* msgtype, char* buffer) {
    /* Send message in standard format */
    Serial_sendByte(sp, START_FRAME);
    Serial_send(sp, msgtype, strlen(msgtype));
    Serial_sendByte(sp, TYPE_DIVIDER);
    Serial_send(sp, buffer, strlen(buffer));
    Serial_sendByte(sp, END_FRAME);

    /* Newline will be ignored but will make console debugging nice */
    Serial_sendByte(sp, '\n');
}

/**
 * Perform a handshake with an Arduino
 */
void ArdComm_handshake(SerialPort sp) {
    /* Send established connection message */
    ArdComm_sendMessage(sp, "ESTABLISHED", "NULL");
    tcdrain(sp); /* Wait for output to be drained */
    tcflush(sp, TCIFLUSH); /* Zero input buffers */
    ArdComm_sendMessage(sp, "READY", "NULL");
}

/**
 * Get the identifier from an Arduino
 */
int ArdComm_getId(SerialPort sp, char* id) {
    char type[16] = {[0] = '\0'};

    /* Get a message */
    if(ArdComm_getMessage(sp, type, id) == -1) {
        return -1;
    }

    /* Log possible error */
    if(strcmp(type, "ID") != 0) {
        id[0] = '\0'; /* Clear ID string */
        Logging_log(ERROR, "Error while attempting to read ID");
        return -1;
    }

    return 0;
}

