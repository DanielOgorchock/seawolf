
#include "seawolf.h"

void dataOut(double mv) {
    Notify_send("THRUSTER_REQUEST", Util_format("Yaw %d %d", (int) mv, (int) -mv));
}

int main(void) {
    Seawolf_loadConfig("../conf/seawolf.conf");
    Seawolf_init("Yaw PID");

    PID* pid;
    char action[64], data[64];
    double mv;
    bool do_yaw;
    float yaw;
    float yaw_heading;
    
    Notify_filter(FILTER_MATCH, "UPDATED IMU");
    Notify_filter(FILTER_MATCH, "UPDATED YawPID");
    Notify_filter(FILTER_MATCH, "UPDATED YawHeading");
    Notify_filter(FILTER_MATCH, "UPDATED PIDDoYaw");

    pid = PID_new(SeaSQL_getYawHeading(),
                  SeaSQL_getYawPID_p(),
                  SeaSQL_getYawPID_i(),
                  SeaSQL_getYawPID_d());

    mv = PID_start(pid, SeaSQL_getSEA_Yaw());
    dataOut(mv);
    while(true) {
        Notify_get(action, data);

        if(strcmp(data, "YawPID") == 0) {
            PID_setCoefficients(pid,
                                SeaSQL_getYawPID_p(),
                                SeaSQL_getYawPID_i(),
                                SeaSQL_getYawPID_d());
            PID_resetIntegral(pid);
        } else if(strcmp(data, "YawHeading") == 0) {
            yaw_heading = SeaSQL_getYawHeading();
            PID_setSetPoint(pid, yaw_heading);
            mv = PID_start(pid, SeaSQL_getSEA_Yaw());
        } else if(strcmp(data, "PIDDoYaw") == 0) {
            do_yaw = (SeaSQL_getPIDDoYaw() == 1.0);
        } else {
            yaw = SeaSQL_getSEA_Yaw();
            if(yaw - yaw_heading > 0 && yaw - yaw_heading < 180) {
                mv = PID_update(pid, yaw);
            } else if(yaw - yaw_heading > 180) {
                mv = PID_update(pid, yaw - 360);
            } else if(yaw - yaw_heading < 0 && yaw - yaw_heading > -180) {
                mv = PID_update(pid, yaw);
            } else {
                mv = PID_update(pid, 360 - yaw);
            }
        }
        
        if(do_yaw) {
            dataOut(mv);
        }
    }

    Seawolf_close();
    return 0;
}
