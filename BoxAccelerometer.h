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
            NONE = 0x00,
            LEFT = 0x01,
            RIGHT = 0x02, //17,34,68 - 34
            FRONT = 0x04, //16, 17
            BACK = 0x08,
            TOP = 0x10, //16,32,64,68,24 - 16
            BOTTOM = 0x20, //17

            LEFT_FRONT = LEFT + FRONT,
            RIGHT_FRONT = RIGHT + FRONT,
            LEFT_BACK = LEFT + BACK,
            RIGHT_BACK = RIGHT + BACK
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