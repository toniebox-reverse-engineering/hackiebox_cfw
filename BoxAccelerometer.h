#ifndef BoxAccelerometer_h
#define BoxAccelerometer_h

#include "BaseHeader.h"
#include <EnhancedThread.h>

#include <SparkFun_MMA8452Q.h>

class BoxAccelerometer : public EnhancedThread {
    public:
        enum class Orientation {
            EARS_UP = 0x7,
            EARS_DOWN = 0x5,
            EARS_FRONT = 0x3,
            EARS_BACK = 0x0,
            EARS_LEFT = 0x2,
            EARS_RIGHT = 0x1,
            OTHER = LOCKOUT,
            
            EARS_UP2 = 0x6,
            EARS_DOWN2 = 0x4
        };
        enum class TapOn {
            NONE = 0x0,
            LEFT = 0x1,
            RIGHT = 0x2, //17,34,68 - 34
            FRONT = 0x3, //16, 17
            BACK = 0x4,
            TOP = 0x5, //16,32,64,68,24 - 16
            BOTTOM = 0x6 //17
        };

        void
            begin(),
            loop(),
            reloadConfig();

        BoxAccelerometer::Orientation getOrientation();
        

    private:
        MMA8452Q _accel;
        BoxAccelerometer::Orientation _orientation;
};

#endif