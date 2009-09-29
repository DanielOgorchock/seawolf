
#ifndef __SEAWOLF_ROOT_INCLUDE_H
#define __SEAWOLF_ROOT_INCLUDE_H

#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 500

/* Make sure some standard includes are available */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Include all Seawolf III development headers */
#include "seawolf/seasql.h"
#include "seawolf/seasql_config.h"
#include "seawolf/seawolf_config.h"
#include "seawolf/logging.h"
#include "seawolf/notify.h"
#include "seawolf/serial.h"
#include "seawolf/ardcomm.h"
#include "seawolf/util.h"
#include "seawolf/timer.h"
#include "seawolf/stack.h"
#include "seawolf/task.h"
#include "seawolf/pid.h"

/* Definitions */
#define SETPOINT_SOURCE_VISION 1
#define SETPOINT_SOURCE_ACOUSTICS 2
#define SETPOINT_SOURCE_OVERRIDE 3

/* Vision targets */
#define VISIONTARGET_WAIT 0
#define VISIONTARGET_GATE 1
#define VISIONTARGET_BOUEY 2
#define VISIONTARGET_PATH 3
#define VISIONTARGET_BRIEFCASE 4
#define VISIONTARGET_TORPEDO 5
#define VISIONTARGET_OCTOGON 6
#define VISIONTARGET_DROPZONE 7
#define VISIONTARGET_WHITE_GATE 9

/* Status */
#define STATUS_OFF   ((unsigned char)0)
#define STATUS_BLINK ((unsigned char)1)
#define STATUS_ON    ((unsigned char)2)

/* Max thrust */
#define THRUSTER_MAX 63

/* Initialize and close */
void Seawolf_init(const char* name);
void Seawolf_close(void);
char* Seawolf_getName(void);

#endif // #ifndef __SEAWOLF_ROOT_INCLUDE_H
